/* Hologram Dash Recharge Sketch          */
/* Don't connect to the cloud, just snooze for 10 mins */
/* wake up, report the battery and go back to sleep */

int writerCounter = 0;
float distanceMeasurement = 10;

void setup() {

  /* Serial Setup */
  SerialUSB.begin(9600); /* USB UART */
  Serial2.begin(9600); /* TTL UART */
  delay(5000); /* Delay 4 seconds to wait for Usb Serial to Init.*/

  /* Setup Konekt Dash */
  Dash.begin();
  Dash.pulseLED(100,15000); /* Set the User Led to flash every 15 seconds */

  /* Serial Print Info */
  SerialUSB.println("Hologram Dash Battery Recharging Started!");
  SerialUSB.print("Using Boot Version: ");
  SerialUSB.println(Dash.bootVersion()); /* Print Dash Bootloader Version */
}

void loop() {
  char currChar;
  int batteryPercent;
  long batteryMilvolts;
  String messageString;
  String jsonString = "";
  
  writerCounter++;

  batteryPercent = Dash.batteryPercentage();
  batteryMilvolts = Dash.batteryMillivolts();

  jsonString = "{";
  jsonString.concat("\"message_nbr\":\"" + String(writerCounter) + "\",");
  jsonString.concat("\"distance\":\"" + String(distanceMeasurement) + "\",");
  jsonString.concat("\"battery_percentage\":\"" + String(batteryPercent) + "\",");
  jsonString.concat("\"battery_voltage\":\"" + String(batteryMilvolts) + "\"}");
                                   
  
  messageString = "**PROGRAM - Recharging!   "   "Bat %: " + String(batteryPercent) + "   Bat mV: " + String(batteryMilvolts);
  SerialUSB.println(messageString);
  SerialUSB.println(jsonString);

//      Dash.deepSleep();  // sleep for 30 mins
      Dash.snooze(30000);

}
