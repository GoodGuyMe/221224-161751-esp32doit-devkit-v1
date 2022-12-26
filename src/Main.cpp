#include <Main.h>
#define ARDUINOJSON_POSITIVE_EXPONENTIATION_THRESHOLD 1e8
#include <ArduinoJson.h>

DynamicJsonDocument doc(2024);
JsonArray data_points = doc.createNestedArray("dp");
char output[1024];

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

void setup() {
    Serial.begin(115200);
    delay(10);

    Serial.println("GPS NEO 6M + GSM + BMP280");
    Serial.setTimeout(500);
    // setupMQTT();
    setupGPS();
    setupGSM();
    setupBMP();
}

void addMeasurement(double lat, double lng, double spd, double temp, double dir, double datestamp, double timestamp) {
    JsonObject point = data_points.createNestedObject();
    point["t"] = timestamp;
    point["d"] = datestamp;
    JsonObject measured_data = point.createNestedObject("p");
    measured_data["A"] = lat;
    measured_data["O"] = lng;
    measured_data["S"] = spd;
    measured_data["T"] = temp;
    measured_data["D"] = dir;
}

void displayInfo()
{
    if (validGPS()) {
        Serial.print("Satalites: ");
        Serial.println(gps.satellites.value());
        avg_lat += gps.location.lat();
        avg_lng += gps.location.lng();
        avg_speed += gps.speed.knots();
        avg_dir += gps.course.deg();
        if ((millis() - start_time) > period) {

            start_time = millis();
            float datestamp = gps.date.value();
            float timestamp = gps.time.value();

            addMeasurement(
                avg_lat / (double)count, 
                avg_lng / (double)count, 
                round2(avg_speed / (double)count), 
                round2(getTemperature()), 
                round2(avg_dir / (double)count),
                datestamp,
                timestamp / 100.0
            );
            avg_lat = 0;
            avg_lng = 0;
            avg_speed = 0;
            avg_dir = 0;
            count = 0;
        }
        count++;   
    } else {   
        if (gps.satellites.value() == 0) {
            Serial.print(".");
        } else {
            Serial.println("Not all items valid");
        }
    }
}

void loop() {
    // updateMQTT();
    updateGSM();
    updateGPS(displayInfo);

    if (doc.memoryUsage() > 1200) {
        size_t size_output = serializeJson(doc, output);
        if (size_output > 950) {
            Serial.println();
            Serial.println(size_output);
            Serial.println(doc.memoryUsage());
            makeWifiPost(output);
            serializeJsonPretty(doc, Serial);
            doc.clear();
            data_points = doc.createNestedArray("dp");
        }
    }
}
