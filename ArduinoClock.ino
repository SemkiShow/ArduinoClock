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

long screenSaver;
int currentMode;

//EEPROM & alarm
#include <EEPROM.h>
char alarmTimes[10][10];
int lightTime;
long alarmCheck;
#define LIGHT 11
#define MOTOR 12
bool isLightOn, isMotorOn = false;

int currentMenu;

void setup() 
{
  Serial.begin(9600);

  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  
  // Clock initialization  
  clock.begin();
  clock.setTime(COMPILE_TIME);
  now = clock.getTime();

  // Display initialization
  display.clear();
  display.brightness(1);

  EEPROM.get(0, lightTime);
  EEPROM.get(sizeof(int), alarmTimes);

  screenSaver = millis();
  alarmCheck = millis();
  currentMode = 0;
  SwitchMode();
}

void loop()
{
  if (millis() - screenSaver > 500)
  {
    currentMode = NextMode();
    screenSaver = millis();
    display.clear();
  }
  if (millis() < screenSaver)
  {
    screenSaver = millis();
  }
  SwitchMode();

  if (millis() - alarmCheck > 60000)
  {
    for (int i = 0; i < 10; i++)
    {
      // Alarm times init
      int deltaAlarmHour;
      int deltaAlarmMinute;
      if (alarmTimes[i][2] - lightTime < 0)
      {
        if (alarmTimes[i][1] - clock.getHours() == 1 || (alarmTimes[i][1] == 0 && clock.getHours() == 23))
        {
          deltaAlarmHours = 0;
          deltaAlarmMinutes = 60 - alarmTimes[i][1];
        }
      }
      else
      {
        deltaAlarmHours = alarmTimes[i][1] - clock.getHours();
        deltaAlarmMinutes = alarmTimes[i][2] - clock.getMinutes();
      }

      // Light on check
      if (deltaAlarmHours == 0 && alarmTimes[i][0] == 1 && deltaAlarmMinutes < lightTime + 1 && alarmTimes[i][clock.getDay() + 2] == 1)
      {
        if (deltaAlarmMinutes < lightTime + 1)
        {
          analogWrite(LIGHT, 255 / (lightTime / 2 * 60));
        }

        if (deltaAlarmMinutes < lightTime / 2 + 1)
        {
          digitalWrite(LIGHT, HIGH);
        }

        isLightOn = true;
      }

      // Alarm (motor) on check
      if (deltaAlarmMinutes <= 0 && alarmTimes[i][clock.getDay() + 2] == 1)
      {
        digitalWrite(MOTOR, HIGH);

        isMotorOn = true;
      }
    }
  }

  encoder.tick();

  if (encoder.click() && !isLightOn && !isMotorOn)
  {
    Settings();
  }

  if (encoder.click() && (isMotorOn || isLightOn))
  {
    digitalWrite(MOTOR, LOW);
    digitalWrite(LIGHT, LOW);

    isMotorOn = false;
    isLightOn = false;
  }

  delay(1);
}

byte NumCharToByte(char input)
{
  switch (input)
  {
    case '1':
      return _1;
      break;
    case '2':
      return _2;
      break;
    case '3':
      return _3;
      break;
    case '4':
      return _4;
      break;
    case '5':
      return _5;
      break;
    case '6':
      return _6;
      break;
    case '7':
      return _7;
      break;
    case '8':
      return _8;
      break;
    case '9':
      return _9;
      break;
    case '0':
      return _0;
      break;
  }
}

void ShowClockOn()
{
  display.point(0);
  display.displayClock(clock.getHours(), clock.getMinutes());
}

void ShowClockOff()
{
  display.point(1);
  display.displayClock(clock.getHours(), clock.getMinutes());
}

void ShowDate()
{
  display.point(1);
  display.displayClock(clock.getDate(), clock.getMonth());
}

void ShowYear()
{
  display.point(0);
  display.displayInt(clock.getYear());
}

void ShowDay()
{
  display.point(0);
  switch (now.day)
    {
      case 1:
        display.displayByte(0x31, 0x37, _o, _n);
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

void ShowTemperature()
{
  display.point(0);
  String temperature = String(clock.getTemperature());
  display.displayByte(NumCharToByte(temperature[0]), NumCharToByte(temperature[1]), 0x63, _C);
}

void SwitchMode()
{
  if (currentMode < 20 && currentMode % 2 == 0)
  {
    ShowClockOn();
  }

  if (currentMode < 20 && currentMode % 2 == 1)
  {
    ShowClockOff();
  }
  
  if (currentMode >= 20 && currentMode < 30)
  {
    ShowDate();
  }

  /*if (currentMode >= 30 && currentMode < 40)
  {
    ShowYear();
  }

  if (currentMode >= 40 && currentMode < 50)
  {
    ShowDay();
  }
  
  if (currentMode >= 50 && currentMode < 60)
  {
    ShowTemperature();
  }*/

  if (currentMode >= 30 && currentMode < 40)
  {
    ShowDay();
  }
  
  if (currentMode >= 40 && currentMode < 50)
  {
    ShowTemperature();
  }
}

int NextMode()
{
  if (currentMode >= 50)
  {
    return 0;
  }
  return currentMode + 1;
}

void Settings()
{
  display.point(0);
  currentMenu = 0;
  display.clear();
  
  while (true)
  {
    encoder.tick();

    if (encoder.left())
    {
      currentMenu--;
      if (currentMenu < 0)
      {
        currentMenu = 0;
      }
      display.clear();
    }

    if (encoder.right())
    {
      currentMenu++;
      if (currentMenu >= 6)
      {
        currentMenu = 5;
      }
      display.clear();
    }

    if (currentMenu == 0)
    {
      byte clock[] = {_C, _l, _o, _c};
      display.displayByte(clock);
    }
    if (currentMenu == 1)
    {
      byte date[] = {_d, _A, _t, _e};
      display.displayByte(date);
    }
    if (currentMenu == 2)
    {
      byte year[] = {_Y, _e, _A, _r};
      display.displayByte(year);
    }
    if (currentMenu == 3)
    {
      byte day[] = {_d, _A, _y, _empty};
      display.displayByte(day);
    }
    if (currentMenu == 4)
    {
      byte brightness[] = {_b, _r, _i, _G};
      display.displayByte(brightness);
    }
    if (currentMenu == 5)
    {
      byte alarm[] = {_A, _l, _A, _r};
      display.displayByte(alarm);
    }

    if (encoder.click())
    {
      display.clear();
      switch (currentMenu)
      {
        case 0:
          Clock();
          break;
        case 1:
          Date();
          break;
        case 2:
          Year();
          break;
        case 3:
          Day();
          break;
        case 4:
          Brightness();
          break;
        case 5:
          Alarm();
          break;
      }
    }

    if (encoder.hold())
    {
      display.clear();
      break;
    }

    Serial.println(currentMenu);
  }
}

void Clock()
{
  display.point(1);

  while (true)
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
  
  while (true)
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
  
  clock.setTime(now);
  display.point(0);
  Settings();
}

void Date()
{
  display.point(1);

  while (true)
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
  while (true)
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

  clock.setTime(now);
  display.point(0);
  Settings();
}

void Year()
{
  while (true)
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

  clock.setTime(now);
  Settings();
}

void Day()
{
  while (true)
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
        display.displayByte(0x31, 0x37, _o, _n);
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
  Settings();
}

void Brightness()
{
  int brightness = 1;
  
  while (true)
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

  Settings();
}

void Alarm()
{
  display.point(0);
  currentMenu = 0;
  display.clear();
  
  while (true)
  {
    encoder.tick();

    if (encoder.left())
    {
      currentMenu--;
      if (currentMenu < 0)
      {
        currentMenu = 0;
      }
      display.clear();
    }

    if (encoder.right())
    {
      currentMenu++;
      if (currentMenu >= 11)
      {
        currentMenu = 10;
      }
      display.clear();
    }

    if (currentMenu == 0)
    {
      byte lightTime[] = {_L, _i, _G, _h};
      display.displayByte(lightTime);
    }
    if (currentMenu == 1)
    {
      byte alarm1[] = {_1, _empty, _A, _l};
      display.displayByte(alarm1);
    }
    if (currentMenu == 2)
    {
      byte alarm2[] = {_2, _empty, _A, _l};
      display.displayByte(alarm2);
    }
    if (currentMenu == 3)
    {
      byte alarm3[] = {_3, _empty, _A, _l};
      display.displayByte(alarm3);
    }
    if (currentMenu == 4)
    {
      byte alarm4[] = {_4, _empty, _A, _l};
      display.displayByte(alarm4);
    }
    if (currentMenu == 5)
    {
      byte alarm5[] = {_5, _empty, _A, _l};
      display.displayByte(alarm5);
    }
    if (currentMenu == 6)
    {
      byte alarm6[] = {_6, _empty, _A, _l};
      display.displayByte(alarm6);
    }
    if (currentMenu == 7)
    {
      byte alarm7[] = {_7, _empty, _A, _l};
      display.displayByte(alarm7);
    }
    if (currentMenu == 8)
    {
      byte alarm8[] = {_8, _empty, _A, _l};
      display.displayByte(alarm8);
    }
    if (currentMenu == 9)
    {
      byte alarm9[] = {_9, _empty, _A, _l};
      display.displayByte(alarm9);
    }
    if (currentMenu == 10)
    {
      byte alarm10[] = {_1, _0, _empty, _A};
      display.displayByte(alarm10);
    }

    if (encoder.click())
    {
      display.clear();
      switch (currentMenu)
      {
        case 0:
          LightTime();
          break;
        case 1:
          AlarmConfig(1);
          break;
        case 2:
          AlarmConfig(2);
          break;
        case 3:
          AlarmConfig(3);
          break;
        case 4:
          AlarmConfig(4);
          break;
        case 5:
          AlarmConfig(5);
          break;
        case 6:
          AlarmConfig(6);
          break;
        case 7:
          AlarmConfig(7);
          break;
        case 8:
          AlarmConfig(8);
          break;
        case 9:
          AlarmConfig(9);
          break;
        case 10:
          AlarmConfig(10);
          break;
      }
    }

    if (encoder.hold())
    {
      display.clear();
      break;
    }

    Serial.println(currentMenu);
  }

  EEPROM.put(0, lightTime);
  EEPROM.put(sizeof(int), alarmTimes);
  Settings();
}

void LightTime()
{
  while (true)
  {
    encoder.tick();
    
    if (encoder.click())
    {
      break;
    }
    
    if (encoder.right())
    {
      lightTime++;   
    }
    
    if (encoder.left())
    {
      lightTime--;  
    }

    if (lightTime < 0) {lightTime = 0;}
    
    display.displayInt(lightTime);
  } 

  Alarm();
}

void AlarmConfig(int alarmNumber)
{
  display.displayByte(_empty, _empty, _O, _n);
  alarmTimes[alarmNumber - 1][0] = 1;

  while (true)
  {
    encoder.tick();

    if (encoder.click())
    {
      break;
    }

    if (encoder.right())
    {
      display.displayByte(_empty, _empty, _O, _n);
      alarmTimes[alarmNumber - 1][0] = 1;
    }

    if (encoder.left())
    {
      display.displayByte(_empty, _O, _f, _f);
      alarmTimes[alarmNumber - 1][0] = 0;
    }
  }

  display.point(1);

  while (true)
  {
    encoder.tick();

    if (encoder.click())
    {
      break;
    }

    if (encoder.right())
    {
      alarmTimes[alarmNumber - 1][1]++;   
    }

    if (encoder.left())
    {
      alarmTimes[alarmNumber - 1][1]--;   
    }
    
    display.displayClock(alarmTimes[alarmNumber - 1][1], alarmTimes[alarmNumber - 1][2]);
  }  
  
  while (true)
  {
    encoder.tick();

    if (encoder.click())
    {
      break;
    }

    if (encoder.right())
    {
      alarmTimes[alarmNumber - 1][2]++;   
    }

    if (encoder.left())
    {
      alarmTimes[alarmNumber - 1][2]--;   
    }
    
    display.displayClock(alarmTimes[alarmNumber - 1][1], alarmTimes[alarmNumber - 1][2]);
  }  

  display.point(0);

  for (int i = 1; i <= 7; i++)
  {
    while (true)
    {
      encoder.tick();

      if (encoder.click())
      {
        break;
      }

      if (encoder.left())
      {
        alarmTimes[alarmNumber - 1][i + 2] = 0;
      }
      
      if (encoder.right())
      {
        alarmTimes[alarmNumber - 1][i + 2] = 1;
      }

      byte isOn;

      if (alarmTimes[alarmNumber - 1][i + 2] == 0) {isOn = _empty;};
      if (alarmTimes[alarmNumber - 1][i + 2] == 1) {isOn = _O;};

      display.displayByte(_empty, 0x39, isOn, 0x0f);
      display.display(0, i);
    }
  }

  Alarm();
}
