/**
 * LED Strips Signboard - Main Board Controller
 * 
 * This is the main Arduino sketch for the LED signboard system.
 * It handles serial communication, command processing, display management,
 * timer functionality, and remote control integration.
 * 
 * Communication Protocol:
 * - Uses ASCII protocol with start/end markers (0x0C and 0x0F)
 * - Format: <START><COMMAND><DATA><END>
 * - Commands are 4-digit numeric codes (1001-5004)
 * 
 * @author Original team + Refactored by Robert-beep773
 * @version 2.0
 */

#include "Display.h"
#include "Timer.h"
#include <Wire.h>
#include <RTClib.h>
#include "Remote.h"

// ============================================================================
// GLOBAL OBJECTS AND INSTANCES
// ============================================================================

RTC_DS3231 rtc;              // Real-Time Clock instance for timekeeping
bool useBigFont = true;      // Toggle between 7x7 (small) and 15x15 (large) font sizes
Timer timer;                 // Timer subsystem for countdown and time display
RemoteControl remote;        // Remote control subsystem for wireless communication
Display& display = Display::getInstance();  // Display singleton instance

// ============================================================================
// SERIAL COMMUNICATION PROTOCOL CONSTANTS
// ============================================================================

const char PROTOCOL_START = 12;    // 0x0C - Start of message marker
const char PROTOCOL_END = 15;      // 0x0F - End of message marker
const int COMMAND_LENGTH = 4;      // Length of command code (e.g., "1001")
const int MAX_DATA_LENGTH = 150;   // Maximum data length (supports 120 chars + overhead)

// ============================================================================
// MESSAGE PARSING VARIABLES
// ============================================================================

String command = "";         // Extracted command code from received message
String message = "";         // Parsed message text (legacy, may be unused)
String message2 = "";        // Second line of message (legacy, may be unused)
int messageSize = 0;         // Size of received message

// Legacy variables for direct input (may be unused in current implementation)
int intByte;
const uint8_t numRawChar = 100;  // Buffer size for raw character input
char msgRaw[numRawChar];
uint8_t charCount = 0;
bool dataToSend = false;

// ============================================================================
// BAUD RATE CONFIGURATION
// ============================================================================

unsigned long currentBaud = 9600UL;  // Current serial baud rate (starts at 9600, waits for input)
String baudInput = "";                // Buffer for baud rate input

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

void parseInput(String input);
void processCommand(String cmd, String data);
void displayText(String text1, String text2, String command, String displayType, int scrollSpeed = 50);
uint32_t parseHexColor(String colorStr);
void sendSuccessResponse(String cmd, String message = "OK");
void sendErrorResponse(int errorCode, String message);

// ============================================================================
// ARDUINO SETUP FUNCTION
// ============================================================================

/**
 * Initialization function called once on Arduino boot.
 * Waits for baud rate input from serial connection before booting.
 * Once baud rate is received, switches to that rate and displays it, then continues boot.
 */
void setup()
{
  // Initialize display first (needed to show waiting/baud rate)
  display.setup(7);
  
  // Start serial at default rate (9600) to receive baud rate input
  Serial.begin(9600);
  delay(200);  // Give serial time to initialize

  // Wait indefinitely for baud rate input
  // Sign will not boot until it receives a valid baud rate
  baudInput = "";
  bool baudReceived = false;
  
  while (!baudReceived)
  {
    if (Serial.available())
    {
      char c = Serial.read();
      
      // Enter or newline ends input
      if (c == '\n' || c == '\r')
      {
        if (baudInput.length() > 0)
        {
          unsigned long requestedBaud = baudInput.toInt();
          
          // Validate range (common Arduino baud rates)
          if (requestedBaud >= 1200UL && requestedBaud <= 115200UL)
          {
            currentBaud = requestedBaud;
            baudReceived = true;
          }
          else
          {
            // Invalid baud rate, clear and wait for new input
            baudInput = "";
          }
        }
      }
      // Only accept digits
      else if (c >= '0' && c <= '9')
      {
        baudInput += c;
      }
    }
  }

  // Restart serial at the new baud rate
  Serial.flush();
  delay(10);
  Serial.end();
  delay(10);
  Serial.begin(currentBaud);
  delay(50);  // Allow serial to stabilize

  // Initialize other subsystems now that baud rate is set
  timer.setupRTC();        // Setup Real-Time Clock
  remote.setupRemote();   // Setup remote control receiver

  // Display the new baud rate on the sign
  // Top line: "TEST BAUD", Bottom line: actual baud rate number
  char baudStr[16];
  sprintf(baudStr, "%lu", currentBaud);
  display.displayText("TEST BAUD", baudStr, "static", "no");
  delay(3000);  // Show for 3 seconds

  // Display boot message
  display.displayText("LED STRIPS", "SIGNBOARD", "static", "no");
  
  // Confirm baud rate over serial
  Serial.print(F("Baud rate set to: "));
  Serial.println(currentBaud);
}

// ============================================================================
// ARDUINO MAIN LOOP
// ============================================================================

/**
 * Main program loop - runs continuously after setup().
 * Handles serial communication, display updates, and subsystem maintenance.
 * All operations are non-blocking to maintain responsiveness.
 */
void loop()
{
  // Priority 1: Check for incoming serial commands
  // This must be checked first to maintain responsiveness
  if (Serial.available())
  {
    String input = Serial.readStringUntil('\n');  // Read complete line
    parseInput(input);
  }

  // Priority 2: Update display animations (non-blocking scroll, etc.)
  display.updateDisplay();
  
  // Priority 3: Update other subsystems
  remote.useRemote();   // Check for remote control input
  timer.updateTimer();  // Update timer countdown/display
}

// ============================================================================
// SERIAL COMMUNICATION PARSING
// ============================================================================

/**
 * Parses incoming serial input according to the ASCII protocol.
 * Protocol format: <START><COMMAND><DATA><END>
 * 
 * @param input The raw string received from serial port
 */
void parseInput(String input)
{
    // Minimal logging to save memory
    Serial.println("RX");
    
    // Stop time display if active (to show new command)
    timer.displayTimeOfDay(false);

    // Validation 1: Check message length to prevent crashes
    // Minimum: <START><4-char-command><END> = 6 characters
    // Maximum: <START><4-char-command><150-char-data><END> = 156 characters
    if (input.length() < 6 || input.length() > MAX_DATA_LENGTH + 6)
    {
        sendErrorResponse(9001, "Invalid length");
        return;
    }

    // Validation 2: Check protocol start/end markers
    if (input.charAt(0) != PROTOCOL_START || input.charAt(input.length() - 1) != PROTOCOL_END)
    {
        sendErrorResponse(9001, "Invalid format");
        return;
    }
    
    // Extract 4-character command code (positions 1-4)
    command = input.substring(1, COMMAND_LENGTH + 1);
    
    // Extract data payload (everything between command and end marker)
    String data = "";
    if (input.length() > COMMAND_LENGTH + 2)
    {
        data = input.substring(COMMAND_LENGTH + 1, input.length() - 1);
    }

    // Process the parsed command
    processCommand(command, data);
}

// ============================================================================
// COMMAND PROCESSOR
// ============================================================================

/**
 * Processes commands received via serial communication.
 * 
 * Command Categories:
 * - 1001-1010: Text Display Commands (static, scroll, fade, breathe)
 * - 2001-2006: Timer Commands (start, pause, resume, reset, stop, time display)
 * - 3001-3005: Settings Commands (brightness, colors, all settings)
 * - 4001-4004: Custom Pixel Commands (set pixel, clear, custom drawings)
 * - 5001-5004: System Commands (status, reset, clear, default message)
 * 
 * @param cmd 4-character command code (e.g., "1001")
 * @param data Command data payload (format depends on command)
 */
void processCommand(String cmd, String data)
{
    int cmdCode = cmd.toInt();
    
    switch (cmdCode)
    {
        // ====================================================================
        // TEXT DISPLAY COMMANDS (1001-1010)
        // ====================================================================
        // Memory optimized - uses chunked display for large text
        case 1001: // Static Text (Small Font) - Split by comma for top/bottom rows
            {
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
            if (data.length() > 10) data = data.substring(0, 10); // Limit for large font
            displayText(data, "", "static", "yes");
            sendSuccessResponse(cmd);
            break;
            
        case 1003: // Scroll Text Continuous (Small Font) - SLOW
            {
                int commaIndex = data.indexOf(',');
                String text1 = (commaIndex > 0) ? data.substring(0, commaIndex) : data;
                String text2 = (commaIndex > 0 && commaIndex < data.length() - 1) ? data.substring(commaIndex + 1) : "";
                if (text1.length() > 120) text1 = text1.substring(0, 120);
                if (text2.length() > 120) text2 = text2.substring(0, 120);
                displayText(text1, text2, "scrolC", "no", 150); // Slow = 150ms
                sendSuccessResponse(cmd);
            }
            break;
            
        case 1004: // Scroll Text Continuous (Large Font) - SLOW
            if (data.length() > 120) data = data.substring(0, 120);
            displayText(data, "", "scrolC", "yes", 150); // Slow = 150ms
            sendSuccessResponse(cmd);
            break;
            
        case 1005: // Scroll Text and Stop (Small Font) - SLOW
            {
                int commaIndex = data.indexOf(',');
                String text1 = (commaIndex > 0) ? data.substring(0, commaIndex) : data;
                String text2 = (commaIndex > 0 && commaIndex < data.length() - 1) ? data.substring(commaIndex + 1) : "";
                if (text1.length() > 120) text1 = text1.substring(0, 120);
                if (text2.length() > 120) text2 = text2.substring(0, 120);
                displayText(text1, text2, "scrolS", "no", 150); // Slow = 150ms
                sendSuccessResponse(cmd);
            }
            break;
            
        case 1006: // Scroll Text and Stop (Large Font) - SLOW
            if (data.length() > 120) data = data.substring(0, 120);
            displayText(data, "", "scrolS", "yes", 150); // Slow = 150ms
            sendSuccessResponse(cmd);
            break;
            
        case 1013: // Scroll Text Continuous (Small Font) - FAST
            {
                int commaIndex = data.indexOf(',');
                String text1 = (commaIndex > 0) ? data.substring(0, commaIndex) : data;
                String text2 = (commaIndex > 0 && commaIndex < data.length() - 1) ? data.substring(commaIndex + 1) : "";
                if (text1.length() > 120) text1 = text1.substring(0, 120);
                if (text2.length() > 120) text2 = text2.substring(0, 120);
                displayText(text1, text2, "scrolC", "no", 50); // Fast = 50ms
                sendSuccessResponse(cmd);
            }
            break;
            
        case 1014: // Scroll Text Continuous (Large Font) - FAST
            if (data.length() > 120) data = data.substring(0, 120);
            displayText(data, "", "scrolC", "yes", 50); // Fast = 50ms
            sendSuccessResponse(cmd);
            break;
            
        case 1015: // Scroll Text and Stop (Small Font) - FAST
            {
                int commaIndex = data.indexOf(',');
                String text1 = (commaIndex > 0) ? data.substring(0, commaIndex) : data;
                String text2 = (commaIndex > 0 && commaIndex < data.length() - 1) ? data.substring(commaIndex + 1) : "";
                if (text1.length() > 120) text1 = text1.substring(0, 120);
                if (text2.length() > 120) text2 = text2.substring(0, 120);
                displayText(text1, text2, "scrolS", "no", 50); // Fast = 50ms
                sendSuccessResponse(cmd);
            }
            break;
            
        case 1016: // Scroll Text and Stop (Large Font) - FAST
            if (data.length() > 120) data = data.substring(0, 120);
            displayText(data, "", "scrolS", "yes", 50); // Fast = 50ms
            sendSuccessResponse(cmd);
            break;
            
        case 1007: // Fade In Text (Small Font) - Split by comma for top/bottom rows
            {
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
            if (data.length() > 10) data = data.substring(0, 10);
            displayText(data, "", "fadeIn", "yes");
            sendSuccessResponse(cmd);
            break;
            
        case 1009: // Breathe Text (Small Font) - Split by comma for top/bottom rows
            {
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
            if (data.length() > 10) data = data.substring(0, 10);
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
                int comma1 = data.indexOf(',');
                int comma2 = data.indexOf(',', comma1 + 1);
                int comma3 = data.indexOf(',', comma2 + 1);
                
                if (comma1 != -1 && comma2 != -1 && comma3 != -1)
                {
                    String brightnessStr = data.substring(0, comma1);
                    String topColor = data.substring(comma1 + 1, comma2);
                    String bottomColor = data.substring(comma2 + 1, comma3);
                    String fullColor = data.substring(comma3 + 1);
                    
                    // Only set brightness if provided (not empty)
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
                        }
                    }
                    
                    // Only set colors if valid (6-character hex strings)
                    if (topColor.length() == 6)
                    {
                        uint32_t color = parseHexColor(topColor);
                        if (color != 0xFFFFFFFF)
                        {
                            display.setTopColour(color);
                        }
                    }
                    
                    if (bottomColor.length() == 6)
                    {
                        uint32_t color = parseHexColor(bottomColor);
                        if (color != 0xFFFFFFFF)
                        {
                            display.setBottomColour(color);
                        }
                    }
                    
                    if (fullColor.length() == 6)
                    {
                        uint32_t color = parseHexColor(fullColor);
                        if (color != 0xFFFFFFFF)
                        {
                            display.setFullColour(color);
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
                // Stop any active scroll animation first
                display.stopScrollAnimation();
                
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
                        // Don't call updateLEDs() here - it blocks the main loop
                        // The display will update naturally through the main loop
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
            // Stop any active scroll animation first
            display.stopScrollAnimation();
            delay(10);  // Small delay to ensure cleanup
            display.clearBuffer(true);
            display.updateLEDs();
            sendSuccessResponse(cmd);
            break;
            
        case 4003: // Clear Custom Pixel
            {
                // Stop any active scroll animation first
                display.stopScrollAnimation();
                
                int comma = data.indexOf(',');
                if (comma != -1)
                {
                    int x = data.substring(0, comma).toInt();
                    int y = data.substring(comma + 1).toInt();
                    
                    if (x >= 0 && x < 60 && y >= 0 && y < 15)
                    {
                        display.setPixel(x, y, 0x000000);
                        // Don't call updateLEDs() here - it blocks the main loop
                        // The display will update naturally through the main loop
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
            
        case 4004: // Set Custom Pixel Row (HEAVILY OPTIMIZED - zero String operations)
            {
                // Stop any active scroll animation first
                display.stopScrollAnimation();
                
                // Get pointer to data buffer for direct parsing
                const char* ptr = data.c_str();
                if (!ptr || *ptr == '\0') {
                    sendErrorResponse(9002, "Empty data");
                    break;
                }
                
                // Parse row number (fast integer parsing - no String operations)
                int y = 0;
                while (*ptr >= '0' && *ptr <= '9') {
                    y = y * 10 + (*ptr - '0');
                    ptr++;
                }
                
                if (*ptr != ',' || y < 0 || y >= 15) {
                    sendErrorResponse(9002, "Invalid row");
                    break;
                }
                ptr++; // Skip comma
                
                // Parse pixel pairs: col,color,col,color,...
                // Process up to 30 pixels per chunk (safety limit)
                int pixelsSet = 0;
                const int MAX_PIXELS = 30;
                
                while (*ptr != '\0' && pixelsSet < MAX_PIXELS)
                {
                    // Fast parse column number
                    int x = 0;
                    const char* xStart = ptr;
                    while (*ptr >= '0' && *ptr <= '9') {
                        x = x * 10 + (*ptr - '0');
                        ptr++;
                    }
                    
                    // Validate column and check for comma
                    if (*ptr != ',' || x < 0 || x >= 60) {
                        // Invalid column - skip to next pair
                        while (*ptr != '\0' && *ptr != ',') ptr++;
                        if (*ptr == ',') ptr++;
                        continue;
                    }
                    ptr++; // Skip comma
                    
                    // Fast hex color parsing (6 digits) - no strlen, direct check
                    if (ptr[0] == '\0' || ptr[1] == '\0' || ptr[2] == '\0' || 
                        ptr[3] == '\0' || ptr[4] == '\0' || ptr[5] == '\0') {
                        break; // Not enough characters for color
                    }
                    
                    // Parse 6 hex digits directly
                    uint32_t color = 0;
                    bool valid = true;
                    for (int i = 0; i < 6; i++) {
                        char c = ptr[i];
                        uint8_t val;
                        if (c >= '0' && c <= '9') val = c - '0';
                        else if (c >= 'A' && c <= 'F') val = c - 'A' + 10;
                        else if (c >= 'a' && c <= 'f') val = c - 'a' + 10;
                        else { valid = false; break; }
                        color = (color << 4) | val;
                    }
                    
                    if (valid) {
                        display.setPixel(x, y, color);
                        pixelsSet++;
                    }
                    
                    // Move to next pair (skip 6 hex digits + optional comma)
                    ptr += 6;
                    if (*ptr == ',') ptr++;
                }
                
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
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Converts String parameters to C-style char arrays and calls display function.
 * This wrapper is necessary because the Display class uses char* instead of String
 * to save memory on Arduino.
 * 
 * @param text1 First line of text (top row for small font, or full text for large font)
 * @param text2 Second line of text (bottom row for small font, empty for large font)
 * @param command Display command: "static", "scrolC", "scrolS", "fadeIn", "breath"
 * @param displayType "yes" for large font (15x15), "no" for small font (7x7)
 */

void displayText(String text1, String text2, String command, String displayType, int scrollSpeed)
{
    char currentCommand[command.length() + 1];
    command.toCharArray(currentCommand, command.length() + 1);
    
    char currentDisplayType[displayType.length() + 1];
    displayType.toCharArray(currentDisplayType, displayType.length() + 1);
    
    char currentMessage[text1.length() + 1];
    text1.toCharArray(currentMessage, text1.length() + 1);
    
    char currentMessage2[text2.length() + 1];
    text2.toCharArray(currentMessage2, text2.length() + 1);
    
    display.displayText(currentMessage, currentMessage2, currentCommand, currentDisplayType, scrollSpeed);
}

/**
 * Parses a hexadecimal color string to a 32-bit color value.
 * 
 * @param colorStr 6-character hex string (e.g., "FF0000" for red)
 * @return 32-bit color value, or 0xFFFFFFFF if invalid
 */
uint32_t parseHexColor(String colorStr)
{
    if (colorStr.length() == 6)
    {
        return (uint32_t)strtoul(colorStr.c_str(), NULL, 16);
    }
    return 0xFFFFFFFF; // Invalid color indicator
}

/**
 * Sends a success response back to the client.
 * Format: <START><COMMAND><MESSAGE><END>
 * 
 * @param cmd The original command code
 * @param message Optional success message (defaults to "OK")
 */
void sendSuccessResponse(String cmd, String message = "OK")
{
    String response = String(PROTOCOL_START) + cmd + message + String(PROTOCOL_END);
    Serial.println(response);
}

/**
 * Sends an error response back to the client.
 * Format: <START><ERROR_CODE><MESSAGE><END>
 * 
 * @param errorCode Numeric error code (e.g., 9001, 9002)
 * @param message Error description message
 */
void sendErrorResponse(int errorCode, String message)
{
    String response = String(PROTOCOL_START) + String(errorCode) + message + String(PROTOCOL_END);
    Serial.println(response);
}
