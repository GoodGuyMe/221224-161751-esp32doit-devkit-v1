#pragma once

#include <WiFi.h>
#include <secrets.h>
#include <HTTPClient.h>

extern WiFiClient wifiClient;

void setupWiFi();
void makeWifiPost(char*);