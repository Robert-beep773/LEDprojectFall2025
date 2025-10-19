#ifndef DISPLAY_H
#define DISPLAY_H
#include <Adafruit_NeoPixel.h>
#include "CharacterSet.h"
#include "CharacterSet15x15Hex.h"
#include <Arduino.h>
#define NUMPIXELS 60  
#define NUM_STRIPS 15
// Colour variable
extern uint32_t currentTopColourHex;
extern uint32_t currentBottomColourHex;
extern uint32_t currentFullColourHex;
extern int currentBrightness;
class Display
{
private:
  // Singleton instance
  static Display* instance;
  Adafruit_NeoPixel strips[NUM_STRIPS];
 
  // Private constructor for singleton pattern
  Display();
 
  // LED strip configuration
  const int stripPins[NUM_STRIPS] = {30, 31, 32, 33, 34, 35, 36, 37, 29, 28, 27, 26, 25, 24, 23};
 
  // Frame buffer for pixel data
  uint32_t frameBuffer[NUM_STRIPS][NUMPIXELS];
  
  // Interrupt flag for scrolling operations
  volatile bool scrollInterrupt = false;
 
  // Character drawing helper methods
  int getCharacterWidth7x7(char c);
  int getCharacterWidth15x15(char c);
  void drawCharacter7x7(char c, int x, int y, uint32_t color);
  void drawCharacter15x15(char c, int x, int y, uint32_t color);
  
  // Helper methods for displayText function
  int calculateTextWidth(const char* text, bool useBigFont);
  void scrollTextContinuous(const char* text1, const char* text2, int totalWidth, bool useBigFont);
  void drawTextLine(const char* text, int textLen, int* charWidths, int startX, int y, uint32_t color, int totalWidth, bool useBigFont);
  void scrollTextAndStop(const char* text1, const char* text2, int totalWidth, bool useBigFont);
  void fadeInText(const char* text1, const char* text2, bool useBigFont);
  void breatheText(const char* text1, const char* text2, bool useBigFont);
  void displayStaticText(const char* text1, const char* text2, bool useBigFont);
  
public:
  // Singleton accessor
  static Display& getInstance();
 
  // Delete copy constructor and assignment operator
  Display(const Display&) = delete;
  void operator=(const Display&) = delete;
 
  // Display setup
  void setup(int brightness);
 
 
  //some Dynamic display calculation and height offset
  bool needsSpacing(char current, char next, bool useBigFont);
  int getCharVerticalOffset(char c, bool useBigFont);


 
  // Buffer management
  void clearBuffer(bool bigFont);
  void updateLEDs();
  void setPixel(int x, int y, uint32_t color);
  void setBrightness(int brightness);
 
  // Text display methods
  void displayText(const char* text1, const char* text2, const char* command, const char* displayType);
  // Colour management
  void setTopColour(const uint32_t colourHex);
  void setBottomColour(const uint32_t colourHex);
  void setFullColour(const uint32_t colourHex);
  // For custom drawings
  void displayCustomPixels(const char* input, const char* chunckPos);
};
#endif // DISPLAY_H
