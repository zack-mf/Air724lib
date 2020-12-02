 
#include "Air724lib.h"

#define RST PC13 // MCU pin to control module reset

void led_switch(Air724lib module, String topic, String payload){
    String requestId = topic.substring(topic.lastIndexOf("/")+1, topic.length()-1);
    JSONVar payload_json = JSON.parse(payload.c_str());
    //Serial.println(payload);
    Serial.println(payload_json["method"]);
    Serial.println(payload_json["params"]);
    
    if(JSON.stringify(payload_json["method"]).equals("\"getLight1SwitchStatus\"")){
       Serial.println("getvalue request getLight1SwitchStatus");
       module.publish("v1/devices/me/rpc/response/"+requestId, "20");
    }
    
    if(JSON.stringify(payload_json["method"]).equals("\"setLight1SwitchStatus\"")){
       Serial.println("setvalue request getLight1SwitchStatus");
       String params = JSON.stringify(payload_json["params"]);
       Serial.println("setValue:"+params);
    }

    if(JSON.stringify(payload_json["method"]).equals("\"getLight2SwitchStatus\"")){
       Serial.println("getvalue request getLight2SwitchStatus");
       module.publish("v1/devices/me/rpc/response/"+requestId, "21");
    }

    if(JSON.stringify(payload_json["method"]).equals("\"setLight2SwitchStatus\"")){
       Serial.println("setvalue request setLight2SwitchStatus");
       String params = JSON.stringify(payload_json["params"]);
       Serial.println("setValue:"+params); 
    }
    
}

Air724lib module=NULL;

void setup() {
  Serial.begin(115200);
  delay(3000);
  // Hardware serial is also possible!
  HardwareSerial *moduleSerial = &Serial1;
  module = Air724lib(RST, true); // Use "false" if you don't want AT commands with newline, "true" otherwise
  
  moduleSerial->begin(115200); // Verify your module's baud rate
  module.begin(*moduleSerial);
  module.connect_mqtt("182.92.223.22", "1883", "DHT11_DEMO_TOKEN", "");
  module.subscribe("v1/devices/me/rpc/request/+", true);
}

void loop() {
  // Nothing here
 // "AT+MPUB=\"v1/devices/me/telemetry\",0,0,\"{'temperature': 66, 'humidity': 66}\""
  int i = 0;
  do{
    String publish_top= "v1/devices/me/telemetry";
    String publish_msg = "{'temperature': "+ String(i) +", 'humidity': "+ String(i) +"}";
    module.publish(publish_top, publish_msg);
    i++;
    digitalWrite(RST,HIGH);
    delay(500);
    digitalWrite(RST,LOW);
    delay(500);

    // got some one subscribe messages.
    String payload_msg=module.get_subscribe_msg("v1/devices/me/rpc/request/");
    if(!payload_msg.equals(""))
      Serial.println("payload info="+payload_msg);

    
    // handler all subscribe messages.
    //String sub_msg_json=module.get_subscribe_msg();
    //module.subscribe_msg_handler(module, sub_msg_json, led_switch);
    
    }while(1);
    
}
