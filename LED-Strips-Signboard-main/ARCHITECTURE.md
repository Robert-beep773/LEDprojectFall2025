# System Architecture Documentation

## Overview

The LED Strips Signboard is a distributed system consisting of three main components:
1. **Arduino Firmware** - Controls the LED display hardware
2. **PowerShell Web Server** - Bridges HTTP and Serial communication
3. **Web Interface** - User-facing HTML/CSS/JavaScript application

## System Architecture Diagram

```
┌─────────────────┐
│   Web Browser   │
│  (HTML/CSS/JS)  │
└────────┬────────┘
         │ HTTP (JSON)
         │
┌────────▼─────────────────────┐
│   PowerShell Web Server      │
│   (run.ps1)                  │
│   - HTTP Listener            │
│   - JSON → ASCII Converter   │
│   - Serial Communication     │
└────────┬─────────────────────┘
         │ Serial (ASCII Protocol)
         │ 9600 baud
         │
┌────────▼─────────────────────┐
│      Arduino Mega 2560       │
│   - MainBoard.ino            │
│   - Display subsystem        │
│   - Timer subsystem          │
│   - Remote subsystem         │
└────────┬─────────────────────┘
         │
┌────────▼─────────────────────┐
│   Hardware Components         │
│   - 15× NeoPixel Strips       │
│   - DS3231 RTC Module         │
│   - IR Receiver               │
└───────────────────────────────┘
```

## Component Details

### 1. Arduino Firmware

**Location:** `Board/MainBoard/`

**Main Files:**
- `MainBoard.ino` - Entry point, command parsing, main loop
- `Display.h/cpp` - LED matrix control, text rendering, animations
- `Timer.h/cpp` - Countdown timer, stopwatch, time-of-day
- `Remote.h/cpp` - IR remote control handling
- `CharacterSet*.h/cpp` - Font definitions (7×7 and 15×15)

**Key Responsibilities:**
- Parse ASCII protocol commands from serial
- Render text on LED matrix with various animations
- Manage timer countdown and time display
- Handle IR remote button presses
- Manage display brightness and colors

**Design Patterns:**
- **Singleton Pattern**: `Display` class uses singleton to ensure single instance
- **State Machine**: Scroll animations use non-blocking state machine
- **Frame Buffer**: Double-buffered rendering for smooth updates

**Memory Management:**
- Stack allocation (no dynamic `new`/`delete`)
- Fixed-size buffers (100-150 chars max)
- Text length limits (30 chars small font, 10 chars large font)
- Reduced logging to save memory

### 2. PowerShell Web Server

**Location:** `Website/run.ps1`

**Key Responsibilities:**
- HTTP server (listens on configurable port, default 8080)
- JSON to ASCII protocol conversion
- Serial port communication (9600 baud)
- CORS header management
- Error handling and timeout management

**Protocol Conversion:**
The server converts user-friendly JSON commands to Arduino's ASCII protocol:

**JSON Format:**
```json
{
  "command": "static",
  "isBig": "yes",
  "data": "Hello"
}
```

**ASCII Protocol:**
```
[12][1002][Hello][15]
 ↑    ↑      ↑     ↑
START CMD   DATA  END
```

**Serial Settings:**
- Baud Rate: 9600 (fixed)
- Data Bits: 8
- Parity: None
- Stop Bits: 1
- Write Buffer: 65536 bytes
- Write Timeout: 1200ms

### 3. Web Interface

**Location:** `Website/Pages/` and `Website/scripts/`

**Pages:**
- `Dashboard.html` - Text input and display control
- `time.html` - Timer and time-of-day controls
- `custom.html` - Custom pixel drawing interface
- `settings.html` - Brightness and color settings

**JavaScript Modules:**
- `Dashboard.js` - Main API functions, text/timer/settings
- `custom.js` - Custom pixel drawing logic
- `validator.js` - Input validation

**Styling:**
- `style.css` - Main stylesheet
- `custom.css` - Custom page styles
- `time.css` - Timer page styles
- `settings.css` - Settings page styles

## Communication Flow

### Text Display Flow

1. **User Input** (Browser)
   - User enters text in Dashboard form
   - Selects animation type and font size
   - Clicks "Display Message"

2. **HTTP Request** (Browser → Server)
   ```javascript
   POST /dashboard/post
   {
     "command": "static",
     "isBig": "yes",
     "data": "Hello World"
   }
   ```

3. **Protocol Conversion** (Server)
   - Server receives JSON
   - Converts to ASCII: `121002Hello World15`
   - Validates format

4. **Serial Transmission** (Server → Arduino)
   - Writes ASCII string to serial port
   - Adds newline terminator
   - Handles timeout errors

5. **Command Parsing** (Arduino)
   - Reads serial input until newline
   - Validates protocol markers (START/END)
   - Extracts command code (1002)
   - Extracts data ("Hello World")

6. **Display Rendering** (Arduino)
   - Calls `Display::displayText()`
   - Renders text on LED matrix
   - Updates frame buffer
   - Displays on LEDs

### Timer Flow

1. **User Input** (Browser)
   - User enters minutes/seconds
   - Clicks "Start"

2. **HTTP Request**
   ```json
   {
     "command": "sTimer",
     "data": "05:30"
   }
   ```

3. **Protocol Conversion**
   - Converts to: `12200105:3015`

4. **Arduino Processing**
   - Parses MM:SS format
   - Starts countdown timer
   - Updates display every second
   - Handles pause/resume/reset

## Data Structures

### Display Frame Buffer

```cpp
uint32_t frameBuffer[NUM_STRIPS][NUMPIXELS];
// NUM_STRIPS = 15 (rows)
// NUMPIXELS = 60 (columns per row)
// Each pixel: 32-bit RGB color (0xRRGGBB)
```

### Scroll State Machine

```cpp
struct ScrollState {
    bool isActive;
    bool useBigFont;
    unsigned long previousMillis;
    unsigned long shift;
    int scrollSpeed;
    int totalWidth;
    char text1Copy[121];
    char text2Copy[121];
};
```

### Timer State

```cpp
class Timer {
    bool timerRunning;
    bool timerPaused;
    int minutes;
    int seconds;
    DateTime startTime;
    // ...
};
```

## Protocol Specification

### ASCII Protocol Format

**Structure:**
```
[START][COMMAND][DATA][END]
```

**Components:**
- **START**: ASCII 12 (0x0C) - Single byte
- **COMMAND**: 4-digit ASCII code (e.g., "1001")
- **DATA**: Variable length string (0-150 chars)
- **END**: ASCII 15 (0x0F) - Single byte

**Example:**
```
12 1001 Hello,World 15
│  │    │           │
│  │    └─ Data
│  └────── Command
└───────── START
           └──────── END
```

### Command Categories

1. **Text Display (1001-1010)**
   - Static, scroll, fade, breathe animations
   - Small (7×7) and large (15×15) fonts

2. **Timer (2001-2006)**
   - Start, pause, resume, reset
   - Time-of-day display

3. **Settings (3001-3005)**
   - Brightness (0-255)
   - Colors (RRGGBB hex format)

4. **Custom Pixels (4001-4004)**
   - Individual pixel control
   - Row-based updates

5. **System (5001-5004)**
   - Status, reset, clear, default message

6. **Errors (9001-9005)**
   - Invalid command, format, buffer overflow, etc.

## Hardware Configuration

### LED Matrix Layout

```
    0    1    2  ...  59  (Columns)
0  [ ]  [ ]  [ ]  ... [ ]
1  [ ]  [ ]  [ ]  ... [ ]
2  [ ]  [ ]  [ ]  ... [ ]
...
14 [ ]  [ ]  [ ]  ... [ ]
(Rows)
```

**Physical Connection:**
- 15 NeoPixel strips (WS2812B or compatible)
- 60 pixels per strip
- Connected to Arduino digital pins 23-37
- Requires external 5V power supply (LEDs draw significant current)

### Pin Assignments

| Component | Pin(s) | Type |
|-----------|--------|------|
| LED Strips | 23-37 | Digital Output |
| IR Receiver | 10 | Digital Input |
| RTC Module | SDA/SCL | I2C |

### Power Requirements

- **Arduino Mega**: USB or 7-12V DC
- **LED Strips**: 5V DC (external supply recommended)
- **Current Draw**: ~18A at full brightness (all white, 255 brightness)
- **Recommended**: 20A+ 5V power supply

## Memory Constraints

### Arduino Mega 2560 Specifications
- **Flash**: 256 KB
- **SRAM**: 8 KB
- **EEPROM**: 4 KB

### Memory Usage

**SRAM Usage:**
- Frame buffer: ~3.6 KB (15×60×4 bytes)
- Scroll state: ~250 bytes
- Serial buffer: ~100 bytes
- Stack: ~1-2 KB
- **Total**: ~5-6 KB (within 8 KB limit)

**Optimization Strategies:**
- Stack allocation (no heap)
- Fixed-size buffers
- Text length limits
- Minimal logging
- Reuse buffers

## Error Handling

### Server-Side Errors

1. **Port Already in Use**
   - Server prompts for different port
   - Checks port availability before binding

2. **Serial Timeout**
   - Logs error message
   - Discards output buffer
   - Continues operation
   - Prevents system hang

3. **Invalid JSON**
   - Request ignored
   - No response sent
   - Logs to console

### Arduino-Side Errors

1. **Invalid Protocol Format**
   - Sends error code 9001
   - Logs "Invalid format"
   - Continues listening

2. **Buffer Overflow**
   - Sends error code 9003
   - Clears buffer
   - Prevents crash

3. **Invalid Command**
   - Sends error code 9001
   - Logs command code
   - Continues operation

## Performance Characteristics

### Display Update Rate
- **Scroll Animation**: ~10 FPS (100ms per frame)
- **Fade Animation**: Variable (depends on fade duration)
- **Static Display**: Instant
- **Timer Update**: 1 Hz (once per second)

### Serial Communication
- **Baud Rate**: 9600 bps
- **Command Latency**: ~50-200ms (depending on data length)
- **Throughput**: ~960 bytes/second theoretical

### Web Interface
- **Response Time**: <100ms (local server)
- **Update Frequency**: On-demand (user actions)

## Security Considerations

### Current Implementation
- **Local Only**: Server binds to 127.0.0.1 (localhost)
- **No Authentication**: Open access to anyone on local machine
- **No Encryption**: HTTP only (not HTTPS)

### Recommendations for Production
- Add authentication (API keys or user login)
- Use HTTPS for encrypted communication
- Restrict server binding to specific IP addresses
- Add rate limiting to prevent command flooding
- Validate all user inputs server-side

## Extension Points

### Adding New Commands

1. **Define Command Code**
   - Add to `Communication_Protocol_Documentation.txt`
   - Choose unused code in appropriate range

2. **Update Server** (`run.ps1`)
   - Add case to `ConvertTo-AsciiProtocol()` function
   - Map JSON command to ASCII protocol

3. **Update Arduino** (`MainBoard.ino`)
   - Add case to `processCommand()` switch statement
   - Implement command handler

4. **Update Web Interface** (if needed)
   - Add UI controls
   - Add JavaScript function
   - Call API endpoint

### Adding New Animations

1. **Update Display Class** (`Display.cpp`)
   - Add animation method (e.g., `waveText()`)
   - Implement frame-by-frame logic
   - Add to `displayText()` switch

2. **Update Protocol**
   - Assign command code
   - Document in protocol docs

3. **Update Web Interface**
   - Add animation option to UI
   - Update JavaScript command mapping

## Testing Strategy

### Unit Testing
- Command parsing validation
- Protocol conversion accuracy
- Buffer overflow protection
- Error handling paths

### Integration Testing
- End-to-end command flow
- Serial communication reliability
- Display rendering accuracy
- Timer accuracy

### Hardware Testing
- LED strip functionality
- Power supply adequacy
- Signal integrity
- Temperature management

## Future Enhancements

### Potential Improvements
1. **Network Support**: WiFi/Ethernet module for remote access
2. **Preset Management**: Save/load display configurations
3. **Scheduling**: Time-based automatic displays
4. **Multi-Client**: Support multiple simultaneous connections
5. **Logging**: Persistent command/error logs
6. **Backup Display**: Secondary display for redundancy
7. **Sensor Integration**: Temperature, humidity displays
8. **Mobile App**: Native mobile application

## Dependencies

### Arduino Libraries
- `Adafruit_NeoPixel` - LED strip control
- `RTClib` - Real-time clock
- `IRremote` - IR receiver
- `Wire` - I2C communication

### System Requirements
- Windows 10/11
- PowerShell 5.1+
- Arduino IDE 1.8.x or 2.x
- Modern web browser

## See Also

- `README.md` - Project overview and setup
- `API_DOCUMENTATION.md` - Web API reference
- `Communication_Protocol_Documentation.txt` - Protocol specification
- `CHANGES_SUMMARY.txt` - Development history

