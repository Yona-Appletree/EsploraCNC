#ifndef ECNC_H
#define ECNC_H

#include <Esplora.h>
#include "MachineControl.h"
#include "Vector3.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

// Machine limits in 100ths of a mm
#define MIN_MACHINE_X 500
#define MIN_MACHINE_Y 500
#define MAX_MACHINE_X 80000
#define MAX_MACHINE_Y 80000

#define Z_WORKING_HEIGHT_001MM GCODE_NUM(2)
#define Z_MOVING_HEIGHT_001MM GCODE_NUM(10)
#define TOUCH_PLATE_THICKNESS_001MM GCODE_NUM(1.6)

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
void updateText(
	uint16_t y,
	bool center,
	uint8_t size,
	const char* format,
	...
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

#define MENU_ITEMS_PER_PAGE 7

typedef void (* PageSetupFunc)();
typedef void (* PageLoopFunc)();
typedef void (* GetMenuItemFunc)(uint8_t, char*, uint8_t);

enum MenuButton {
	MENU_BUTTON_NONE = 0,
	MENU_BUTTON_BACK = 1,
	MENU_BUTTON_ENTER = 2,
};

void handleMenuUpDown(
	uint8_t &menuPageTopIndex,
	uint8_t &menuIndex,
	uint8_t menuItemCount,
	GetMenuItemFunc itemFunc
);
void drawMenu(
	uint8_t menuPageTopIndex,
	uint8_t menuIndex,
	uint8_t menuItemCount,
	GetMenuItemFunc itemFunc
);
MenuButton checkOkBackButtons(bool requireTap);


void setupErrorPage(const char* error);
void setupSavePosPage(PageSetupFunc snp);



#endif