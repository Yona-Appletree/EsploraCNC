#include "ECNC.h"

ECncPage ecncPage;

void setupEsploraCnc() {
	ecncPage = ECNC_PAGE_MAIN;
	setupMainPage();
}

void loopEsploraCnc() {
	switch (ecncPage) {
		case ECNC_PAGE_MAIN: loopMainPage(); break;
		case ECNC_PAGE_NEW_POS: loopNewPosPage(); break;
		case ECNC_PAGE_LOAD_POS: loopLoadPosPage(); break;
		case ECNC_PAGE_FREE_JOG: loopFreeJogPage(); break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PAGE: Main
static uint8_t mainMenuPos = 0;

enum MainMenuItems {
	MAIN_MENU_NEW_POS = 0,
	MAIN_MENU_LOAD_POS = 1,
	MAIN_MENU_FREE_JOG = 2,
};

void setupMainPage() {
	ecncPage = ECNC_PAGE_MAIN;
	drawMenuStart(mainMenuPos);
	drawMenuItem(MAIN_MENU_NEW_POS, "New Pos");
	drawMenuItem(MAIN_MENU_LOAD_POS, "Load Pos");
	drawMenuItem(MAIN_MENU_FREE_JOG, "Free Jog");
}


void loopMainPage() {
	if (checkOkBackButtons() == MENU_BUTTON_ENTER) {
		switch (mainMenuPos) {
			case MAIN_MENU_NEW_POS:
				setupNewPosPage();
				break;

			case MAIN_MENU_LOAD_POS:
				setupLoadPosPage();
				break;

			case MAIN_MENU_FREE_JOG:
				setupFreeJogPage();
		}
	}

	handleMenuUpDown(mainMenuPos, 3);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PAGE: New Position

enum NewPosPageState {
	NEW_POS_START = 0,
	NEW_POS_ZPROBE = 1,
	NEW_POS_JOG = 3,
	NEW_POS_SAVE = 4,
	NEW_POS_FAILED = 5
} newPosPageState = NEW_POS_START;

void setupNewPosPage() {
	ecncPage = ECNC_PAGE_NEW_POS;

	Serial.println("New Pos Page");

	clearScreen();
	drawCenteredText(0, "New Position", 2);
	drawCenteredText(25, "Press OK to Home", 1);

	newPosPageState = NEW_POS_START;

	startJogControl();
}

bool newPosPageError(const char* error) {
	newPosPageState = NEW_POS_FAILED;
	clearScreen();
	drawCenteredText(20, error, 2);
	return false;
}

bool loopNewPagePosZProbe() {
	// Try Probing
	int result = machineZProbe(GCODE_NUM(-1), GCODE_NUM(60));
	if (result == -1) {
		result = machineZProbe(GCODE_NUM(1), GCODE_NUM(60));
	}
	if (result == 1) {
		clearScreen();
		drawCenteredText(0, "Z Probe", 2);
		drawCenteredText(30, "Probing...", 2);

		// Actual Probe
		if (!machineZProbe(GCODE_NUM(-100), GCODE_NUM(100)))
			return newPosPageError("Probe Failed");

		// Zero Coordinates
		if (!setMachineZero(TOUCH_PLATE_THICKNESS))
			return newPosPageError("Zeroing Failed");

		// Move up
		if (!moveMachineTo(Vector3(GCODE_NUM(10), GCODE_NUM(10), GCODE_NUM(10)), GCODE_NUM(3000)))
			return newPosPageError("Moving Failed");

		// Done!
		setupFreeJogPage();
	} else if (result == 0) {
		newPosPageError("Waiting Failed");
	}
}

void loopNewPosPage() {
	MenuButton buttonState = checkOkBackButtons();

	switch (newPosPageState) {
		case NEW_POS_FAILED:
			if (buttonState == MENU_BUTTON_BACK) {
				setupMainPage();
			}
			break;

		case NEW_POS_START:
			if (buttonState == MENU_BUTTON_ENTER) {
				clearScreen();
				drawCenteredText(20, "Homing...", 3);

				if (homeMachine()) {
					clearScreen();
					drawCenteredText(20, "Done!", 3);
					delay(1000);
					newPosPageState = NEW_POS_ZPROBE;
					clearScreen();
					drawCenteredText(0, "Z Probe", 2);
					drawCenteredText(25, "Tap Probe, plz", 2);
				} else {
					newPosPageError("Homing Failed");
				}
			} else if (buttonState == MENU_BUTTON_BACK) {
				setupMainPage();
			}
			break;

		case NEW_POS_ZPROBE:
			if (buttonState == MENU_BUTTON_BACK) {
				setupMainPage();
			} else {
				loopNewPagePosZProbe();
			}
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PAGE: Load Position

void setupLoadPosPage() {
	ecncPage = ECNC_PAGE_LOAD_POS;
	drawMenuStart(0);
	drawMenuItem(0, "Load Pos Page");
}


void loopLoadPosPage() {
	static uint8_t menuPos = 0;

	if (checkOkBackButtons() == MENU_BUTTON_BACK) {
		setupMainPage();
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PAGE: Free Jog

void setupFreeJogPage() {
	ecncPage = ECNC_PAGE_FREE_JOG;
	clearScreen();
	drawCenteredText(0, "Free Jogging", 2);

	startJogControl();
}

void loopFreeJogPage() {
	static const uint8_t joystickMinThreshold = 25;

	loopJogControl();

	if (checkOkBackButtons() == MENU_BUTTON_BACK) {
		stopJogControl();
		setupMainPage();
	}

	int32_t speed = (1023-Esplora.readSlider()) * 5;

	int joyX = -readJoystickX();
	int joyY = -readJoystickY();

	setDesiredVelocity(Vector3(
		map(abs(joyX) < joystickMinThreshold ? 0 : joyX, -512, 512, -speed, speed),
		map(abs(joyY) < joystickMinThreshold ? 0 : joyY, -512, 512, -speed, speed),
		0
	));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Menu Common
bool prevButtonStates[] = {false, false, false, false, false};

bool handleMenuUpDown(
	uint8_t &menuIndex,
	uint8_t menuItemCount
) {
	bool result = false;

	bool newButtonStates[4];
	newButtonStates[SWITCH_UP]    = Esplora.readButton(SWITCH_UP) == 0;
	newButtonStates[SWITCH_DOWN]  = Esplora.readButton(SWITCH_DOWN) == 0;

	if (prevButtonStates[SWITCH_UP] && !newButtonStates[SWITCH_UP]) {
		clearMenuCursor(menuIndex);
		menuIndex = (menuIndex - 1) % menuItemCount;
		result = true;
		drawMenuCursor(menuIndex);
	} else if (prevButtonStates[SWITCH_DOWN] && !newButtonStates[SWITCH_DOWN]) {
		clearMenuCursor(menuIndex);
		menuIndex = (menuIndex + 1) % menuItemCount;
		result = true;
		drawMenuCursor(menuIndex);
	}

	prevButtonStates[SWITCH_UP] = newButtonStates[SWITCH_UP];
	prevButtonStates[SWITCH_DOWN] = newButtonStates[SWITCH_DOWN];

	return result;
}

MenuButton checkOkBackButtons() {
	MenuButton result = MENU_BUTTON_NONE;

	bool newButtonStates[4];
	newButtonStates[SWITCH_LEFT]  = Esplora.readButton(SWITCH_LEFT) == 0;
	newButtonStates[SWITCH_RIGHT] = Esplora.readButton(SWITCH_RIGHT) == 0;

	if (prevButtonStates[SWITCH_RIGHT] && !newButtonStates[SWITCH_RIGHT]) {
		result = MENU_BUTTON_ENTER;
	} else if (prevButtonStates[SWITCH_LEFT] && !newButtonStates[SWITCH_LEFT]) {
		result = MENU_BUTTON_BACK;
	}

	prevButtonStates[SWITCH_LEFT] = newButtonStates[SWITCH_LEFT];
	prevButtonStates[SWITCH_RIGHT] = newButtonStates[SWITCH_RIGHT];

	return result;
}


