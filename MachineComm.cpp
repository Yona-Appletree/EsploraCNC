//
// Created by yona on 11/13/16.
//

#include "MachineComm.h"
#include "Vector3.h"
#include <Esplora.h>

#define DEBUG_ECHO 0
#define DEBUG_MESSAGES 0

Stream& debugSerial = Serial;
Stream& machineSerial = Serial1;

#define RECEIVE_BUFFER_SIZE 64
char lastMachineResponseBuffer[RECEIVE_BUFFER_SIZE];
char* lastMachineResponse = lastMachineResponseBuffer;

void clearSerialInputBuffer() {
	int available = machineSerial.available();

	if (available > 0) {
		readMachineResponse(1000);
	}
}

bool readMachineResponse(uint32_t timeoutMs) {
	// Zero the response. Slow, but certain.
	memset(lastMachineResponse, 0, RECEIVE_BUFFER_SIZE);

	long stopMs = millis() + timeoutMs;
	char* buffer = lastMachineResponse;
	size_t countInBuffer = 0;

	while (millis() < stopMs) {
		int c = machineSerial.read();

		if (c < 0) {
			delay(1);
			continue;
		}

#if DEBUG_ECHO == 1
		debugSerial.print(char(c));
#endif

		if (countInBuffer == 0) {
			if (c == '\n' || c == '\r' || c == ' ' || c == '\t') {
				continue;
			}
		}

		if (c == '\n') {
			return true;
		}

		*(buffer++) = (char) c;
		countInBuffer++;

		if (countInBuffer >= RECEIVE_BUFFER_SIZE - 1) {
			// Buffer full!
			lastMachineResponse[countInBuffer] = 0;

			// Read until end of line
			while (millis() < stopMs) {
				if (machineSerial.read() == '\n') return true;
			}

			// Timeout!
			return false;
		}
	}

	// Timeout!
	return false;
}

bool checkMachineResponse(const char* commandName, const char*expectedResponse, long startMs) {
	if (memcmp(lastMachineResponse, expectedResponse, strlen(expectedResponse)) == 0) {
#if DEBUG_MESSAGES == 1
		debugSerial.print("SUCCESS: Got '");
		debugSerial.print(lastMachineResponse);
		debugSerial.print("' from ");
		debugSerial.print(commandName);
		debugSerial.print("; in ");
		debugSerial.print(millis() - startMs);
		debugSerial.println("ms");
#endif
		return true;
	} else {
		debugSerial.print("ERROR: Expected '");
		debugSerial.print(expectedResponse);
		debugSerial.print("' from ");
		debugSerial.print(commandName);
		debugSerial.print("; got '");
		debugSerial.print(lastMachineResponse);
		debugSerial.print("' instead in ");
		debugSerial.print(millis() - startMs);
		debugSerial.println("ms");
		return false;
	}
}

bool awaitResponse(const char* commandName, const char*expectedResponse, uint32_t timeoutMs) {
	long startMs = millis();
	if (! readMachineResponse(timeoutMs)) {
		debugSerial.print("ERROR: Timeout from command '");
		debugSerial.print(commandName);
		debugSerial.print("; in ");
		debugSerial.print(millis() - startMs);
		debugSerial.println("ms");
		return false;
	}

	return checkMachineResponse(commandName, expectedResponse, startMs);
}

bool awaitOkResponse(const char* commandName, uint32_t timeoutMs) {
	return awaitResponse(commandName, "ok", timeoutMs);
}

bool sendAndAwait(const char* command, const char* expectedResponse, uint32_t timeoutMs) {
	sendLine(command);
	return awaitResponse(command, expectedResponse, timeoutMs);
}

bool sendAndAwaitOk(uint32_t timeoutMs) {
	sendLine();
	return awaitResponse("<unknown>", "ok", timeoutMs);
}

bool sendAndAwaitOk(const char* command, uint32_t timeoutMs) {
	sendLine(command);
	return awaitResponse(command, "ok", timeoutMs);
}

void sendLine(const char* line) {
	machineSerial.print(line);
#if DEBUG_ECHO == 1
	debugSerial.print(line);
#endif
	sendLine();
}
void sendLine() {
	machineSerial.print("\n");
#if DEBUG_ECHO == 1
	debugSerial.print("\n");
#endif
}
void sendStr(const char* text) {
	machineSerial.print(text);
#if DEBUG_ECHO == 1
	debugSerial.print(text);
#endif
}
void sendFixed(int32_t fixed01) {
	writeFixedTo(machineSerial, fixed01);
#if DEBUG_ECHO == 1
	writeFixedTo(debugSerial, fixed01);
#endif
}
void sendXYZ(Vector3& vec) {
	vec.writeXYZTo(machineSerial);
#if DEBUG_ECHO == 1
	vec.writeXYZTo(debugSerial);
#endif
}
void sendXY(Vector3& vec) {
	vec.writeXYTo(machineSerial);
#if DEBUG_ECHO == 1
	vec.writeXYTo(debugSerial);
#endif
}
void sendInt(int32_t num) {
	machineSerial.print(num);
#if DEBUG_ECHO == 1
	debugSerial.print(num);
#endif
}
void sendBool(bool b) {
	machineSerial.print(b);
#if DEBUG_ECHO == 1
	debugSerial.print(b);
#endif
}
void sendChar(char c) {
	machineSerial.print(c);
#if DEBUG_ECHO == 1
	debugSerial.print(c);
#endif
}