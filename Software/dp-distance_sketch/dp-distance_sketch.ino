
// ************************************************************************************
// Distance measuring sketch for the Dash Pro Cellular cloud
// 
// Goal is to test the HC-SR04 Distance sensor on the Dash Pro along with the NewPing libary.
//
// **************************************************************************************

#define DO_BITWISE false // necessary to compile for the DashPro card rather than the Arduino
#define TRIGGER_PIN D10 // D01 and D02 are the GPIO pins on the Dashpro
#define ECHO_PIN D09
const int MAX_DISTANCE = 200; 
#include <NewPing.h>

NewPing sonar_obj(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

void setup() {
  

  // Start the Serial 
  Serial.begin(9600); 

  // Start the Dash class
  Dash.begin();

  // Initialize the NewPing object
//  sonar_obj = NewPing(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
  
}

void loop() { // put your main code here, to run repeatedly:
  //NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
  unsigned int uS = sonar_obj.ping_median(5);
  Serial.print("Ping: ");
  Serial.print(uS / US_ROUNDTRIP_CM);
  Serial.println("cm");

  // Flash the LED to show we read from the sensor
  Dash.onLED();
  delay(500);

  Dash.offLED();
  delay(10000);
  
}
