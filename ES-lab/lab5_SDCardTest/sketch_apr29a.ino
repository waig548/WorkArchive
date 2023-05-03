#include <SPI.h>
#include <SD.h>
//#include <Timezone.h>

File outFile;

int boot = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial)
  {
    ;
  }

  Serial.println("Initializing SD card...");

  if(!SD.begin(4))
  {
    Serial.println("Initialization failed");
    while(1);
  }
  Serial.println("Initialization done");

  outFile = SD.open("output.txt", FILE_WRITE);

  if(outFile)
  {
    Serial.println("Writing debug info...");
    Serial.println(millis());
    //outFile.close();
  }
  delay(1000);
  boot = millis();
}

void _log()
{
  float stamp = (millis()-boot)/1000.0;
  int value1 = analogRead(A0);
  int value2 = analogRead(A1);
  float voltage1 = value1 * (5.0/1023);
  float voltage2 = value2 * (5.0/1023);
  Serial.println(stamp);
  outFile.print(stamp);
  outFile.print(",");
  outFile.print(value1);
  outFile.print(",");
  outFile.print(value2);
  outFile.print(",");
  outFile.print(voltage1);
  outFile.print(",");
  outFile.println(voltage2);
  //outFile.flush();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available())
    if (Serial.read())
    {
      outFile.close();
      Serial.println("Session ended");
      while(1);
    }
  if (!((millis()-boot) % 20))
    _log();
}
