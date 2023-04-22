#pragma once

#include "main.h"
#include "ArduinoJson.h"

class DataPoint {
    private:
        bool only_temp;
        double time;
        double date;
        double temp;
        double dir;
        double speed;
        double lat;
        double lng;

    public:
        DataPoint(double time, double date, double temp, double dir, double speed, double lat, double lng);
        DataPoint(double time, double date, double temp);
        DataPoint();
        ~DataPoint();
        void addJson(JsonArray data_points);

};

class Data {
    private:
        DataPoint **data;
        size_t max_size;
        size_t curr_size;

    public:
        Data(size_t initialSize);
        ~Data();
        void add(DataPoint *dp);
        size_t getSerializedJson(char *output, size_t output_size);
};