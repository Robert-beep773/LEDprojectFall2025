/**
 * LED Strips Signboard - Main Arduino Program
 * 
 * This is the main entry point for the Arduino Mega firmware.
 * It handles serial communication, command parsing, and coordinates
 * all subsystems (Display, Timer, Remote Control).
 * 
 * Communication Protocol:
 * - Format: [START][COMMAND][DATA][END]
 * - START: ASCII 12 (0x0C)
 * - COMMAND: 4-digit ASCII code (1001-5004)
 * - DATA: Variable length message
 * - END: ASCII 15 (0x0F)
 * 
 * Hardware:
 * - Arduino Mega 2560
 * - 15× NeoPixel LED Strips (60 pixels each)
 * - DS3231 RTC Module (I2C)
 * - IR Receiver (pin 10)
 * 
 * @author LED Strips Signboard Team
 * @version 2.0
 */

#include "Display.h"
#include "Timer.h"
#include <Wire.h>
#include <RTClib.h>
#include "Remote.h"

// Global RTC instance for real-time clock functionality
RTC_DS3231 rtc;

// Font size toggle: true = 15x15 (large), false = 7x7 (small)
bool useBigFont = true;

// Subsystem instances
Timer timer; 
RemoteControl remote;

<<<<<<< Updated upstream
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

=======
// Variables for parsing received serial messages
String command = "";      // Extracted 4-digit command code
String message = "";     // First message (top row or full screen)
String message2 = "";    // Second message (bottom row, if applicable)
int messageSize = 0;      // Size of received message

// Raw serial input buffer variables
int intByte;             // Current byte being read
const uint8_t numRawChar = 100;  // Buffer size (reduced for memory optimization)
char msgRaw[numRawChar]; // Raw character buffer
uint8_t charCount = 0;   // Character counter
bool dataToSend = false; // Flag indicating data ready to process

// ASCII Protocol Constants
const char PROTOCOL_START = 12;  // 0x0C - Start of frame marker
const char PROTOCOL_END = 15;    // 0x0F - End of frame marker
const int COMMAND_LENGTH = 4;    // Command codes are always 4 digits
const int MAX_DATA_LENGTH = 150; // Maximum data length (120 chars + overhead)

// Display singleton instance
>>>>>>> Stashed changes
Display& display = Display::getInstance();

<<<<<<< Updated upstream
void setup()
{
  Serial.begin(9600);
=======
// Function declarations
void parseInput(String input);                                    // Parse incoming serial data
void processCommand(String cmd, String data);                     // Process parsed command
void displayText(String text1, String text2, String command, String displayType); // Display text with animation
uint32_t parseHexColor(String colorStr);                         // Convert hex string to RGB color
void sendSuccessResponse(String cmd, String message = "OK");     // Send success response
void sendErrorResponse(int errorCode, String message);            // Send error response

/**
 * Arduino setup function - Initializes all subsystems
 * Called once at startup
 */
void setup()
{
  // Initialize serial communication at fixed 9600 baud rate
  // This must match the web server's baud rate setting
  Serial.begin(9600);
  
  // Initialize Real-Time Clock module
>>>>>>> Stashed changes
  timer.setupRTC();
  
  // Initialize IR remote control receiver
  remote.setupRemote();
  
  // Initialize LED display with initial brightness (0-255, 7 is very dim)
  display.setup(7);

  // Display startup message
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

/**
 * Main loop - Continuously runs after setup()
 * Handles serial communication, display updates, and subsystem polling
 * 
 * Priority order:
 * 1. Serial commands (highest - user input)
 * 2. Display animations (non-blocking updates)
 * 3. Remote control polling
 * 4. Timer updates
 */
void loop()
{
<<<<<<< Updated upstream
  if (Serial.available())
  {
    // Serial.begin(9600); Resetting the arduino with every message
    // display.setup(100);
    // Extracting size
    String input = Serial.readStringUntil('\n');  // Read until newline
    parseInput(input);
  }

=======
  // Check for incoming serial commands (highest priority)
  if (Serial.available())
  {
    // Safety check: If buffer is overflowing, clear it to prevent system hang
    // This prevents memory issues from rapid command sending
    if (Serial.available() > 60) {
      // Buffer has too much data - clear it and skip this command
      while (Serial.available() > 0) {
        Serial.read();
      }
      return;  // Skip processing to allow main loop to continue
    }
    
    // Read complete line until newline character
    String input = Serial.readStringUntil('\n');
    
    // Only process if we got valid input (non-empty)
    if (input.length() > 0)
    {
      parseInput(input);
    }
  }

  // Update display animations (non-blocking scroll, fade, breathe effects)
  // This must be called regularly for smooth animations
  display.updateDisplay();
  
  // Poll IR remote for button presses
>>>>>>> Stashed changes
  remote.useRemote();
  
  // Update timer countdown and time-of-day display
  timer.updateTimer();
}

/**
 * Parse incoming serial input according to ASCII protocol
 * 
 * Protocol format: [START][COMMAND][DATA][END]
 * - START: ASCII 12 (0x0C)
 * - COMMAND: 4-digit code (1001-5004)
 * - DATA: Variable length (optional)
 * - END: ASCII 15 (0x0F)
 * 
 * @param input Raw serial input string
 */
void parseInput(String input)
{
<<<<<<< Updated upstream
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
=======
    // Minimal logging to save memory (just acknowledge receipt)
    Serial.println("RX");
    
    // Stop timer/time-of-day display to prevent overwriting incoming commands
    // User commands take priority over automatic displays
    timer.displayTimeOfDay(false);
    
    // Also pause timer if it's running to prevent countdown from overwriting display
    if (timer.getTimerRunning() && !timer.getTimerPaused()) {
        timer.pauseTimer();
    }

    // Quick validation - check length first to avoid crashes
    // Minimum: START(1) + COMMAND(4) + END(1) = 6 characters
    // Maximum: START(1) + COMMAND(4) + DATA(150) + END(1) = 156 characters
    if (input.length() < 6 || input.length() > MAX_DATA_LENGTH + 6)
    {
        sendErrorResponse(9001, "Invalid length");
        return;
    }

    // Validate protocol markers (START and END)
    if (input.charAt(0) != PROTOCOL_START || input.charAt(input.length() - 1) != PROTOCOL_END)
    {
        sendErrorResponse(9001, "Invalid format");
        return;
    }
    
    // Extract 4-digit command code (positions 1-4)
    command = input.substring(1, COMMAND_LENGTH + 1);
    
    // Extract data portion (everything between command and END marker)
    String data = "";
    if (input.length() > COMMAND_LENGTH + 2)  // If there's data beyond command + markers
    {
        data = input.substring(COMMAND_LENGTH + 1, input.length() - 1);
    }

    // Process the extracted command and data
    processCommand(command, data);
}

void processCommand(String cmd, String data)
{
    int cmdCode = cmd.toInt();
    
    switch (cmdCode)
    {
        // TEXT DISPLAY COMMANDS - Memory optimized
        case 1001: // Static Text (Small Font) - Split by comma for top/bottom rows
            {
                // Stop timer countdown so it doesn't overwrite the text
                timer.displayTimeOfDay(false);
                if (timer.getTimerRunning() && !timer.getTimerPaused()) {
                    timer.pauseTimer();
                }
                int commaIndex = data.indexOf(',');
                String text1 = (commaIndex > 0) ? data.substring(0, commaIndex) : data;
                String text2 = (commaIndex > 0 && commaIndex < data.length() - 1) ? data.substring(commaIndex + 1) : "";
                if (text1.length() > 30) text1 = text1.substring(0, 30);
                if (text2.length() > 30) text2 = text2.substring(0, 30);
                displayText(text1, text2, "static", "no");
                sendSuccessResponse(cmd);
            }
            break;
            
        case 1002: // Static Text (Large Font)
            // Stop timer countdown so it doesn't overwrite the text
            timer.displayTimeOfDay(false);
            if (timer.getTimerRunning() && !timer.getTimerPaused()) {
                timer.pauseTimer();
            }
            if (data.length() > 120) data = data.substring(0, 120); // Updated limit for large font
            displayText(data, "", "static", "yes");
            sendSuccessResponse(cmd);
            break;
            
        case 1003: // Scroll Text Continuous (Small Font) - Split by comma for top/bottom rows
            {
                // Stop timer countdown so it doesn't overwrite the text
                timer.displayTimeOfDay(false);
                if (timer.getTimerRunning() && !timer.getTimerPaused()) {
                    timer.pauseTimer();
                }
                int commaIndex = data.indexOf(',');
                String text1 = (commaIndex > 0) ? data.substring(0, commaIndex) : data;
                String text2 = (commaIndex > 0 && commaIndex < data.length() - 1) ? data.substring(commaIndex + 1) : "";
                if (text1.length() > 120) text1 = text1.substring(0, 120);
                if (text2.length() > 120) text2 = text2.substring(0, 120);
                displayText(text1, text2, "scrolC", "no");
                sendSuccessResponse(cmd);
            }
            break;
            
        case 1004: // Scroll Text Continuous (Large Font)
            // Stop timer countdown so it doesn't overwrite the text
            timer.displayTimeOfDay(false);
            if (timer.getTimerRunning() && !timer.getTimerPaused()) {
                timer.pauseTimer();
            }
            // Allow longer text for scrolling (120 chars)
            if (data.length() > 120) data = data.substring(0, 120);
            displayText(data, "", "scrolC", "yes");
            sendSuccessResponse(cmd);
            break;
            
        case 1005: // Scroll Text and Stop (Small Font) - Split by comma for top/bottom rows
            {
                // Stop timer countdown so it doesn't overwrite the text
                timer.displayTimeOfDay(false);
                if (timer.getTimerRunning() && !timer.getTimerPaused()) {
                    timer.pauseTimer();
                }
                int commaIndex = data.indexOf(',');
                String text1 = (commaIndex > 0) ? data.substring(0, commaIndex) : data;
                String text2 = (commaIndex > 0 && commaIndex < data.length() - 1) ? data.substring(commaIndex + 1) : "";
                if (text1.length() > 120) text1 = text1.substring(0, 120);
                if (text2.length() > 120) text2 = text2.substring(0, 120);
                displayText(text1, text2, "scrolS", "no");
                sendSuccessResponse(cmd);
            }
            break;
            
        case 1006: // Scroll Text and Stop (Large Font)
            // Stop timer countdown so it doesn't overwrite the text
            timer.displayTimeOfDay(false);
            if (timer.getTimerRunning() && !timer.getTimerPaused()) {
                timer.pauseTimer();
            }
            // Allow longer text for scrolling (120 chars)
            if (data.length() > 120) data = data.substring(0, 120);
            displayText(data, "", "scrolS", "yes");
            sendSuccessResponse(cmd);
            break;
            
        case 1007: // Fade In Text (Small Font) - Split by comma for top/bottom rows
            {
                // Stop timer countdown so it doesn't overwrite the text
                timer.displayTimeOfDay(false);
                if (timer.getTimerRunning() && !timer.getTimerPaused()) {
                    timer.pauseTimer();
                }
                int commaIndex = data.indexOf(',');
                String text1 = (commaIndex > 0) ? data.substring(0, commaIndex) : data;
                String text2 = (commaIndex > 0 && commaIndex < data.length() - 1) ? data.substring(commaIndex + 1) : "";
                if (text1.length() > 30) text1 = text1.substring(0, 30);
                if (text2.length() > 30) text2 = text2.substring(0, 30);
                displayText(text1, text2, "fadeIn", "no");
                sendSuccessResponse(cmd);
            }
            break;
            
        case 1008: // Fade In Text (Large Font)
            // Stop timer countdown so it doesn't overwrite the text
            timer.displayTimeOfDay(false);
            if (timer.getTimerRunning() && !timer.getTimerPaused()) {
                timer.pauseTimer();
            }
            if (data.length() > 120) data = data.substring(0, 120);
            displayText(data, "", "fadeIn", "yes");
            sendSuccessResponse(cmd);
            break;
            
        case 1009: // Breathe Text (Small Font) - Split by comma for top/bottom rows
            {
                // Stop timer countdown so it doesn't overwrite the text
                timer.displayTimeOfDay(false);
                if (timer.getTimerRunning() && !timer.getTimerPaused()) {
                    timer.pauseTimer();
                }
                int commaIndex = data.indexOf(',');
                String text1 = (commaIndex > 0) ? data.substring(0, commaIndex) : data;
                String text2 = (commaIndex > 0 && commaIndex < data.length() - 1) ? data.substring(commaIndex + 1) : "";
                if (text1.length() > 30) text1 = text1.substring(0, 30);
                if (text2.length() > 30) text2 = text2.substring(0, 30);
                displayText(text1, text2, "breath", "no");
                sendSuccessResponse(cmd);
            }
            break;
            
        case 1010: // Breathe Text (Large Font)
            // Stop timer countdown so it doesn't overwrite the text
            timer.displayTimeOfDay(false);
            if (timer.getTimerRunning() && !timer.getTimerPaused()) {
                timer.pauseTimer();
            }
            if (data.length() > 120) data = data.substring(0, 120);
            displayText(data, "", "breath", "yes");
            sendSuccessResponse(cmd);
            break;
            
        // TIMER COMMANDS
        case 2001: // Start Timer
            if (data.length() == 5 && data.charAt(2) == ':')
            {
                String minStr = data.substring(0, 2);
                String secStr = data.substring(3, 5);
                int minutes = minStr.toInt();
                int seconds = secStr.toInt();
                timer.startTimer(minutes, seconds);
                sendSuccessResponse(cmd);
            }
            else
            {
                sendErrorResponse(9002, "Invalid timer format. Use MM:SS");
            }
            break;
            
        case 2002: // Pause Timer
            timer.pauseTimer();
            sendSuccessResponse(cmd);
            break;
            
        case 2003: // Resume Timer
            timer.resumeTimer();
            sendSuccessResponse(cmd);
            break;
            
        case 2004: // Reset Timer
            timer.resetTimer();
            sendSuccessResponse(cmd);
            break;
            
        case 2005: // Stop Timer
            timer.stopTimer();
            sendSuccessResponse(cmd);
            break;
            
        case 2006: // Display Time of Day
            timer.displayTimeOfDay(true);
            sendSuccessResponse(cmd);
            break;
            
        // SETTINGS COMMANDS
        case 3001: // Set Brightness
            {
                int brightness = data.toInt();
                if (brightness >= 0 && brightness <= 255)
                {
                    display.setBrightness(brightness);
                    sendSuccessResponse(cmd);
                }
                else
                {
                    sendErrorResponse(9002, "Brightness must be 0-255");
                }
            }
            break;
            
        case 3002: // Set Top Text Color
            {
                uint32_t color = parseHexColor(data);
                if (color != 0xFFFFFFFF)
                {
                    display.setTopColour(color);
                    sendSuccessResponse(cmd);
                }
                else
                {
                    sendErrorResponse(9002, "Invalid color format. Use RRGGBB");
                }
            }
            break;
            
        case 3003: // Set Bottom Text Color
            {
                uint32_t color = parseHexColor(data);
                if (color != 0xFFFFFFFF)
                {
                    display.setBottomColour(color);
                    sendSuccessResponse(cmd);
                }
                else
                {
                    sendErrorResponse(9002, "Invalid color format. Use RRGGBB");
                }
            }
            break;
            
        case 3004: // Set Full Text Color
            {
                uint32_t color = parseHexColor(data);
                if (color != 0xFFFFFFFF)
                {
                    display.setFullColour(color);
                    sendSuccessResponse(cmd);
  }
  else
  {
                    sendErrorResponse(9002, "Invalid color format. Use RRGGBB");
                }
            }
            break;
            
        case 3005: // Set All Settings
            {
                // Stop any active scroll animation to allow settings to take effect
                display.stopScrollAnimation();
                
                // Stop timer so it doesn't overwrite display after settings change
                timer.displayTimeOfDay(false);
                if (timer.getTimerRunning() && !timer.getTimerPaused()) {
                    timer.pauseTimer();
                }
                
                // Safety: Validate data length first to prevent crashes
                if (data.length() < 4) {
                    sendErrorResponse(9002, "Invalid settings format");
                    break;
                }
                
                int comma1 = data.indexOf(',');
                int comma2 = data.indexOf(',', comma1 + 1);
                int comma3 = data.indexOf(',', comma2 + 1);
                
                if (comma1 != -1 && comma2 != -1 && comma3 != -1)
                {
                    String brightnessStr = data.substring(0, comma1);
                    String topColor = data.substring(comma1 + 1, comma2);
                    String bottomColor = data.substring(comma2 + 1, comma3);
                    String fullColor = data.substring(comma3 + 1);
                    
                    // Only set brightness if valid (non-empty and in range)
                    // Note: brightnessStr can be "0" which is valid, so check length > 0
                    if (brightnessStr.length() > 0)
                    {
                        int brightness = brightnessStr.toInt();
                        // Check if conversion was successful (toInt returns 0 for invalid, but 0 is valid brightness)
                        // So we check the string is numeric
                        bool validBrightness = true;
                        for (int i = 0; i < brightnessStr.length(); i++) {
                            if (!isDigit(brightnessStr.charAt(i))) {
                                validBrightness = false;
                                break;
                            }
                        }
                        if (validBrightness && brightness >= 0 && brightness <= 255)
                        {
                            display.setBrightness(brightness);
                            // Removed debug prints to prevent serial buffer overflow
                        }
                    }
                    
                    // Only set colors if valid (6-character hex strings)
                    if (topColor.length() == 6)
                    {
                        uint32_t color = parseHexColor(topColor);
                        if (color != 0xFFFFFFFF)
                        {
                            display.setTopColour(color);
                            // Removed debug prints to prevent serial buffer overflow
                        }
                    }
                    
                    if (bottomColor.length() == 6)
                    {
                        uint32_t color = parseHexColor(bottomColor);
                        if (color != 0xFFFFFFFF)
                        {
                            display.setBottomColour(color);
                            // Removed debug prints to prevent serial buffer overflow
                        }
                    }
                    
                    if (fullColor.length() == 6)
                    {
                        uint32_t color = parseHexColor(fullColor);
                        if (color != 0xFFFFFFFF)
                        {
                            display.setFullColour(color);
                            // Removed debug prints to prevent serial buffer overflow
                        }
                    }
                    
                    // Send success response immediately - critical for responsiveness
                    sendSuccessResponse(cmd);
                }
                else
                {
                    sendErrorResponse(9002, "Invalid settings format");
                }
            }
            break;
            
        // CUSTOM PIXEL COMMANDS
        case 4001: // Set Custom Pixel
            {
                int comma1 = data.indexOf(',');
                int comma2 = data.indexOf(',', comma1 + 1);
                
                if (comma1 != -1 && comma2 != -1)
                {
                    int x = data.substring(0, comma1).toInt();
                    int y = data.substring(comma1 + 1, comma2).toInt();
                    String colorStr = data.substring(comma2 + 1);
                    uint32_t color = parseHexColor(colorStr);
                    
                    if (x >= 0 && x < 60 && y >= 0 && y < 15 && color != 0xFFFFFFFF)
                    {
                        display.setPixel(x, y, color);
                        display.updateLEDs();
                        sendSuccessResponse(cmd);
                    }
                    else
                    {
                        sendErrorResponse(9002, "Invalid pixel coordinates or color");
                    }
                }
                else
                {
                    sendErrorResponse(9002, "Invalid pixel format. Use X,Y,RRGGBB");
                }
            }
            break;
            
        case 4002: // Clear All Pixels
            display.clearBuffer(true);
            display.updateLEDs();
            sendSuccessResponse(cmd);
            break;
            
        case 4003: // Clear Custom Pixel
            {
                int comma = data.indexOf(',');
                if (comma != -1)
                {
                    int x = data.substring(0, comma).toInt();
                    int y = data.substring(comma + 1).toInt();
                    
                    if (x >= 0 && x < 60 && y >= 0 && y < 15)
                    {
                        display.setPixel(x, y, 0x000000);
                        display.updateLEDs();
                        sendSuccessResponse(cmd);
                    }
                    else
                    {
                        sendErrorResponse(9002, "Invalid pixel coordinates");
                    }
  }
  else
  {
                    sendErrorResponse(9002, "Invalid pixel format. Use X,Y");
                }
            }
            break;
            
        case 4004: // Set Custom Pixel Row (Efficient batch mode)
            {
                // Format: row,col1,color1,col2,color2,col3,color3,...
                // Parse row first
                int firstComma = data.indexOf(',');
                if (firstComma == -1)
                {
                    sendErrorResponse(9002, "Invalid row format");
                    break;
                }
                
                int y = data.substring(0, firstComma).toInt();
                if (y < 0 || y >= 15)
                {
                    sendErrorResponse(9002, "Invalid row number");
                    break;
                }
                
                // Parse remaining data: col1,color1,col2,color2,...
                String remaining = data.substring(firstComma + 1);
                int pos = 0;
                int pixelsSet = 0;
                
                while (pos < remaining.length())
                {
                    // Find next comma (column number)
                    int colComma = remaining.indexOf(',', pos);
                    if (colComma == -1) break;
                    
                    int x = remaining.substring(pos, colComma).toInt();
                    if (x < 0 || x >= 60)
                    {
                        pos = colComma + 1;
                        continue; // Skip invalid column
                    }
                    
                    // Find next comma (end of color) or end of string
                    int nextComma = remaining.indexOf(',', colComma + 1);
                    String colorStr;
                    if (nextComma == -1)
                    {
                        // Last color in row
                        colorStr = remaining.substring(colComma + 1);
                        pos = remaining.length();
                    }
                    else
                    {
                        colorStr = remaining.substring(colComma + 1, nextComma);
                        pos = nextComma + 1;
                    }
                    
                    // Parse color (should be 6 hex digits)
                    if (colorStr.length() == 6)
                    {
                        uint32_t color = parseHexColor(colorStr);
                        if (color != 0xFFFFFFFF)
                        {
                            display.setPixel(x, y, color);
                            pixelsSet++;
                        }
                    }
                }
                
                // Update LEDs once after setting all pixels in the row
                display.updateLEDs();
                sendSuccessResponse(cmd);
            }
            break;
            
        // SYSTEM COMMANDS
        case 5001: // System Status Request
            sendSuccessResponse(cmd, "System OK");
            break;
            
        case 5002: // System Reset
            display.displayText("RESET", "", "static", "yes");
            delay(1000);
            sendSuccessResponse(cmd);
            break;
            
        case 5003: // Clear Display
            display.clearBuffer(true);
            display.updateLEDs();
            sendSuccessResponse(cmd);
            break;
            
        case 5004: // Default Message
            display.displayText("LED STRIPS", "SIGNBOARD", "static", "no");
            sendSuccessResponse(cmd);
            break;
            
        default:
            sendErrorResponse(9001, "Unknown command: " + cmd);
            break;
    }
>>>>>>> Stashed changes
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

<<<<<<< Updated upstream
  display.displayText("Done", "", "static", "yes");
=======
uint32_t parseHexColor(String colorStr)
{
    if (colorStr.length() == 6)
    {
        return (uint32_t)strtoul(colorStr.c_str(), NULL, 16);
    }
    return 0xFFFFFFFF; // Invalid color
}

void sendSuccessResponse(String cmd, String message = "OK")
{
    // Check Serial buffer space to prevent blocking/hanging
    // If buffer is more than 75% full, skip response to prevent blocking
    if (Serial.availableForWrite() < 20) {
        // Buffer nearly full, don't block - just return
        return;
    }
    
    // Use minimal string operations to prevent memory fragmentation
    Serial.write(PROTOCOL_START);
    Serial.print(cmd);
    Serial.print(message);
    Serial.write(PROTOCOL_END);
    Serial.write('\n');  // Ensure newline for response parsing
}

void sendErrorResponse(int errorCode, String message)
{
    // Check Serial buffer space to prevent blocking/hanging
    // If buffer is more than 75% full, skip response to prevent blocking
    if (Serial.availableForWrite() < 20) {
        // Buffer nearly full, don't block - just return
        return;
    }
    
    // Use minimal string operations to prevent memory fragmentation
    Serial.write(PROTOCOL_START);
    Serial.print(errorCode);
    Serial.print(message);
    Serial.write(PROTOCOL_END);
    Serial.write('\n');  // Ensure newline for response parsing
>>>>>>> Stashed changes
}
