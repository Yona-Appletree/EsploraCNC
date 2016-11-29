//
// Created by yona on 11/16/16.
//

#include "Persistence.h"
#include <EEPROM.h>
#include <stddef.h>
#include <Esplora.h>

#define SAVED_POS_FIELD_ADDR(index, field) (index * sizeof(SavedPosInfo) + offsetof(SavedPosInfo, field))

template <class T> int eepromWriteData(uint16_t address, const T& value)
{
	const uint8_t * p = (const uint8_t *)(const void*)&value;
	unsigned int i;
	for (i = 0; i < sizeof(value); i++)
		EEPROM.write(address++, *p++);
	return i;
}

template <class T> int eepromReadData(uint16_t address, T& value)
{
	uint8_t * p = (uint8_t *)(void*)&value;
	unsigned int i;
	for (i = 0; i < sizeof(value); i++)
		*p++ = EEPROM.read(address++);
	return i;
}


SavedPosName* listSavedNames() {
	static SavedPosName names[SAVED_POS_COUNT];

	uint8_t count = 0;
	for (uint8_t i=0; i<SAVED_POS_COUNT; i++) {
		uint8_t firstChar = EEPROM.read(SAVED_POS_FIELD_ADDR(i, name));

		if (firstChar != 0) {
			eepromReadData(SAVED_POS_FIELD_ADDR(i, name), names[i]);
			count ++;
		}
	}

	return names;
}

void clearAllSaved() {
	Serial.println("All saved positions cleared!");

	for (uint8_t i=0; i<SAVED_POS_COUNT; i++) {
		EEPROM.write(SAVED_POS_FIELD_ADDR(i, name), 0);
	}
}

void loadPos(uint8_t index, SavedPos& state) {
	eepromReadData(SAVED_POS_FIELD_ADDR(index, pos), state);
}
void savePos(uint8_t index, SavedPosName& name, SavedPos& pos) {
	Serial.print("Saving pos");Serial.print(index);
	Serial.print(" as ");
	Serial.print(name.name);Serial.print(": (");
	Serial.print(pos.xMm);Serial.print(", ");
	Serial.print(pos.yMm);Serial.print(", ");
	Serial.print(pos.zMm);Serial.println(")");
	eepromWriteData(SAVED_POS_FIELD_ADDR(index, name), name);
	eepromWriteData(SAVED_POS_FIELD_ADDR(index, pos), pos);
}