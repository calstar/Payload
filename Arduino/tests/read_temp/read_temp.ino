void setup()
{                
  Serial.begin(38400);
  analogReadResolution(13);
}

int rawAnalog;
double tAmbient;
double tAmbientF;
const double resolution = 0.25193548; //in mV, 3.3V / 2^13-1 or actually just a magic number
const double vOffset = 500; //in mV, from datasheet
const int tempCoeff = 10;
double vOut;
bool verbose = false;

void loop()
{
  rawAnalog = analogRead(31);
  if (verbose) {
    Serial.print("Raw is: ");
    Serial.println(rawAnalog);
  }
  //calculate analog voltage
  vOut = resolution*rawAnalog;
  if (verbose) {
    Serial.print("Voltage is: ");
    Serial.println(vOut);
  }
  
  //calculate ambient temperature;
  tAmbient = (vOut - vOffset) / tempCoeff;
  if (verbose) {
    Serial.print("Temperature is: ");
    Serial.print(tAmbient);
    Serial.print("C ");
  }

  tAmbientF = ((9.0/5)*tAmbient) + 32;
  Serial.print(tAmbientF);
  if (verbose) {
    Serial.println("F");
  } else {
    Serial.println();
  }
  delay(250);
}
