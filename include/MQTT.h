#pragma once

#include <CustomWifi.h>
#include <PubSubClient.h>
#include <secrets.h>

void setupMQTT();
void reconnect();
void updateMQTT();