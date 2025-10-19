# LED Strips Sign Board

## Overview
The LED Strips Sign Board is a **web and remote-controlled scoreboard** powered by an **Arduino Mega**. It can display text and time, with a web interface for easy control and customization.

## Key Features
- **Text Display** – Show messages on the scoreboard.
- **Website/Web Server Communication** – Control the scoreboard via a web interface.
- **Remote Control** – Send updates and timers wirelessly.
- **Customization Options** – Configure display settings as needed.

## Installation
### **Arduino Setup**
1. Navigate to `Board > MainBoard`.
2. Open **MainBoard.ino** in the Arduino IDE.
3. Upload the code to your **Arduino Mega**.

### **Website Setup**
1. Navigate to the `Website` directory.
2. Run **displayDriver_usb.bat** to start the server.
3. Open **dashBoard_small.html** in a browser to access the interface.

## Platform Compatibility
- **Windows Only** – The server script and display driver require Windows to function.

## Authors
Developed by:
- [@AhmadAzeez999](https://github.com/AhmadAzeez999) (Website, Webserver, & Webserver-to-Board Communication)
- [@WeziKaonga](https://github.com/WeziKaonga) (Remote Communication & Display)
- [@Cipher-935](https://github.com/Cipher-935) (Bluetooth Communication, Website Backend, & Webserver-to-Board Communication)
- [@Bhu77ar](https://github.com/Bhu77ar) (Font Design & Implementation)
- [@ChanndKaleka](https://github.com/ChanndKaleka) (Text Display & Font Implementation)

