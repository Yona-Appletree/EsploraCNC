#ifndef ECNC_H
#define ECNC_H

#include <Esplora.h>
#include "ECNCGCode.h"
#include "Vector3.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

// Machine limits in 100ths of a mm
#define MIN_MACHINE_X 500
#define MIN_MACHINE_Y 500
#define MAX_MACHINE_X 80000
#define MAX_MACHINE_Y 80000

#define TOUCH_PLATE_THICKNESS GCODE_NUM(1.6)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main Program Loop

void setupEsploraCnc();
void loopEsploraCnc();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Graphics Drawing Routines

void clearScreen();
void drawMenuStart(uint8_t cursorPos);
void drawMenuItem(
	uint8_t index,
	const char* text
);
void drawCenteredText(
	uint16_t y,
	const char* text,
	uint8_t size
);
void drawMenuCursor(
	uint8_t index
);
void clearMenuCursor(
	uint8_t index
);

bool drawJoystickIndicator(
	uint8_t centerX,
	uint8_t centerY,
	uint8_t rOuter,
	uint8_t rInner
);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Esplora Helper Routines

void setupEsploraHelper();
int readJoystickX();
int readJoystickY();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LCD Setup Routines

void setupLCD();


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ECNC Page Handling

enum MenuButton {
	MENU_BUTTON_NONE = 0,
	MENU_BUTTON_BACK = 1,
	MENU_BUTTON_ENTER = 2,
};

void setupMainPage();
void loopMainPage();
bool handleMenuUpDown(
	uint8_t &menuIndex,
	uint8_t menuItemCount
);
MenuButton checkOkBackButtons();
void setupNewPosPage();
void loopNewPosPage();
void setupLoadPosPage();
void loopLoadPosPage();
void setupFreeJogPage();
void loopFreeJogPage();

enum ECncPage {
	ECNC_PAGE_MAIN = 0,
	ECNC_PAGE_NEW_POS = 1,
	ECNC_PAGE_LOAD_POS = 2,
	ECNC_PAGE_FREE_JOG = 3,
};


#endif