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
<<<<<<< Updated upstream
=======
  // Space character uses fixed width
  if (c == ' ') return 3;  // 3 pixels for space
  
>>>>>>> Stashed changes
  int index = getCharIndex(c);
  if (index == -1) return 0;  // Invalid character

  // Find the actual bounding box of the character
  int minCol = 7, maxCol = 0;
  for (int row = 0; row < 7; row++)
  {
    for (int col = 0; col < 7; col++)
    {
      if (pgm_read_byte(&(charSet7x7[index][row][col])))
      {
        if (col < minCol) minCol = col;
        if (col > maxCol) maxCol = col;
      }
    }
  }

  // Calculate width and limit to 5 pixels maximum for compact display
  int width = (maxCol >= minCol) ? (maxCol - minCol + 1) : 1;
  return (width > 5) ? 5 : width;
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
<<<<<<< Updated upstream
=======
  // Space character doesn't need drawing (width already accounted for)
  if (c == ' ') return;
  
  // Early exit if character is completely off-screen
  int charWidth = getCharacterWidth7x7(c);
  if (x + charWidth < 0 || x >= NUMPIXELS) return;
  
>>>>>>> Stashed changes
  int index = getCharIndex(c);
  if (index == -1) return;  // Invalid character

  // Find the actual bounding box of the character
  int minCol = 7, maxCol = 0;
  for (int row = 0; row < 7; row++)
  {
    for (int col = 0; col < 7; col++)
    {
      if (pgm_read_byte(&(charSet7x7[index][row][col])))
      {
        if (col < minCol) minCol = col;
        if (col > maxCol) maxCol = col;
      }
    }
  }

  // Calculate actual width
  int actualWidth = maxCol - minCol + 1;
  
  // If character is 5 pixels or less, draw it normally
  if (actualWidth <= 5) {
    // Draw the character normally with bounds checking
    for (int row = 0; row < 7; row++)
    {
      for (int col = minCol; col <= maxCol; col++)
      {
        if (pgm_read_byte(&(charSet7x7[index][row][col])))
        {
          int pixelX = x + (col - minCol);
          // Only draw pixels that are within screen bounds
          if (pixelX >= 0 && pixelX < NUMPIXELS)
          {
            setPixel(pixelX, y + row, color);
          }
        }
      }
    }
  }
  else {
    // Character is wider than 5 pixels - scale it down with edge preservation
    // Always preserve leftmost and rightmost edges for better character recognition
    for (int row = 0; row < 7; row++)
    {
      for (int outputCol = 0; outputCol < 5; outputCol++)
      {
        bool pixelOn = false;
        
        // Special handling for edge pixels to preserve character shape
        if (outputCol == 0) {
          // Leftmost pixel: always sample from the leftmost source column
          pixelOn = pgm_read_byte(&(charSet7x7[index][row][minCol])) != 0;
        }
        else if (outputCol == 4) {
          // Rightmost pixel: always sample from the rightmost source column
          pixelOn = pgm_read_byte(&(charSet7x7[index][row][maxCol])) != 0;
        }
        else {
          // Middle pixels (1, 2, 3): use area-based sampling
          int middleWidth = actualWidth - 2;  // Width excluding left and right edges
          
          if (middleWidth > 0) {
            // Map output column 1-3 to source columns (excluding edges)
            float sourcePos = (float)(outputCol - 1) / 2.0f * (middleWidth - 1);
            int sourceCol = minCol + 1 + (int)(sourcePos + 0.5f);
            
            // Clamp to valid range (between edges)
            if (sourceCol < minCol + 1) sourceCol = minCol + 1;
            if (sourceCol > maxCol - 1) sourceCol = maxCol - 1;
            
            // Sample the pixel and its neighbors for better quality
            for (int offset = -1; offset <= 1; offset++)
            {
              int checkCol = sourceCol + offset;
              if (checkCol > minCol && checkCol < maxCol)  // Don't check edges
              {
                if (pgm_read_byte(&(charSet7x7[index][row][checkCol])))
                {
                  pixelOn = true;
                  break;
                }
              }
            }
          }
        }
        
        if (pixelOn)
        {
          int pixelX = x + outputCol;
          // Only draw pixels that are within screen bounds
          if (pixelX >= 0 && pixelX < NUMPIXELS)
          {
            setPixel(pixelX, y + row, color);
          }
        }
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


void Display::displayText(const char* text1, const char* text2, const char* command, const char* displayType, int scrollSpeed)
{
  bool useBigFont = (strcmp(displayType, "yes") == 0);
<<<<<<< Updated upstream
=======
  
  // Stop any active scroll when new command arrives
  // Add small delay to ensure cleanup completes before starting new scroll
  if (scrollState.isActive) {
    stopScrollAnimation();
    delay(10);  // Small delay to ensure state is fully cleared
  }
  
>>>>>>> Stashed changes
  clearBuffer(useBigFont);

  int text1Len = strlen(text1);
  int text2Len = strlen(text2);

  // Calculate width based on the longer text
  int totalWidth = calculateTextWidth((text1Len > text2Len) ? text1 : text2, useBigFont);
  
  if (strcmp(command, "scrolC") == 0)
  {
<<<<<<< Updated upstream
    // Continuous scrolling implementation
    scrollTextContinuous(text1, text2, totalWidth, useBigFont);
  }
  else if (strcmp(command, "scrolS") == 0)
  {
    // Scroll from right then stop at the left
    scrollTextAndStop(text1, text2, totalWidth, useBigFont);
=======
    // Continuous scrolling implementation (non-blocking)
    scrollTextContinuous(text1, text2, totalWidth, useBigFont, scrollSpeed);
  }
  else if (strcmp(command, "scrolS") == 0)
  {
    // Scroll from right then stop at the left (blocking but short)
    scrollTextAndStop(text1, text2, totalWidth, useBigFont, scrollSpeed);
>>>>>>> Stashed changes
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
    // Add 1 pixel spacing between characters (not after last character)
    if (i + 1 < textLen)
      totalWidth += 1;
  }
  
  return totalWidth;
}

<<<<<<< Updated upstream
// Continuous scrolling implementation
void Display::scrollTextContinuous(const char* text1, const char* text2, int totalWidth, bool useBigFont)
=======
// Continuous scrolling implementation - NON-BLOCKING
// Initializes scroll state and returns immediately
void Display::scrollTextContinuous(const char* text1, const char* text2, int totalWidth, bool useBigFont, int scrollSpeed)
>>>>>>> Stashed changes
{
  int scrollSpeed = 100;
  int text1Len = strlen(text1);
  int text2Len = strlen(text2);
  int longerTextLen = (text1Len > text2Len) ? text1Len : text2Len;
  int shift = 0;

<<<<<<< Updated upstream
=======
  // Stop any existing scroll
  stopScrollAnimation();
  
  // Initialize scroll state
  scrollState.isActive = true;
  scrollState.useBigFont = useBigFont;
  scrollState.previousMillis = millis();
  scrollState.shift = 0;
  scrollState.scrollSpeed = scrollSpeed;  // Use provided speed
  scrollState.totalWidth = totalWidth;
  scrollState.text1Len = text1Len;
  scrollState.text2Len = text2Len;
  scrollState.longerTextLen = (text1Len > text2Len) ? text1Len : text2Len;
  
  // Copy text strings
  strncpy(scrollState.text1Copy, text1, 120);
  strncpy(scrollState.text2Copy, text2, 120);
  scrollState.text1Copy[120] = '\0';
  scrollState.text2Copy[120] = '\0';
  
  // Draw first frame immediately
  updateScrollAnimation();
}

// Non-blocking scroll animation update - call from main loop
void Display::updateScrollAnimation()
{
  if (!scrollState.isActive) return;
  
  // Safety check: ensure scroll state is valid
  if (scrollState.longerTextLen <= 0 || scrollState.longerTextLen > 120) {
    stopScrollAnimation();
    return;
  }
  
  // Check for Serial interrupt first (listening device requirement)
  // Just stop animation - don't clear buffer, let main loop handle the command
  if (Serial.available() > 0)
  {
    stopScrollAnimation();
    return;
  }
  
  unsigned long currentMillis = millis();
  
  // Check if it's time to update the frame
  if (currentMillis - scrollState.previousMillis >= scrollState.scrollSpeed)
  {
    scrollState.previousMillis = currentMillis;
    clearBuffer(scrollState.useBigFont);
    
    // Safety check: prevent division by zero
    if (scrollState.totalWidth <= 0) {
      stopScrollAnimation();
      return;
    }
    
    // Use modulo to prevent overflow and create seamless loop
    unsigned long effectiveShift = scrollState.shift % (scrollState.totalWidth + NUMPIXELS);
    int currentX = NUMPIXELS - (int)effectiveShift;

    // Bounds check for text length
    int maxLen = min(scrollState.longerTextLen, 120);
    for (int i = 0; i < maxLen; i++)
    {
      // Safety check: ensure we don't access invalid array indices
      if (i >= 120) break;
      
      char char1 = (i < scrollState.text1Len && i < 120) ? scrollState.text1Copy[i] : '\0';
      char char2 = (i < scrollState.text2Len && i < 120) ? scrollState.text2Copy[i] : '\0';
      
      int charWidth = scrollState.useBigFont ? 
        getCharacterWidth15x15(char1) :
        getCharacterWidth7x7((scrollState.text1Len > scrollState.text2Len) ? char1 : char2);

      if (currentX >= -charWidth && currentX < NUMPIXELS)
      {
        if (scrollState.useBigFont)
        {
          if (char1 != '\0') drawCharacter15x15(char1, currentX, 1, currentFullColourHex);
        }
        else
        {
          if (i < scrollState.text1Len && char1 != '\0') drawCharacter7x7(char1, currentX, 0, currentTopColourHex);
          if (i < scrollState.text2Len && char2 != '\0') drawCharacter7x7(char2, currentX, 8, currentBottomColourHex);
        }
      }

      // Second draw for seamless loop
      int loopX = currentX + scrollState.totalWidth + NUMPIXELS;
      if (loopX >= 0 && loopX < NUMPIXELS * 2)
      {
        if (scrollState.useBigFont)
        {
          if (char1 != '\0') drawCharacter15x15(char1, loopX, 1, currentFullColourHex);
        }
        else
        {
          if (i < scrollState.text1Len && char1 != '\0') drawCharacter7x7(char1, loopX, 0, currentTopColourHex);
          if (i < scrollState.text2Len && char2 != '\0') drawCharacter7x7(char2, loopX, 8, currentBottomColourHex);
        }
      }

      // Spacing logic with bounds checking
      currentX += charWidth;
      if (scrollState.useBigFont)
      {
        if (i + 1 < scrollState.text1Len && i + 1 < 120 && 
            needsSpacing(char1, scrollState.text1Copy[i + 1], true))
          currentX += 1;
      }
      else
      {
        bool spaced = false;
        if (i + 1 < scrollState.text1Len && i + 1 < 120 && 
            needsSpacing(char1, scrollState.text1Copy[i + 1], false))
          spaced = true;
        if (i + 1 < scrollState.text2Len && i + 1 < 120 && 
            needsSpacing(char2, scrollState.text2Copy[i + 1], false))
          spaced = true;
        if (spaced)
          currentX += 1;
      }
    }

    updateLEDs();
    scrollState.shift++;
  }
}

// Stop scroll animation and clean up
void Display::stopScrollAnimation()
{
  if (scrollState.isActive)
  {
    // Set inactive first to prevent updateScrollAnimation from running
    scrollState.isActive = false;
<<<<<<< Updated upstream
=======
    
    // Clear text buffers to free memory
    memset(scrollState.text1Copy, 0, sizeof(scrollState.text1Copy));
    memset(scrollState.text2Copy, 0, sizeof(scrollState.text2Copy));
    
    // Reset all scroll state values
    scrollState.shift = 0;
    scrollState.totalWidth = 0;
    scrollState.text1Len = 0;
    scrollState.text2Len = 0;
    scrollState.longerTextLen = 0;
    
>>>>>>> Stashed changes
    clearBuffer(scrollState.useBigFont);
    updateLEDs();
  }
>>>>>>> Stashed changes
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
void Display::scrollTextAndStop(const char* text1, const char* text2, int totalWidth, bool useBigFont, int scrollSpeed) {
  int speed = scrollSpeed;  // Use provided speed
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

  int text1Len = strlen(text1);
  int text2Len = strlen(text2);
  
  // Calculate total width for centering
  int totalText1Width = calculateTextWidth(text1, useBigFont);
  int totalText2Width = text2Len > 0 ? calculateTextWidth(text2, false) : 0;
  
  // Center the text (or left-align if too wide)
  int topX = (NUMPIXELS - totalText1Width) / 2;
  if (topX < 0) topX = 0;
  if (topX + totalText1Width > NUMPIXELS) topX = 0;
  
  int bottomX = text2Len > 0 ? (NUMPIXELS - totalText2Width) / 2 : 0;
  if (bottomX < 0) bottomX = 0;
  if (bottomX + totalText2Width > NUMPIXELS) bottomX = 0;
  
  // Draw text1 (top line or full screen)
  int currentX = topX;
  for (int i = 0; i < text1Len; i++)
  {
<<<<<<< Updated upstream
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
=======
    int charWidth = useBigFont ? getCharacterWidth15x15(text1[i]) : getCharacterWidth7x7(text1[i]);
    
    if (currentX < NUMPIXELS && currentX + charWidth > 0)
    {
      if (useBigFont)
      {
        drawCharacter15x15(text1[i], currentX, 1, currentFullColourHex);
      }
      else
      {
        drawCharacter7x7(text1[i], currentX, 0, currentTopColourHex);
      }
    }
    
    currentX += charWidth;
    // Add 1 pixel spacing between characters (not after last character)
    if (i + 1 < text1Len)
      currentX += 1;
>>>>>>> Stashed changes
  }
  
  // Draw text2 (bottom line, only for small font)
  if (!useBigFont && text2Len > 0)
  {
<<<<<<< Updated upstream
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
=======
    currentX = bottomX;
    for (int i = 0; i < text2Len; i++)
    {
      int charWidth = getCharacterWidth7x7(text2[i]);
      
      if (currentX < NUMPIXELS && currentX + charWidth > 0)
      {
        drawCharacter7x7(text2[i], currentX, 8, currentBottomColourHex);
      }
      
      currentX += charWidth;
      // Add 1 pixel spacing between characters (not after last character)
      if (i + 1 < text2Len)
>>>>>>> Stashed changes
        currentX += 1;
    }
  }
  
  updateLEDs();
}

<<<<<<< Updated upstream
=======
// Chunked text display - divides text into 5 segments and displays line by line
void Display::displayTextChunked(const char* text1, const char* text2, bool useBigFont, uint32_t color1)
{
  int text1Len = strlen(text1);
  int text2Len = strlen(text2);
  const int NUM_CHUNKS = 5;
  
  // Calculate total width of entire text (not just chunks) for proper centering
  int totalText1Width = 0;
  int totalText2Width = 0;
  
  // Calculate total width for text1
  for (int i = 0; i < text1Len; i++)
  {
    totalText1Width += useBigFont ? getCharacterWidth15x15(text1[i]) : getCharacterWidth7x7(text1[i]);
    if (i + 1 < text1Len && needsSpacing(text1[i], text1[i + 1], useBigFont))
      totalText1Width += 1;
  }
  
  // Calculate total width for text2
  if (text2Len > 0)
  {
    for (int i = 0; i < text2Len; i++)
    {
      totalText2Width += getCharacterWidth7x7(text2[i]);
      if (i + 1 < text2Len && needsSpacing(text2[i], text2[i + 1], false))
        totalText2Width += 1;
    }
  }
  
  // Center the entire text once (not each chunk)
  int topX = (NUMPIXELS - totalText1Width) / 2;
  int bottomX = text2Len > 0 ? (NUMPIXELS - totalText2Width) / 2 : 0;
  
  // Calculate chunk size for each line
  int chunk1Size = (text1Len + NUM_CHUNKS - 1) / NUM_CHUNKS; // Round up division
  int chunk2Size = text2Len > 0 ? (text2Len + NUM_CHUNKS - 1) / NUM_CHUNKS : 0;
  
  // Display each chunk sequentially
  for (int chunk = 0; chunk < NUM_CHUNKS; chunk++)
  {
    int start1 = chunk * chunk1Size;
    int end1 = (start1 + chunk1Size < text1Len) ? start1 + chunk1Size : text1Len;
    int start2 = chunk * chunk2Size;
    int end2 = text2Len > 0 ? ((start2 + chunk2Size < text2Len) ? start2 + chunk2Size : text2Len) : 0;
    
    // Only process if there's text in this chunk
    if (start1 < text1Len || (text2Len > 0 && start2 < text2Len))
    {
      // Calculate the X position for this chunk based on the centered start position
      int chunk1StartX = topX;
      int chunk2StartX = bottomX;
      
      // Calculate offset for this chunk (sum of widths of previous chunks)
      for (int i = 0; i < start1; i++)
      {
        chunk1StartX += useBigFont ? getCharacterWidth15x15(text1[i]) : getCharacterWidth7x7(text1[i]);
        if (i + 1 < text1Len && needsSpacing(text1[i], text1[i + 1], useBigFont))
          chunk1StartX += 1;
      }
      
      if (text2Len > 0)
      {
        for (int i = 0; i < start2; i++)
        {
          chunk2StartX += getCharacterWidth7x7(text2[i]);
          if (i + 1 < text2Len && needsSpacing(text2[i], text2[i + 1], false))
            chunk2StartX += 1;
        }
      }
      
      // Draw top line chunk
      int currentX = chunk1StartX;
      for (int i = start1; i < end1; i++)
      {
        int charWidth = useBigFont ? getCharacterWidth15x15(text1[i]) : getCharacterWidth7x7(text1[i]);
        if (useBigFont)
        {
          drawCharacter15x15(text1[i], currentX, 1, color1);
        }
        else
        {
          drawCharacter7x7(text1[i], currentX, 0, color1);
        }
        currentX += charWidth;
        
        if (i + 1 < end1 && needsSpacing(text1[i], text1[i + 1], useBigFont))
          currentX += 1;
      }
      
      // Draw bottom line chunk (only for small font)
      if (!useBigFont && text2Len > 0)
      {
        currentX = chunk2StartX;
        for (int i = start2; i < end2; i++)
        {
          int charWidth = getCharacterWidth7x7(text2[i]);
          drawCharacter7x7(text2[i], currentX, 8, currentBottomColourHex);
          currentX += charWidth;
          
          if (i + 1 < end2 && needsSpacing(text2[i], text2[i + 1], false))
            currentX += 1;
        }
      }
      
      // Small delay between chunks to show progression
      updateLEDs();
      delay(100); // Brief pause between chunks
    }
  }
}

>>>>>>> Stashed changes

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
