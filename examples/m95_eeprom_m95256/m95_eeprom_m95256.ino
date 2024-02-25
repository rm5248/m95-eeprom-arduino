#include <SPI.h>

#include "M95_EEPROM.h"

static const byte EEPROM_CS = 10;

/*
 * This shows a quick example of reading bytes from an EEPROM.
 * In this particular example, we use a M95256 chip from ST.  This
 * chip has a 64-byte page size and no ID page.
 */
M95_EEPROM eeprom(SPI, EEPROM_CS, 64);

void setup() {
  SPI.begin();
  eeprom.begin();

  Serial.begin (9600) ;
  while (!Serial) {
    delay (50) ;
    digitalWrite (LED_BUILTIN, !digitalRead (LED_BUILTIN)) ;
  }

  Serial.print("Found eeprom? ");
  Serial.println((int)eeprom.exists());

  uint8_t bytes[8];

  // Read from the EEPROM.  This simply reads 8 bytes from offset 0x6000
  eeprom.read(0x6000, 8, bytes);

  Serial.print("bytes @0x6000: ");
  for(int x = 0; x < 8; x++){
    Serial.print(bytes[x], HEX);
  }
  Serial.println();
}

void loop() {

}
