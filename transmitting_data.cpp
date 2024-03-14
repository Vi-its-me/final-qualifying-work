// #include <ESP8266WiFi.h>
// #include <espnow.h>
// // ЗАМЕНИТЕ МАС-АДРЕС платы, на которую отправляем данные.
// uint8_t broadcastAddress[] = {0xC8, 0xC9, 0xA3, 0x5B, 0x96, 0x22};


// // Переменная для хранения состояния отправки
// String success;
// //Пример структуры для отправки
// //Должна совпадать со структурой на плате-приемнике
// typedef struct struct_message 
// {
//   float temp;
//   float hum;
// } struct_message;
// // Создаем переменную для хранения отправляемого сообщения
// struct_message DHTReadings;
// // То же, но для принимаемого сообщения
// struct_message incomingReadings;
// // Callback-функция для получения состояния отправки
// void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) 
// {
//   Serial.print("Last Packet Send Status: ");
//   if (sendStatus == 0)
//   {
//   Serial.println("Delivery success");
//   }
//   else
//   {
//   Serial.println("Delivery fail");
//   }
// }
// // То же, для индикации состояния приема данных
// void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) 
// {
//   memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
//   Serial.print("Bytes received: ");
//   Serial.println(len);
//   incomingTemp = incomingReadings.temp;
//   incomingHum = incomingReadings.hum;
// }
// void getReadings()
// {
//   // Снимаем значение температуры
//   temperature = dht.readTemperature();
//   if (isnan(temperature)){
//   Serial.println("Failed to read from DHT");
//   temperature = 0.0;
//   }
//   humidity = dht.readHumidity();
//   if (isnan(humidity)){
//   Serial.println("Failed to read from DHT");
//   humidity = 0.0;
//   }
// }
// void printIncomingReadings()
// {
//   // Отображаем показания в мониторе порта
//   Serial.println("INCOMING READINGS");
//   Serial.print("Temperature: ");
//   Serial.print(incomingTemp);
//   Serial.println(" ºC");
//   Serial.print("Humidity: ");
//   Serial.print(incomingHum);
//   Serial.println(" %");
// }

// void setup() 
// {
//   // Запускаем монитор порта
//   Serial.begin(115200);
//   // Включаем датчик
//   dht.begin();
//   // Выставляем режим работы Wi-Fi
//   WiFi.mode(WIFI_STA);
//   WiFi.disconnect();
//   // Инициализируем протокол ESP-NOW
//   if (esp_now_init() != 0) {
//   Serial.println("Error initializing ESP-NOW");
//   return;
//   }
//   // Указываем роль платы в сети
//   esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
//   // Регистрируем callback-функцию для получения статуса отправки
//   esp_now_register_send_cb(OnDataSent);
//   // Регистрируем пиры
//   esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
//   // Регистрируем callback-функцию для получения статуса приема
//   esp_now_register_recv_cb(OnDataRecv);
// }
// void loop() 
// {
//   unsigned long currentMillis = millis();
//   if (currentMillis - previousMillis >= interval) {
//   // Сохраняем время последнего обновления показаний
//   previousMillis = currentMillis;
//   //Запрашиваем показания датчика
//   getReadings();
//   //Записываем их в переменные
//   DHTReadings.temp = temperature;
//   DHTReadings.hum = humidity;
//   // Отправляем сообщение
//   esp_now_send(broadcastAddress, (uint8_t *) &DHTReadings, sizeof(DHTReadings));
//   // Выводим входящие данные
//   printIncomingReadings();
//   }
// }