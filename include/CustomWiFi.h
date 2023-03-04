#pragma once

#include <WiFi.h>
#include <secrets.h>
#include <HTTPClient.h>

extern WiFiClient wifiClient;

wl_status_t setupWiFi(bool force);
void makeWifiPost(char*);