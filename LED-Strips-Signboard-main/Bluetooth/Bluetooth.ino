#include <SoftwareSerial.h>

const byte rxPin = 9;
const byte txPin = 8;
const byte passThroughPin = 13;
SoftwareSerial BTSerial(rxPin, txPin); // RX TX

int intByte;
int sendMethod = 1;
char controlChar = ' ';
char displayOption = '<';

const uint8_t numRawChar = 90;
char msgRaw[numRawChar];

uint8_t charCount = 0;
bool dataToSend = false;

void setup()
{
  // define pin modes for tx, rx:
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  pinMode(passThroughPin, INPUT_PULLUP);
  BTSerial.begin(9600);
  Serial.begin(1200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

void loop()
{
  while (Serial.available() > 0)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    intByte = Serial.read();

    BTSerial.write(intByte);
  }
}