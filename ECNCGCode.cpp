//
// Created by yona on 11/5/16.
//

#include "ECNC.h"
#include "MachineComm.h"

#define DEFAULT_SERIAL_TIMEOUT (1000)

void clearSerialInputBuffer();

MachineControlState machineState = {
	.positionRequested = false,
	.positionReceived = false,
	.positionError = false,
	.joggingError = false,
};

Vector3 lastReadMachinePos;
Vector3 desiredMachinePos;
Vector3 desiredMachineVel01MmPerSec;

// TODO: Replace velocity in mm/sec with mm/min for consistency
void setDesiredVelocity(Vector3 vel01MmPerSec) {
	desiredMachineVel01MmPerSec = vel01MmPerSec;
}

bool jogControlEnabled = false;
bool gotInitialPosition = false;

const unsigned long movementStepMs = 250;

unsigned long lastPosCheckMs = 0;

const unsigned long posCheckIntervalMs = 50;

void syncMachineMotion() {
	// Precompute for speed
	int32_t desiredVel01MmPerSec = desiredMachineVel01MmPerSec.size();

	// Are we close enough to our target position to move?
	int32_t distanceToTargetMm = (desiredMachinePos - lastReadMachinePos).size();

	// How should it take to get to the destination at the given speed?
	// Adjust a little to account for various latency (experimentally derived)
	int32_t expectedRemainingMs = (distanceToTargetMm * 1000) / desiredVel01MmPerSec - 50;

	if (expectedRemainingMs < 0) expectedRemainingMs = 0;

	// Are we likely to get there soon?
	if (expectedRemainingMs <= 150 && desiredVel01MmPerSec > 500) {
		// Cool! Lets tell the machine to move some more!

		int32_t desiredAdditionalMoveMs = movementStepMs - expectedRemainingMs;
		desiredMachinePos += (desiredMachineVel01MmPerSec * desiredAdditionalMoveMs) / 1000;

		// Restrict to available area (no limits, please!)
		if (desiredMachinePos.x < MIN_MACHINE_X) desiredMachinePos.x = MIN_MACHINE_X;
		if (desiredMachinePos.y < MIN_MACHINE_Y) desiredMachinePos.y = MIN_MACHINE_Y;
		if (desiredMachinePos.x > MAX_MACHINE_X) desiredMachinePos.x = MAX_MACHINE_X;
		if (desiredMachinePos.y > MAX_MACHINE_Y) desiredMachinePos.y = MAX_MACHINE_Y;

		// Actually send the command
		sendStr("G0");
		sendXY(desiredMachinePos);
		sendStr(" F");
		int32_t desiredVel01MmPerMin = desiredVel01MmPerSec * 60;
		sendFixed(desiredVel01MmPerMin);
		sendLine();

		if (!awaitOkResponse("G0", DEFAULT_SERIAL_TIMEOUT)) {
			machineState.joggingError = true;
		}
	}
}

void setupJogControl() {
	if (!jogControlEnabled) {
		jogControlEnabled = true;
		gotInitialPosition = false;

		machineState.positionRequested = true;
		machineState.positionReceived = false;
		machineState.positionError = false;
		machineState.joggingError = false;
	}
}

void loopJogControl() {
	if (jogControlEnabled) {
		if (gotInitialPosition) {
			if (millis() - lastPosCheckMs > posCheckIntervalMs) {
				if (readMachineActualPos(&lastReadMachinePos)) {
					lastPosCheckMs = millis();
					syncMachineMotion();
				}
			}
		} else {
			if (readMachineActualPos(&lastReadMachinePos)) {
				Serial.println("Got initial position");
				gotInitialPosition = true;
				desiredMachinePos = lastReadMachinePos;
				desiredMachineVel01MmPerSec = Vector3(0, 0, 0);
				machineState.positionReceived = true;
			} else {
				Serial.println("Initial position request failed. Will retry.");
				machineState.positionError = true;
			}
		}
	}
}

void stopJogControl() {
	jogControlEnabled = false;

	machineState.positionRequested = false;
	machineState.positionReceived = false;
	machineState.positionError = false;
	machineState.joggingError = false;
}

/**
 * Reads the next decimal number from a string and returns it, *100 as an integer. Designed for reading positions
 * from GCode responses in .01mm resolution.
 */
int32_t readNextPos0_01mm(
	char*& c
) {
	int32_t v = 0;
	uint8_t state = 0; // 0 - parsing number, 1 = seen decimal, 2 = done

	// Search for a number
	while (*c && !(*c == '-' || *c == '.' || (*c >= '0' && *c <= '9'))) c++;

	// Parse that number
	for (;state<3;c++) {
		if (*c == '-') {
			v = -v;
		} else if (*c == '.') {
			state = 1;
		} else if (*c >= '0' && *c <= '9') {
			v = v*10 + (*c - '0');
			if (state > 0) state ++;
		} else {
			state = 3;
		}
	}

	// Search for a non-number
	while (*c && (*c == '-' || *c == '.' || (*c >= '0' && *c <= '9'))) c++;

	return v; // No decimal place
}

bool homeMachine() {
	return sendAndAwaitOk("G28.2", 60000);
}

bool readMachineActualPos(Vector3 *pos) {
	clearSerialInputBuffer();

	// Expected Response
	// ok WPOS: X:400.0000 Y:400.0000 Z:0.0000
	if (sendAndAwait("M114.1", "ok WPOS:", DEFAULT_SERIAL_TIMEOUT)) {
		char* c = lastMachineResponse;
		pos->x = readNextPos0_01mm(c);
		pos->y = readNextPos0_01mm(c);
		pos->z = readNextPos0_01mm(c);

		return true;
	} else {
		return false;
	}
}

bool readMachineTargetPos(Vector3 * pos) {
	clearSerialInputBuffer();

	if (sendAndAwait("M114", "ok C: ", DEFAULT_SERIAL_TIMEOUT)) {
		char* c = lastMachineResponse;
		pos->x = readNextPos0_01mm(c);
		pos->y = readNextPos0_01mm(c);
		pos->z = readNextPos0_01mm(c);

		return true;
	} else {
		return false;
	}
}

int machineZProbe(
	int16_t probe01Mm,
	int16_t speed01MmPerMin
) {
	sendStr("G38.3 Z");
	sendFixed(probe01Mm);
	if (speed01MmPerMin > 0) {
		sendStr(" F");
		sendFixed(speed01MmPerMin);
	}
	sendLine();
	clearSerialInputBuffer();

	// Expect: [PRB:0.000,0.000,-8.029:0]
	if (awaitResponse("G38.3", "[PRB:", 60000)) {
		char* c = lastMachineResponse;
		readNextPos0_01mm(c); // X
		readNextPos0_01mm(c); // Y
		readNextPos0_01mm(c); // Z
		int status = readNextPos0_01mm(c); // Status

		// Also expect OK
		if (! awaitOkResponse("G38.3", DEFAULT_SERIAL_TIMEOUT)) return 0;

		return status > 0 ? 1 : -1;
	} else {

		return 0;
	}
}

bool setMachineZero(int32_t zOffset01mm) {
	sendStr("G92 X0 Y0 Z");
	sendFixed(zOffset01mm);
	sendLine();
	return awaitOkResponse("G92", DEFAULT_SERIAL_TIMEOUT);
}

bool moveMachineTo(Vector3 vec, int32_t speed01MmPerMin) {
	sendStr("G0 ");
	sendXYZ(vec);
	if (speed01MmPerMin > 0) {
		sendStr(" F");
		sendFixed(speed01MmPerMin);
	}
	sendLine();
	return awaitOkResponse("G0", DEFAULT_SERIAL_TIMEOUT);
}