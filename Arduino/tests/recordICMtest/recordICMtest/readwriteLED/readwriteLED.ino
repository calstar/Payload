// Variable to keep track of whether IRIS should be recording.
int rec_var = 0;

// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int led = 13;

void setup() {
  // put your setup code here, to run once:
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);     
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:

  // Serial.available gets the number of bytes (characters) available for reading from the serial port.
  // If there is readable data inputted into the serial port:
  if (Serial.available() > 0) {
  
      // Reads incoming serial data. Sets rec_var to "on" if 'i' is inputted into serial monitor.
      if (Serial.read() == 'i') {
        rec_var = 1;
        Serial.print("Now recording.");       // status message
        Serial.print('\n');                   // starts a new line
      }
      // Reads incoming serial data. Sets rec_var to "off" if 'o' is inputted into serial monitor.
      else if (Serial.read() == 'o') {
        rec_var = 0;
        Serial.print("Recording stopped.");   // status message
        Serial.print('\n');                   // starts a new line.
      }
      else {
      }
  }
  else {
  }

  // Blinks the LED on and off repeatedly if rec_var is set to "on".
  if (rec_var == 1) {
    digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
//    Serial.println("LED ON");
    delay(100);                // wait for 100ms
    digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
//    Serial.println("LED OFF");
    Serial.println("IRIS is recording.");
    Serial.print('\n');
    delay(1000);               // wait for 100ms
  }
  // Does nothing or stops blinking if rec_var is set to "off".
  else if (rec_var == 0) {
    Serial.println("IRIS is not recording.");
    Serial.print('\n');
    delay(1000);
  }
  
}
