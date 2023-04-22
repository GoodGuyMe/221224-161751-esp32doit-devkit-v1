#include <Main.h>
#define ARDUINOJSON_POSITIVE_EXPONENTIATION_THRESHOLD 1e8
#include <ArduinoJson.h>

Data *data;
size_t size_output = 100000;
char *output;

const size_t size_moving_average = 512;
double moving_average[size_moving_average]; 
unsigned int moving_average_count = 0;

unsigned int message_count = 0;
unsigned long count = 0;

double avg_lat = 0;
double avg_lng = 0;
double avg_speed = 0;
double avg_dir = 0;
double min_speed = 2.5;
unsigned long start_time = 0;
unsigned int slow_period = 3600e3;      // 1 hour     (ms)
unsigned int fast_period =   30e3;      // 30 seconds (ms)
unsigned int gps_occurences = 6;
volatile unsigned long period = slow_period;
unsigned int time_fast_period = 5 * 60e6; // 5 Minutes (us)
uint8_t gps_overload = 0;

hw_timer_t * period_timer;

// rounds a number to 2 decimal places
// example: round(3.14159) -> 3.14
double round2(double value) {
    return (long)(value * 100L + 0.5) / 100.0;
}

void setup() {
    delay(10);
    
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);

    data = new Data(10);
    output = (char *)malloc(sizeof(char) * 100000);
    if (output == NULL) {
        Serial.println("Could not allocate memory");
    }

    Serial.begin(115200);

    period_timer = NULL;

    Serial.println("GPS NEO 6M + GSM + BMP280 V2.0");
    Serial.setTimeout(500);
    setupGPS();
    setupGSM();
    setupBMP();

    for (int i = 0; i < size_moving_average; i++) {
        moving_average[i] = 0.0;
    }

    digitalWrite(2, LOW);
}

void ARDUINO_ISR_ATTR on_timer() {
    period = slow_period;
    digitalWrite(2, LOW);
}

double movingAverage(double speed) {
    moving_average[moving_average_count++] = speed;
    moving_average_count = moving_average_count % size_moving_average;
    double result = 0.0;
    for (int i = 0; i < size_moving_average; i++) {
        result += moving_average[i];
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
            digitalWrite(2, HIGH);
            if (period_timer == NULL) {
                period_timer = timerBegin(0, 80, true);
                timerAttachInterrupt(period_timer, &on_timer, true);
                timerAlarmWrite(period_timer, time_fast_period, false);
                timerAlarmEnable(period_timer);
            } else {
                timerRestart(period_timer);
                timerAlarmEnable(period_timer);
            }
            period = fast_period;
        }
        if ((millis() - start_time) > period) {

            start_time = millis();
            float datestamp = gps.date.value();
            float timestamp = gps.time.value();

            DataPoint *dp = (period == fast_period || message_count == 0) ?
                new DataPoint(
                    timestamp / 100.0,
                    datestamp, 
                    round2(getTemperature()), 
                    round2(avg_dir   / (double)count), 
                    round2(avg_speed / (double)count), 
                    avg_lat / (double)count, 
                    avg_lng / (double)count
                ) :
                new DataPoint(
                    timestamp / 100.0, 
                    datestamp, 
                    round2(getTemperature())
                );

            data->add(dp);

            avg_lat = 0;
            avg_lng = 0;
            avg_speed = 0;
            avg_dir = 0;
            count = 0;
        }
        count++;
    }
}

uint32_t max_size_output = 800;

void loop() {
    updateGPS(displayInfo);

    if (Serial.available() > 0) {
        char *buf = (char *)malloc(sizeof(char));
        Serial.read(buf, 1);
        if (*buf == '0') {
            gps_overload = 0;
        } else if (*buf == '1') {
            gps_overload = 1;
        } else if (*buf == 'm') {
            Serial.print("Free heap: ");
            Serial.println(ESP.getFreeHeap());
        }
        free(buf);
    }

    size_t size = data->getSerializedJson(output, size_output);

    if (size > max_size_output) {
        Result result = SUCCESS;

        result = sendGSM(output);
        
        if (result == SUCCESS) {
            delete data;

            data = new Data(10);
            
            max_size_output = 800;
        } else {
            max_size_output += 800;
            Serial.print("max_size_output: ");
            Serial.println(max_size_output);
        } 
    }
}
