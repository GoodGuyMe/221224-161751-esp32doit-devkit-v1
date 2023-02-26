#pragma once

#define _SS_MAX_RX_BUFF 512
#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>

#include "HTTP.h"

#define MYPORT_TX 12
#define MYPORT_RX 13

void setupGSM();
void sendGSM(const char* body);