void setup() {
  // put your setup code here, to run once:
  pinMode(10, INPUT);
  pinMode(11, INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(digitalRead(10)||digitalRead(11))
    Serial.println("NIL");
  else
  {
    int value = analogRead(A0);
    Serial.println(value);
  }
  delay(1000/300);
}
