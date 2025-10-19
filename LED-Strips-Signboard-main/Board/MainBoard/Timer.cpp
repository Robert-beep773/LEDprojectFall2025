#include "Timer.h"
#include <Arduino.h> // Required for millis()
#include <Wire.h>
#include "Display.h"

Timer* Timer::instance = nullptr;

Timer& Timer::getInstance() //Timer singleton 
{
  if (instance == nullptr)
  {
    instance = new Timer();
  }

  return *instance;
}

// Constructor
Timer::Timer() : rtc() {}

void Timer::setupRTC()
{
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    while (1); // freeze the program
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); 
}

void Timer::startTimer(int minutes, int seconds)
{
  currentMin = minutes;
  currentSec = seconds;

  remainingTime = TimeSpan(0, 0, minutes, seconds);
  targetTime = rtc.now() + remainingTime;
  timerActive = true;
  timerPaused = false;
  Serial.println(rtc.now().timestamp());
}

void Timer::pauseTimer()
{
  if (timerActive && !timerPaused)
  {
    remainingTime = targetTime - rtc.now();
    timerPaused = true;
  }
}

void Timer::resumeTimer()
{
  if (timerActive && timerPaused)
  {
    targetTime = rtc.now() + remainingTime;
    timerPaused = false;
  }
}

void Timer::resetTimer()
{
  if (timerActive)
  {
    remainingTime = TimeSpan(0, 0, currentMin, currentSec);
    targetTime = rtc.now() + remainingTime;
    timerActive = true;
    timerPaused = false;
  }
}

void Timer::stopTimer()
{
  timerActive = false;
  timerPaused = false;
  Display::getInstance().displayText("STOP", "", "static", "yes");
}

void Timer::displayTimeOfDay(bool tod)
{
  if(tod == true)
  {
    currentMode = MODE_CLOCK;
  }
  else
  {
    currentMode = MODE_TIMER;
  }
}

void Timer::updateTimer()
{
  if (millis() - lastUpdateMillis >= 1000)
  {
    switch (currentMode)
    {
      case MODE_TIMER:
        if (!timerActive || timerPaused) break;
        updateCountdown();
        break;

      case MODE_CLOCK:
        updateTimeOfDay();
        break;
    }
    lastUpdateMillis = millis();
  }
}

void Timer::updateCountdown()
{
  DateTime now = rtc.now();
  TimeSpan remaining = targetTime - now;

    if (remaining.totalseconds() <= 0)
    {
      Display::getInstance().displayText("0+:00", "", "static", "yes");
      timerActive = false;
    }
    else
    {
      int mins = max(0, remaining.minutes());
      int secs = max(0, remaining.seconds());
      char text[8];
      sprintf(text, "%d+:%02d", mins, secs);
      Display::getInstance().displayText(text, "", "static", "yes");
    }
}

void Timer::updateTimeOfDay()
{
    DateTime now = rtc.now();
    int mins = now.hour();
    int secs = now.minute();
    char text[8];
    sprintf(text, "%d+:%02d", mins, secs);
    Display::getInstance().displayText(text, "", "static", "yes");
}



bool Timer::getTimerRunning()
{
  return timerActive;
}

bool Timer::getTimerPaused()
{
  return timerPaused;
}

void Timer::parseTimerInput(String input)
{
  // Find brackets
  int openBracket = input.indexOf('[');
  int closeBracket = input.indexOf(']');
  
  // Check if brackets are valid
  if (openBracket == -1 || closeBracket == -1 || openBracket >= closeBracket)
  {
    // Invalid input format
    Serial.println("Error: Invalid time input format");
    return;
  }
  
  // Find the comma separating minutes and seconds
  int commaIndex = input.indexOf(":", openBracket);
  
  // Check if comma is valid
  if (commaIndex == -1 || commaIndex >= closeBracket)
  {
    Serial.println("Error: Invalid time input format");
    return;
  }
  
  String minStr = input.substring(openBracket + 1, commaIndex);
  String secStr = input.substring(commaIndex + 1, closeBracket);
  
  int minutes = minStr.toInt();
  int seconds = secStr.toInt();
  
  // Start timer with parsed minutes and seconds
  startTimer(minutes, seconds);
}
