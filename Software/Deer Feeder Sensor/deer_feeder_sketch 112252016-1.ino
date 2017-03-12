
// ************************************************************************************
// Distance measuring sketch for the Dash Cellular cloud
// 
// Author: Phillip Rapatz
// Date:   November 16th, 2016
// Updates: 
//  - 11/17/2016 - Added new code from the Dash Time Out test sketch
//
//
// Description:
// 
// The Deer Feeder Sketch will measure the distance from the top of the deer feeder to 
// the top of the loaded corn in the feeder.  When the distance is at the minimum level,
// the feeder is considered full.  When the distance is at the maximum level, the feeder
// is considered empty.  The sensor package will send the feeder level data once every
// 24 hours to the Hologram Cloud. Web hook applications will forward the appropriate
// information to the owner.
//
// **************************************************************************************

#define DO_BITWISE false // necessary to compile for the DashPro card rather than the Arduino
#define TRIGGER_PIN D10 // D10 and D09 are the GPIO pins on the Dashpro
#define ECHO_PIN D09

//settings for the HC SR04 Sonic Distance sensor
#define MAX_DISTANCE 200        // Max distance allowed by sensor
#define SLEEP_TIME_MINS 60      // time between waking up and sending data
#define WAKE_TIME_SECONDS 600   // time allow to wake up and send data

#include <NewPing.h>

int writeCounter = 0;
long wakeTimer = 0;
long resyncTimer = 0;
bool wakeFlag = false;
bool resyncFlag = false;



NewPing sonar_obj(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

void setup() {

  
  /* Serial Setup */
  SerialUSB.begin(9600); /* USB UART */
  Serial2.begin(9600); /* TTL UART */
  SerialCloud.begin(115200); /* Konekt Cloud */
  delay(4000); /* Delay 4 seconds to wait for Usb Serial to Init.*/

  /* Setup Konekt Dash */
  Dash.begin();
  Dash.pulseLED(100,5000); /* Set the User Led to flash every 5 seconds */

  /* Serial Print Info */
  SerialUSB.println("Hologram Dash Hello World Example Started!");
  SerialUSB.print("Using Boot Version: ");
  SerialUSB.println(Dash.bootVersion()); /* Print Dash Bootloader Version */
  
  SerialUSB.println("Deer Feeder Sensor Sketch Initiated...");

  // Initialize variables controlling flow
  wakeFlag = false;
  resyncFlag = false;
  wakeTimer = millis();
  resyncTimer = wakeTimer;
}

void loop() { // put your main code here, to run repeatedly:

  char currChar;
  unsigned int timeDelta;
  float distanceMeasurement;
  int batteryPercent;
  long batteryMilvolts;
  String messageString;
  String jsonString;
  
  /* Set up a communication loop that sends a message every 60 seconds for 5 mins */
  /* and then goes to sleep for a period of time                                  */ 

  /* check the wakeFlag.  It should only be false if it is waking up.  Otherwise it should be true */
  /* and the wakeTimer needs to be initialized. */


  
  if (!wakeFlag) {    
    SerialUSB.println("PROGRAM: wakeFlag is false.  Time to wake up!");
    wakeFlag = true;    // wakeFlag = true when the system is awake
    resyncFlag = false; // resyncFlag = false when still resyncing
    resyncTimer = millis();
  }

  /* check to see if we should write to the cloud.  If the timer is greater than the writeTime then  */
  /* keep running and sending to the cloud. Otherwise, set the flag to false and go to sleep */
  /* for the specified period of time.                                                       */

  if (!resyncFlag & millis() - resyncTimer >= 120000) { // resync period is over.  Write message to the cloud
    resyncFlag = true; // resync is complete
    SerialUSB.println("PROGRAM: End of the resync period, ready to transmit!");

    wakeTimer = millis();  // ready to transmit, start calculating write period and wake period
 
    // check the battery charge
    batteryPercent = Dash.batteryPercentage();
    batteryMilvolts = Dash.batteryMillivolts();

    // get the distance measurement from the HC SR04
    timeDelta = sonar_obj.ping_median(5); // ranging time in micro seconds
    distanceMeasurement = timeDelta / US_ROUNDTRIP_CM;

    // construct the JSON output string to send to the cloud
    writeCounter = writeCounter + 1;
    messageString = "**PROGRAM - Cloud Message: #" + String(writeCounter) + "   Bat %: " + String(batteryPercent) + "   Bat mV: " + String(batteryMilvolts);


    jsonString = "{";
    jsonString.concat("\"message_nbr\":\"" + String(writeCounter) + "\",");
    jsonString.concat("\"distance\":\"" + String(distanceMeasurement) + "\",");
    jsonString.concat("\"battery_percentage\":\"" + String(batteryPercent) + "\",");
    jsonString.concat("\"battery_voltage\":\"" + String(batteryMilvolts) + "\"}");
                                     
    SerialCloud.println(jsonString);
    SerialUSB.println(messageString);
    
  }
  
    /* check to see if we should be awake.  If the total time awake is greater than the wakeTime then   */
  /* its time to go to sleep.  Set the flag to false and go to sleep for the specified period of time.*/

  if (resyncFlag & millis() - wakeTimer > 180000) {
    SerialUSB.println("PROGRAM: End of Wake Period! Time to go to sleep for 120 mins");
    wakeFlag = false;
    resyncFlag = false;

    Dash.deepSleepMin(120);   // deepSleep for 120 mins
   
  }

/* This is the simple serial write code */

  /* the code here will pass data between Cloud<-->UART */

  while (SerialUSB.available()) {
    SerialCloud.write(SerialUSB.read());
  }

  while (Serial2.available()) {
    SerialCloud.write(Serial2.read());
  }

  while (SerialCloud.available()) {
    currChar = (char)SerialCloud.read();
    SerialUSB.write(currChar);
    Serial2.write(currChar);
  }

 // delay(5);

  
} // end of LOOP routine
