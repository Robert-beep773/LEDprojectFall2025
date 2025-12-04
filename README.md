# LED Strips Signboard

A fun, customizable LED matrix signboard that you can control from your computer, phone, or even a remote control! Perfect for scoreboards, info displays, or just showing off some cool text animations.

## What is This?

Think of it as a digital billboard that fits on your wall. You can display text, run timers, create scrolling messages, and even draw custom pixel art. It's powered by an Arduino Mega and uses 15 LED strips to create a 60×15 pixel display. Pretty neat, right?

## What Can It Do?

### Show Text in Cool Ways
- **Static text** - Just show some words (small or large font)
- **Scrolling text** - Make your message scroll across the screen continuously
- **Scroll and stop** - Scroll from right to left, then stop when it's centered
- **Fade in** - Text smoothly fades in (looks really nice!)
- **Breathe** - Text pulses like it's breathing (great for attention-grabbing)

### Timer Features
- Countdown timers (perfect for games or presentations)
- Show the current time
- Full timer controls: start, pause, resume, reset, stop

### Make It Yours
- Change colors for top row, bottom row, or everything
- Adjust brightness (0-255, so you can make it super dim or blindingly bright)
- Draw custom pixel art by controlling individual LEDs
- Update multiple pixels at once for efficiency

### Control It Your Way
- Web interface (use it from any browser)
- Serial commands (for the tech-savvy)
- Remote control (if you set it up)

## What You'll Need

Here's the shopping list:

- **Arduino Mega 2560** (or something compatible)
- **15 NeoPixel LED strips** - 60 pixels each (WS2812B works great)
- **RTC Module** (DS3231) - for keeping time
- **Remote receiver** - depends on what kind you want (IR or RF)
- **Power supply** - LED strips are hungry! You'll need a good 5V supply with plenty of current

**Quick note on wiring:**
- LED strips connect to pins 23-37 (one pin per strip)
- RTC module uses I2C (SDA/SCL pins)
- Communication happens over USB/Serial

## Getting Started

### Setting Up the Arduino

1. **Install the libraries you need:**
   - Adafruit NeoPixel (for the LED strips)
   - RTClib (for the clock)
   - Wire library (usually comes with Arduino IDE)

2. **Open the project:**
   - Go to `Board/MainBoard/` folder
   - Open `MainBoard.ino` in Arduino IDE

3. **Tell Arduino IDE what you're using:**
   - Go to **Tools → Board** and pick **Arduino Mega 2560**
   - Pick the right COM port (the one your Arduino is on)
   - If the display seems too bright or dim, you can change the brightness in the `setup()` function (default is 7)

4. **Upload the code:**
   - Hit the Upload button
   - Wait for "Done uploading" - you're good to go!

### Setting Up the Website

1. **Go to the Website folder:**
   ```bash
   cd Website
   ```

2. **Start the server:**
   - On Windows: Double-click `start.bat` or `displayDriver_usb.bat`
   - Or use PowerShell: Run `run.ps1`

3. **Open the control panel:**
   - Open `Pages/Dashboard.html` in your browser
   - That's your control center!

## How to Use It

### Quick Start with Serial Commands

If you're comfortable with serial communication, you can send commands directly. The format is pretty simple:

```
<START_MARKER><COMMAND><DATA><END_MARKER>
```

The markers are special characters (0x0C to start, 0x0F to end), but you don't need to worry about that if you're using the web interface.

**Example:** To show "Hello" on top and "World" on bottom:
```
\x0C1001Hello,World\x0F
```

### Command Cheat Sheet

#### Text Commands (1001-1010)

Want to display text? Here are your options:

| Code | What It Does | Example Data | Font Size |
|------|--------------|--------------|-----------|
| 1001 | Show static text (small) | `Hello,World` or just `Hello` | Small (7×7) |
| 1002 | Show static text (large) | `HELLO` | Large (15×15) |
| 1003 | Scroll continuously (small) | `Long message here` | Small |
| 1004 | Scroll continuously (large) | `SCROLLING` | Large |
| 1005 | Scroll then stop (small) | `Message` | Small |
| 1006 | Scroll then stop (large) | `TEXT` | Large |
| 1007 | Fade in (small) | `Fade,In` | Small |
| 1008 | Fade in (large) | `FADE` | Large |
| 1009 | Breathe effect (small) | `Breathe,Text` | Small |
| 1010 | Breathe effect (large) | `BREATHE` | Large |

**Pro tip:** For small font commands, you can use a comma to split text into two rows. "Hello,World" shows "Hello" on top and "World" on bottom.

#### Timer Commands (2001-2006)

| Code | What It Does | Example |
|------|--------------|---------|
| 2001 | Start a countdown | `05:30` (5 minutes, 30 seconds) |
| 2002 | Pause the timer | (no data needed) |
| 2003 | Resume the timer | (no data needed) |
| 2004 | Reset the timer | (no data needed) |
| 2005 | Stop the timer | (no data needed) |
| 2006 | Show current time | (no data needed) |

#### Settings Commands (3001-3005)

Make it look how you want:

| Code | What It Does | Example |
|------|--------------|---------|
| 3001 | Change brightness | `128` (0 = off, 255 = max) |
| 3002 | Top row color | `FF0000` (red in hex) |
| 3003 | Bottom row color | `00FF00` (green in hex) |
| 3004 | Full text color | `0000FF` (blue in hex) |
| 3005 | Set everything at once | `128,FF0000,00FF00,0000FF` |

Colors are in hex format (RRGGBB). Don't know hex? Just use an online color picker and it'll give you the hex code!

#### Custom Pixel Commands (4001-4004)

Want to draw something custom? Go for it:

| Code | What It Does | Example |
|------|--------------|---------|
| 4001 | Set one pixel | `10,5,FF0000` (x=10, y=5, red) |
| 4002 | Clear everything | (no data) |
| 4003 | Clear one pixel | `10,5` |
| 4004 | Set a whole row | `5,0,FF0000,1,00FF00,2,0000FF` (row 5, pixels at x=0,1,2) |

**⚠️ Important Note:** Custom pixel functionality is not fully optimized and may be prone to crashing, especially with complex images or when sending many pixels at once. If you experience crashes, try:
- Sending smaller chunks of pixels
- Reducing the number of pixels per command
- Adding delays between commands
- Using simpler custom images

#### System Commands (5001-5004)

Housekeeping stuff:

| Code | What It Does |
|------|--------------|
| 5001 | Check if system is working |
| 5002 | Reset the system |
| 5003 | Clear the display |
| 5004 | Show default message |

### What to Expect Back

When you send a command, you'll get a response:

**If it worked:**
```
<0x0C><YOUR_COMMAND>OK<0x0F>
```

**If something went wrong:**
```
<0x0C><ERROR_CODE><WHAT_WENT_WRONG><0x0F>
```

Error codes:
- `9001` - Something's wrong with the format or the command doesn't exist
- `9002` - You sent a bad parameter (wrong format, out of range, etc.)

## How It's Organized

Here's what's in this project:

```
LED-Strips-Signboard-main/
├── Board/
│   ├── MainBoard/              # The Arduino code
│   │   ├── MainBoard.ino       # Main program
│   │   ├── Display.cpp/.h      # Handles all the LED stuff
│   │   ├── Timer.cpp/.h        # Timer and clock features
│   │   ├── Remote.cpp/.h       # Remote control stuff
│   │   └── CharacterSet files  # The fonts
│   └── App/                    # Helper utilities
├── Website/                     # Web interface
│   ├── Pages/                  # The HTML pages you see
│   ├── scripts/                # JavaScript magic
│   └── css/                    # Making it look nice
└── README.md                   # You're reading it!
```

## Display Specs (The Technical Stuff)

- **Size:** 60 pixels wide × 15 pixels tall
- **Character width:** Max 3 pixels (we made them compact so more text fits)
- **Spacing:** 1 pixel between characters
- **Spaces:** 3 pixels wide
- **Fonts:**
  - Small: 7×7 pixels (can do two rows - top and bottom)
  - Large: 15×15 pixels (one big row, centered)

## How Commands Work (For the Curious)

The system uses a simple protocol. Every command looks like this:

```
[START][COMMAND][DATA][END]
```

- **START** = `0x0C` (a special character)
- **COMMAND** = 4 numbers (like "1001")
- **DATA** = whatever the command needs (could be text, colors, etc.)
- **END** = `0x0F` (another special character)

**Real examples:**

```cpp
// Show "Hello" on top, "World" on bottom
\x0C1001Hello,World\x0F

// Make it brighter
\x0C3001128\x0F

// Start a 5-minute timer
\x0C200105:00\x0F

// Make the top text red
\x0C3002FF0000\x0F
```

## For Developers

### How We Made It Efficient

Since Arduino has limited memory, we had to be smart:

- **Chunked rendering** - Big text gets drawn in pieces so we don't run out of memory
- **Non-blocking animations** - Everything updates smoothly without freezing
- **Smart memory use** - We convert Strings to char arrays to avoid memory fragmentation
- **Frame buffer** - We draw to a buffer first, then update the LEDs all at once

### Code Structure

- **Singleton pattern** - The Display class is a singleton (only one instance exists)
- **Non-blocking design** - Nothing blocks the main loop, so it stays responsive
- **Modular** - Display, Timer, and Remote are separate classes (easier to work with)

### Font Limitations

We limited characters to 3 pixels wide so text fits better on the 60-pixel display. Some characters might look a bit narrower than you'd expect, but everything should still be readable!

## What Works Where

- **Arduino code:** Works on Arduino Mega 2560 (or compatible boards)
- **Website server:** Windows only (uses batch scripts)
- **Web interface:** Works in any modern browser (Chrome, Firefox, Edge, etc.)

## Who Made This?

### The Original Team
- [@AhmadAzeez999](https://github.com/AhmadAzeez999) - Built the website, web server, and made everything talk to each other
- [@WeziKaonga](https://github.com/WeziKaonga) - Remote control and display features
- [@Cipher-935](https://github.com/Cipher-935) - Bluetooth, website backend, and communication protocols
- [@Bhu77ar](https://github.com/Bhu77ar) - Designed and implemented the fonts
- [@ChanndKaleka](https://github.com/ChanndKaleka) - Text display and font implementation

### Recent Improvements
- [@Robert-beep773](https://github.com/Robert-beep773) - Cleaned up the code, wrote documentation, and made it easier to understand

## License

This is part of a TRU Project Course project. Check with your course guidelines for how you can use it.

## Having Problems?

### Display Not Working?

- **Check your power supply** - LED strips need a lot of juice! Make sure your power supply can handle it
- **Check all connections** - Are the wires in the right places?
- **Is it too dim?** - Try increasing the brightness setting
- **Wrong pins?** - Double-check that your LED strips are on pins 23-37

### Serial Communication Not Working?

- **Baud rate** - Make sure it's set to 9600
- **COM port** - Is the right port selected in Arduino IDE?
- **Cable** - Try a different USB cable
- **Too many commands?** - Slow down! Sending commands too fast can cause issues

### Text Getting Cut Off?

- Characters are limited to 3 pixels wide (by design, so more fits)
- Try shorter messages
- Use the smaller font if you need to fit more
- Check your spacing settings

### Can't Upload Code?

- **Board selection** - Make sure Arduino Mega 2560 is selected
- **Port permissions** - Sometimes Windows needs admin rights
- **Try a different USB port** - Some ports work better than others
- **Bootloader mode** - If the board is stuck, you might need to reset it

### Custom Pixels Crashing?

- **⚠️ Known Issue:** Custom pixel functionality is not fully optimized and can crash with complex images
- **Send smaller chunks** - The website automatically chunks data, but very complex images may still cause issues
- **Reduce pixel count** - Try simpler images with fewer pixels
- **Add delays** - If sending manually, add small delays between commands
- **Restart the sign** - If it crashes, power cycle the Arduino to reset

## Helpful Links

- [Adafruit NeoPixel Library](https://github.com/adafruit/Adafruit_NeoPixel) - For controlling the LEDs
- [RTClib Documentation](https://github.com/adafruit/RTClib) - For the clock module
- Check the other documentation files in this folder for more details

---

**Questions? Issues?** Feel free to check the GitHub repository or reach out to the contributors. We're here to help!

Happy building! 🎉
