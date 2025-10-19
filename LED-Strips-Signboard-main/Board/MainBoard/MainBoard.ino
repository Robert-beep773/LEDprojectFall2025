#include "Display.h"
#include "Timer.h"
#include <Wire.h>
#include <RTClib.h>
#include "Remote.h"

RTC_DS3231 rtc;// Global RTC instance
bool useBigFont = true;  // Toggle between 7x7 and 15x15 font sizes
Timer timer; 
RemoteControl remote; // Global remote variable

// Variables for parsing the received message
String command = "";
String isBig = "";
String message = "";
String message2 = "";
int messageSize = 0;

// Variable for receiving input directly
int intByte;
const uint8_t numRawChar = 90;
char msgRaw[numRawChar];
uint8_t charCount = 0;
bool dataToSend = false;

Display& display = Display::getInstance();
//Display d;

void setup()
{
  Serial.begin(9600);
  timer.setupRTC();
  remote.setupRemote();
  display.setup(7);

  display.displayText("LED STRIPS", "SIGNBOARD", "static", "no");

  // display.displayText("2+:00", "", "static", "yes");

  // parseInput("$custom$start[(0,0,#2309ec),(0,1,#2309ec),(0,2,#ffffff)]");
  // parseInput("$custom$no[(1,0,#2309ec),(2,0,#2309ec),(3,0,#ffffff)]");
  // parseInput("$custom$no[(5,0,#2309ec),(5,1,#2309ec),(5,2,#ffffff)]");

  // parseInput("$custom$start[(0,1,#2309ec),(0,2,#2309ec)");

  // parseInput("$sTimer$yes[10,00]");
  // parseInput("$pTimer$");
  // parseInput("$rTimer$");


}

void loop()
{
  if (Serial.available())
  {
    // Serial.begin(9600); Resetting the arduino with every message
    // display.setup(100);
    // Extracting size
    String input = Serial.readStringUntil('\n');  // Read until newline
    parseInput(input);
  }

  remote.useRemote();
  timer.updateTimer();
}

void parseInput(String input)
{
    Serial.print(input);
    timer.displayTimeOfDay(false); // In case the time of day is being displayed

  // Check if input starts with $
  if (input.charAt(0) != '$')
  {
      Serial.println("Invalid format: must start with $");
      return;
  }

  // Find all $ positions
  int firstDollar = input.indexOf('$');
  int secondDollar = input.indexOf('$', firstDollar + 1);

  messageSize = input.length();

  // Extract command
  command = input.substring(firstDollar + 1, secondDollar);

  // Using the command to determine how we will be parsing the input
  if (command == "custom")
  {
    int openBracket = input.indexOf('[');
    String chunckPos = input.substring(secondDollar + 1, openBracket);
    display.displayCustomPixels(input.c_str(), chunckPos.c_str());
  }
  else if (command == "settns")
  {
    updateSettings(input);
  }
  else if (command == "sTimer")
  {
    timer.parseTimerInput(input);
  }
  else if (command == "pTimer")
  {
    timer.pauseTimer();
  }
  else if (command == "rTimer")
  {
    timer.resetTimer();
  }
  else if (command == "resume")
  {
    timer.resumeTimer();
  }
  else if(command=="tod")
  {
    timer.displayTimeOfDay(true);
  }
  else
  {
    int openBracket = input.indexOf('[');
    // Extract display type
    isBig = input.substring(secondDollar + 1, openBracket);

    showMessage(input);
  }
}

void showMessage(String input)
{
  // Find brackets
  int openBracket = input.indexOf('[');
  int closeBracket = input.indexOf(']');

  if (isBig == "no")
  {
    // Extract message (before the comma)
    message = input.substring(openBracket + 1, input.indexOf(",", openBracket));

    // Extract message2 (after the comma)
    message2 = input.substring(input.indexOf(",", openBracket) + 1, closeBracket);
  }
  else
  {
    message = input.substring(openBracket + 1, input.length() - 1);
  }

  // Creating temp veriable to store values
  // Allocate char array and copy the content of command
  char currentCommand[command.length() + 1];  // +1 for null-terminator
  command.toCharArray(currentCommand, command.length() + 1);  // Copy String to char array

  char currentIsBig[isBig.length() + 1];
  isBig.toCharArray(currentIsBig, isBig.length() + 1);
  
  char currentMessage[message.length() + 1];
  message.toCharArray(currentMessage, message.length() + 1);
      
  char currentMessage2[message2.length() + 1];
  message2.toCharArray(currentMessage2, message2.length() + 1);

  display.displayText(currentMessage, currentMessage2, currentCommand, currentIsBig);
}

void updateSettings(String input)
{
  // Find brackets
  int openBracket = input.indexOf('[');
  int closeBracket = input.indexOf(']');
  
  // Extract values
  int firstComma = input.indexOf(',', openBracket);
  int secondComma = input.indexOf(',', firstComma + 1);
  int thirdComma = input.indexOf(',', secondComma + 1);
  
  // Parse brightness
  String brightnessStr = input.substring(openBracket + 1, firstComma);
  int brightness = brightnessStr.toInt();
  
  // Parse colors (remove # and convert to hex)
  String topColour = input.substring(firstComma + 2, secondComma);
  String bottomColour = input.substring(secondComma + 2, thirdComma);
  String fullColour = input.substring(thirdComma + 2, closeBracket);
  
  // Convert hex color strings to uint32_t
  uint32_t topColr = (uint32_t)strtoul(topColour.c_str() + 1, NULL, 16);
  uint32_t bottomColr = (uint32_t)strtoul(bottomColour.c_str() + 1, NULL, 16);
  uint32_t fullColr = (uint32_t)strtoul(fullColour.c_str() + 1, NULL, 16);
  
  // Set brightness
  display.setBrightness(brightness);
  
  // Set color settings
  display.setTopColour(topColr);
  display.setBottomColour(bottomColr);
  display.setFullColour(fullColr);

  display.displayText("Done", "", "static", "yes");
}
