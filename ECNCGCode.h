//
// Created by yona on 11/5/16.
//

#ifndef EXPLORACNC_ECNCGCODE_H
#define EXPLORACNC_ECNCGCODE_H

#include "Vector3.h"

#define GCODE_NUM(v) (int32_t(v)*100)

bool readMachineActualPos(Vector3 *pos);

void startJogControl();
void loopJogControl();
void stopJogControl();
void setDesiredVelocity(Vector3 vel);
bool homeMachine();
/**
 * Do a z probe for the given distance and speed.
 * Return of 0 indicates error; -1 indicates probe NOT detected; 1 indicates probe WAS detected;
 */
int machineZProbe(
	int16_t probe01Mm,
	int16_t speed01MmPerMin
);
bool setMachineZero(int32_t zOffset01mm);
bool moveMachineTo(Vector3 vec, int32_t speed01MmPerMin);

struct MachineControlState {
	bool positionRequested: 1;
	bool positionReceived: 1;
	bool positionError: 1;

	bool joggingError: 1;
};

extern MachineControlState machineState;

#endif //EXPLORACNC_ECNCGCODE_H
