#include "Display.h"
#include <IRremote.h>
#include "Remote.h"
#include "Timer.h"

Timer timers;
#define IR_PIN 10

RemoteControl::RemoteControl() : bright(50), remoteStatus(false), enteredValue(0), minu(0), fIndex(0), tbIndex(0), timerInputMode(false) {}

void RemoteControl::setupRemote()
{
    //timers.setupRTC();
    IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);
}

void RemoteControl::displayDefaultMessage()
{
    Display::getInstance().clearBuffer(true);
    Display::getInstance().displayText("LED STRIPS", "SIGNBOARD", "static", "no");
    Display::getInstance().updateLEDs();
}

void RemoteControl::adjustBrightness(float change)
{
  if ((change > 0 && bright < 255) || (change < 0 && bright > 25.5))
  {
    bright += change;
    Serial.println(bright);
    Display::getInstance().clearBuffer(true);
    if(bright == 255){
      Display::getInstance().displayText("BrMax", "", "static", "yes");
    }
    else if(bright == 25.5)
    {
      Display::getInstance().displayText("BrMin", "", "static", "yes");
    }
    else
    {
      Display::getInstance().displayText(change > 0 ? "BrUp" : "BrDwn", "", "static", "yes");
    }
    
    Display::getInstance().updateLEDs();

    for (int x = 0; x < NUM_STRIPS; x++)
    {
        Display::getInstance().setBrightness(bright);
        
    }
  }
}

void RemoteControl::toggleRemote(String remoteCode)
{
  if (remoteCode == "fd02")
  {
    remoteStatus = !remoteStatus;
    Display::getInstance().clearBuffer(true);
    Display::getInstance().displayText(remoteStatus ? "RMON" : "RMOFF", "", "static", "yes");
    Display::getInstance().updateLEDs();
    Timer::getInstance().displayTimeOfDay(false);
    delay(500);
  }
}

void RemoteControl::handleTimerCodes(String remoteCode)
{
  char text[8];
  if(Timer::getInstance().getTimerRunning() == false)
  {
    for (uint8_t i = 0; i < 10; i++)
    {
        if (remoteCode == timerCodes[i])
        {
            sprintf(text, "%d+:%02d", i+1, 0);
            Display::getInstance().displayText(text, "", "static", "yes");
            minu = i+1;
            Timer::getInstance().displayTimeOfDay(false);
            break;
        }
    }
  }

  if (remoteCode == "9768" && Timer::getInstance().getTimerRunning() == false)
  {
    Serial.println(minu);
    Timer::getInstance().startTimer(minu, 0);
  }
  else if(remoteCode == "b946" && Timer::getInstance().getTimerRunning() == true)
  {
    Timer::getInstance().stopTimer();
    minu = 0;
  }
  else if(remoteCode == "b847" && Timer::getInstance().getTimerPaused() == true)
  {
    Timer::getInstance().resumeTimer();
  }
  else if(remoteCode == "b54a" && Timer::getInstance().getTimerPaused() == false)
  {
    Timer::getInstance().pauseTimer();
  }
  else if (remoteCode == "9768" && Timer::getInstance().getTimerRunning() == true) {
    Timer::getInstance().resetTimer();
  }
}

void RemoteControl::setDefaultMessage(String remoteCode)
{
  if(remoteCode == "8679")
  {
    displayDefaultMessage();
    Timer::getInstance().displayTimeOfDay(false);
  }
}

void RemoteControl::useRemote()
{
  if (IrReceiver.decode())
  {
      uint32_t remoteCode = IrReceiver.decodedIRData.decodedRawData;
      Serial.print("Raw Hex Code: ");
      Serial.println(remoteCode, HEX);

      String remoteValue = String(IrReceiver.decodedIRData.decodedRawData, HEX).substring(0, 4);

      toggleRemote(remoteValue);

      if (remoteStatus)
      {
        setDefaultMessage(remoteValue);
          
        handleTimerCodes(remoteValue);

        if(remoteValue == "9f60")
        {
          tbIndex = (tbIndex + 1) %6;
          changeTBColourScheme();   
        }
        else if(remoteValue == "9e61")
        {
          tbIndex = tbIndex - 1;
          changeTBColourScheme();
        }
        else if(remoteValue == "9a65")
        {
          fIndex = fIndex - 1;
          changeFColourScheme();
        }
        else if(remoteValue == "9d62")
        {
          fIndex = fIndex + 1;
          changeFColourScheme();
        }

        if(tbIndex > 5)
        {
          tbIndex = 0;
          changeTBColourScheme();
        } 
        else if(tbIndex < 0)
        {
          tbIndex = 5;
          changeTBColourScheme();
        } 

        if(fIndex > 2) 
        {
          fIndex = 0;
          changeFColourScheme();
        }
        else if(fIndex < 0) 
        {
          fIndex = 2;
          changeFColourScheme();
        }

        if (remoteValue == "b649") Timer::getInstance().displayTimeOfDay(true);

        if (remoteValue == "f807") adjustBrightness(25.5);
        else if (remoteValue == "f40b") adjustBrightness(-25.5);

        if(remoteValue == "946b")
        {
          Display::getInstance().displayText("00+:00", "", "static", "yes");
          timerInputMode = true;
          Timer::getInstance().displayTimeOfDay(false);
          manualTimerInput(); // while loop
          delay(500);
        }
      }
        IrReceiver.resume();
    }  
}

uint8_t RemoteControl::getBrightness()
{
  return bright;
}


void RemoteControl::manualTimerInput()
{
  char num[11] = ""; // Supports up to 10 digits + null terminator
  String inputToSend = "";

  while(timerInputMode) 
  {
    IrReceiver.resume();

    if(IrReceiver.decode())
    {
      String remoteCode = String(IrReceiver.decodedIRData.decodedRawData, HEX).substring(0, 4);
      Serial.println("Decoded Raw Data: ");
      Serial.println(remoteCode);

      // Check for exit code
      if (remoteCode == "946b")
      {
        Serial.println("Exit Command Received.");
        timerInputMode = false;
        Timer::getInstance().parseTimerInput(inputToSend);
      }

      // Convert IR code to a digit
      int digit = getNumberFromIR(remoteCode);
      
      if (digit == -1)
      {
        Serial.println("Invalid IR Code. Ignored.");
      }
      else
      {
        size_t len = strlen(num);

        if (len < 4)
        {
          char digitChar = '0' + digit; // Convert to char
          num[len] = digitChar;
          num[len + 1] = '\0';
          Serial.print("Current Input: ");
          Serial.println(num);
          if(len < 2)
          {
            Display::getInstance().clearBuffer(true);
            char text[5];
            sprintf(text, "%s+:00", num);
            Serial.println(num[0]);
            Serial.println(text);
            Display::getInstance().displayText(text, "", "static", "yes");
            inputToSend = "[" + String(text) + "]";
          }
          else if(len == 2)
          {
            Serial.println(num[2]);
            char num1[3] = {num[0], num[1], '\0'};
            char num2[2] = {num[2], '\0'};
            Display::getInstance().clearBuffer(true);
            char text[5];
            sprintf(text, "%s+:0%s", num1, num2);
            Serial.println(text);
            Display::getInstance().displayText(text, "", "static", "yes");
            inputToSend = "[" + String(text) + "]";
          }
          else if(len == 3)
          {
            char num1[3] = {num[0], num[1], '\0'};
            char num2[3] = {num[2], num[3], '\0'};
            Display::getInstance().clearBuffer(true);
            char text[5];
            sprintf(text, "%s+:%s", num1, num2);
            Serial.println(text);
            Display::getInstance().displayText(text, "", "static", "yes");
            inputToSend = "[" + String(text) + "]";
          } 
        }
      }
    }
  }
}

int RemoteControl::getNumberFromIR(String command)
{
  for (uint8_t i = 0; i < 10; i++)
  {
    if (command == timerCodes[i])
    {
      if(i == 9)
      {
        return 0;
      }
      else
      {
        return i+1;
      }
    }
  }
  return -1; // Invalid input
}

void RemoteControl::changeTBColourScheme()
{
  switch(tbIndex)
  {
    case 0: Display::getInstance().setTopColour(0xff0000);
            Display::getInstance().setBottomColour(0x00ff00);
            Display::getInstance().displayText("Red", "Green", "static", "no");
            break;

    case 1: Display::getInstance().setTopColour(0xff0000);
            Display::getInstance().setBottomColour(0x0000ff);
            Display::getInstance().displayText("Red", "Blue", "static", "no");
            break;

    case 2: Display::getInstance().setTopColour(0x00ff00);
            Display::getInstance().setBottomColour(0xff0000);
            Display::getInstance().displayText("Green", "Red", "static", "no");
            break;

    case 3: Display::getInstance().setTopColour(0x00ff00);
            Display::getInstance().setBottomColour(0x0000ff);
            Display::getInstance().displayText("Green", "Blue", "static", "no");
            break;

    case 4: Display::getInstance().setTopColour(0x0000ff);
            Display::getInstance().setBottomColour(0xff0000);
            Display::getInstance().displayText("Blue", "Red", "static", "no");
            break;

    case 5: Display::getInstance().setTopColour(0x0000ff);
            Display::getInstance().setBottomColour(0x00ff00);
            Display::getInstance().displayText("Blue", "Green", "static", "no");
            break;
  }
}

void RemoteControl::changeFColourScheme()
{
  switch(fIndex)
  {
    case 0: Display::getInstance().setTopColour(0xff0000);
            Display::getInstance().setBottomColour(0xff0000);
            Display::getInstance().setFullColour(0xff0000);
            Display::getInstance().displayText("Red", "", "static", "yes");
            break;

    case 1: Display::getInstance().setTopColour(0x00ff00);
            Display::getInstance().setBottomColour(0x00ff00);
            Display::getInstance().setFullColour(0x00ff00);
            Display::getInstance().displayText("Green", "", "static", "yes");
            break;

    case 2: Display::getInstance().setTopColour(0x0000ff);
            Display::getInstance().setBottomColour(0x0000ff);
            Display::getInstance().setFullColour(0x0000ff);
            Display::getInstance().displayText("Blue", "", "static", "yes");
            break;
  }
}


