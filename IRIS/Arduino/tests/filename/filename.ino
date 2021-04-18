int filecounter = 1;
String filename = "FILE" + String(filecounter) +".txt";

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

  // if the filename already exists
  while (SD.exists(filename)) {
    filecounter++;    // increment filecount by one
    filename = "FILE" + String(filecounter) + ".txt";
  }

   myFile = SD.open(filename, FILE_WRITE);
    
}
