#include <ESP8266WiFi.h>
#include <espnow.h>
#include "signal_amount_method.h" //#include "time_count_method.h"
#define RECEIVE // MAC || AMOUNT || RECEIVE || TRANSMIT

#if defined RECEIVE || defined TRANSMIT
  // MAC-адреса плат
  uint8_t broadcastAddress[] = {0xE8, 0xDB, 0x84, 0xBB, 0xE8, 0x3B};//Where to send-2
  // Класс для хранения массивов данных, пересылаемых между платами
  class sendClass
  {
  public:
    int to_second_point_packets_array[10] = {0,0,0,0,0,0,0,0,0,0};
    int sent_time[10] = {0,0,0,0,0,0,0,0,0,0};
  };
  sendClass send_instance;
  // sendClass sendClasses_array[1] = {send_instance};
#endif // RECEIVE || TRANSMIT
#ifdef TRANSMIT
  // Функция для обратной связи, отправлено ли сообщение
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
    // Функция стандартной библиотеки C, копирующая содержимое одной области
    // памяти в другую 
    memcpy(&send_instance, incomingData, sizeof(send_instance));
    // sendClasses_array[0].id = send_instance.id;
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
    // Инициализируем модули приема-передачи
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
    // Как только ESPNow инициализируется, мы зарегистрируем Send CB
    // для получения статуса переданного пакета 
    esp_now_register_send_cb(OnDataSent);
    // Регистрируем пир
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
    // Отправляем поочереди каждый из элементов массива
    for(int i = 0; i < 10; i++)
    {
      send_instance.to_second_point_packets_array[i] = 1;
      // Засекаем время для каждого отправленного
      send_instance.sent_time[i] = 
      esp_now_send(0, (uint8_t *) &send_instance, sizeof(send_instance));
    }
    delay(10000);
  #endif // TRANSMIT
  #ifdef RECEIVE
    Serial.printf("send_instance.to_sec...[0] = %d\n", send_instance.to_second_point_packets_array[0]);
    delay(10000);
  #endif // RECEIVE
}