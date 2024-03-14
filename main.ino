#include <ESP8266WiFi.h>
#include <espnow.h>
#include "signal_amount_method.h" //#include "time_count_method.h"
#define TRANSMIT // MAC || AMOUNT || RECEIVE || TRANSMIT

#if defined RECEIVE || defined TRANSMIT
  uint8_t broadcastAddress[] = {0x84, 0xF3, 0xEB, 0xA4, 0x11, 0x2A};//Where to send-2

  class sendClass
  {
  public:
    int id;
    int processing_time;
  };
  sendClass send_instance;
  sendClass sendClasses_array[1] = {send_instance};
#endif // RECEIVE || TRANSMIT
#ifdef TRANSMIT
  void OnDataSent (uint8_t *mac_address, uint8_t sendStatus)
  {
    Serial.println("Last packet send status: ");
    if (sendStatus == 0) 
      Serial.println("Message sent");
    else Serial.println("Message not sent");
  }
#endif // TRANSMIT
#ifdef RECEIVE
  void OnDataRecv(uint8_t *mac_address, uint8_t *incomingData, uint8_t length)
  {
    memcpy(&send_instance, incomingData, sizeof(send_instance));
    sendClasses_array[0].id = send_instance.id;
    // send_instance.processing_time = this->send_instance.processing_time;
  }
#endif // RECEIVE
void setup()
{
  Serial.begin(115200);
  #ifdef MAC
    delay(1000);
    Serial.print("Board MAC-address:\t"); Serial.println(WiFi.macAddress());
  #endif // MAC
  #ifdef AMOUNT
    signal_amount();
  #endif // AMOUNT
  #if defined TRANSMIT || defined RECEIVE
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    if (esp_now_init() != 0)
    {
      Serial.println("ESP-NOW initialization error");
      return;
    }
  #endif // RECEIVE || TRANSMIT
  #ifdef TRANSMIT
    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    esp_now_register_send_cb(OnDataSent);
    esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
  #endif // TRANSMIT
  #ifdef RECEIVE
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_recv_cb(OnDataRecv);
  #endif // RECEIVE
}

void loop() 
{
  #ifdef TRANSMIT
    send_instance.id = 1;
    esp_now_send(0, (uint8_t *) &send_instance, sizeof(send_instance));
    delay(10000);
  #endif // TRANSMIT
  #ifdef RECEIVE
    Serial.printf("send_instance.id = %d\n", send_instance.id);
    delay(10000);
  #endif // RECEIVE
}