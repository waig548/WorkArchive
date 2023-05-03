//#include <SoftwareSerial.h>
#include <Wire.h>

int state = 0;
//SoftwareSerial tty(10, 11); //RX, TX

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin(8);
  Wire.onReceive(trigger);
  Wire.onRequest(requestOutput);
  //tty.begin(9600);
  //tty.println("Begin listening...");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);
  //if(tty.available())
  //trigger(tty.read());
}

void trigger(int amount)
{
  int data = 0;
  if (Wire.available())
    data = Wire.read();
  Serial.println(data);
  switch (data)
  {
    case 'A':
      if (!state || state == 1)
        state++;
      else if (state == 3)
        state = 1;
      break;
    case 'B':
      if (state == 2)
        state--;
      else if (state == 3)
        state++;
      break;
    case 'C':
      if (state == 2)
        state <<= 1;
      break;
    case 'D':
      if (state == 4)
        state--;
      break;
    case 'R':
      state = 0;
      break;
    default:
      break;
  }
  Serial.println(state);
  //tty.print("Current state: S");
  //tty.println(state);
}

void requestOutput()
{
  Wire.write(state);
}
