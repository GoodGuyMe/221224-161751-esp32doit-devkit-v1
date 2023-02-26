#include <GSM.h>

// SoftwareSerial gsmSerial;

void setupGSM() {
  // gsmSerial.begin(9600, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX, false);
  // if (!gsmSerial) { // If the object did not initialize, then its configuration is invalid
  //   Serial.println("Invalid SoftwareSerial pin configuration, check config"); 
  //   while (1) { // Don't continue with invalid configuration
  //     delay (1000);
  //   }
  // } 
}

void sendGSM(const char* body) {
  HTTP http(9600, MYPORT_RX, MYPORT_TX, 12);
  http.connect();

  char response[64];
  Result result = http.post("bootje.erickemmeren.nl/data", body, response);

  Serial.println(response);
  http.disconnect();
  // while (Serial.available())  {
  //   gsmSerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  // }
  // while (gsmSerial.available())  {
  //   Serial.write(gsmSerial.read());//Forward what Software Serial received to Serial Port
  // }
}