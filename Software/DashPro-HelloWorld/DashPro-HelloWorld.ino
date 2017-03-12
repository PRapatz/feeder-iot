

int numSends; // counter to count number of sends to Cloud

void setup() {
  // put your setup code here, to run once:
  SerialCloud.begin(115200);
  SerialUSB.begin(9600);
  SerialUSB.println("Hello Cloud example has started...");
  numSends = 0; // count number of sends
}

void loop() {
  // put your main code here, to run repeatedly:
  // every 60 seconds, send a message to the Cloud
  if((numSends < 6) && (millis() % 60000 == 0)) {
    SerialUSB.println("Sending a new message to the Cloud...");
    SerialCloud.println("Hello, Cloud! Phil Sending a message!"); // send to Cloud
    SerialUSB.println("Message sent!");
    numSends++; // increase the number-of-sends counter
  }

  // two-way serial passthrough for seeing debug statements
  while(SerialUSB.available()) {
    SerialCloud.write(SerialUSB.read());
  }

  while(SerialCloud.available()) {
    SerialUSB.write((char)SerialCloud.read());
  }
}
