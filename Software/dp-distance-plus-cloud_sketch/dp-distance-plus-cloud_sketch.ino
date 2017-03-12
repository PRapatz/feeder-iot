
// ************************************************************************************
// Distance measuring sketch for the Dash Pro Cellular cloud
// 
// Goal is to test the HC-SR04 Distance sensor on the Dash Pro along with the NewPing libary.
//
// **************************************************************************************

#define DO_BITWISE false // necessary to compile for the DashPro card rather than the Arduino
#define TRIGGER_PIN D10 // D10 and D09 are the GPIO pins on the Dashpro
#define ECHO_PIN D09
#include <NewPing.h>
int numSends;
const int MAX_DISTANCE = 200;


NewPing sonar_obj(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

void setup() {


  // Start the Serial 
//  Serial.begin(9600); 

  // Start the Dash class
  Dash.begin();

  SerialCloud.begin(115200);
  SerialUSB.begin(9600);
  SerialUSB.println("Hello Cloud example has started...");
  numSends = 0; // count number of sends
  
}

void loop() { // put your main code here, to run repeatedly:

  // every 60 seconds, send a message to the Cloud

  if((numSends < 6) && (millis() % 60000 == 0)) {
    SerialUSB.println("Sending distance message # " + String(numSends) + " to the Cloud...");
    unsigned int uS = sonar_obj.ping_median(5);
//    unsigned int uS = 1000;
//    Serial.print("Ping: ");
//    Serial.print(uS / US_ROUNDTRIP_CM);
//    Serial.println("cm");
    SerialUSB.println("Cloud Message # " + String(numSends) + "  Ping: " + String(uS/US_ROUNDTRIP_CM) + "cm"); // send to TTY
    SerialCloud.println("Cloud Message # " + String(numSends) + "  Ping: " + String(uS/US_ROUNDTRIP_CM) + "cm"); // send to Cloud
    
    SerialUSB.println("Message sent!");
    numSends++; // increase the number-of-sends counter

  // Flash the LED to show we read from the sensor
//    Dash.onLED();
//    Dash.offLED();
  
  }

  // two-way serial passthrough for seeing debug statements
  while(SerialUSB.available()) {
    SerialCloud.write(SerialUSB.read());
  }

  while(SerialCloud.available()) {
    SerialUSB.write((char)SerialCloud.read());
  }




  
}
