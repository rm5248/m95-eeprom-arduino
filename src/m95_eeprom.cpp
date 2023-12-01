#include "m95_eeprom.h"

static const byte EEPROM_WRITE_ENABLE = 0x6;
static const byte EEPROM_READ_STATUS_REGISTER = 0x5;
static const byte EEPROM_WRITE_STATUS_REGISTER = 0x1;
static const byte EEPROM_READ_MEMORY_ARRAY = 0x3;
static const byte EEPROM_WRITE_MEMORY_ARRAY = 0x2;
static const byte EEPROM_WRITE_DISABLE = 0x4;
static const byte EEPROM_READ_ID_PAGE = 0x83;
static const byte EEPROM_WRITE_ID_PAGE = 0x82;

M95_EEPROM::M95_EEPROM(SPIClass* spi, int cs, int page_size) :
  M95_EEPROM(spi, cs, page_size, 2, false){

}

M95_EEPROM::M95_EEPROM(SPIClass* spi, int cs, int page_size, int address_bytes, bool lockable_page) :
  m_spi(spi),
  m_cs_pin(cs),
  m_page_size(page_size),
  m_num_address_bytes(address_bytes),
  m_id_page(lockable_page){

}

void M95_EEPROM::begin(){
  pinMode(m_cs_pin, OUTPUT);
  digitalWrite(m_cs_pin, HIGH);
}

void M95_EEPROM::end(){
  digitalWrite(m_cs_pin, LOW);
}

int M95_EEPROM::read(uint32_t address, uint16_t num_bytes, void* buffer){
  return read_internal(EEPROM_READ_MEMORY_ARRAY, address, num_bytes, buffer);
}

int M95_EEPROM::read_internal(byte command, uint32_t address, uint16_t num_bytes, void* buffer){
  digitalWrite(m_cs_pin, LOW);
  delay(1);
  m_spi->transfer(command);

  if(m_num_address_bytes == 1){
    m_spi->transfer((address & 0xFF) >> 0);
  }else if(m_num_address_bytes == 2){
    m_spi->transfer((address & 0xFF00) >> 8);
    m_spi->transfer((address & 0x00FF) >> 0);
  }else if(m_num_address_bytes == 3){
    m_spi->transfer((address & 0xFF0000) >> 16);
    m_spi->transfer((address & 0x00FF00) >> 8);
    m_spi->transfer((address & 0x0000FF) >> 0);
  }else{
    digitalWrite(m_cs_pin, HIGH);
    return -1;
  }

  uint8_t* u8_data = buffer;
  while(num_bytes > 0){
    num_bytes--;
    *u8_data = m_spi->transfer(0xFF); // dummy byte
    u8_data++;
  }

  digitalWrite(m_cs_pin, HIGH);

  return 0;
}

int M95_EEPROM::write(uint32_t address, uint16_t num_bytes, void* buffer){
  digitalWrite(m_cs_pin, LOW);
  delay(1);
  SPI.transfer(EEPROM_WRITE_ENABLE);
  digitalWrite(m_cs_pin, HIGH);

  delay(1);

  return write_internal(EEPROM_WRITE_MEMORY_ARRAY, address, num_bytes, buffer);
}

int M95_EEPROM::write_internal(byte command, uint32_t address, uint16_t num_bytes, void* buffer){
  digitalWrite(m_cs_pin, LOW);
  SPI.transfer(command);

  if(m_num_address_bytes == 1){
    m_spi->transfer((address & 0xFF) >> 0);
  }else if(m_num_address_bytes == 2){
    m_spi->transfer((address & 0xFF00) >> 8);
    m_spi->transfer((address & 0x00FF) >> 0);
  }else if(m_num_address_bytes == 3){
    m_spi->transfer((address & 0xFF0000) >> 16);
    m_spi->transfer((address & 0x00FF00) >> 8);
    m_spi->transfer((address & 0x0000FF) >> 0);
  }else{
    digitalWrite(m_cs_pin, HIGH);
    return -1;
  }

  uint8_t* u8_data = buffer;
  while(num_bytes > 0){
    num_bytes--;
    SPI.transfer(*u8_data); // data byte
    u8_data++;
  }

  digitalWrite(m_cs_pin, HIGH);

  int numTimes = 0;
  while(numTimes < 50){
    // Read until the write in progress bit is 0
    numTimes++;
    delay(1);

    int status_reg = status_register();
    if(status_reg & 0x01 == 0){
      break;
    }
  }

  return 0;
}

bool M95_EEPROM::exists(){
  digitalWrite(m_cs_pin, LOW);
  m_spi->transfer(EEPROM_WRITE_ENABLE);
  digitalWrite(m_cs_pin, HIGH);

  delay(1);

  digitalWrite(m_cs_pin, LOW);
  m_spi->transfer(EEPROM_READ_STATUS_REGISTER);
  int read_status_reg = m_spi->transfer(0xFF);
  digitalWrite(m_cs_pin, HIGH);

  if(read_status_reg != 0xFF &&
     read_status_reg & (0x01 << 1)){
    // WEL bit is set, so we are talking with the EEPROM!
    // Let's go and disable it again
    digitalWrite(m_cs_pin, LOW);
    m_spi->transfer(EEPROM_WRITE_DISABLE);
    digitalWrite(m_cs_pin, HIGH);
    return true;
  }

  return false;
}

int M95_EEPROM::page_size(){
  return m_page_size;
}

int M95_EEPROM::read_id_page(uint16_t num_bytes, void* buffer){
  return read_internal(EEPROM_READ_ID_PAGE, 0, num_bytes, buffer);
}

int M95_EEPROM::write_id_page(uint16_t num_bytes, void* buffer){
  return write_internal(EEPROM_WRITE_ID_PAGE, 0, num_bytes, buffer);
}

int M95_EEPROM::lock_id_page(){

}

bool M95_EEPROM::id_page_locked(){
  digitalWrite(m_cs_pin, LOW);
  delay(1);

  m_spi->transfer(EEPROM_READ_ID_PAGE);

  int address = 0x20;

  if(m_num_address_bytes == 1){
    m_spi->transfer((address & 0xFF) >> 0);
  }else if(m_num_address_bytes == 2){
    m_spi->transfer((address & 0xFF00) >> 8);
    m_spi->transfer((address & 0x00FF) >> 0);
  }else if(m_num_address_bytes == 3){
    m_spi->transfer((address & 0xFF0000) >> 16);
    m_spi->transfer((address & 0x00FF00) >> 8);
    m_spi->transfer((address & 0x0000FF) >> 0);
  }else{
    digitalWrite(m_cs_pin, HIGH);
    return false;
  }

  int page_locked = m_spi->transfer(0xFF);
  digitalWrite(m_cs_pin, HIGH);

  return !!page_locked;
}

uint8_t M95_EEPROM::status_register(){
  digitalWrite(m_cs_pin, LOW);
  delay(1);
  m_spi->transfer(EEPROM_READ_STATUS_REGISTER);
  int read_status_reg = m_spi->transfer(0xFF);
  digitalWrite(m_cs_pin, HIGH);

  return read_status_reg;
}