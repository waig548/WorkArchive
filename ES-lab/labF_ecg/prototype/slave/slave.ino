#include <Wire.h>

#include <SPI.h>
#include <U8g2lib.h>

U8G2_SSD1306_128X32_UNIVISION_F_4W_HW_SPI u8g2(U8G2_R0); 

void setup()
{
  Serial.begin(9600);

  
  u8g2.begin();
  delay(1000);
}

void loop()
{
  
}
