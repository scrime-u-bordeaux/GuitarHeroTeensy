
#include <i2c_t3.h>

// Function prototypes
void print_scan_status(uint8_t target, uint8_t all);

uint8_t found, target, all;

void setup()
{
  
  Serial.begin(115200);

  Serial.print("\t");
  pinMode(LED_BUILTIN, OUTPUT);   // LED
  // pull pin 11 low for a more verbose result (shows both ACK and NACK)
delay(200);
  // Setup for Master mode, pins 18/19, external pullups, 400kHz, 10ms default timeout
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);
  Wire.setDefaultTimeout(10000); // 10ms
 delay(10);
  Wire.beginTransmission (0x52); // Séquence d'initialisation nunchuck2 data non-codées
  Wire.write (0xF0);
  Wire.write (0x55); 
  Wire.endTransmission ();
  delay(10);
  Wire.beginTransmission (0x52);
  Wire.write (0xFB);
  Wire.write ((byte) 0x00); 
  Wire.endTransmission ();
  delay(10);
}

void loop()
{
  int count = 0;
  int values[6];
  Wire.requestFrom(0x52, 6);
  while (Wire.available())
  {
    values[count] = Wire.read();
    count++;
    
  }
delay(10);
  Wire.beginTransmission (0x52);
  Wire.write ((byte)0x00);
  Wire.write ((byte)0x00);
  Wire.endTransmission ();
  delay(10);
  Serial.print(values[0], BIN); Serial.print("\t"); //joyX
  Serial.print(values[1], BIN); Serial.print("\t"); //joyY
  Serial.print(values[2], BIN ); Serial.print("\t");
  Serial.print(values[3], BIN); Serial.print("\t");
  Serial.print(values[4], BIN); Serial.print("\t");
  Serial.print(values[5], BIN); Serial.print("\t");
  Serial.println();
  //delay(10);

  //delay(1500); // delay to space out tests

}
