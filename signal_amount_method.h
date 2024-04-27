String access_point_name = "NodeMCU #2";
bool is_found = false;

void signal_amount()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  int counter = 1; 
  while (counter < 11)
  {
    int scanResult = WiFi.scanNetworks();
    for (int i = 0; i < scanResult; i++)
    {
      if (WiFi.SSID(i) == access_point_name)
      { 
        Serial.printf("Take %d| \tSignal amount of \"%s\" = %ddB\mW", counter, 
        WiFi.SSID(i), WiFi.RSSI(i));
        is_found = true; 
        counter++;
      }
    }
    if (!is_found)
      Serial.printf("%s is not found", access_point_name);
    Serial.printf("\n");
  }
}

int get_signal_amount(String access_point)
{
  int scanResult = WiFi.scanNetworks();
  for(int j = 0; j < scanResult; j++)
  {
    if(WiFi.SSID(j) == access_point)
      return WiFi.RSSI(j);
  }
  return NULL;
}