
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

//#define PS_INCLUDE_SOFTWARESERIAL // needs to be defined within PingSerial as well.  To be
				  // removed if using a hardware serial port


#include <PingSerial.h>
#include <DHT.h>

#define TX2_PIN D14 // TX2 for UART2 on the Dash (access with Serial2) 
#define RX2_PIN D12 // RX2 for UART2 on the Dash (access with Serial2)

#define DHTPIN D10 // D10 is a GPIO pin on the Dash/Dashpro
#define DHTTYPE DHT22

//settings for the US-100 Sonic Distance sensor
#define MAX_DISTANCE 1500           // Max distance allowed by sensor (mm)
#define MIN_DISTANCE 20             // Min distance allowed by sensor (mm)

// settings for controlling wakeup, resynch and snooze times
#define SLEEP_TIME_MINS 176        // time period the card goes to sleep for 24 hours (minus the wake time) 1440 = 24 hrs 720 = 12 hrs
#define WAKE_TIME_MILSECS 120000    // time to leave the card awake and sending data
#define RESYNC_TIME_MILSECS 120000  // time to allow the card to resync the data transmission
#define SNOOZE_TIME_MILSECS 180000  // time to let the card snooze - testing purposes only

// US-100 Timer settings
#define PING_TIME 1000              // time between distance pings during resync
#define TEMP_TIME 3000              // time between temperature measurements during resync

// variables to control period timers
int writeCounter = 0;
long wakeTimer = 0;
long resyncTimer = 0;
long pingTimer = 0;		    // timer for distance pings
long tempTimer = 0;		    // timer for taking temp measurements

// flags to trigger re-initialization on wakeup
bool wakeFlag = false;
bool resyncFlag = false;

// variables to hold latest distance/temp measurments
float latestDistanceMeasurement = 0;
float latestTemperatureMeasurement = 0;


/* Initialize the SerialPing distance Sensor and make it global */
PingSerial us100_sensor(Serial2, MIN_DISTANCE, MAX_DISTANCE);

/* Initialize the DHT Temp/Humidity Sensor */
DHT dht_sensor(DHTPIN, DHTTYPE);

/*						  */
/*						  */
/* Setup code - Initialization and run on startup */
/* 	      		       	       	  	  */
/*						  */
void setup() {

  
  /* Serial Setup */
  SerialUSB.begin(9600);	/* USB UART	- Dash built-in	  */
  Serial2.begin(9600); 		/* TTL UART 	- UART2 Dash built-in */
  SerialCloud.begin(115200); 	/* Cloud UART	- UART1 Dash built-in */


  delay(10000); /* Delay 10 seconds to wait for Usb Serial to Init.*/

  /* Initialize and setup Dash Class */
  Dash.begin();
  Dash.pulseLED(100,5000); /* Set the User Led to flash every 5 seconds */

  /* Initialize the DHT Sensor */
  dht_sensor.begin();

  /* Initialize the us100 sensor */
  us100_sensor.begin();

  /* Serial Print Info */
  SerialUSB.println("Deer Feeder Sensor Sketch v(0.9) has now started!");
  SerialUSB.print("Using Boot Version: ");
  SerialUSB.println(Dash.bootVersion()); /* Print Dash Bootloader Version */
  
  SerialUSB.println("Deer Feeder Sensor Sketch Initiated...");

  // Initialize variables controlling flow
  wakeFlag = false;
  resyncFlag = false;
  wakeTimer = millis();
  resyncTimer = wakeTimer;
}

/*				    *
/*           Main Loop              */
/*	     	  		    */
void loop() { // put your main code here, to run repeatedly:

  char currChar;
  unsigned int timeDelta;
  int batteryPercent;
  long batteryMilvolts;
  float dht_relativeHumidity;
  float dht_temperature;
  float dht_heatIndex;
  
  String messageString1;
  String messageString2;
  String jsonString;

  byte data_available;
  
  /* Set up a communication loop that sends a message every 60 seconds for 5 mins */
  /* and then goes to sleep for a period of time                                  */ 

  /* check the wakeFlag.  It should only be false if it is waking up.  Otherwise it should be true */
  /* and the wakeTimer needs to be initialized. */
  
  if (!wakeFlag) {    
    SerialUSB.println("DFS: Time to wake up! Entering resynchronization mode for " + String(RESYNC_TIME_MILSECS/1000) + " seconds.");
    wakeFlag = true;    // wakeFlag = true when the system is awake
    resyncFlag = false; // resyncFlag = false when still resyncing
    resyncTimer = millis();
  }

  /* Resycnc is underway until the resync timer is over.  In the meantime, lets warm up */
  /* the distance and temperature sensor.                                               */

  if (millis() - pingTimer >= PING_TIME) { // delay between pings is over
    us100_sensor.request_distance();
    pingTimer = millis();
  }

  if (millis() - tempTimer >= TEMP_TIME) { // delay between temp measurements is over
    us100_sensor.request_temperature();
    tempTimer = millis();
  }

  // continully check to see if the u100 has data
  data_available = us100_sensor.data_available();

  if (data_available & DISTANCE) { // uses the AND operator to see if there is a distance measurement
      latestDistanceMeasurement = us100_sensor.get_distance();
//      SerialUSB.println("found a distance: " + String(latestDistanceMeasurement));
  }

  if (data_available & TEMPERATURE) { // uses the AND operator to see if there is a temp measurement
      latestTemperatureMeasurement = us100_sensor.get_temperature();
//      SerialUSB.println("found a temperature: " + String(latestTemperatureMeasurement));
  }



  /* check to see if we should write to the cloud.  If the timer is greater than the writeTime then  */
  /* keep running and sending to the cloud. Otherwise, set the flag to false and go to sleep */
  /* for the specified period of time.                                                       */

  if (!resyncFlag & millis() - resyncTimer >= RESYNC_TIME_MILSECS) { // resync period is over.  Write message to the cloud
    resyncFlag = true; // resync is complete
    SerialUSB.println("DFS: End of the resync period. Ready to transmit for the next " + String(WAKE_TIME_MILSECS/1000) + " seconds.");

    wakeTimer = millis();  // ready to transmit, start calculating write period and wake period
 
    // check the battery charge
    batteryPercent = Dash.batteryPercentage();
    batteryMilvolts = Dash.batteryMillivolts();

    // get the temperature, humdity and heat index
    dht_temperature = dht_sensor.readTemperature();
    dht_relativeHumidity = dht_sensor.readHumidity();
    dht_heatIndex = dht_sensor.computeHeatIndex(dht_temperature, dht_relativeHumidity, false);

    // construct the Message output string to send to the serial output
    writeCounter = writeCounter + 1;
    messageString1 = "**PROGRAM - Cloud Message: #" + String(writeCounter) + "   Distance: "        + String(latestDistanceMeasurement) +
                                                                             "   Bat %: "           + String(batteryPercent) + 
                                                                             "   Bat mV: "          + String(batteryMilvolts);
                                                                             
    messageString2 = "**PROGRAM - Cloud Message: #" + String(writeCounter) + "   Temp (C): "        + String(dht_temperature) + 
                                                                             "   Humidity (%): "    + String(dht_relativeHumidity) + 
                                                                             "   Heat Index (C): "  + String(dht_heatIndex);


    // Create the JSON string to send to the cloud
    jsonString = "{";
    jsonString.concat("\"message_nbr\":\""        + String(writeCounter)        + "\",");
    jsonString.concat("\"distance\":\""           + String(latestDistanceMeasurement) + "\",");
    jsonString.concat("\"battery_percentage\":\"" + String(batteryPercent)      + "\",");
    jsonString.concat("\"battery_voltage\":\""    + String(batteryMilvolts)     + "\",");    
    jsonString.concat("\"temperature\":\""        + String(dht_temperature)         + "\",");
    jsonString.concat("\"relative_humidity\":\""  + String(dht_relativeHumidity)    + "\",");
    jsonString.concat("\"heat_index\":\""         + String(dht_heatIndex)           + "\"}");

    SerialCloud.println(jsonString);
    SerialUSB.println(messageString1);
    SerialUSB.println(messageString2);
    
  }
  
  /* check to see if we should be awake.  If the total time awake is greater than the wakeTime then   */
  /* its time to go to sleep.  Set the flag to false and go to sleep for the specified period of time.*/

  if (resyncFlag & millis() - wakeTimer > WAKE_TIME_MILSECS) {

    wakeFlag = false;
    resyncFlag = false;
    
/* Test Timers only           */
/* comment out for production */    
//    SerialUSB.println("DFS: End of Wake Period! Time to go to sleep for " + String(SNOOZE_TIME_MILSECS/1000) + " seconds");
//    Dash.snooze(SNOOZE_TIME_MILSECS);       // snooze time. used for testing only. Uncomment line below for prod

/* Production sleep timer */
    SerialUSB.println("PROGRAM: End of Wake Period! Time to go to sleep for " + String(SLEEP_TIME_MINS) + " mins");
    Dash.deepSleepMin(SLEEP_TIME_MINS);   // go to sleep for the designated time
   
  }

/* This is the simple serial write code */
/* It is used to send debug messages    */
/* back and forth between serial monitor*/
/* and the cloud service                */

  /* the code here will pass data between Cloud<-->UART */

  while (SerialUSB.available()) {
    SerialCloud.write(SerialUSB.read());
  }

//  while (Serial2.available()) {
//    SerialCloud.write(Serial2.read());
//  }

  while (SerialCloud.available()) {
    currChar = (char)SerialCloud.read();
    SerialUSB.write(currChar);
//    Serial2.write(currChar);
  }

 // delay(5);

  
} // end of LOOP routine
