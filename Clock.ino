// Display
#include "GyverTM1637.h"
#define CLK 2
#define DIO 3
GyverTM1637 display(CLK, DIO);

// Clock
// SDA -> A4
// SCL -> A5
#include "microDS3231.h"
MicroDS3231 clock;
DateTime now;

// Encoder
#include "EncButton.h"
#define S1 6
#define S2 7
#define KEY 8
EncButton<EB_TICK, S1, S2, KEY> encoder;

// SD
#include "SD.h"
#include "SPI.h"
#define CS 4

// Music
#include "TMRpcm.h"
#define SPK 9
#define SPK_ON 10
TMRpcm music;

void setup() 
{
  Serial.begin(9600);
  
  // Clock initialization  
  clock.begin();
  now = clock.getTime();
  clock.setTime(now);

  // Display initialization
  display.clear();
  display.brightness(1);

  // SD initialization
  if (!SD.begin(CS)) 
  {
    Serial.println("SD fail");
  }
  else
  {   
    Serial.println("SD ok");   
  }

  // Music initialization
  pinMode(SPK_ON, OUTPUT);
  digitalWrite(SPK_ON, 0);
  music.speakerPin = SPK;    
  music.setVolume(10);
  music.quality(1);
  // music.play("test");
  // Serial.println(music.isPlaying());
}

void loop()
{
  ShowClock();
  encoder.tick();
  if (encoder.hold())
  {
    Settings();
  }
  if (encoder.click())
  {
    digitalWrite(SPK_ON, 1);
    music.play("test.wav");
  }
  if (encoder.right() || encoder.left())
  {
    digitalWrite(SPK_ON, 0);
    music.stopPlayback();
  }
}

void ShowClock()
{
  display.point(1);
  display.displayClock(clock.getHours(), clock.getMinutes());
}

void Settings()
{
  display.point(0); 
  display.clear();
  byte runningString[] = {_C, _l, _o, _c, _H};
  display.runningString(runningString, sizeof(runningString), 200);
  display.point(1);
  bool loop = 1;
  while (loop)
  {
    encoder.tick();
    if (encoder.click())
    {
      break;
    }
    if (encoder.right())
    {
      now.hour++;   
    }
    if (encoder.left())
    {
      now.hour--;   
    }
    display.displayClock(now.hour, now.minute);
  }  
  loop = 1;
  while (loop)
  {
    encoder.tick();
    if (encoder.click())
    {
      break;
    }
    if (encoder.right())
    {
      now.minute++;   
    }
    if (encoder.left())
    {
      now.minute--;   
    }
    display.displayClock(now.hour, now.minute);
  } 
  display.point(0); 

  runningString[0] = _d;
  runningString[1] = _A;
  runningString[2] = _t;
  runningString[3] = _E;
  runningString[4] = _empty;
  display.runningString(runningString, sizeof(runningString), 200);
  loop = 1;
  while (loop)
  {
    encoder.tick();
    if (encoder.click())
    {
      break;
    }
    if (encoder.right())
    {
      now.date++;   
    }
    if (encoder.left())
    {
      now.date--;   
    }
    display.displayClock(now.date, now.month);
  } 
  loop = 1;
  while (loop)
  {
    encoder.tick();
    if (encoder.click())
    {
      break;
    }
    if (encoder.right())
    {
      now.month++;  
    }
    if (encoder.left())
    {
      now.month--;   
    }
    display.displayClock(now.date, now.month);
  } 
  loop = 1;
  while (loop)
  {
    encoder.tick();
    if (encoder.click())
    {
      break;
    }
    if (encoder.right())
    {
      now.year++;   
    }
    if (encoder.left())
    {
      now.year--;  
    }
    display.displayInt(now.year);
  } 
  loop = 1;
  while (loop)
  {
    encoder.tick();
    if (encoder.click())
    {
      break;
    }
    if (encoder.right())
    {
      now.day++; 
    }
    if (encoder.left())
    {
      now.day--;  
    }
    switch (now.day)
    {
      case 1:
        display.displayByte(0x70, 0x76, _o, _n);
        break;
      case 2:
        display.displayByte(_t, _u, _E, _empty);
        break;
      case 3:
        display.displayByte(_L, _U, _E, _d);
        break;
      case 4:
        display.displayByte(_t, _h, _u, _empty);
        break;
      case 5:
        display.displayByte(_F, _r, _i, _empty);
        break;
      case 6:
        display.displayByte(_S, _A, _t, _empty);
        break;
      case 7:
        display.displayByte(_S, _u, _n, _empty);
        break;      
    }
  }
  clock.setTime(now);  

  runningString[0] = _b;
  runningString[1] = _r;
  runningString[2] = _i;
  runningString[3] = _6;
  runningString[4] = _H;
  runningString[5] = _t;
  display.runningString(runningString, sizeof(runningString), 200);
  loop = 1;
  int brightness = 1;
  while (loop)
  {
    encoder.tick();
    if (encoder.click())
    {
      break;
    }
    if (encoder.right())
    {
      brightness++;   
    }
    if (encoder.left())
    {
      brightness--;   
    }
    if (brightness < 0) brightness = 0;
    if (brightness > 7) brightness = 7;
    display.displayInt(brightness);
    display.brightness(brightness);
  }  
}