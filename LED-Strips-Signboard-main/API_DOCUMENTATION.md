# API Documentation

## Web Server API

The PowerShell web server (`run.ps1`) provides a REST API that bridges HTTP requests to Arduino serial communication.

### Base URL
```
http://127.0.0.1:8080
```
(Default port is 8080, but can be configured at startup)

---

## Endpoints

### POST `/dashboard/post`

Sends commands to the Arduino display.

**Request Format:**
```json
{
  "command": "string",
  "isBig": "yes" | "no",
  "data": "string",
  "brightness": "number" | "",
  "tcolor": "string" | "",
  "bcolor": "string" | "",
  "fcolor": "string" | ""
}
```

**Response:**
- Status: `200 OK` (on success)
- Body: Empty

**Command Types:**

#### Text Display Commands

| Command | Description | Font Size | Data Format |
|---------|-------------|-----------|-------------|
| `static` | Static text display | Small/Large | Text string |
| `scrolC` | Continuous scroll | Small/Large | Text string |
| `scrolS` | Scroll and stop | Small/Large | Text string |
| `fadeIn` | Fade in animation | Small/Large | Text string |
| `breath` | Breathe animation | Small/Large | Text string |

**Example Request:**
```json
{
  "command": "static",
  "isBig": "yes",
  "data": "Hello World"
}
```

**Arduino Protocol Conversion:**
- `static` + `isBig: "yes"` → `121002Hello World15`
- `static` + `isBig: "no"` → `121001Top,Bottom15`

#### Timer Commands

| Command | Description | Data Format |
|---------|-------------|-------------|
| `sTimer` | Start countdown timer | `MM:SS` (e.g., "05:30") |
| `pTimer` | Pause timer | (no data) |
| `resume` | Resume timer | (no data) |
| `rTimer` | Reset timer | (no data) |
| `tod` | Display time of day | (no data) |

**Example Request:**
```json
{
  "command": "sTimer",
  "isBig": "yes",
  "data": "05:30"
}
```

**Arduino Protocol Conversion:**
- `sTimer` → `12200105:3015`

#### Settings Commands

| Command | Description | Data Format |
|---------|-------------|-------------|
| `settns` | Update settings | Comma-separated: `brightness,topColor,bottomColor,fullColor` |

**Example Request (Brightness):**
```json
{
  "command": "settns",
  "brightness": "128",
  "tcolor": "",
  "bcolor": "",
  "fcolor": ""
}
```

**Example Request (Colors):**
```json
{
  "command": "settns",
  "brightness": "",
  "tcolor": "FF0000",
  "bcolor": "00FF00",
  "fcolor": "0000FF"
}
```

**Arduino Protocol Conversion:**
- `settns` → `123005128,FF0000,00FF00,0000FF15`

#### Custom Pixel Commands

| Command | Description | Data Format |
|---------|-------------|-------------|
| `custom` | Draw custom pixels | See Custom Pixel API below |

**Example Request:**
```json
{
  "command": "custom",
  "param": "row",
  "data": "0,10,FF0000,20,00FF00"
}
```

---

### GET `/kill`

Gracefully shuts down the web server.

**Response:**
- Status: `200 OK`
- Body: Empty

**Example:**
```javascript
fetch('http://127.0.0.1:8080/kill')
```

---

## Custom Pixel API

The custom pixel drawing feature allows drawing individual pixels on the 15×60 LED matrix.

### Pixel Coordinate System
- **X-axis (columns)**: 0-59 (left to right)
- **Y-axis (rows)**: 0-14 (top to bottom)
- **Origin**: Top-left corner (0,0)

### Command Format

#### Clear All Pixels
```json
{
  "command": "custom",
  "param": "start",
  "data": ""
}
```
**Arduino Protocol:** `12400215`

#### Set Single Pixel (Legacy)
```json
{
  "command": "custom",
  "data": "10,5,FF0000"
}
```
**Format:** `X,Y,RRGGBB`
**Arduino Protocol:** `12400110,5,FF000015`

#### Set Row of Pixels (Recommended)
```json
{
  "command": "custom",
  "param": "row",
  "data": "0,10,FF0000,20,00FF00,30,0000FF"
}
```
**Format:** `row,col1,color1,col2,color2,...`
- First value is the row number (0-14)
- Subsequent pairs are column,color
- **Arduino Protocol:** `1240040,10,FF0000,20,00FF00,30,0000FF15`

**Example - Draw a red line across row 7:**
```json
{
  "command": "custom",
  "param": "row",
  "data": "7,0,FF0000,1,FF0000,2,FF0000,3,FF0000"
}
```

---

## Protocol Conversion

The PowerShell server converts JSON commands to ASCII protocol format:

**Format:** `[START][COMMAND][DATA][END]`
- **START**: ASCII 12 (0x0C)
- **COMMAND**: 4-digit code
- **DATA**: Variable length
- **END**: ASCII 15 (0x0F)

### Command Code Mapping

| JSON Command | Font | Arduino Code |
|--------------|------|--------------|
| `static` | Small | `1001` |
| `static` | Large | `1002` |
| `scrolC` | Small | `1003` |
| `scrolC` | Large | `1004` |
| `scrolS` | Small | `1005` |
| `scrolS` | Large | `1006` |
| `fadeIn` | Small | `1007` |
| `fadeIn` | Large | `1008` |
| `breath` | Small | `1009` |
| `breath` | Large | `1010` |
| `sTimer` | - | `2001` |
| `pTimer` | - | `2002` |
| `resume` | - | `2003` |
| `rTimer` | - | `2004` |
| `tod` | - | `2006` |
| `settns` | - | `3005` |
| `custom` (start) | - | `4002` |
| `custom` (single) | - | `4001` |
| `custom` (row) | - | `4004` |

---

## Error Handling

### Server Errors
- **Port in use**: Server will prompt for different port
- **Serial timeout**: Server logs error, discards buffer, continues
- **Invalid JSON**: Request ignored, no response

### Arduino Errors
Arduino sends error responses in protocol format:
- Format: `[START][ERROR_CODE][MESSAGE][END]`
- Error codes: `9001`-`9005`

See `Communication_Protocol_Documentation.txt` for error code details.

---

## JavaScript Usage Examples

### Send Text Message
```javascript
async function sendText() {
  const response = await fetch('http://127.0.0.1:8080/dashboard/post', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({
      command: 'static',
      isBig: 'yes',
      data: 'Hello World'
    })
  });
}
```

### Start Timer
```javascript
async function startTimer(minutes, seconds) {
  const time = `${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`;
  const response = await fetch('http://127.0.0.1:8080/dashboard/post', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({
      command: 'sTimer',
      isBig: 'yes',
      data: time
    })
  });
}
```

### Set Brightness
```javascript
async function setBrightness(percent) {
  const brightness = Math.round((percent / 100) * 255);
  const response = await fetch('http://127.0.0.1:8080/dashboard/post', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({
      command: 'settns',
      brightness: brightness.toString(),
      tcolor: '',
      bcolor: '',
      fcolor: ''
    })
  });
}
```

### Draw Custom Pixel
```javascript
async function drawPixel(row, col, color) {
  // Remove # from hex color
  const hexColor = color.replace('#', '');
  const response = await fetch('http://127.0.0.1:8080/dashboard/post', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({
      command: 'custom',
      param: 'row',
      data: `${row},${col},${hexColor}`
    })
  });
}
```

---

## CORS Headers

The server automatically adds CORS headers to all responses:
```
Access-Control-Allow-Origin: *
Access-Control-Allow-Methods: GET, POST, OPTIONS
Access-Control-Allow-Headers: Content-Type
```

This allows the web interface to make requests from any origin.

---

## Serial Communication

### Baud Rate
- **Fixed**: 9600 baud
- **Data Bits**: 8
- **Parity**: None
- **Stop Bits**: 1

### Buffer Settings
- **Write Buffer Size**: 65536 bytes
- **Write Timeout**: 1200ms
- **Read Timeout**: 1000ms (Arduino side)

### Message Format
All messages sent to Arduino end with newline (`\n`).

---

## Troubleshooting

### Connection Issues
1. Verify server is running (check PowerShell window)
2. Verify correct port number
3. Check firewall settings
4. Try `localhost` instead of `127.0.0.1`

### Serial Communication Issues
1. Verify correct COM port
2. Check Arduino is connected and powered
3. Verify baud rate is 9600
4. Check serial port isn't in use by another program

### Command Not Working
1. Check browser console for errors
2. Verify JSON format is correct
3. Check server logs for protocol conversion
4. Verify Arduino received command (check Serial Monitor)

---

## See Also

- `Communication_Protocol_Documentation.txt` - Complete Arduino protocol reference
- `README.md` - Project overview and setup
- `CHANGES_SUMMARY.txt` - Development history

