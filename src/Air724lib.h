#ifndef Air724lib_h
#define Air724lib_h

#include "Arduino.h"
#include <Arduino_JSON.h>
  
#define default_pulse_ms 100
#define default_timeout_ms 1000


class Air724lib {
  public:
    Air724lib(int8_t rst, bool newline = false);
    void begin(Stream &port);
    void reset(uint8_t pulse, uint16_t duration = default_pulse_ms);
    bool sendBlindCommand(String command);
    bool sendCommand(String command, uint16_t timeout = default_timeout_ms);
    bool sendCommand(String command, String reply, uint16_t timeout = default_timeout_ms);
    bool connect_mqtt(String ip, String port, String username, String passwd);
    bool publish(String top, String msg);
    void subscribe(String top, bool confirm);
    void dis_connect_mqtt(String top, String msg);
    String get_subscribe_msg(String topic, uint16_t timeout = default_timeout_ms);
    String get_subscribe_msg(uint16_t timeout = default_timeout_ms);
    void subscribe_msg_handler(Air724lib module, String msg_info, void (*task_callback)(Air724lib module, String topic, String payload_json));
    
  private:
    int8_t _rst;
    bool newline = false;
    unsigned long timer;
    Stream *mySerial;

};

#endif
