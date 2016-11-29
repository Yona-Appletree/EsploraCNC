//
// Created by yona on 11/16/16.
//

#ifndef EXPLORACNC_PERSISTENCE_H
#define EXPLORACNC_PERSISTENCE_H

#include <stdint.h>

#define SAVED_POS_COUNT 16


struct SavedPos {
	int16_t xMm;
	int16_t yMm;
	int16_t zMm;
};

struct SavedPosName {
	char name[16];
};

struct SavedPosInfo {
	SavedPosName name;
	SavedPos pos;
};

void clearAllSaved();
SavedPosName* listSavedNames();
void loadPos(uint8_t index, SavedPos& state);
void savePos(uint8_t index, SavedPosName& name, SavedPos& pos);

#endif //EXPLORACNC_PERSISTENCE_H
