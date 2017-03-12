
#define DO_BITWISE false

#include <NewPing.h>

#define LED_PIN D01


bool led_on;
int counter;

void setup() {

  
  // put your setup code here, to run once:
  // Start the Serial Outputer
  Serial.begin(9600); 
  Dash.begin();
  pinMode(LED_PIN, OUTPUT); //trigger pin

  led_on = true;
  counter = 1;

}

void loop() {
  // put your main code here, to run repeatedly:

// check the led_flag.  If true then light is already on and need to be
// turned off
  if (led_on)
  { 
      digitalWrite(LED_PIN, LOW);
      Dash.offLED();
      Serial.println("Turning LED off! Counter: " + String(counter));
      led_on = false; //reset the flag for the next go around
  }
  else // led is not on so needs to turned on
  {
    digitalWrite(LED_PIN, HIGH);
    Dash.onLED();
    Serial.println("Turning LED on! Counter: " + String(counter));
    led_on = true;
  }
    
 
  delay(3000); // delay 3 seconds and do it again
  counter = counter + 1;
  
}
