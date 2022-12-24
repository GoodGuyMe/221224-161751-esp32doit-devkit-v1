#include <BMP280.h>

Adafruit_BMP280 bmp;

void setupBMP() {
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_4000); /* Standby time. */

    while (!bmp.begin(0x76)) {
        Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                        "try a different address!"));
        delay(1000);
    }
}

double getTemperature() {
    return (double)bmp.readTemperature() + tempOffset;
}