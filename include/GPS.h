#pragma once

#include <TinyGPSPlus.h>

extern TinyGPSPlus gps;

void setupGPS();
void updateGPS(std::function<void(void)>);

bool validGPS();