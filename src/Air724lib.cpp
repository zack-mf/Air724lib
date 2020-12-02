#include "Arduino.h"
#include "Air724lib.h"

Air724lib::Air724lib(int8_t rst, bool lineEnding) {
  pinMode(rst, OUTPUT);
  _rst = rst;
  mySerial = 0;
  newline = lineEnding;
}

void Air724lib::begin(Stream &port) {
  mySerial = &port;
}

void Air724lib::reset(uint8_t pulse, uint16_t duration) {
  if (pulse == LOW) {
    digitalWrite(_rst, HIGH);
    delay(10);
    digitalWrite(_rst, LOW);
    delay(duration);
    digitalWrite(_rst, HIGH);
  }
  else if (pulse == HIGH) {
    digitalWrite(_rst, LOW);
    delay(10);
    digitalWrite(_rst, HIGH);
    delay(duration);
    digitalWrite(_rst, LOW);
  }
}

bool Air724lib::sendBlindCommand(String command) {
  Serial.print(F("\t---> ")); Serial.println(command);
  if (newline) mySerial->println(command);
  else mySerial->print(command);
}

bool Air724lib::sendCommand(String command, uint16_t timeout) {
  while(mySerial->available()) mySerial->read(); // Clear input buffer

  Serial.print(F("\t---> ")); Serial.println(command);
  if (newline) mySerial->println(command);
  else mySerial->print(command);

  bool gotReply = false;
  timer = millis();

  while (!gotReply && millis() - timer < timeout) {
    if (mySerial->available()) gotReply = true;
  }

  if (gotReply) return true;
  else return false;
}

bool Air724lib::sendCommand(String command, String reply, uint16_t timeout) {
  while(mySerial->available()) mySerial->read(); // Clear input buffer

  Serial.print(F("\t---> ")); Serial.println(command);
  if (newline) mySerial->println(command);
  else mySerial->print(command);

  bool replyMatch = false;
  String read_line;
  timer = millis();
  
  Serial.print(F("\t<--- "));
  while (!replyMatch && millis() - timer < timeout) {
    if (mySerial->available()) {
      read_line=mySerial->readStringUntil('\n');
      Serial.println(read_line);
      if (read_line.startsWith(reply)) {
        replyMatch = true;
      }
    }
  }
  
  if (replyMatch) return true;
  else return false;
}

bool Air724lib::connect_mqtt(String ip, String port, String username, String passwd) {
  String AT_MCONFIG = "AT+MCONFIG=\"AIR724UG\",\""+username+"\",\""+passwd+"\"";
  String AT_MIPSTART = "AT+MIPSTART=\""+ip+"\","+port+"";
  for(int i=0; i< 10; i++){
      sendBlindCommand("AT");
      delay(100); // May want to include a small delay depending on what module you're using
  }
  sendBlindCommand("AT+MDISCONNECT");
  delay(200);
  sendBlindCommand("AT+MIPCLOSE");
  delay(200);

  if (!sendCommand("AT", "OK")){ 
    Serial.println(F("AT Command failed!"));  
    goto err;
  }
  delay(200);
  
  if (!sendCommand("AT+CGREG?", "OK")) {
    Serial.println(F("AT+CGREG? Command failed!"));
    goto err;
  }
  delay(200);
  
  if (!sendCommand("AT+CGATT?", "OK")) {
    Serial.println(F("AT+CGATT? Command failed!"));
    goto err;
  }
  delay(200);
  
  if (!sendCommand(AT_MCONFIG, "OK")) {
    Serial.println(F("AT+MCONFIG Command failed!"));
    goto err;
  }
  delay(200);
  
  if (!sendCommand(AT_MIPSTART, "OK")) {
    Serial.println(F("AT+MIPSTART Command failed!"));
    goto err;
  }
  delay(200);
  
  if (!sendCommand("AT+MCONNECT=1,60", "CONNACK OK")) {
    Serial.println(F("AT+MCONNECT Command failed!"));
    goto err;
  }
  delay(200);

  if (!sendCommand("AT+MQTTMSGSET=1", "OK")) {
    Serial.println(F("AT+MCONNECT Command failed!"));
    goto err;
  }
  delay(200);
  
err: return false;
  return true;
}


bool Air724lib::publish(String top, String msg) {
  String AT_MPUB="AT+MPUB=\""+top+"\",0,0,\""+msg+"\"";
  //sendBlindCommand("AT+MPUB=\"v1/devices/me/telemetry\"\,0\,0\,\"{'temperature': 66, 'humidity': 66}\"");
  if (!sendCommand((char *)AT_MPUB.c_str(), "OK", 1000)){ 
    Serial.println(F("AT_MPUB Command failed!"));  
    return false;
  }else{
    return true;
  }
}

void Air724lib::subscribe(String top, bool confirm) {
  //bool exist = ((IList)subscribe_top).Contains(top);
  if(confirm) {
    String AT_MSUB="AT+MSUB=\""+top+"\",0";
    if (!sendCommand((char *)AT_MSUB.c_str(), "SUBACK", 1000)) Serial.println(F("AT_MSUB Command failed!"));
  }else {
    String AT_MUNSUB="AT+MUNSUB=\""+top+"\"";
    sendCommand((char *)AT_MUNSUB.c_str());
  }
}

void Air724lib::dis_connect_mqtt(String top, String msg) {
  sendBlindCommand((char *)"AT+MDISCONNECT");
}

String Air724lib::get_subscribe_msg(String topic, uint16_t timeout) {  
  while(mySerial->available()) mySerial->read(); // Clear input buffer

  if (newline) mySerial->println("AT+MQTTMSGGET");
  else mySerial->print("AT+MQTTMSGGET");
  String reply = "+MSUB: \"";
  String read_line;
  String this_topic;
  String this_payload="";
  timer = millis();

  while (millis() - timer < timeout) {
    if (mySerial->available()) {
      read_line=mySerial->readStringUntil('\n');
      if (read_line.startsWith("OK")) {
        break;
      }  
    }
        
    if (read_line.startsWith(reply)) {
      read_line=read_line.substring(read_line.indexOf(" ",0)+1,read_line.length());
      this_topic=read_line.substring(0,read_line.indexOf(",",0));
      this_topic=this_topic.substring(1,this_topic.length()-1);
      //Serial.println(read_line);
      if (this_topic.startsWith(topic)){
         this_payload=read_line.substring(read_line.indexOf(",",0)+1,read_line.length());
         this_payload=this_payload.substring(this_payload.indexOf(",",0)+1,this_payload.length());
      }
      //Serial.println(this_topic);
      //Serial.println(this_payload);
    }
  }
  return this_payload;  
}

String Air724lib::get_subscribe_msg(uint16_t timeout) {  
  while(mySerial->available()) mySerial->read(); // Clear input buffer

  if (newline) mySerial->println("AT+MQTTMSGGET");
  else mySerial->print("AT+MQTTMSGGET");

  JSONVar sub_msg_json;
  String reply = "+MSUB: \"";
  String read_line;
  String this_topic;
  String this_payload="";
  uint8_t msg_number=0;
  timer = millis();

  while (millis() - timer < timeout) {
    if (mySerial->available()) {
      read_line=mySerial->readStringUntil('\n');
      if (read_line.startsWith("OK")) {
        break;
      }  
    }
            
    if (read_line.startsWith(reply)) {
      read_line=read_line.substring(read_line.indexOf(" ",0)+1,read_line.length());
      this_topic=read_line.substring(0,read_line.indexOf(",",0));
      this_topic=this_topic.substring(1,this_topic.length()-1);
      //Serial.println(read_line);
      this_payload=read_line.substring(read_line.indexOf(",",0)+1,read_line.length());
      this_payload=this_payload.substring(this_payload.indexOf(",",0)+1,this_payload.length());     
      //Serial.println(this_topic);
      //Serial.println(this_payload);
      sub_msg_json["msg_number"] = msg_number;
      String topic_key="topic"+String(msg_number);
      sub_msg_json[topic_key] = this_topic;
      String payload_key="payload"+String(msg_number);
      sub_msg_json[payload_key] = this_payload;
      msg_number ++;
    }
  }
  
  return JSON.stringify(sub_msg_json);
}

void Air724lib::subscribe_msg_handler(Air724lib module, String msg_info, void (*task_callback)(Air724lib module, String topic, String payload_json)){
  if(!msg_info.equals("")){
    JSONVar myObject = JSON.parse(msg_info.c_str());
    //Serial.println(msg_info.c_str());
    int msg_number = -1;
    // JSON.typeof(jsonVar) can be used to get the type of the var
    if (JSON.typeof(myObject) == "undefined") {
      Serial.println("Parsing msg_info failed!");
    }
    
    if (myObject.hasOwnProperty("msg_number")) {
      msg_number = myObject["msg_number"];
    }
    
    for(int i=0; i <= msg_number; i++){
      String topic_key="topic"+String(i);
      String payload_key="payload"+String(i);
      if (myObject.hasOwnProperty(topic_key)) {  
        if (myObject.hasOwnProperty(payload_key)) {  
            //Serial.print("topic info=");Serial.println(myObject[topic_key]);
            //Serial.print("payload info=");Serial.println(myObject[payload_key]);
            task_callback(module, (const char* )myObject[topic_key], (const char* )myObject[payload_key]);
        }
      }
    }
  }    
}
