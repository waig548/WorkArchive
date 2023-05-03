int baseP[4] = {4, 5, 6, 7};
int swpP[3] = {8, 9, 10};
int PAUP = 2;
int RSTP = 3;

const int interval = 1;
static int BCD[10][4] = {
  {LOW, LOW, LOW, LOW},
  {HIGH, LOW, LOW, LOW},
  {LOW, HIGH, LOW, LOW},
  {HIGH, HIGH, LOW, LOW},
  {LOW, LOW, HIGH, LOW},
  {HIGH, LOW, HIGH, LOW},
  {LOW, HIGH, HIGH, LOW},
  {HIGH, HIGH, HIGH, LOW},
  {LOW, LOW, LOW, HIGH},
  {HIGH, LOW, LOW, HIGH}
};

int bits[12] = {0};
int swp = 0;

int timer = 0;
unsigned long counter = 0;
unsigned long prev = 0;

void setup() {
  for (int i = 4; i < 11; i++)
    pinMode(i, OUTPUT);
  pinMode(PAUP, INPUT_PULLUP);
  pinMode(RSTP, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(PAUP), pause, RISING);
  attachInterrupt(digitalPinToInterrupt(RSTP), reset, RISING);
}

void loop() {
  unsigned long cur = millis();
  unsigned long diff = cur - prev;
  if (prev)
    if (diff >= interval) 
      if (timer|1) 
        counter += diff;
  
  unsigned long tmp = counter / 100;
  for (int i = 0; i < 3; i++, tmp/=10)
    for (int j = 0; j < 4; j++)
      bits[4*i+j] = BCD[tmp%10][j];
    //memcpy(bits+4*i, BCD[tmp%10], sizeof(BCD[tmp%10][0])*4);

  output();
  prev = cur;
  delay(1);
}

void output() {
  for (int i = 0; i < 4; i++)
    digitalWrite(baseP[i], bits[4*swp+i]);
  for (int i = 0; i < 3; i++)
    digitalWrite(swpP[i], i == swp? HIGH: LOW);
  swp = (swp+1)%3;
}

void pause() {
  timer = !timer;
}

void reset() {
  timer = 0;
  counter = 0;
}
