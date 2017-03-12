
const int MAX_SENDS = 5;
const int SEND_CYCLE = 30000; // milliseconds between serial writes
const int SLEEP_CYCLE_MINS = 1; // Sleep time in minutes

bool wakeFlag;
int numSends;
void setup() {
  // put your setup code here, to run once:
  Dash.begin();
  SerialUSB.begin(9600);
  wakeFlag = true;
  numSends = 0;
}

void loop() {
  // put your main code here, to run repeatedly:

  if (wakeFlag) {
    Dash.onLED();
    if ((numSends < MAX_SENDS) && (millis() % 30000 == 0)) {
        numSends = numSends + 1;
        SerialUSB.println("This a message in the wake cycle #" + String(numSends));
    }
  
  }

  if (numSends == MAX_SENDS) {
    numSends = 0;
    Dash.offLED();
    Dash.deepSleepMin(SLEEP_CYCLE_MINS);
  } 
}


