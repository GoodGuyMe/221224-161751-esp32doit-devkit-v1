#include <GPS.h>

TinyGPSPlus gps;
bool gps_started = false;

void setupGPS() {
    Serial2.begin(9600);
}

void updateGPS(std::function<void(void)> return_func) {
    while (Serial2.available() > 0) {
        if (gps.encode(Serial2.read())) {
            return_func();
        }
    }
    if (millis() > 5000 && gps.charsProcessed() < 10)
    {
        Serial.println(F("No GPS detected: check wiring."));
    }
}

bool validGPS() {
    if (gps_started) {
        return gps.location.isValid() && gps.time.isValid() && gps.speed.isValid() && 
        (gps.location.isUpdated() || gps.time.isUpdated() || gps.speed.isUpdated());
    }
    if (millis() > 60000) {
        Serial.println("Valid gps signal!");
        gps_started = true;
        return validGPS();
    }
    return false;
}