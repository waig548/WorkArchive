#define BUFFER_SIZE 100

#include <SPI.h>
#include <U8g2lib.h>

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0); 
long T1 = 0;
//int last_observed = 0;
int observed_list[BUFFER_SIZE] = {0};
int tick = 0;
long last = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  u8g2.begin();
  delay(1000);
  last = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  long t = millis();
  if((t-last)>=10)
  {
    detect();
    //Serial.println(t);
    last = t;
  }
}

void detect()
{
  int observed = analogRead(A0);
  long T2 = millis();
  observed_list[tick%BUFFER_SIZE] = observed;
  tick++;
  double tmp = 0.0;
  for (int i = 0; i < BUFFER_SIZE; i++)
    tmp += pow(5.0*observed_list[i]/1023-0.5, 2);
  double rms = sqrt(tmp/BUFFER_SIZE);

  Serial.print(T1); Serial.print("\t");
  Serial.print(T2); Serial.print("\t");
  Serial.print(T2-T1); Serial.print("\t");
  //Serial.print(last_observed); Serial.print("\t");
  Serial.print(observed); Serial.print("\t");
  Serial.println(rms);

  //last_observed = observed;
  T1 = T2;

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso28_tr);
  u8g2.clear();
  //u8g2.drawStr(8,29,"MYBOTIC");
  u8g2.setCursor(8, 29);
  u8g2.print(rms);
  u8g2.sendBuffer();
}
