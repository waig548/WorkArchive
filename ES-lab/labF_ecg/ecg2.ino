#include <SD.h>
#include <U8g2lib.h>

#define LO_POS 8
#define LO_NEG 9
#define OUTPIN A0

int threshold = 450;
float freq = 60.0;
bool rec = false;
byte peak = false;
long last = 0, last_peak = -1;
unsigned short last_beats[10] = {0};
byte idx = 0;

File rawOutput, RROutput;
U8G2_SSD1306_128X32_UNIVISION_1_HW_I2C u8g2(U8G2_R0); 

void setup() {
  // put your setup code here, to run once:
  pinMode(LO_POS, INPUT);
  pinMode(LO_NEG, INPUT);

  Serial.begin(9600);
  while(!Serial);

  if(!SD.begin(4))
  {
    Serial.println("init_fail");
    while(1);
  }
  Serial.println("init_done");
  
  //rawOutput = SD.open("ECG.txt", FILE_WRITE);

  u8g2.begin();
  printIdle();
  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available())
  {
    char K = Serial.read();
    switch(K)
    {
      case 'F':
      case 'f':
        freq = Serial.parseFloat();
        Serial.println(freq);
        break;
      case 'B':
      case 'b':
        trig_start_rec();
        break;
      case 'E':
      case 'e':
        trig_end_rec();
        break;
      default:
        break;
    }
  }
  if(rec)
    trig_rec_loop();
}

void trig_start_rec()
{
  if(rec)
    return;
  //String outName = Serial.readStringUntil('\n');
  if(SD.exists("ECG_raw.txt"))
    SD.remove("ECG_raw.txt");
  rawOutput = SD.open("ECG_raw.txt", FILE_WRITE);
  rawOutput.print("Sample Frequency: ");
  rawOutput.println(freq);
  
  if(SD.exists("ECG_RR.txt"))
    SD.remove("ECG_RR.txt");
  RROutput = SD.open("ECG_RR.txt", FILE_WRITE);
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
  rawOutput.close();
  RROutput.close();
  rec = false;
  peak = false;
  last = 0, last_peak = -1;
  for (int i = 0; i < 10; i++)
    last_beats[i] = 0;
  idx = 0;
  printIdle();
}

void trig_rec_loop()
{
  if(!rec)
    return;
  long cur = millis();
  if((cur - last) >= (int)(1000.0/freq))
  { 
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
  if(digitalRead(LO_POS)||digitalRead(LO_NEG))
  {
    //rawOutput.println("NIL");
    Serial.println("NIL");
    return;
  }
  int value = analogRead(OUTPIN);
  long cur = millis();
  last = cur;
  rawOutput.print(cur);
  rawOutput.print(", ");
  rawOutput.println(value);
  rawOutput.flush();
  //Serial.print(cur);
  //Serial.print(", ");
  Serial.println(value);
  if(value >= threshold && !peak)
    peak = true;
  else if (value < threshold && peak)
    trig_peak(cur);
}

void trig_peak(long cur)
{
  peak = false;
  if(!(~last_peak))
  {
    last_peak = cur;
    return;
  }
  unsigned short interval = cur - last_peak;
  last_peak = cur;
  last_beats[idx] = interval;
  idx++;
  idx%=10;
  RROutput.print(cur);
  RROutput.print(", ");
  RROutput.println(interval);
  RROutput.flush();
  int sum = 0;
  for (int i = 0; i < 10; i++)
    sum += last_beats[i];
  printBPM(60000.0/sum*10);
}

void printIdle()
{

  u8g2.firstPage();
  do {
    /* all graphics commands have to appear within the loop body. */    
    u8g2.setFont(u8g2_font_logisoso28_tr);
    u8g2.drawStr(8,29,"IDLE");
  } while ( u8g2.nextPage() );
  
  //u8g2.clearBuffer();
  //u8g2.clear();
  //u8g2.setFont(u8g2_font_logisoso28_tr);
  
  //u8g2.sendBuffer();
}

void printBPM(double BPM)
{
  u8g2.firstPage();
  do {
    /* all graphics commands have to appear within the loop body. */    
    u8g2.setFont(u8g2_font_logisoso28_tr);
    u8g2.setCursor(8, 29);
    u8g2.print(BPM);
  } while ( u8g2.nextPage() );

  /*
  u8g2.clearBuffer();
  u8g2.clear();
  u8g2.setFont(u8g2_font_logisoso28_tr);
  u8g2.setCursor(8, 29);
  u8g2.print(BPM);
  u8g2.sendBuffer();
  */
}
