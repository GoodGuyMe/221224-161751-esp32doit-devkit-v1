#include <Main.h>
#define ARDUINOJSON_POSITIVE_EXPONENTIATION_THRESHOLD 1e8
#include <ArduinoJson.h>

DynamicJsonDocument doc(2024);
JsonArray data_points = doc.createNestedArray("dp");
char output[1024];
size_t size_output = 0;

unsigned long count = 0;

double avg_lat = 0;
double avg_lng = 0;
double avg_speed = 0;
double avg_dir = 0;
double min_speed = 1.5;
unsigned long start_time = 0;
unsigned int slow_period = 10 * 60e3;   // 10 Minutes (s)
unsigned int fast_period = 1e3;         //  1 Second  (s)
unsigned int period = slow_period;
unsigned int time_fast_period = 1 * 60e6; // 1 Minute (us)

hw_timer_t * period_timer;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// rounds a number to 2 decimal places
// example: round(3.14159) -> 3.14
double round2(double value) {
    return (int)(value * 100 + 0.5) / 100.0;
}


void setup() {
    Serial.begin(115200);
    delay(10);

    period_timer = NULL;

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

void ARDUINO_ISR_ATTR on_timer() {
    portENTER_CRITICAL_ISR(&timerMux);
    period = slow_period;
    portEXIT_CRITICAL_ISR(&timerMux);

}

void displayInfo()
{
    if (validGPS()) {
        avg_lat += gps.location.lat();
        avg_lng += gps.location.lng();
        avg_speed += gps.speed.knots();
        avg_dir += gps.course.deg();
        if (gps.speed.knots() > min_speed) {
            portENTER_CRITICAL(&timerMux);
            period = fast_period;
            if (period_timer == NULL) {
                period_timer = timerBegin(0, 80, true);
                if (period_timer == NULL) {
                    period = fast_period;
                }
                timerAttachInterrupt(period_timer, &on_timer, true);
                timerAlarmWrite(period_timer, time_fast_period, false);
                timerAlarmEnable(period_timer);
            } else {
                timerRestart(period_timer);
                timerAlarmEnable(period_timer);
            }
            portEXIT_CRITICAL(&timerMux);
        }
        if ((millis() - start_time) > period) {
            Serial.print("Satalites: ");
            Serial.println(gps.satellites.value());

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

            size_output = serializeJson(doc, output);
            Serial.println(size_output);
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
