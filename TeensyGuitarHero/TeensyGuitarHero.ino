#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>
#include "Mapping.h"

// Accelerometer is a MMA845X wired on pins 18 & 19 (Wire)
Adafruit_MMA8451 mma = Adafruit_MMA8451();
float x, y, z;

// Nunchuk is wired on pins 16 & 17 (Wire1)
float joyMinX = 196;
float joyMaxX = 254;
float joyMinY = 198;
float joyMaxY = 253;
float vibMin  = 240;
float vibMax  = 250;
byte values[6]; // read nunchuk values in there

Mapping m(false); // don't retrig sounding notes
MIDITransformer transformer(&m);

void setup()
{
  // Serial.begin(115200);
  // Serial.print("\t");

  pinMode(LED_BUILTIN, OUTPUT); // LED
  delay(200);

  // ACCELEROMETER ////////////////////////////////////////////////////////////
  mma.begin(0x1D, &Wire);
  mma.setRange(MMA8451_RANGE_2_G);
  
  // NUNCHUK //////////////////////////////////////////////////////////////////
  Wire1.begin();
  Wire1.setTimeout(10000);

  delay(10);
  // sequence to ask for raw nunchuk2 data
  Wire1.beginTransmission(0x52);
  Wire1.write(0xF0);
  Wire1.write(0x55); 
  Wire1.endTransmission();
  delay(10);
  Wire1.beginTransmission(0x52);
  Wire1.write(0xFB);
  Wire1.write((byte) 0x00); 
  Wire1.endTransmission();
  delay(10);

  delay(200);
}

void loop()
{
  // ACCELEROMETER ////////////////////////////////////////////////////////////
  mma.read();
  sensors_event_t event; 
  mma.getEvent(&event);

  // we transmit acceleration in G (default unit is m.s^-2)
  x = event.acceleration.x / 9.81;
  y = event.acceleration.y / 9.81;
  z = event.acceleration.z / 9.81;
  // Serial.print(x); Serial.print("\t");
  // Serial.print(y); Serial.print("\t");
  // Serial.print(z); Serial.print("\t");
  // Serial.println();

  transformer.setAccel(x, y, z);

  // GUITAR CONTROLS //////////////////////////////////////////////////////////
  int count = 0;
  Wire1.requestFrom(0x52, 6);
  while (Wire1.available())
  {
    values[count] = Wire1.read();
    count++; 
  }
  delay(2);
  Wire1.beginTransmission(0x52);
  Wire1.write((byte) 0x00);
  Wire1.write((byte) 0x00);
  Wire1.endTransmission();

  // delay(10);
  // Serial.print(values[0]); Serial.print("\t"); // joyX (from 196 to 254)
  // Serial.print(values[1]); Serial.print("\t"); // joyY (from 198 to 253)
  // Serial.print(values[2]); Serial.print("\t"); // nothing from guitar hero controler here ...
  // Serial.print(values[3]); Serial.print("\t"); // vibrato bar (from 238 to 250, but rest value seems to be 240)
  // Serial.print(values[4], BIN); Serial.print("\t"); // pluck down, +/- buttons
  // Serial.print(values[5], BIN); Serial.print("\t"); // pluck up, neck buttons
  // Serial.println();
  // delay(10);

  float joyX  = map(constrain(values[0], joyMinX, joyMaxX), joyMinX, joyMaxX, 0.f, 1.f);
  float joyY  = map(constrain(values[1], joyMinY, joyMaxY), joyMinY, joyMaxY, 0.f, 1.f);
  float vib   = map(constrain(values[3], vibMin,  vibMax),  vibMin,   vibMax, 0.f, 1.f);
  transformer.setJoyX(joyX);
  transformer.setJoyY(joyY);
  transformer.setVib(vib);

  bool pluckDown  = bitRead(values[4], 6) == 0;
  bool pluckUp    = bitRead(values[5], 0) == 0;
  transformer.setPluckDown(pluckDown);
  transformer.setPluckUp(pluckUp);

  bool plusBtn    = bitRead(values[4], 2) == 0;
  bool minusBtn   = bitRead(values[4], 4) == 0;
  transformer.setPlus(plusBtn);
  transformer.setMinus(minusBtn);
  
  bool greenBtn   = bitRead(values[5], 4) == 0;
  bool redBtn     = bitRead(values[5], 6) == 0;
  bool yellowBtn  = bitRead(values[5], 3) == 0;
  bool blueBtn    = bitRead(values[5], 5) == 0;
  bool orangeBtn  = bitRead(values[5], 7) == 0;
  transformer.setFret(0, greenBtn);
  transformer.setFret(1, redBtn);
  transformer.setFret(2, yellowBtn);
  transformer.setFret(3, blueBtn);
  transformer.setFret(4, orangeBtn);

  transformer.update();

  // usbMIDI.sendNoteOn(60, 99, channel, cable);
  // usbMIDI.sendNoteOff(60, 0, channel, cable);
  // usbMIDI.sendControlChange(7, 100, channel, cable);
  // usbMIDI.sendPitchBend(911, channel, cable);
  delay(2);
}
