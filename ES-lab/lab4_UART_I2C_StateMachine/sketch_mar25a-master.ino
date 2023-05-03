#include <SoftwareSerial.h>
#include <Wire.h>

//int state = 0;
SoftwareSerial tty(10, 11); //RX, TX

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(9600);
  while (!Serial)
  {
    
  }

  Serial.println("Begin listening...");

  tty.begin(9600);
  tty.println("Begin listening...");
}

void loop() {
  // put your main code here, to run repeatedly:
  if(tty.available())
    trigger(tty.read());
}

void trigger(int data)
{
  Serial.println(data);
  Wire.beginTransmission(8);
  Wire.write(data);
  Wire.endTransmission();

  Wire.requestFrom(8, 1);
  int state = -1;
  if (Wire.available())
    state = Wire.read();
  tty.print("Current state: S");
  tty.println(state);
}

void output(int amount)
{
  int state = -1;
  if (Wire.available())
    state = Wire.read();
  tty.print("Current state: S");
  tty.println(state);
}
