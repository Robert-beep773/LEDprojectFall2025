# LED Strips Sign Board

## Overview
The LED Strips Sign Board is a **web and remote-controlled scoreboard** powered by an **Arduino Mega**. It features a 15×60 pixel LED matrix display that can show text messages, timers, custom pixel art, and various animations. The system is controlled through a modern web interface or via IR remote control.

## Key Features
- **Text Display** – Display static, scrolling (fast/slow), fade-in, and breathe animations with two font sizes (7×7 and 15×15)
- **Timer Functionality** – Countdown timer, stopwatch, and time-of-day display with full controls (start, pause, resume, reset, stop)
- **Custom Pixel Drawing** – Draw custom 15×60 pixel images directly on the display with optimized row-based updates
- **Web Interface** – Modern multi-page dashboard for easy control
- **IR Remote Control** – Wireless control via infrared remote
- **Color Customization** – Adjustable brightness (0-255) and RGB color settings for top, bottom, and full-screen text
- **Real-time Clock** – DS3231 RTC module for accurate timekeeping
- **Memory Optimized** – Carefully optimized for Arduino Mega's 8KB SRAM with stack allocation and bounded buffers

## System Architecture

### Hardware Components
- **Arduino Mega 2560** – Main microcontroller
- **15× NeoPixel LED Strips** – 60 pixels per strip (900 total pixels)
- **DS3231 RTC Module** – Real-time clock for timers
- **IR Receiver** – For remote control functionality
- **USB Serial Connection** – Communication with web server

### Software Components
- **Arduino Firmware** (`Board/MainBoard/`) – Core display and communication logic
- **PowerShell Web Server** (`Website/run.ps1`) – HTTP server and serial communication bridge
- **Web Interface** (`Website/Pages/`) – HTML/CSS/JavaScript frontend
- **Communication Protocol** – ASCII-based protocol (see `Communication_Protocol_Documentation.txt`)

## Installation

### Prerequisites
- Windows 10/11
- Arduino IDE (latest version)
- Arduino Mega 2560
- USB cable for Arduino connection
- Required Arduino Libraries:
  - Adafruit NeoPixel
  - RTClib
  - IRremote

### Arduino Setup
1. Install required libraries via Arduino IDE Library Manager
2. Navigate to `Board/MainBoard/`
3. Open **MainBoard.ino** in the Arduino IDE
4. Select **Tools > Board > Arduino Mega or Mega 2560**
5. Select the correct COM port under **Tools > Port**
6. Click **Upload** to compile and upload the firmware
7. Verify the display shows "LED STRIPS SIGNBOARD" on startup

### Website Setup
1. Navigate to the `Website` directory
2. Run **start.bat** (or manually run `run.ps1` in PowerShell)
3. When prompted:
   - Enter a server port (default: 8080)
   - Enter the COM port number (e.g., 3 for COM3)
   - Enter the baud rate (default: 9600, range: 1200-115200)
4. The server will connect to Arduino and send the baud rate
5. Open **Dashboard.html** in a web browser
6. The interface should connect automatically to the server

### Hardware Wiring
- **LED Strips**: Connect to digital pins 23-37 (see `Display.h` for pin mapping)
- **RTC Module**: Connect to I2C (SDA/SCL pins)
- **IR Receiver**: Connect to digital pin 10

## Usage Guide

### Text Display
1. Open the **Dashboard** page
2. Enter your message in the "Top" and/or "Below" fields
3. Select display mode:
   - **Top/Bottom**: Two-line display (small font)
   - **Full Screen**: Single-line display (large font)
4. Choose animation style:
   - **Static**: Fixed text display
   - **Scroll**: Continuous or stop-after-scroll (with fast/slow speed options)
   - **Fade In**: Text fades in gradually
   - **Breathe**: Text pulses with brightness
5. For scroll animations, select scroll speed (fast or slow)
6. Click **Display Message**

### Timer Functions
1. Open the **Timer** page
2. **Countdown Timer**:
   - Enter minutes and seconds
   - Click **Start** to begin countdown
   - Use **Pause/Resume** to control
   - Click **Reset** to return to initial time
3. **Time of Day**: Click to display current time

### Custom Pixel Drawing
1. Open the **Custom** page
2. Click pixels on the 15×60 grid to draw
3. Use the color picker to select colors
4. Click **Send to Display** to render on the LED matrix
5. The system uses optimized row-based updates for better performance

### Settings
1. Open the **Settings** page
2. **Brightness**: Adjust slider (0-100%, maps to 0-255)
3. **Colors**: Set RGB colors for:
   - Top text
   - Bottom text
   - Full screen text
4. Click **Save** to apply changes

## Communication Protocol

The system uses an ASCII-based protocol for Arduino communication:

**Format**: `[START][COMMAND][DATA][END]`
- START: ASCII 12 (0x0C)
- COMMAND: 4-digit code (1001-5004)
- DATA: Variable length message
- END: ASCII 15 (0x0F)

**Example**: `125001Hello15`
- `12`: Start marker
- `5001`: Command code
- `Hello`: Data
- `15`: End marker

See `Communication_Protocol_Documentation.txt` for complete command reference.

## Project Structure

```
LED-Strips-Signboard-main/
├── Board/
│   ├── MainBoard/          # Arduino firmware
│   │   ├── MainBoard.ino   # Main program entry point
│   │   ├── Display.h/cpp   # LED display control
│   │   ├── Timer.h/cpp     # Timer functionality
│   │   ├── Remote.h/cpp    # IR remote control
│   │   └── CharacterSet*   # Font definitions
│   └── App/                # Arduino reset utility
├── Website/
│   ├── Pages/              # HTML pages
│   │   ├── Dashboard.html  # Text input interface
│   │   ├── time.html       # Timer interface
│   │   ├── custom.html     # Pixel drawing interface
│   │   └── settings.html   # Settings interface
│   ├── scripts/            # JavaScript files
│   │   ├── Dashboard.js    # Main API functions
│   │   ├── custom.js       # Custom pixel handling
│   │   └── validator.js    # Input validation
│   ├── css/                # Stylesheets
│   ├── run.ps1             # PowerShell web server
│   └── start.bat           # Server launcher
├── Communication_Protocol_Documentation.txt
├── CHANGES_SUMMARY.txt
├── ARDUINO_MEMORY_OPTIMIZATIONS.txt
└── README.md
```

## Troubleshooting

### Arduino Not Responding
- Check USB cable connection
- Verify correct COM port in server
- Ensure baud rate matches (default: 9600, but configurable)
- Arduino waits for baud rate input on startup - ensure server sends it
- Try resetting Arduino (use ArduinoResetter.exe if needed)

### Web Interface Not Connecting
- Verify server is running (check PowerShell window)
- Check firewall settings for port 8080
- Try changing server port if 8080 is in use
- Ensure browser allows localhost connections

### Display Issues
- Verify all LED strips are connected correctly
- Check power supply (LEDs require significant current)
- Ensure pins match configuration in `Display.h`
- Test individual strips if some don't work

### Memory Errors
- Reduce text length:
  - Static/Fade/Breathe: max 30 chars (small font), 10 chars (large font)
  - Scroll: up to 120 chars supported
- Clear display before sending new commands
- Avoid rapid command sending (wait 200ms between commands)
- Use scroll animations for longer messages instead of static text

## Technical Specifications

- **Display Resolution**: 15 rows × 60 columns (900 pixels)
- **LED Type**: WS2812B NeoPixels (or compatible)
- **Communication**: Serial (9600 baud)
- **Protocol**: ASCII-based with start/end markers
- **Font Sizes**: 7×7 (small) and 15×15 (large)
- **Color Depth**: 24-bit RGB (8 bits per channel)
- **Brightness Range**: 0-255 (0-100% in UI)
- **Max Text Length**: 
  - Static/Fade/Breathe: 30 chars (small font), 10 chars (large font)
  - Scroll: 120 chars (both fonts)
- **Scroll Speed**: Fast (50ms per frame) or Slow (100ms per frame)
- **Buffer Sizes**: 150 chars (MAX_DATA_LENGTH), 100 chars (raw input)

## Development

### Adding New Commands
1. Add command code to `Communication_Protocol_Documentation.txt`
2. Update `ConvertTo-AsciiProtocol()` in `run.ps1`
3. Add handler in `MainBoard.ino` `processCommand()` function
4. Update web interface if needed

### Modifying Display Behavior
- Edit `Display.cpp` for display logic
- Modify `CharacterSet*.cpp` for font changes
- Update `Display.h` for configuration changes

## Platform Compatibility
- **Windows Only** – The PowerShell server requires Windows PowerShell
- **Arduino IDE** – Compatible with Arduino IDE 1.8.x and 2.x
- **Browsers** – Modern browsers (Chrome, Firefox, Edge, Safari)

## License
[Add your license information here]

## Authors
### Original Development Team:
- [@AhmadAzeez999](https://github.com/AhmadAzeez999) (Website, Webserver, & Webserver-to-Board Communication)
- [@WeziKaonga](https://github.com/WeziKaonga) (Remote Communication & Display)
- [@Cipher-935](https://github.com/Cipher-935) (Bluetooth Communication, Website Backend, & Webserver-to-Board Communication)
- [@Bhu77ar](https://github.com/Bhu77ar) (Font Design & Implementation)
- [@ChanndKaleka](https://github.com/ChanndKaleka) (Text Display & Font Implementation)

### Recent Improvements:
- [@Robert-beep773](https://github.com/Robert-beep773) (Code refactoring, documentation, protocol standardization, memory optimizations, fast scroll implementation, error handling improvements)

## Additional Documentation
- `Communication_Protocol_Documentation.txt` - Complete protocol reference
- `CHANGES_SUMMARY.txt` - Development history and changes
- `ARDUINO_MEMORY_OPTIMIZATIONS.txt` - Memory optimization details

