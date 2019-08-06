#include "Settings.h"

//This function will write a 2 byte integer to the eeprom at the specified address and address + 1
void Settings::writeInt(int p_address, int p_value)
 {
   byte lowByte = ((p_value >> 0) & 0xFF);
   byte highByte = ((p_value >> 8) & 0xFF);

   EEPROM.write(p_address, lowByte);
   EEPROM.write(p_address + 1, highByte);
 }

//This function will read a 2 byte integer from the eeprom at the specified address and address + 1
unsigned int Settings::readInt(int p_address)
 {
   byte lowByte = EEPROM.read(p_address);
   byte highByte = EEPROM.read(p_address + 1);

   return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
 }
