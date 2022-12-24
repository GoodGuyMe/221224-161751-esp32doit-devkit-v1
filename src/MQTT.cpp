#include <MQTT.h>

PubSubClient client(espClient); 

void setupMQTT() {
    if (WiFi.status() != WL_CONNECTED) {
        setupWiFi();
    }
    client.setServer(mqtt_server, mqtt_port);
    void reconnect();
}

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP32Boat-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (client.connect(clientId.c_str(),MQTT_USER,MQTT_PASSWORD)) {
            Serial.println("connected");
            // Subscribe to all paths here
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void updateMQTT() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop(); 
}