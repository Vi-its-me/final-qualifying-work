#include <ESP8266WiFi.h>
#include <espnow.h>
#include "signal_amount_method.h"
#define TRANSMIT // MAC || AMOUNT || RECEIVE || TRANSMIT

#if defined RECEIVE || defined TRANSMIT
  int signal_amount_array[10] = {0,0,0,0,0,0,0,0,0,0};
  //Переменные для обеспечения "распараллеливания" в loop()
  int interval = 0; // (ms)Интервал между отправкой и приемом(в конце его вычтем)
  int delay_time = 500; // (ms)Интервал, установленный после отправки
                      // (чтобы 1 модуль успел принять данные)
  int last_send_time = 0;
  bool is_result_printed = false;//Для предотвращения бесконечной печати результата
  // MAC-адреса плат
  uint8_t MAC_1[] = {0xC8, 0xC9, 0xA3, 0x5B, 0x96, 0x22};
  uint8_t MAC_2[] = {0xE8, 0xDB, 0x84, 0xBB, 0xE8, 0x3B};//Where to send-2
  // Класс для хранения массивов данных, пересылаемых между платами
  class sendClass
  {
  public:
    int to_second_point_packets_array[10] = {0,0,0,0,0,0,0,0,0,0};
    int processing_time[10] = {0,0,0,0,0,0,0,0,0,0};
  };
  sendClass send_instance;
  // Функция для обратной связи, отправлено ли сообщение
  void OnDataSent (uint8_t *mac_address, uint8_t sendStatus) {}
  void OnDataRecv(uint8_t *mac_address, uint8_t *incomingData, uint8_t length)
  {
    // Функция стандартной библиотеки C, копирующая содержимое одной области
    // памяти в другую
    memcpy(&send_instance, incomingData, sizeof(send_instance));
  }
#endif // RECEIVE || TRANSMIT
void setup()
{
  Serial.begin(115200);
  #ifdef AMOUNT
    signal_amount();
  #endif // AMOUNT
  #if defined TRANSMIT || defined RECEIVE
    // Инициализируем модули приема-передачи
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    if (esp_now_init() != 0)
    {
      // Serial.println("ESP-NOW initialization error");
      return;
    }
  #endif // TRANSMIT || RECEIVE
  #ifdef RECEIVE
    WiFi.softAP("NodeMCU");
  #endif // RECEIVE
  #if defined TRANSMIT || defined MAC
    delay(1000);
  #endif // TRANSMIT || MAC
  #ifdef MAC
    Serial.print("Board MAC-address:\t"); Serial.println(WiFi.macAddress());
  #endif // MAC
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
      // Поочереди перебираем элементы массива в поисках нуля,
      // если находим, отправляем весь объект с массивом на второй модуль
      // и выходим из (цикла)процесса нахождения
      for(int i = 0; i < 10; i++)
      {
        if(send_instance.to_second_point_packets_array[i] == 0)
        {
          // Если мы "попали" на 0, а перед ним есть 1 или 2, то мы вместо них
          // записываем 4 (как не полученные)
          for(int j = 0; j < i; j++)
          {
            if(send_instance.to_second_point_packets_array[j] == (1 || 2))
            {
              send_instance.to_second_point_packets_array[j] = 4;
              send_instance.processing_time[j] = 404;
            }
          }
          send_instance.to_second_point_packets_array[i] = 1;
          // Ищем точку доступа по имени и записываем уровень её сигнала
          signal_amount_array[i] = get_signal_amount("NodeMCU");
          send_instance.processing_time[i] = micros(); // Засекаем время отправки
          esp_now_send(0, (uint8_t *) &send_instance, sizeof(send_instance));
          delay(delay_time); // Оставляем время, чтобы модуль 1 успел отправить
          last_send_time = millis();
          break;
        }
        else if(send_instance.to_second_point_packets_array[i] == 2)
        {
          // Если в отправляемом массиве находим 1-ки до 2-ек, то
          // записываем вместо них 4
          for(int j = 0; j < i; j++)
          {
            if(send_instance.to_second_point_packets_array[j] == 1)
              {
                send_instance.to_second_point_packets_array[j] = 4;
              }
          }
          // Подсчитываем и фиксируем время принятия
          send_instance.processing_time[i] = micros()
          - send_instance.processing_time[i] - (interval + delay_time) * 1000;
          // Фиксируем факт принятия данных
          send_instance.to_second_point_packets_array[i] = 3;
          break;
        }
        // если мы получили последний пакет, то считаем к-во отправ. и принятых
        else if(i == 9 && !is_result_printed)
        {
          int sent_packets_count = 0; int received_packets_count = 0;
          for(int j = 0; j < 10; j++)
          {
            if(send_instance.to_second_point_packets_array[j] == 4)
              sent_packets_count++;
            else if(send_instance.to_second_point_packets_array[j] == 3)
              received_packets_count++;
          }
          // Выводим массив с данными и их временем получения в порт Serial
          Serial.printf("begin\n");
          for(int j = 0; j < 10; j++)
          {
            Serial.printf("%d\t%d\t%d\n", j + 1, signal_amount_array[j], send_instance.processing_time[j]);
          }
          Serial.printf("end\n");
          is_result_printed = true;
        }
      }
    }
    // Инициализация модуля в качестве приемника и получение пакетов
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_recv_cb(OnDataRecv);
  #endif // TRANSMIT
  #ifdef RECEIVE
    // Распараллелим задачу по периодической отправке массива двоек на 1 модуль
    if((millis() - last_send_time) >= interval)
    {
      // Инициализируем 2 модуль как отправляющий
      esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
      esp_now_register_send_cb(OnDataSent);
      esp_now_add_peer(MAC_1, ESP_NOW_ROLE_SLAVE, 0, NULL, 1);
      for(int i = 0; i < 10; i++)
      {
        // Перебираем принятый массив поочереди в поисках единички, если она
        // есть, то отправляем объект с массивом на 1 модуль
        if(send_instance.to_second_point_packets_array[i] == 1)
        {
          Serial.printf("send_instance.to_second_point_packets_array[%d] = %d\n",
          i, send_instance.to_second_point_packets_array[i]);
          send_instance.to_second_point_packets_array[i] = 2;
          esp_now_send(0, (uint8_t *) &send_instance, sizeof(send_instance));
          break;
        }
      }
    }
    // Инициализируем 2 модуль как приемник и в свободное от отправки время
    // принимаем пакеты от 1 модуля
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_recv_cb(OnDataRecv);
  #endif // RECEIVE
}
