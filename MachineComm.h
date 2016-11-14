//
// Created by yona on 11/13/16.
//

#ifndef EXPLORACNC_MACHINECOMM_H
#define EXPLORACNC_MACHINECOMM_H

#include <stdint.h>
#include "Vector3.h"

extern char* lastMachineResponse;

void clearSerialInputBuffer();
bool checkMachineResponse(const char* commandName, const char* expectedResponse, long startMs);
bool awaitResponse(const char* commandName, const char*expectedResponse, uint32_t timeoutMs);
bool readMachineResponse(uint32_t timeoutMs);
bool awaitOkResponse(const char* commandName, uint32_t timeoutMs);
bool sendAndAwait(const char* command, const char* expectedResponse, uint32_t timeoutMs);
bool sendAndAwaitOk(const char* command, uint32_t timeoutMs);

void sendLine(const char* line);
void sendLine();
void sendStr(const char* text);
void sendFixed(int32_t fixed01);
void sendXYZ(Vector3& vec);
void sendXY(Vector3& vec);
void sendInt(int32_t num);
void sendBool(bool b);
void sendChar(char c);

#endif //EXPLORACNC_MACHINECOMM_H
