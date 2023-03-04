#include <CustomWiFi.h>

WiFiClient wifiClient;

wl_status_t setupWiFi(bool force = true) {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  int connection_count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (connection_count++ > 20) {
      if (force) {
        setupWiFi(true);
      } else {
        return WiFi.status();
      }
    }
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  return WiFi.status();
}

void makeWifiPost(char* json) {
  if(WiFi.status()!= WL_CONNECTED) { 
    setupWiFi(true);
  }
  HTTPClient http;
  String serverPath = "https://bootje.erickemmeren.nl/data";
  http.begin(serverPath);
  http.addHeader("Content-Type", "application/json");
  int responseCode = http.POST(json);
  Serial.println(responseCode);
}