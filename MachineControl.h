//
// Created by yona on 11/5/16.
//

#ifndef EXPLORACNC_ECNCGCODE_H
#define EXPLORACNC_ECNCGCODE_H

#include "Vector3.h"

#define GCODE_NUM(v) (int32_t(v)*100)
#define FROM_GCODE_NUM(v) (int32_t(v)/100)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Low-level Machine Control

/**
 * Gets the current (actual) position of the tool in workspace coordinates.
 */
bool readCurrentWorkspacePos(Vector3 *pos);

/**
 * Gets the target (eventual) position of the tool in workspace coordinates.
 */
bool readTargetWorkspacePos(Vector3 *pos);

/**
 * Gets the current (actual) position of the tool in machine coordinates (relative to the last homing or startup position).
 */
bool readCurrentMachinePos(Vector3 *pos);
/**
 * Gets the target (eventual) position of the tool in machine coordinates (relative to the last homing or startup position).
 */
bool readTargetMachinePos(Vector3 *pos);

/**
 * Tell the machine to do a homing procedure and reset the zero offset to that new position. Note that the machine
 * will likely not be at (0,0,0) when finished -- the machine usually moves away from it's limit switches after homing.
 * Also, the Z zero will not be relative to the work-piece, and will need to be probed separately.
 */
bool homeMachine();

/**
 * Do a z probe for the given distance and speed.
 * Return of 0 indicates error; -1 indicates probe NOT detected; 1 indicates probe WAS detected;
 */
int machineZProbe(
	int16_t probe01Mm,
	int16_t speed01MmPerMin
);

/**
 * Set the current machine position as Z zero, minus some offset for the z plate tool. This only affects the Z axis.
 */
bool setMachineCurrentZAsZero(int32_t zOffset01mm);

/**
 * Set the machine zero position to the given value in machine coordinates. Current worksapce coordinates are ignored.
 * This does not move the head of the machine.
 */
bool setMachineZeroTo(Vector3 zero);

/**
 * Reset the workspace coordinate system to match the machine position.
 */
bool clearWorkspaceOffsets();

/**
 * Move the machine to the given location, considering the given axes at the given speed. This operation is non-blocking,
 * use waitForMachineToReachTarget afterwards to await arrival at the location.
 */
bool moveMachineTo(Vector3 vec, bool x, bool y, bool z, int32_t speed01MmPerMin);

/**
 * Waits the specified time for the machine to reach it's target position. Use after a move to wait for the machine
 * to finish the move.
 */
bool waitForMachineToReachTarget(uint32_t timeoutMs);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Velocity-based Jog Control


/**
 * Set up the local knowledge of the machine's position for velocity-based jogging.
 */
void startJogControl();
/**
 * Loop the velocity-based jogging system.
 */
void loopJogControl();
/**
 * Gets the local desired jog position.
 */
Vector3 getCurrentJogTarget();
/**
 * Stop the velocity-based jogging system.
 */
void stopJogControlAndRound();
/**
 * Set the desired velocity in the velocity-based jogging system.
 */
void setDesiredVelocity(Vector3 vel);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Machine State

struct MachineControlState {
	bool positionRequested: 1;
	bool positionReceived: 1;
	bool positionError: 1;

	bool joggingError: 1;
};

extern MachineControlState machineState;

#endif //EXPLORACNC_ECNCGCODE_H
