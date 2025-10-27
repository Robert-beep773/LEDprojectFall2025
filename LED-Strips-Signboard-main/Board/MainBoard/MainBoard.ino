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
String message = "";
String message2 = "";
int messageSize = 0;

// Variable for receiving input directly
int intByte;
const uint8_t numRawChar = 100;  // Reduced buffer size for stability
char msgRaw[numRawChar];
uint8_t charCount = 0;
bool dataToSend = false;

// ASCII Protocol Constants
const char PROTOCOL_START = 12;  // 0x0C
const char PROTOCOL_END = 15;    // 0x0F
const int COMMAND_LENGTH = 4;
const int MAX_DATA_LENGTH = 50;  // Reduced for stability

Display& display = Display::getInstance();
//Display d;

// Function declarations
void parseInput(String input);
void processCommand(String cmd, String data);
void displayText(String text1, String text2, String command, String displayType);
uint32_t parseHexColor(String colorStr);
void sendSuccessResponse(String cmd, String message = "OK");
void sendErrorResponse(int errorCode, String message);

void setup()
{
  Serial.begin(9600);  // Fixed baud rate for stability
  timer.setupRTC();
  remote.setupRemote();
  display.setup(7);

  display.displayText("LED STRIPS", "SIGNBOARD", "static", "no");
}

void loop()
{
  if (Serial.available())
  {
    String input = Serial.readStringUntil('\n');  // Read until newline
    parseInput(input);
  }

  remote.useRemote();
  timer.updateTimer();
}

void parseInput(String input)
{
    // Minimal logging to save memory
    Serial.println("RX");
    
    timer.displayTimeOfDay(false); // In case the time of day is being displayed

    // Quick validation - check length first to avoid crashes
    if (input.length() < 6 || input.length() > MAX_DATA_LENGTH + 6)
    {
        sendErrorResponse(9001, "Invalid length");
        return;
    }

    // Check protocol markers
    if (input.charAt(0) != PROTOCOL_START || input.charAt(input.length() - 1) != PROTOCOL_END)
    {
        sendErrorResponse(9001, "Invalid format");
        return;
    }
    
    // Extract command safely
    command = input.substring(1, COMMAND_LENGTH + 1);
    
    // Extract data safely
    String data = "";
    if (input.length() > COMMAND_LENGTH + 2)
    {
        data = input.substring(COMMAND_LENGTH + 1, input.length() - 1);
    }

    // Process command
    processCommand(command, data);
}

void processCommand(String cmd, String data)
{
    int cmdCode = cmd.toInt();
    
    switch (cmdCode)
    {
        // TEXT DISPLAY COMMANDS - Memory optimized
        case 1001: // Static Text (Small Font)
            if (data.length() > 30) data = data.substring(0, 30); // Limit length
            displayText(data, "", "static", "no");
            sendSuccessResponse(cmd);
            break;
            
        case 1002: // Static Text (Large Font)
            if (data.length() > 10) data = data.substring(0, 10); // Limit for large font
            displayText(data, "", "static", "yes");
            sendSuccessResponse(cmd);
            break;
            
        case 1003: // Scroll Text Continuous (Small Font)
            if (data.length() > 30) data = data.substring(0, 30);
            displayText(data, "", "scrolC", "no");
            sendSuccessResponse(cmd);
            break;
            
        case 1004: // Scroll Text Continuous (Large Font)
            if (data.length() > 10) data = data.substring(0, 10);
            displayText(data, "", "scrolC", "yes");
            sendSuccessResponse(cmd);
            break;
            
        case 1005: // Scroll Text and Stop (Small Font)
            if (data.length() > 30) data = data.substring(0, 30);
            displayText(data, "", "scrolS", "no");
            sendSuccessResponse(cmd);
            break;
            
        case 1006: // Scroll Text and Stop (Large Font)
            if (data.length() > 10) data = data.substring(0, 10);
            displayText(data, "", "scrolS", "yes");
            sendSuccessResponse(cmd);
            break;
            
        case 1007: // Fade In Text (Small Font)
            if (data.length() > 30) data = data.substring(0, 30);
            displayText(data, "", "fadeIn", "no");
            sendSuccessResponse(cmd);
            break;
            
        case 1008: // Fade In Text (Large Font)
            if (data.length() > 10) data = data.substring(0, 10);
            displayText(data, "", "fadeIn", "yes");
            sendSuccessResponse(cmd);
            break;
            
        case 1009: // Breathe Text (Small Font)
            if (data.length() > 30) data = data.substring(0, 30);
            displayText(data, "", "breath", "no");
            sendSuccessResponse(cmd);
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
                    int brightness = data.substring(0, comma1).toInt();
                    String topColor = data.substring(comma1 + 1, comma2);
                    String bottomColor = data.substring(comma2 + 1, comma3);
                    String fullColor = data.substring(comma3 + 1);
                    
                    if (brightness >= 0 && brightness <= 255)
                    {
                        display.setBrightness(brightness);
                        display.setTopColour(parseHexColor(topColor));
                        display.setBottomColour(parseHexColor(bottomColor));
                        display.setFullColour(parseHexColor(fullColor));
                        sendSuccessResponse(cmd);
                    }
                    else
                    {
                        sendErrorResponse(9002, "Invalid brightness value");
                    }
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

void displayText(String text1, String text2, String command, String displayType)
{
    char currentCommand[command.length() + 1];
    command.toCharArray(currentCommand, command.length() + 1);
    
    char currentDisplayType[displayType.length() + 1];
    displayType.toCharArray(currentDisplayType, displayType.length() + 1);
    
    char currentMessage[text1.length() + 1];
    text1.toCharArray(currentMessage, text1.length() + 1);
    
    char currentMessage2[text2.length() + 1];
    text2.toCharArray(currentMessage2, text2.length() + 1);
    
    display.displayText(currentMessage, currentMessage2, currentCommand, currentDisplayType);
}

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
    String response = String(PROTOCOL_START) + cmd + message + String(PROTOCOL_END);
    Serial.println(response);
}

void sendErrorResponse(int errorCode, String message)
{
    String response = String(PROTOCOL_START) + String(errorCode) + message + String(PROTOCOL_END);
    Serial.println(response);
}
