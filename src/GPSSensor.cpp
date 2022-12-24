#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#define ARDUINOJSON_POSITIVE_EXPONENTIATION_THRESHOLD 1e8
#include <ArduinoJson.h>
#include "secrets.h"
#define _SS_MAX_RX_BUFF 512

#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>

#define MYPORT_TX 12
#define MYPORT_RX 13

DynamicJsonDocument doc(2024);
JsonArray data_points = doc.createNestedArray("dp");
char output[1024];

#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)

double tempOffset = -1.5;

Adafruit_BMP280 bmp;

SoftwareSerial gsmSerial;

TinyGPSPlus gps;

WiFiClient espClient;
PubSubClient client(espClient);

const char* publish_path = "boat";

#define MSG_BUFFER_SIZE  (1024)
char msg[MSG_BUFFER_SIZE];

unsigned long count = 0;

double avg_lat = 0;
double avg_lng = 0;
double avg_speed = 0;
double avg_dir = 0;
unsigned long start_time = 0;
unsigned int period = 1000;

// rounds a number to 2 decimal places
// example: round(3.14159) -> 3.14
double round2(double value) {
   return (int)(value * 100 + 0.5) / 100.0;
}

void setup_wifi() {
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

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  gsmSerial.begin(9600, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX, false);
  if (!gsmSerial) { // If the object did not initialize, then its configuration is invalid
    Serial.println("Invalid SoftwareSerial pin configuration, check config"); 
    while (1) { // Don't continue with invalid configuration
      delay (1000);
    }
  } 
  Serial2.begin(9600);
  delay(10);
  pinMode(23, OUTPUT);
  digitalWrite(23, HIGH);

  while (!bmp.begin(0x76)) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    delay(1000);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_4000); /* Standby time. */
  
  Serial.println("GPS NEO 6M");

  
  Serial.setTimeout(500);// Set time out for 
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  reconnect();
}

void updateSerial(){
  while (Serial.available())  {
    gsmSerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while (gsmSerial.available())  {
    Serial.write(gsmSerial.read());//Forward what Software Serial received to Serial Port
  }
}

void addMeasurement(double lat, double lng, double spd, double temp, double dir, double timestamp) {
  JsonObject point = data_points.createNestedObject();
  point["t"] = timestamp;
  JsonObject measured_data = point.createNestedObject("d");
  measured_data["A"] = lat;
  measured_data["O"] = lng;
  measured_data["S"] = spd;
  measured_data["T"] = temp;
  measured_data["D"] = dir;
}

void displayInfo()
{
  if (gps.location.isValid() && gps.time.isValid() && gps.speed.isValid() && 
     (gps.location.isUpdated() || gps.time.isUpdated() || gps.speed.isUpdated())) {
    Serial.print("Satalites: ");
    Serial.println(gps.satellites.value());
    avg_lat += gps.location.lat();
    avg_lng += gps.location.lng();
    avg_speed += gps.speed.knots();
    avg_dir += gps.course.value();
    if ((millis() - start_time) > period) {
      start_time = millis();
      double temp = (double)bmp.readTemperature() + tempOffset;
      float timestamp = gps.time.value();
      
      addMeasurement(
        avg_lat / (double)count, 
        avg_lng / (double)count, 
        round2(avg_speed / (double)count), 
        round2(temp), 
        round2(avg_dir / (double)count),
        timestamp / 100.0
      );
      avg_lat = 0;
      avg_lng = 0;
      avg_speed = 0;
      count = 0;
    }
    count++;   
  } else if (gps.location.isValid() && gps.time.isValid() && gps.speed.isValid()) {
    Serial.println("All items valid, but no bueno");
  } else {
    Serial.println("Not all items valid");
  }
}

void makeWifiPost(char* json) {
  if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
      String serverPath = "bootje.erickemmeren.nl/data";
      http.begin(serverPath);
      http.addHeader("Content-Type", "application/json");
      int responseCode = http.POST(json);
      Serial.println(responseCode);
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  updateSerial();
  while (Serial2.available() > 0) {
    if (gps.encode(Serial2.read())) {
      displayInfo();
    }
  }
  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while (true);
  }
  if (doc.memoryUsage() > 120) {
     size_t size_output = serializeJson(doc, output);
     if (size_output > 95) {
       Serial.println();
       Serial.println(size_output);
       Serial.println(doc.memoryUsage());
       serializeJson(doc, output);
       makeWifiPost(output);
       serializeJsonPretty(doc, Serial);
       doc.clear();
       data_points = doc.createNestedArray("dp");
     }
  }
}
