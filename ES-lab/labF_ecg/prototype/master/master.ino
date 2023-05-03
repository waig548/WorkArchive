#include <Wire.h>
#include <SD.h>

#define LO_POS 11
#define LO_NEG 12
#define OUTPIN A0

int threshold = 512;
float freq = 32.0;
bool rec = false;
long last = 0;

File outFile;

void setup() {
  // put your setup code here, to run once:
  pinMode(LO_POS, INPUT);
  pinMode(LO_NEG, INPUT);

  Wire.begin();
  Serial.begin(9600);
  while(!Serial);

  if(!SD.begin(4))
  {
    Serial.println("init_fail");
    while(1);
  }
  Serial.println("init_done");
  
  //outFile = SD.open("ECG.txt", FILE_WRITE);


  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available())
  {
    if(Serial.find("freq"))
      freq = Serial.parseFloat();
    else if(Serial.find("start"))
      trig_start_rec();
    else if(Serial.find("stop"))
      trig_end_rec();
    else
      Serial.read();
  }
  if(rec)
    trig_rec_loop();
}

void trig_start_rec()
{
  if(rec)
    return;
  //String outName = Serial.readStringUntil('\n');
  if(SD.exists("ECG.csv"))
    SD.remove("ECG.csv");
  outFile = SD.open("ECG.csv");
  rec = true;
  delay(1000);
  last = millis();
  
}

void trig_end_rec()
{
  if(!rec)
  {
    Serial.println("Not recording.");
    return;
  }
  outFile.close();
  rec = false;
}

void trig_rec_loop()
{
  if(!rec)
    return;
  long cur = millis();
  if((cur - last) >= (int)(1000/freq))
  { 
    last = cur;
    _rec_loop();
  }
}

void trig_disp_help()
{
  if(rec)
    return;
  if(false);
}

void _rec_loop()
{
  if(digitalRead(10)||digitalRead(11))
  {
    outFile.println("NIL");
    Serial.println("NIL");
  }
  int value = analogRead(OUTPIN);
  long cur = millis();
  outFile.printf("%d, %d\n", cur, value);
  Serial.printf("%d, %d\n", cur, value);
  if(value >= threshold)
  {
    Wire.beginTransmission(8);
    Wire.
  }
}

void _disp_help()
{
  
}
