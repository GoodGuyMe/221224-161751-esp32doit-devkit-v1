#include <CustomWiFi.h>


void setupWiFi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void makeWifiPost(char* json) {
  if(WiFi.status()!= WL_CONNECTED) { 
    setupWiFi();
  }
  HTTPClient http;
  String serverPath = "bootje.erickemmeren.nl/data";
  http.begin(serverPath);
  http.addHeader("Content-Type", "application/json");
  int responseCode = http.POST(json);
  Serial.println(responseCode);
}