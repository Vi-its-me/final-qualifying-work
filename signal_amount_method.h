/*
* @brief Function computing distance<br>
* by counting amount of signal 
* 
*/
void signal_amount()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  int counter = 1; 
  while (counter < 11)
  {
    bool is_found = false;
    int scanResult = WiFi.scanNetworks();
    for (int i = 0; i < scanResult; i++)
    {
      if (WiFi.SSID(i) == "realme C30")
      { 
        Serial.printf("Take %d| \tSignal amount of \"%s\" = %ddB\mW", counter, WiFi.SSID(i), WiFi.RSSI(i));
        is_found = true; 
        counter++;
      }
    }
    if (!is_found)
      Serial.print("\"realme C30\" is not found");
    Serial.printf("\n");
  }
}