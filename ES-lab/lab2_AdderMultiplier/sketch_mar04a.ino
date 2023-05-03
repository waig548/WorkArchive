int D1P[3] = {10, 11, 12};
int D2P[3] = {7, 8, 9};
int SLP = 13;
int O1P[6] = {1, 2, 3, 4, 5, 6};
int O2P[7] = {0, 1, 2, 3, 4, 5, 6};
int O3P[4] = {3, 4, 5, 6};
int SWPP[2] = {1, 2};

int in1[3] = {0};
int in2[3] = {0};
int output[6] = {0};

int converted[8] = {0};

int swp_state[2] = {HIGH, LOW};
int high = 0;

int seg = 1;
int swap = 1;
int mult = 0;

static int bit_to_BCD[8][9] = {
  {HIGH, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW},
  {LOW, HIGH, LOW, LOW, LOW, LOW, LOW, LOW, LOW},
  {LOW, LOW, HIGH, LOW, LOW, LOW, LOW, LOW, LOW},
  {LOW, LOW, LOW, HIGH, LOW, LOW, LOW, LOW, LOW},
  {LOW, HIGH, HIGH, LOW, HIGH, LOW, LOW, LOW, LOW},
  {LOW, HIGH, LOW, LOW, HIGH, HIGH, LOW, LOW, LOW},
  {LOW, LOW, HIGH, LOW, LOW, HIGH, HIGH, LOW, LOW},
  {LOW, LOW, LOW, HIGH, LOW, LOW, LOW, HIGH, HIGH},
  };

void setup() {
  // put your setup code here, to run once:
  for (int i = 0; i < 3; i++)
  {
    pinMode(D1P[i], INPUT);
    pinMode(D2P[i], INPUT);
  }
  pinMode(SLP, INPUT);
  if (seg)
    if(swap)
    {
      for (int i = 0; i < 4; i++)
        pinMode(O3P[i], OUTPUT);
      pinMode(SWPP[0], OUTPUT);
      pinMode(SWPP[1], OUTPUT);
    }else
      for (int i = 0; i < 7; i++)
        pinMode(O2P[i], OUTPUT);
  else
    for (int i = 0; i < 6; i++)
      pinMode(O1P[i], OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i = 0; i < 3; i++)
  {
    in1[i] = digitalRead(D1P[i]);
    in2[i] = digitalRead(D2P[i]);
  }
  for (int i = 0; i < 6; i++)
    output[i] = 0;
  for (int i = 0; i < 8; i++)
    converted[i] = 0;
    
  mult = digitalRead(SLP);

  if (mult)
    mul(in1, in2, output, 3, 3);
  else
    add(in1, in2, output, 3);

  convert();
  
  if (seg)
    if (swap)
    {
      for (int i = 0; i < 4; i++)
        digitalWrite(O3P[i], converted[4*high+i]);
      for (int i = 0; i < 2; i++)
      {
        digitalWrite(SWPP[i], swp_state[i]);
        swp_state[i] ^= HIGH;
      }
      high ^= 1;
    }
    else
      for (int i = 0; i < 7; i++)
        digitalWrite(O2P[i], converted[i]);
  else
    for (int i = 0; i < 6; i++)
      digitalWrite(O1P[i], output[i]);

  delay(1);
}

void add(int *base, int *add_a, int len)
{
  add(base, add_a, base, len);
}

void add(int *add_a, int *add_b, int *out, int len) {
  int i = 0;
  int ci = 0, x = 0, y = 0;
  int C = 0, S = 0;
  for (int i = 0; i < len; i++)
  {
    x = add_a[i];
    y = add_b[i];
    
    S = ci^x^y;
    C = (ci&x)|(x&y)|(ci&y);

    out[i] = S;
    ci = C;
  }
  out[len] = C;
}

void mul(int *base, int *mul, int *out, int len_base, int len_mul) {
  for (int i = 0; i < len_mul; i++)
  {
    int tmp[3] = {base[0] & mul[i], base[1] & mul[i], base[2] & mul[i]};
    add(out+i, tmp, out+i, len_base);
  }
}

void convert(){
  for (int i = 0; i < 6; i++)
  {
    if (output[i])
    {
      add(converted, bit_to_BCD[i], 6);
      if (converted[3] && (converted[1] || converted[2]))
      {
        int tmp[6] = {LOW, HIGH, HIGH, LOW, LOW, LOW};
        add(converted, tmp, 6);
      }
    } 
  }
}
