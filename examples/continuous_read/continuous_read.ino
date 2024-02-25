#include <SPI.h>

#include "M95_EEPROM.h"

static const byte EEPROM_CS = 7;

/*
 * This shows a quick example of reading bytes from an EEPROM continuously.
 * In this particular example, we use a M95256 chip from ST.  This
 * chip has a 256-byte page size with an ID page.
 */
M95_EEPROM eeprom(SPI, EEPROM_CS, 256, 3, true);

static void hexdump(uint32_t offset, uint8_t* bytes_u8){
  // Print out the offset
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "%08X", offset);
  Serial.print(buffer);

  Serial.print("  ");
  // Print out the first 8 bytes
  for(int byte = 0; byte < 8; byte++){
    snprintf(buffer, sizeof(buffer), "%02X", bytes_u8[byte]);
    Serial.print(buffer);
    if(byte != 8){
      Serial.print("  ");
    }
  }

  Serial.print(" ");

  // Print out the last 8 bytes
  for(int byte = 8; byte < 16; byte++){
    snprintf(buffer, sizeof(buffer), "%02X", bytes_u8[byte]);
    Serial.print(buffer);
    if(byte != 15){
      Serial.print(" ");
    }
  }

  // Print out the ASCII text
  Serial.print("  |");
  for(int byte = 0; byte < 16; byte++){
    if(isprint(bytes_u8[ byte])){
      char to_append = bytes_u8[byte];
      Serial.print(to_append);
    }else{
      Serial.print(".");
    }
  }
  Serial.print("|");
}

void setup() {
  SPI.begin();
  eeprom.begin();

  Serial.begin (115200) ;
  while (!Serial) {
    delay (50) ;
    digitalWrite (LED_BUILTIN, !digitalRead (LED_BUILTIN)) ;
  }

  Serial.print("Found eeprom? ");
  Serial.println((int)eeprom.exists());

  if(!eeprom.exists()){
    Serial.println("ERROR: Can't find EEPROM!");
    while(1){}
  }

  unsigned long start_millis = millis();
  eeprom.start_continuous_read(0);

  while(eeprom.current_location() < 0x1000){
    // Dump the first 0x1000 bytes of EEPROM, displaying data in a hexdump-like format
    int byte_num = 0;
    uint8_t buffer[16];
    eeprom.read_continuous(sizeof(buffer), buffer);
    hexdump(eeprom.current_location() - 16, buffer);

    Serial.println();
  }

  eeprom.end_continous_read();

  Serial.print("Reading took ");
  Serial.print(millis() - start_millis);
  Serial.println("ms");

  // Do a normal read to make sure everything is working
  uint8_t some_bytes[16];
  eeprom.read(0x6000, sizeof(some_bytes), some_bytes);
  hexdump(0x6000, some_bytes);

}

void loop() {

}