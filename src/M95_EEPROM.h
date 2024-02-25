#ifndef M95_EEPROM_H
#define M95_EEPROM_H

#include "SPI.h"

class M95_EEPROM{

public:
  /*
   * Create a basic M95_EEPROM with the given page size.  Assumes 2 address bytes and no ID page
   */
  M95_EEPROM(SPIClass& spi, int cs, int page_size);

  M95_EEPROM(SPIClass& spi, int cs, int page_size, int address_bytes, bool id_page);

  void begin();
  void end();

  /*
   * Read bytes from the EEPROM starting at address.
   *
   * @param address The address to read from
   * @param num_bytes How many bytes to read
   * @param buffer The location to put the bytes.  Must be at least num_bytes long
   * @return num bytes written, or -1 on error
   */
  int read(uint32_t address, uint16_t num_bytes, void* buffer);

  /*
   * Write bytes to the EEPROM starting at address.  This takes into account pages on the EEPROM,
   * such that if you write 64 bytes into a 32-byte page it will write two pages.
   *
   * @param address The address to write to
   * @param num_bytes How many bytes to write
   * @param buffer The location bytes are read from
   * @return num bytes written, or -1 on error
   */
  int write(uint32_t address, uint16_t num_bytes, void* buffer);

  /*
   * Check to see if the EEPROM exists.  Reads from the specified chip and makes sure that
   * we can read the status register and this register contains 0x02.
   */
  bool exists();

  int page_size();

  /*
   * Read the ID page of the device.  If the ID page does not exist, returns -1
   */
  int read_id_page(uint16_t num_bytes, void* buffer);

  /*
   * Write the ID page of the device.  If the ID page does not exist, returns -1
   */
  int write_id_page(uint16_t num_bytes, void* buffer);

  /*
   * Lock the ID page.  If the ID page does not exist, returns -1.  Once the ID page is locked, it is read-only.
   */
  int lock_id_page();

  /*
   * Check to see if the ID page is locked
   */
  bool id_page_locked();

  /*
   * Read the status register
   */
  uint8_t status_register();

  /*
   * Start a continuous read from the EEPROM.  You must call end_continuous_read once you are done reading.
   */
  int start_continuous_read(uint32_t address);

  /*
   * Do a continuous read into the buffer.
   */
  int read_continuous(uint32_t num_bytes, void* buffer);

  /*
   * During a continuous read, skip over the next number of bytes
   */
  int read_continuous_skip(uint32_t skip_bytes);

  /*
   * When doing a continuous read, the current location in the EEPROM.
   */
  uint32_t current_location();

  /*
   * Stop reading continously from the EEPROM.
   */
  int end_continous_read();

private:
  int read_internal(byte command, uint32_t address, uint16_t num_bytes, void* buffer);
  int write_internal(byte command, uint32_t address, uint16_t num_bytes, void* buffer);
  void wait_for_write_complete();
  uint8_t status_register_internal();

private:
  SPIClass& m_spi;
  int m_cs_pin;
  int m_page_size;
  int m_num_address_bytes;
  bool m_id_page;
  uint32_t m_address_continuous = 0;
};

#endif /* M95_EEPROM_H */
