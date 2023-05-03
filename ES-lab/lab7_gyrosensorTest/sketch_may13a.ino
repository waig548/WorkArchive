#include <Wire.h>
#include <MPU6050.h>

int ax, ay, az, gx, gy, gz;
MPU6050 accelgyro;

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(9600);
  accelgyro.initialize();
  Serial.print("Connection: ");
  Serial.println(accelgyro.testConnection()? "c":"d");
}

void loop() {
  // put your main code here, to run repeatedly:
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  Serial.print("a/g:\t");
  Serial.print(ax); Serial.print("\t");
  Serial.print(ay); Serial.print("\t");
  Serial.print(az); Serial.print("\t");
  Serial.print(gx); Serial.print("\t");
  Serial.print(gy); Serial.print("\t");
  Serial.println(gz);
  delay(100);
}
