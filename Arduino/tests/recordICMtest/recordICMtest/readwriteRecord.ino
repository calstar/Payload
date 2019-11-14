// Variable to keep track of whether IRIS should be recording.
int rec_var = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:

  // Serial.available gets the number of bytes (characters) available for reading from the serial port.
  // The serial receive buffer can only store up to 64 bytes of data, so the start/stop commands are single characters.
  
  // If there is readable data inputted into the serial port:
  if (Serial.available() > 0) {
  
      // Reads incoming serial data. Sets rec_var to "on" if 'i' is inputted into serial monitor.
      if (Serial.read() == 'i') {
        rec_var = 1;
        Serial.print("Now recording.");       // status message
        Serial.print('\n');                   // starts a new line
      }
      // Reads incoming serial data. Sets rec_var to "off" if 'o' is inputted into serial monitor.
      // For some reason I can't debug, it'll only stop if you type more than one 'o' into the serial monitor.
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

  // Records data onto SD card if rec_var is set to "on".
  if (rec_var == 1) {
    Serial.print("IRIS is recording.");
    Serial.print('\n');
    delay(1000);
    if (myICM.dataReady()) {
      myICM.getAGMT();                // The values are only updated when you call 'getAGMT'
      printScaledAGMT(myICM.agmt);   // This function takes into account the scale settings from when the measurement was made to calculate the values with units
      delay(30);
    } else {
      Serial.println("Waiting for data");
      delay(500);
    }
  }
  // Does nothing or stops recording if rec_var is set to "off".
  else if (rec_var == 0) {
    Serial.print("IRIS is not recording.");
    Serial.print('\n');
    delay(1000);
  }
  
}
