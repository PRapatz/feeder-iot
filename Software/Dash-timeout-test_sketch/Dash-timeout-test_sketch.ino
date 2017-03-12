/* Hologram Dash Hello World
*
* Author: Patrick F. Wilbur <hello@hologram.io> <pdub@pdub.net>
*
* Purpose: This program demonstrates interactive serial mode,
* a mechanism for performing cable replacement serial passthrough
* over cellular to the cloud. This example works out-of-the-box
* with zero configuration.
*
* License: Copyright (c) 2015 Konekt, Inc. All Rights Reserved.
*
* Released under the MIT License (MIT)
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*
*/

//const long WAKE_TIME =   600000;
//const long RESYNC_TIME = 180000;
//const long WRITE_TIME = 30000;
//const int SLEEP_TIME_MINS = 10;

int writeCounter = 0;
long wakeTimer = 0;
long writeTimer = 0;
long resyncTimer = 0;
bool wakeFlag = false;
bool resyncFlag = false;

void setup() {

  wakeFlag = false;
  resyncFlag = false;
  wakeTimer = millis();
  writeTimer = wakeTimer;
  resyncTimer = wakeTimer;
  
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
  SerialCloud.println("Hello, World!"); /* one-time message */
}

void loop() {
  char currChar;
  int batteryPercent;
  long batteryMilvolts;
  String messageString;
  
  /* Set up a communication loop that sends a message every 60 seconds for 5 mins */
  /* and then goes to sleep for a period of time                                  */ 

  /* check the wakeFlag.  It should only be false if going to sleep.  Otherwise it should be true */
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

  if (!resyncFlag & millis() - resyncTimer >= 120000) {
    resyncFlag = true; // resync is complete
    SerialUSB.println("PROGRAM: End of the resync period, ready to transmit!");

    wakeTimer = millis();  // ready to transmit, start calculating write period and wake period
    writeTimer = wakeTimer;
  }

  if (resyncFlag) {
    
    if (millis() - writeTimer >= 30000) {  // 30 seconds between writing to the cloud
      // check the battery charge
      batteryPercent = Dash.batteryPercentage();
      batteryMilvolts = Dash.batteryMillivolts();
      
      writeCounter = writeCounter + 1;
      messageString = "**PROGRAM - Cloud Message: #" + String(writeCounter) + "   Bat %: " + String(batteryPercent) + "   Bat mV: " + String(batteryMilvolts);
      SerialCloud.println(messageString);
      SerialUSB.println(messageString);
      writeTimer = millis(); /* reset the writeTimer so that it delays for the right amount of time */
    }
  
    /* check to see if we should be awake.  If the total time awake is greater than the wakeTime then   */
    /* its time to go to sleep.  Set the flag to false and go to sleep for the specified period of time.*/
  
    if (millis() - wakeTimer > 180000) {
      SerialUSB.println("PROGRAM: End of Wake Period! Time to go to sleep for 5 mins");
      wakeFlag = false;
      resyncFlag = false;
  
      Dash.deepSleepMin(5);   // deepSleep for 5 mins
 //     Dash.snooze(300000);  // snooze for 5 mins
      
    }
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
}
