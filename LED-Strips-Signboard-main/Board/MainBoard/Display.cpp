#include "Display.h"
#include "CharacterSet15x15Hex.h"

// Initialize static instance to nullptr
Display* Display::instance = nullptr;

extern uint32_t currentTopColourHex = 0xFF0000;
extern uint32_t currentBottomColourHex = 0xFF0000;
extern uint32_t currentFullColourHex = 0xFF0000;
extern int currentBrightness = 10;

// Singleton accessor
Display& Display::getInstance()
{
  if (instance == nullptr)
  {
    instance = new Display();
  }

  return *instance;
}

// Constructor
Display::Display()
{
  // Initialize frameBuffer to zero
  for (int i = 0; i < NUM_STRIPS; i++)
  {
    for (int j = 0; j < NUMPIXELS; j++)
    {
      frameBuffer[i][j] = 0;
    }
  }
}

// Initialize LED Matrix
void Display::setup(int brightness)
{
  currentBrightness = brightness;

  for (int i = 0; i < NUM_STRIPS; i++)
  {
    strips[i] = Adafruit_NeoPixel(NUMPIXELS, stripPins[i], NEO_GRB + NEO_KHZ800);
    strips[i].begin();
    strips[i].setBrightness(brightness);
    strips[i].show();
  }
}

// To set the brightness
void Display::setBrightness(int brightness)
{
  currentBrightness = brightness;

  for (int i = 0; i < NUM_STRIPS; i++)
  {
    strips[i].setBrightness(brightness);
  }
}

// Clear Frame Buffer
void Display::clearBuffer(bool bigFont)
{
  int rows = bigFont ? 15 : 7;
  for (int i = 0; i < NUM_STRIPS; i++)
  {
    for (int j = 0; j < NUMPIXELS; j++)
    {
      frameBuffer[i][j] = 0;  // Reset all pixels
    }
  }
}

// Update LEDs from Buffer
void Display::updateLEDs()
{

  for (int i = 0; i < NUM_STRIPS; i++)
  {
    for (int j = 0; j < NUMPIXELS; j++)
    {
      strips[i].setPixelColor(j, frameBuffer[i][j]);
    }
    strips[i].show();
  }

}

void Display::setPixel(int x, int y, uint32_t color)
{
  if (x < 0 || x >= NUMPIXELS || y < 0 || y >= NUM_STRIPS)
    return;  // Ignore out-of-bounds pixels
  frameBuffer[y][x] = color; // Update pixel in frame buffer
}
 
 
//vertical offset for characters
  int Display::getCharVerticalOffset(char c, bool useBigFont) {
  if (!useBigFont) return 0;  // Skip shift for 7x7 font
  int index = getCharIndex15x15(c);
  if (index < 0 || index >= 75) return 0;
  return (int)pgm_read_byte(&(charVerticalOffset15x15[index]));
}
//HERE IS THE NEW SPACING FUNCTION
bool Display::needsSpacing(char current, char next, bool useBigFont) {
  int currentIndex = useBigFont ? getCharIndex15x15(current) : getCharIndex(current);
  int nextIndex    = useBigFont ? getCharIndex15x15(next)    : getCharIndex(next);
  if (currentIndex == -1 || nextIndex == -1) return true;

  int currentRight = -1;
  int nextLeft = 100;

  if (useBigFont) {
    for (int row = 0; row < 13; row++) {
      uint16_t curRow = pgm_read_word(&(charSet15x15Hex[currentIndex][row]));
      uint16_t nextRow = pgm_read_word(&(charSet15x15Hex[nextIndex][row]));

      for (int col = 14; col >= 0; col--) {
        if (curRow & (1 << col)) { currentRight = max(currentRight, col); break; }
      }
      for (int col = 0; col < 15; col++) {
        if (nextRow & (1 << col)) { nextLeft = min(nextLeft, col); break; }
      }
    }
  } else {
    for (int row = 0; row < 7; row++) {
      for (int col = 6; col >= 0; col--) {
        if (pgm_read_byte(&(charSet7x7[currentIndex][row][col]))) {
          currentRight = max(currentRight, col);
          break;
        }
      }
      for (int col = 0; col < 7; col++) {
        if (pgm_read_byte(&(charSet7x7[nextIndex][row][col]))) {
          nextLeft = min(nextLeft, col);
          break;
        }
      }
    }
  }

  return currentRight >= 6 || nextLeft <= 0 || (currentRight + 1 >= 7 - nextLeft);
}


int Display::getCharacterWidth7x7(char c)
{
  int index = getCharIndex(c);
  if (index == -1) return 0;  // Return 0 if character not found

  int minCol = 7, maxCol = 0;

  // Find the actual width of the character
  for (int row = 0; row < 7; row++)
  {
    for (int col = 0; col < 7; col++)
    {
      if (pgm_read_byte(&(charSet7x7[index][row][col])))
      {  // Check if pixel is set
        if (col < minCol) minCol = col;
        if (col > maxCol) maxCol = col;
      }
    }
  }

  return (maxCol >= minCol) ? (maxCol - minCol + 1) : 1;  // Ensure at least 1-pixel width
}

int Display::getCharacterWidth15x15(char c)
{
  int index = getCharIndex15x15(c);
  if (index == -1) return 0;  // Return 0 if character not found

  int minCol = 15, maxCol = 0;

  for (int row = 0; row < 13; row++)  // Scan only 13 rows
  {
    uint16_t rowData = pgm_read_word(&(charSet15x15Hex[index][row]));

    for (int col = 0; col < 15; col++)
    {
      if (rowData & (1 << (14 - col)))   // Check if pixel is set
      {
        if (col < minCol) minCol = col;

        if (col > maxCol) maxCol = col;
      }
    }
  }

  return (maxCol >= minCol) ? (maxCol - minCol + 1) : 0;  // Ensure at least 1-pixel width
}



void Display::drawCharacter7x7(char c, int x, int y, uint32_t color)
{
  int index = getCharIndex(c);
  if (index == -1) return;

  int minCol = 7, maxCol = 0;

  // Find the actual left and right boundaries
  for (int row = 0; row < 7; row++)
  {
    for (int col = 0; col < 7; col++)
    {
      if (pgm_read_byte(&(charSet7x7[index][row][col])))  // If pixel is set
      {
        if (col < minCol) minCol = col;

        if (col > maxCol) maxCol = col;
      }
    }
  }

  int charWidth = maxCol - minCol + 1;

  //

  // Draw only the necessary part of the character
  for (int row = 0; row < 7; row++)
  {
    for (int col = minCol; col <= maxCol; col++)
    {
      if (pgm_read_byte(&(charSet7x7[index][row][col])))
      {
        setPixel(x + (col - minCol), y + row, color);
      }
    }
  }
}

void Display::drawCharacter15x15(char c, int x, int y, uint32_t color)
{
  int index = getCharIndex15x15(c);
  if (index == -1)
    return;

  int minCol = 15, maxCol = 0;

  // Find the actual left and right boundaries
  for (int row = 0; row < 13; row++)
  {
    uint16_t rowData = pgm_read_word(&(charSet15x15Hex[index][row]));

    for (int col = 0; col < 15; col++)
    {
      if (rowData & (1 << (14 - col)))
      {
        if (col < minCol) minCol = col;
        if (col > maxCol) maxCol = col;
      }
    }
  }

  int charWidth = maxCol - minCol;

  // Get offset
  int yOffset = getCharVerticalOffset(c, true);

  // Draw only the necessary part of the character
  for (int row = 0; row < 13; row++)
  {
    uint16_t rowData = pgm_read_word(&(charSet15x15Hex[index][row]));

    for (int col = minCol; col <= maxCol; col++)
    {
      if ((rowData >> (14 - col)) & 1)
      {
        // Apply offset to y
        setPixel(x + (col - minCol), y + row + yOffset, color);
      }
    }
  }
}


void Display::displayText(const char* text1, const char* text2, const char* command, const char* displayType)
{
  bool useBigFont = (strcmp(displayType, "yes") == 0);
  clearBuffer(useBigFont);

  int text1Len = strlen(text1);
  int text2Len = strlen(text2);

  // Calculate width based on the longer text
  int totalWidth = calculateTextWidth((text1Len > text2Len) ? text1 : text2, useBigFont);
  
  if (strcmp(command, "scrolC") == 0)
  {
    // Continuous scrolling implementation
    scrollTextContinuous(text1, text2, totalWidth, useBigFont);
  }
  else if (strcmp(command, "scrolS") == 0)
  {
    // Scroll from right then stop at the left
    scrollTextAndStop(text1, text2, totalWidth, useBigFont);
  }
  else if (strcmp(command, "fadeIn") == 0)
  {
    // Fade in text effect
    fadeInText(text1, text2, useBigFont);
  }
  else if (strcmp(command, "static") == 0)
  {
    // Static display implementation
    displayStaticText(text1, text2, useBigFont);
  }
  else if (strcmp(command, "breath") == 0)
  {
    // Static display implementation
    breatheText(text1, text2, useBigFont);
  }
}

// Helper function to calculate total text width
int Display::calculateTextWidth(const char* text, bool useBigFont)
{
  int totalWidth = 0;
  int textLen = strlen(text);
  
  for (int i = 0; i < textLen; i++)
  {
    totalWidth += useBigFont ? 
      getCharacterWidth15x15(text[i]) : 
      getCharacterWidth7x7(text[i]);
    totalWidth += 1;  // Add spacing between characters
  }
  
  return totalWidth;
}

// Continuous scrolling implementation
void Display::scrollTextContinuous(const char* text1, const char* text2, int totalWidth, bool useBigFont)
{
  int scrollSpeed = 100;
  int text1Len = strlen(text1);
  int text2Len = strlen(text2);
  int longerTextLen = (text1Len > text2Len) ? text1Len : text2Len;
  int shift = 0;

  scrollInterrupt = false;

  char* text1Copy = new char[text1Len + 1];
  char* text2Copy = new char[text2Len + 1];
  strcpy(text1Copy, text1);
  strcpy(text2Copy, text2);

  unsigned long previousMillis = 0;

  while (!scrollInterrupt)
  {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= scrollSpeed)
    {
      previousMillis = currentMillis;
      clearBuffer(useBigFont);
      int effectiveShift = shift % (totalWidth + NUMPIXELS);
      int currentX = NUMPIXELS - effectiveShift;

      for (int i = 0; i < longerTextLen; i++)
      {
        int charWidth = useBigFont ? getCharacterWidth15x15(text1Copy[i])
                                   : getCharacterWidth7x7((text1Len > text2Len) ? text1Copy[i] : text2Copy[i]);

        if (currentX >= -charWidth && currentX < NUMPIXELS)
        {
          if (useBigFont)
            drawCharacter15x15(text1Copy[i], currentX, 1, currentFullColourHex);
          else
          {
            if (i < text1Len) drawCharacter7x7(text1Copy[i], currentX, 0, currentTopColourHex);
            if (i < text2Len) drawCharacter7x7(text2Copy[i], currentX, 8, currentBottomColourHex);
          }
        }

        // second draw for seamless loop
        int loopX = currentX + totalWidth + NUMPIXELS;
        if (loopX >= 0 && loopX < NUMPIXELS * 2)
        {
          if (useBigFont)
            drawCharacter15x15(text1Copy[i], loopX, 1, currentFullColourHex);
          else
          {
            if (i < text1Len) drawCharacter7x7(text1Copy[i], loopX, 0, currentTopColourHex);
            if (i < text2Len) drawCharacter7x7(text2Copy[i], loopX, 8, currentBottomColourHex);
          }
        }

        // spacing logic
        currentX += charWidth;
        if (useBigFont)
        {
          if (i + 1 < text1Len && needsSpacing(text1Copy[i], text1Copy[i + 1], true))
            currentX += 1;
        }
        else
        {
          bool spaced = false;
          if (i + 1 < text1Len && needsSpacing(text1Copy[i], text1Copy[i + 1], false))
            spaced = true;
          if (i + 1 < text2Len && needsSpacing(text2Copy[i], text2Copy[i + 1], false))
            spaced = true;
          if (spaced)
            currentX += 1;
        }
      }

      updateLEDs();
      shift++;

      if (Serial.available() > 0)
      {
        clearBuffer(useBigFont);
        scrollInterrupt = true;
      }
    }
    delay(1);
  }

  delete[] text1Copy;
  delete[] text2Copy;
}

// Breathing implementation
void Display::breatheText(const char* text1, const char* text2, bool useBigFont)
{
  const int minBrightness = 10;
  const int maxBrightness = currentBrightness;
  const int steps = 40;
  const int delayMs = 30;

  int direction = 1;
  int currentStep = 0;
  scrollInterrupt = false;

  // Draw the static text once at brightness 0
  setBrightness(0);
  displayStaticText(text1, text2, useBigFont);

  while (!scrollInterrupt)
  {
    // Calculate brightness value
    float t = (float)currentStep / steps;
    float eased = 0.5 * (1 - cos(PI * t));  // cosine easing for smoothness
    int brightness = minBrightness + (int)(eased * (maxBrightness - minBrightness));

    setBrightness(brightness);
    updateLEDs();

    delay(delayMs);

    currentStep += direction;
    if (currentStep >= steps || currentStep <= 0)
      direction *= -1;

    // Check for user interrupt
    if (Serial.available() > 0)
    {
      scrollInterrupt = true;
      clearBuffer(useBigFont);
      setBrightness(currentBrightness);
    }
  }
}


// Scroll and stop implementation
void Display::scrollTextAndStop(const char* text1, const char* text2, int totalWidth, bool useBigFont) {
  int speed = 50;
  int text1Len = strlen(text1);
  int text2Len = strlen(text2);
  int longerTextLen = (text1Len > text2Len) ? text1Len : text2Len;
  int stopPosition = 0;

  for (int shift = NUMPIXELS; shift >= stopPosition; shift--) {
    clearBuffer(useBigFont);
    int currentX = shift;

    for (int i = 0; i < longerTextLen; i++) {
      int charWidth = useBigFont
        ? getCharacterWidth15x15(text1[i])
        : getCharacterWidth7x7((text1Len > text2Len) ? text1[i] : text2[i]);

      if (currentX >= -charWidth && currentX < NUMPIXELS) {
        if (useBigFont) {
          drawCharacter15x15(text1[i], currentX, 1, currentFullColourHex);
        } else {
          if (i < text1Len)
            drawCharacter7x7(text1[i], currentX, 0, currentTopColourHex);
          if (i < text2Len)
            drawCharacter7x7(text2[i], currentX, 8, currentBottomColourHex);
        }
      }

      // Smart spacing
      currentX += charWidth;
      if (useBigFont) {
        if (i + 1 < text1Len && needsSpacing(text1[i], text1[i + 1], true))
          currentX += 1;
      } else {
        bool spaced = false;
        if (i + 1 < text1Len && needsSpacing(text1[i], text1[i + 1], false))
          spaced = true;
        if (i + 1 < text2Len && needsSpacing(text2[i], text2[i + 1], false))
          spaced = true;
        if (spaced)
          currentX += 1;
      }
    }

    updateLEDs();
    delay(speed);
  }
}

void Display::fadeInText(const char* text1, const char* text2, bool useBigFont)
{
  const int steps = 20;
  const int delay_ms = 50;
  int currentBrightness;

  // Grab brightness from the first strip (assumes consistent brightness)
  int originalBrightness = strips[0].getBrightness();

  // Draw text with brightness at 0
  setBrightness(0);
  displayStaticText(text1, text2, useBigFont);  // already has smart spacing

  // Fade in smoothly
  for (int step = 1; step <= steps; step++) {
    currentBrightness = (originalBrightness * step) / steps;
    setBrightness(currentBrightness);
    updateLEDs();
    delay(delay_ms);
  }

  // Ensure final brightness is exact
  setBrightness(originalBrightness);
}

void Display::displayStaticText(const char* text1, const char* text2, bool useBigFont)
{
  clearBuffer(useBigFont);

  if (useBigFont)
  {
    // Big Font Mode (Single Row)
    int textLen = strlen(text1);
    int totalWidth = calculateTextWidth(text1, true);
    int startX = (NUMPIXELS - totalWidth) / 2;
    int startY = 1;

    int currentX = startX;
    for (int i = 0; i < textLen; i++)
    {
      int charWidth = getCharacterWidth15x15(text1[i]);
      drawCharacter15x15(text1[i], currentX, startY, currentFullColourHex);
      currentX += charWidth;

      if (i + 1 < textLen && needsSpacing(text1[i], text1[i + 1], true))
        currentX += 1;
    }
  }
  else
  {
    // Small Font Mode (Two Rows)
    int topLen = strlen(text1);
    int bottomLen = strlen(text2);

    int topWidth = calculateTextWidth(text1, false);
    int bottomWidth = calculateTextWidth(text2, false);

    int topX = (NUMPIXELS - topWidth) / 2;
    int bottomX = (NUMPIXELS - bottomWidth) / 2;

    // Top row
    int currentX = topX;
    for (int i = 0; i < topLen; i++)
    {
      int charWidth = getCharacterWidth7x7(text1[i]);
      drawCharacter7x7(text1[i], currentX, 0, currentTopColourHex);
      currentX += charWidth;

      if (i + 1 < topLen && needsSpacing(text1[i], text1[i + 1], false))
        currentX += 1;
    }

    // Bottom row
    currentX = bottomX;
    for (int i = 0; i < bottomLen; i++)
    {
      int charWidth = getCharacterWidth7x7(text2[i]);
      drawCharacter7x7(text2[i], currentX, 8, currentBottomColourHex);
      currentX += charWidth;

      if (i + 1 < bottomLen && needsSpacing(text2[i], text2[i + 1], false))
        currentX += 1;
    }
  }

  updateLEDs();
}


void Display::setTopColour(const uint32_t colourHex)
{
  currentTopColourHex = colourHex;
}
void Display::setBottomColour(const uint32_t colourHex)
{
  currentBottomColourHex = colourHex;
}

void Display::setFullColour(const uint32_t colourHex)
{
  currentFullColourHex = colourHex;
}

void Display::displayCustomPixels(const char* input, const char* chunkPos)
{
  // Clear the display if this is the first chunk
  if (strcmp(chunkPos, "start") == 0)
  {
    clearBuffer(true);
  }
  
  // Print chunk position for debugging
  Serial.println(chunkPos);
  
  // Find the opening bracket
  const char* start = strchr(input, '[');
  if (!start) return; // Exit if no opening bracket found
  start++; // Move past the opening bracket
  
  // Process each coordinate pair
  char coordBuffer[30]; // Buffer for holding one coordinate set
  int bufferIndex = 0;
  bool inCoordPair = false;
  
  for (const char* p = start; *p && *p != ']'; p++)
  {
    if (*p == '(')
    {
      inCoordPair = true;
      bufferIndex = 0;
    }
    else if (*p == ')')
    {
      inCoordPair = false;
      coordBuffer[bufferIndex] = '\0'; // Null terminate
      
      // Parse the coordinate pair
      char* xStr = coordBuffer;
      char* yStr = NULL;
      char* colorStr = NULL;
      
      // Find first comma
      char* firstComma = strchr(coordBuffer, ',');
      if (firstComma)
      {
        *firstComma = '\0'; // Split string
        yStr = firstComma + 1;
        
        // Find second comma
        char* secondComma = strchr(yStr, ',');
        if (secondComma)
        {
          *secondComma = '\0'; // Split string
          colorStr = secondComma + 1;
        }
      }
      
      // If we have all three components
      if (xStr && yStr && colorStr)
      {
        int x = atoi(xStr);
        int y = atoi(yStr);
        
        // Handle hex color with or without # prefix
        unsigned long color = 0;
        if (strncmp(colorStr, "#", 1) == 0)
        {
          color = strtoul(colorStr + 1, NULL, 16);
        }
        else
        {
          color = strtoul(colorStr, NULL, 16);
        }
        
        // Set the pixel with parsed color (bounds checking recommended)
        if (x >= 0 && y >= 0)
        {
          setPixel(y, x, color);
        }
      }
    }
    else if (inCoordPair && bufferIndex < sizeof(coordBuffer) - 1)
    {
      coordBuffer[bufferIndex++] = *p;
    }
  }
  
  // Update LEDs
  updateLEDs();
}
