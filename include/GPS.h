#include <TinyGPSPlus.h>

TinyGPSPlus gps;

void setupGPS();
void updateGPS(std::function<void(void)>);

bool validGPS();