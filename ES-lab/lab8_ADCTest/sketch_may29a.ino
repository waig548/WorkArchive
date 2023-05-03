#include <MCP3XXX.h>

#define CS_PIN 12
#define CLOCK_PIN 9
#define MOSI_PIN 11
#define MISO_PIN 10
 
MCP3008 adc;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  adc.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  int value = adc.analogRead(0);
  Serial.println(value);
  delayMicroseconds(10);
}
