#include <ESP8266WiFi.h>
#include <espnow.h>
#include "signal_amount_method.h" //#include "time_count_method.h"
#define TRANSMIT // MAC || AMOUNT || RECEIVE || TRANSMIT

#if defined RECEIVE || defined TRANSMIT
  //Переменные для обеспечения распараллеливания в loop()
  int interval = 1000;
  int last_send_time = 0;
  // MAC-адреса плат
  uint8_t MAC_1[] = {0xC8, 0xC9, 0xA3, 0x5B, 0x96, 0x22};
  uint8_t MAC_2[] = {0xE8, 0xDB, 0x84, 0xBB, 0xE8, 0x3B};//Where to send-2
  // Класс для хранения массивов данных, пересылаемых между платами
  class sendClass
  {
  public:
    int to_second_point_packets_array[10] = {0,0,0,0,0,0,0,0,0,0};
    int received_packets_number = 0;
    int sent_time[10] = {0,0,0,0,0,0,0,0,0,0};
  };
  sendClass send_instance;
  // sendClass sendClasses_array[1] = {send_instance};
  // Функция для обратной связи, отправлено ли сообщение
  void OnDataSent (uint8_t *mac_address, uint8_t sendStatus)
  {
    Serial.println("Last packet send status: ");
    if (sendStatus == 0) 
      Serial.println("Message sent");
    else Serial.println("Message not sent");
  }
  void OnDataRecv(uint8_t *mac_address, uint8_t *incomingData, uint8_t length)
  {
    // Функция стандартной библиотеки C, копирующая содержимое одной области
    // памяти в другую 
    memcpy(&send_instance, incomingData, sizeof(send_instance));
    // sendClasses_array[0].id = send_instance.id;
  }
#endif // RECEIVE || TRANSMIT
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
  #if defined RECEIVE || defined TRANSMIT
    // Инициализируем модули приема-передачи
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    if (esp_now_init() != 0)
    {
      Serial.println("ESP-NOW initialization error");
      return;
    }
  #endif // RECEIVE || TRANSMIT
  #ifdef RECEIVE
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_recv_cb(OnDataRecv);
  #endif // RECEIVE
}
void loop() 
{
  #ifdef TRANSMIT
    // Распараллеливание для отправки данных(отправляет масив единичек 
    // на 2 модуль)
    if((millis() - last_send_time) >= interval)
    {
      // Инициализируем 1 модуль как отправляющий
      esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
      // Как только ESPNow инициализируется, мы зарегистрируем Send CB
      // для получения статуса переданного пакета 
      esp_now_register_send_cb(OnDataSent);
      // Регистрируем пир
      esp_now_add_peer(MAC_2, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
      // Отправляем поочереди каждый из элементов массива на второй модуль
      for(int i = 0; i < 10; i++)
      {
        if(send_instance.to_second_point_packets_array[i] == 0)
          {
            Serial.printf("send_instance.to_second_point_packets_array[%d] = 1\n", 
        i);
            send_instance.to_second_point_packets_array[i] = 1;
            send_instance.sent_time[i] = 0;
            // Увеличиваем количество отправленных единичек на 1
            send_instance.received_packets_number++;
            esp_now_send(0, (uint8_t *) &send_instance, sizeof(send_instance));
            last_send_time = millis();
            break;
          }
        // Засекаем время для каждого отправленного
        
      }
    }
    // если 0 меняется на 2,то фиксирует время и "выкидывает" 
    // в монитор данные об изменившейся переменной
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_recv_cb(OnDataRecv);
  #endif // TRANSMIT
  #ifdef RECEIVE
    // Печатаем в порт Serial каждый элемент из массива и общую сумму принятых п-ов
    for(int i = 0; i < 10; i++)
      {
        Serial.printf("send_instance.to_second_point_packets_array[%d] = %d\n", i, 
        send_instance.to_second_point_packets_array[i]); 
        if(i == 9 && send_instance.received_packets_number != 0) 
          Serial.printf("Packets received: %d/10\n\n", 
          (send_instance.received_packets_number + 1));
        else if(i == 9 && send_instance.received_packets_number == 0)
          Serial.printf("Packets received: %d/10\n\n", 
          send_instance.received_packets_number);
        // Если 1 в элемент массива пришла, 
        // то отправляем 2 в него же на другой модуль
        // Инициализируем второй модуль как отправляющий
        esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
        esp_now_register_send_cb(OnDataSent);
        esp_now_add_peer(MAC_1, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
        // И отправляем на первый модуль 2-ки
        for(int i = 0; i < 10; i++)
        {
          send_instance.to_second_point_packets_array[i] = 2;
          send_instance.received_packets_number++;
          esp_now_send(0, (uint8_t *) &send_instance, sizeof(send_instance));
        }
        // Переназначаем модуль 2 принимающим
        esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
        esp_now_register_recv_cb(OnDataRecv);
        // И обнуляем все значения в массивах и переменных принимающего модуля
        if(send_instance.to_second_point_packets_array[i] == 1)
          {
            send_instance.to_second_point_packets_array[i] = 0; 
            send_instance.sent_time[i] = 0; 
          }
      }
    send_instance.received_packets_number = 0;
    delay(3000);
  #endif // RECEIVE
}