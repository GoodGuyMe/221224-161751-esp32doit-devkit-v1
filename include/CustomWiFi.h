#include <WiFi.h>
#include <secrets.h>
#include <HTTPClient.h>

WiFiClient espClient;

void setupWiFi();
void makeWifiPost(char*);