#include "data.h"

DataPoint::DataPoint(double time, double date, double temp) 
{
    this->time = time;
    this->date = date;
    this->temp = temp;
    this->only_temp = true;
}

DataPoint::DataPoint()
{
    this->time = 0;
    this->date = 0;
    this->temp = 0;
    this->only_temp = true;
}

DataPoint::DataPoint(double time, double date, double temp, double dir, double speed, double lat, double lng) 
{
    this->time = time;
    this->date = date;
    this->temp = temp;
    this->dir = dir;
    this->speed = speed;
    this->lat = lat;
    this->lng = lng;
    this->only_temp = false;
}

DataPoint::~DataPoint() {
    Serial.print(this->time);
    Serial.println(" Datapoint removed");
}

void DataPoint::addJson(JsonArray data_points) 
{
    JsonObject point = data_points.createNestedObject();
    point["t"] = this->time;
    point["d"] = this->date;
    JsonObject measured_data = point.createNestedObject("p");
    measured_data["T"] = this->temp;
    if (!this->only_temp) {
        measured_data["A"] = this->lat;
        measured_data["O"] = this->lng;
        measured_data["S"] = this->speed;
        // measured_data["D"] = this->dir; // Don't really care for the direction graph, don't send this one for now
    }
}

Data::Data(size_t initialSize)
{
    this->data = (DataPoint **)calloc(sizeof(DataPoint), initialSize);
    this->max_size = initialSize;
    this->curr_size = 0;
}

Data::~Data() {
    for (int i = 0; i < curr_size; i++) {
        delete data[i];
    }
    free(data);
}

void Data::add(DataPoint *dp)
{
    if (curr_size == max_size) {
        max_size *= 2;
        data = (DataPoint **)reallocarray(data, sizeof(DataPoint), max_size);
    }
    data[curr_size++] = dp;
}

size_t Data::getSerializedJson(char *output, size_t output_size)
{
    DynamicJsonDocument doc(45000);
    JsonArray dp = doc.createNestedArray("dp");
    for (int i = 0; i < curr_size; i++) {
        DataPoint *point = data[i];
        point->addJson(dp);
    }
    return serializeJson(doc, output, output_size);
}
