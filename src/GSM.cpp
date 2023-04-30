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

Result sendGSM(const char* body) {
  HTTP http(9600, MYPORT_RX, MYPORT_TX, 12);
  Result result_connect = http.connect();
  Serial.println("Result connect:");
  Serial.print(result_connect);
  if (result_connect != SUCCESS) {
    http.disconnect();
    return result_connect;
  }

  char response[64];
  Result result_post = http.post("https://bootje.erickemmeren.nl/data", body, response);
  Serial.print("Result post: ");
  Serial.println(result_post);

  Serial.print("Response: ");
  Serial.println(response); 
  http.disconnect();

  return result_post;
}