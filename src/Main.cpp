#include <Main.h>
#define ARDUINOJSON_POSITIVE_EXPONENTIATION_THRESHOLD 1e8
#include <ArduinoJson.h>

DynamicJsonDocument doc(2024);
JsonArray data_points = doc.createNestedArray("dp");
char output[2024];
size_t size_output = 0;

const size_t size_moving_average = 512;
double moving_average[size_moving_average]; 
unsigned int moving_average_count = 0;

unsigned int message_count = 0;
unsigned long count = 0;

double avg_lat = 0;
double avg_lng = 0;
double avg_speed = 0;
double avg_dir = 0;
double min_speed = 3;
unsigned long start_time = 0;
unsigned int slow_period = 3600e3;      // 1 hour     (ms)
unsigned int fast_period =   10e3;      // 10 seconds (ms)
unsigned int period = slow_period;
unsigned int time_fast_period = 5 * 60e6; // 1 Minute (us)


hw_timer_t * period_timer;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// rounds a number to 2 decimal places
// example: round(3.14159) -> 3.14
double round2(double value) {
    return (long)(value * 100 + 0.5) / 100.0;
}


void setup() {
    delay(10);

    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);
    Serial.begin(115200);

    period_timer = NULL;

    Serial.println("GPS NEO 6M + GSM + BMP280");
    Serial.setTimeout(500);
    setupGPS();
    setupGSM();
    setupBMP();

    for (int i = 0; i < size_moving_average; i++) {
        moving_average[i] = 0.0;
    }
    digitalWrite(2, LOW);
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

void addTempMeasurement(double temp, double datestamp, double timestamp) {
    JsonObject point = data_points.createNestedObject();
    point["t"] = timestamp;
    point["d"] = datestamp;
    JsonObject measured_data = point.createNestedObject("p");
    measured_data["T"] = temp;
}

void ARDUINO_ISR_ATTR on_timer() {
    portENTER_CRITICAL_ISR(&timerMux);
    period = slow_period;
    digitalWrite(2, LOW);
    portEXIT_CRITICAL_ISR(&timerMux);

}

double movingAverage(double speed) {
    moving_average[moving_average_count++] = speed;
    moving_average_count = moving_average_count % size_moving_average;
    double result = 0.0;
    for (int i = 0; i < size_moving_average; i++) {
        result += moving_average[i];
    }
    if (moving_average_count == 0) {
        Serial.println(millis());
    }
    return result / (double)size_moving_average;
}

void displayInfo()
{
    if (validGPS()) {
        avg_lat += gps.location.lat();
        avg_lng += gps.location.lng();
        avg_speed += gps.speed.knots();
        avg_dir += gps.course.deg();
        if (movingAverage(gps.speed.knots()) > min_speed) {
            portENTER_CRITICAL(&timerMux);
            period = fast_period;
            digitalWrite(2, HIGH);
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

            if (period == fast_period || message_count == 0) {
                addMeasurement(
                    avg_lat / (double)count, 
                    avg_lng / (double)count, 
                    round2(avg_speed / (double)count), 
                    round2(getTemperature()), 
                    round2(avg_dir / (double)count),
                    datestamp,
                    timestamp / 100.0
                );
            } else {
                addTempMeasurement(
                    round2(getTemperature()),
                    datestamp,
                    timestamp / 100.0
                );
            }
            message_count = (message_count + 1) % 6;
            size_output = serializeJson(doc, output);
            Serial.print("Size of output: ");
            Serial.println(size_output);
            Serial.print("Count: ");
            Serial.println(count);

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
            Serial.print("location: ");
            Serial.println(gps.location.isValid()); 
            Serial.print("Time: ");
            Serial.println(gps.time.isValid());
            Serial.print("Speed: ");
            Serial.println(gps.speed.isValid());
            Serial.print("Satalites: ");
            Serial.println(gps.satellites.value());
        }
    }
}

void loop() {
    updateGPS(displayInfo);
    if (size_output > 950) {
        Serial.println();
        Serial.println(size_output);
        Serial.println(doc.memoryUsage());
        if (WiFi.status() == WL_CONNECTED || setupWiFi(false) == WL_CONNECTED) {
            makeWifiPost(output);
        } else {
            sendGSM(output);
        }
        serializeJsonPretty(doc, Serial);
        doc.clear();
        size_output = 0;
        data_points = doc.createNestedArray("dp");
    }
}
