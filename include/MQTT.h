#include <CustomWifi.h>
#include <PubSubClient.h>
#include <secrets.h>

const char* publish_path = "boat";

void setupMQTT();
void reconnect();
void updateMQTT();