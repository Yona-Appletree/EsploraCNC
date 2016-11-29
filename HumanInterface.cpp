#include "HumanInterface.h"
#include "Persistence.h"

PageLoopFunc currentPageLoop;
//
//struct NextPage {
//	PageSetupFunc successPage;
//	PageSetupFunc failurePage;
//
//	NextPage(void (*successPage)() const, void (*failurePage)() const)
//		: successPage(successPage), failurePage(failurePage) { }
//
//	NextPage(void (*nextPage)() const)
//		: successPage(nextPage), failurePage(nextPage) { }
//};

void setupMainPage();
void setupNewPosPage();
void setupLoadPosPage(PageSetupFunc setupNextPage);
void setupFreeJogPage(PageSetupFunc setupNextPage);
void setupZProbePage(PageSetupFunc setupNextPage);
void setupHomeMachinePage(PageSetupFunc snp, bool allowSkip);

void setupEsploraCnc() {
	setupMainPage();
}

void loopEsploraCnc() {
	currentPageLoop();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PAGE: Main

enum MainMenuItems {
	MAIN_MENU_NEW_POS = 0,
	MAIN_MENU_LOAD_POS = 1,
	MAIN_MENU_HOME = 2,
	MAIN_MENU_ZPROBE = 3,
	MAIN_MENU_FREE_JOG = 4,
	MAIN_MENU_SAVE_POS = 5,
	MAIN_MENU_COUNT = 6
};

void setupMainPage() {
	static uint8_t mainMenuTopPos, mainMenuPos;

	mainMenuPos = mainMenuTopPos = 0;

	static GetMenuItemFunc itemFunc = [] (uint8_t i, char* buf, uint8_t bufSize) {
		switch (i) {
			case MAIN_MENU_NEW_POS: strncpy(buf, "New Pos", bufSize); return;
			case MAIN_MENU_LOAD_POS: strncpy(buf, "Load Pos", bufSize); return;
			case MAIN_MENU_HOME: strncpy(buf, "Home", bufSize); return;
			case MAIN_MENU_ZPROBE: strncpy(buf, "ZProbe", bufSize); return;
			case MAIN_MENU_FREE_JOG: strncpy(buf, "Free Jog", bufSize); return;
			case MAIN_MENU_SAVE_POS: strncpy(buf, "Save Pos", bufSize); return;
		}
	};

	drawMenu(mainMenuTopPos, mainMenuPos, MAIN_MENU_COUNT, itemFunc);

	currentPageLoop = [] () {
		if (checkOkBackButtons(true) == MENU_BUTTON_ENTER) {
			switch (mainMenuPos) {
				case MAIN_MENU_NEW_POS:
					setupNewPosPage();
					break;

				case MAIN_MENU_LOAD_POS:
					setupLoadPosPage(setupMainPage);
					break;

				case MAIN_MENU_HOME:
					setupHomeMachinePage(setupMainPage, true);
					break;

				case MAIN_MENU_ZPROBE:
					setupZProbePage(setupMainPage);
					break;

				case MAIN_MENU_FREE_JOG:
					setupFreeJogPage(setupMainPage);
					break;

				case MAIN_MENU_SAVE_POS:
					setupSavePosPage(setupMainPage);
					break;
			}
		}

		handleMenuUpDown(mainMenuTopPos, mainMenuPos, MAIN_MENU_COUNT, itemFunc);
	};
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PAGE: Error

void setupErrorPage(const char* error) {
	clearScreen();
	drawCenteredText(0, "Error", 2);
	drawCenteredText(20, error, 2);

	static PageLoopFunc loop = [] () {
		if (checkOkBackButtons(true) != MENU_BUTTON_NONE) {
			setupMainPage();
		}
	};
	currentPageLoop = loop;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PAGE: New Pos

void setupNewPosPage() {
	clearScreen();
	drawCenteredText(0, "New Position", 2);

	static PageLoopFunc afterFreePos = [] () {
		setupSavePosPage(setupMainPage);
	};
	static PageLoopFunc afterZProbe = [] () {
		setupFreeJogPage(afterFreePos);
	};
	static PageLoopFunc afterHome = [] () {
		setupZProbePage(afterZProbe);
	};
	setupHomeMachinePage(afterHome, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PAGE: Save Pos
void setupSavePosPage(PageSetupFunc snp) {
	static PageSetupFunc setupNextPage;
	static SavedPosName* names;
	static uint8_t menuPageTopIndex, menuIndex;

	setupNextPage = snp;

	clearScreen();
	drawCenteredText(0, "Save Position", 2);

	names = listSavedNames();
	menuPageTopIndex = menuIndex = 0;

	static GetMenuItemFunc itemFunc = [] (uint8_t i, char* buf, uint8_t bufSize) {
		snprintf(buf, bufSize, "%X.%s", i, names[i].name[0] ? names[i].name : "<Unused>");
	};

	drawMenu(menuPageTopIndex, menuIndex, SAVED_POS_COUNT, itemFunc);

	static PageLoopFunc savePosLoop = [] () {
		handleMenuUpDown(menuPageTopIndex, menuIndex, SAVED_POS_COUNT, itemFunc);

		switch (checkOkBackButtons(true)) {
			case MENU_BUTTON_BACK:
				setupNextPage();
				break;
			case MENU_BUTTON_ENTER: {
				// Save position!
				SavedPosName name;
				SavedPos pos;
				Vector3 machinePos, workingPos;
				if (readCurrentMachinePos(&machinePos) && readCurrentWorkspacePos(&workingPos)) {
					pos.xMm = FROM_GCODE_NUM(machinePos.x);
					pos.yMm = FROM_GCODE_NUM(machinePos.y);
					pos.zMm = FROM_GCODE_NUM(machinePos.z - workingPos.z);

					snprintf(name.name, sizeof(name.name), "%d,%d,%d", pos.xMm, pos.yMm, pos.zMm);
					savePos(menuIndex, name, pos);

					names = listSavedNames();
					drawMenu(menuPageTopIndex, menuIndex, SAVED_POS_COUNT, itemFunc);
					delay(2000);

					setupNextPage();
				} else {
					setupErrorPage("Failed to Read Pos");
				}
			} default: break;
		}
	};

	currentPageLoop = savePosLoop;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PAGE: Home Machine

void setupHomeMachinePage(PageSetupFunc snp, bool as) {
	static PageSetupFunc setupNextPage;
	static bool allowSkipping;
	setupNextPage = snp;
	allowSkipping = as;

	static PageSetupFunc drawHomeMachineScreen = [] () {
		clearScreen();
		drawCenteredText(0, "Home Machine", 2);
		drawCenteredText(25, "Press OK to Home", 1);
		if (allowSkipping) {
			drawCenteredText(35, "Press Cancel to Skip", 1);
		} else {
			drawCenteredText(35, "Press Cancel to Abort", 1);
		}
	};

	drawHomeMachineScreen();

	static PageLoopFunc loop = [] () {
		MenuButton buttonState = checkOkBackButtons(true);

		if (buttonState == MENU_BUTTON_ENTER) {
			clearScreen();
			drawCenteredText(20, "Homing...", 2);

			if (homeMachine()) {
				clearScreen();
				drawCenteredText(20, "Done!", 2);
				delay(1000);

				setupNextPage();
			} else {
				setupErrorPage("Homing Failed");
			}
		} else if (buttonState == MENU_BUTTON_BACK) {
			if (allowSkipping) {
				clearScreen();
				drawCenteredText(35, "Homing", 2);
				drawCenteredText(55, "Skipped", 2);
				delay(1000);

				// Allow panic out
				if (checkOkBackButtons(false) == MENU_BUTTON_BACK) {
					clearScreen();
					drawCenteredText(35, "WAIT", 2);
					drawCenteredText(55, "STOP!", 2);
					delay(1000);
					setupMainPage();
				} else {
					setupNextPage();
				}
			} else {
				clearScreen();
				drawCenteredText(35, "Cannot", 2);
				drawCenteredText(55, "Skip Home", 2);
				delay(1000);

				if (checkOkBackButtons(false) == MENU_BUTTON_BACK) {
					setupErrorPage("Homing Aborted");
				} else {
					drawHomeMachineScreen();
				}
			}
		}
	};
	currentPageLoop = loop;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PAGE: ZProbe

void setupZProbePage(PageSetupFunc snp) {
	static PageSetupFunc setupNextPage;
	setupNextPage = snp;

	clearScreen();
	drawCenteredText(0, "Z Probe", 2);
	drawCenteredText(25, "Tap Probe, plz", 2);

	static PageLoopFunc loop = [] () {
		// Try Probing
		int result = machineZProbe(GCODE_NUM(-1), GCODE_NUM(60));
		if (result == -1) {
			result = machineZProbe(GCODE_NUM(1), GCODE_NUM(60));
		}
		if (checkOkBackButtons(false) == MENU_BUTTON_BACK) {
			setupMainPage();
		}
		if (result == 1) {
			clearScreen();
			drawCenteredText(0, "Z Probe", 2);
			drawCenteredText(30, "Starting...", 2);
			delay(1000);
			clearScreen();
			drawCenteredText(0, "Z Probe", 2);
			drawCenteredText(30, "Probing...", 2);

			// Actual Probe
			if (!machineZProbe(GCODE_NUM(-100), GCODE_NUM(150)))
				return setupErrorPage("Probe Failed");

			// Zero Coordinates
			if (!setMachineCurrentZAsZero(TOUCH_PLATE_THICKNESS_001MM))
				return setupErrorPage("Zeroing Failed");

			// Move to working height
			if (!moveMachineTo(Vector3(0,0, Z_WORKING_HEIGHT_001MM), false, false, true, GCODE_NUM(3000)))
				return setupErrorPage("Moving Failed");

			// Done, wait for the last move and then go on
			waitForMachineToReachTarget(5000);
			setupNextPage();
		} else if (result == 0) {
			setupErrorPage("Waiting Failed");
		}
	};
	currentPageLoop = loop;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PAGE: Load Position

void setupLoadPosPage(PageSetupFunc snp) {
	static PageSetupFunc setupNextPage;
	static SavedPosName* names;
	static uint8_t menuPageTopIndex, menuIndex;;
	static SavedPos loadedPos;

	setupNextPage = snp;

	names = listSavedNames();
	menuPageTopIndex = menuIndex = 0;

	static GetMenuItemFunc itemFunc = [] (uint8_t i, char* buf, uint8_t bufSize) {
		snprintf(buf, bufSize, "%X.%s", i, names[i].name[0] ? names[i].name : "<Unused>");
	};

	drawMenu(menuPageTopIndex, menuIndex, SAVED_POS_COUNT, itemFunc);

	static PageSetupFunc afterHoming = [] () {
		setMachineZeroTo(
			Vector3(
				GCODE_NUM(loadedPos.xMm),
				GCODE_NUM(loadedPos.yMm),
				GCODE_NUM(loadedPos.zMm)
			));

		// Goto the working pos above this position
		moveMachineTo(Vector3(0,0,Z_MOVING_HEIGHT_001MM), false, false, true, GCODE_NUM(3000));
		moveMachineTo(Vector3(0,0,Z_MOVING_HEIGHT_001MM), true, true, true, GCODE_NUM(3000));
		moveMachineTo(Vector3(0,0,Z_WORKING_HEIGHT_001MM), false, false, true, GCODE_NUM(3000));

		waitForMachineToReachTarget(60000);

		setupNextPage();
	};

	static PageLoopFunc loop = [] () {
		handleMenuUpDown(menuPageTopIndex, menuIndex, SAVED_POS_COUNT, itemFunc);

		switch (checkOkBackButtons(true)) {
			case MENU_BUTTON_BACK:
				setupNextPage();
				break;
			case MENU_BUTTON_ENTER: {
				// Load
				if (names[menuIndex].name[0]) {
					loadPos(menuIndex, loadedPos);

					// Home
					setupHomeMachinePage(afterHoming, true);
				}
			} default: break;
		}
	};

	currentPageLoop = loop;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PAGE: Free Jog

void setupFreeJogPage(PageSetupFunc snp) {
	static PageSetupFunc setupNextPage;
	setupNextPage = snp;

	clearScreen();
	drawCenteredText(0, "Free Jogging", 2);

	startJogControl();

	static PageLoopFunc loop = [] () {
		static const uint8_t joystickMinThreshold = 5;
		static Vector3 lastDisplayedPos(-100,-100,-100);

		loopJogControl();

		Vector3 curPos = getCurrentJogTarget();
		if (curPos != lastDisplayedPos) {
			lastDisplayedPos = curPos;
			updateText(30, true, 2, "X: %03d.%02d", curPos.x / 100, curPos.x % 100);
			updateText(50, true, 2, "Y: %03d.%02d", curPos.y / 100, curPos.y % 100);
			updateText(70, true, 2, "Z: %03d.%02d", curPos.z / 100, curPos.z % 100);
		}

		switch (checkOkBackButtons(true)) {
			case MENU_BUTTON_BACK:
				stopJogControlAndRound();
				setupMainPage();
				return;

			case MENU_BUTTON_ENTER:
				stopJogControlAndRound();
				setupNextPage();
				return;
		}

		int32_t speed = (1023-Esplora.readSlider()) * 5;

		int joyX = -readJoystickX();
		int joyY = -readJoystickY();

		setDesiredVelocity(Vector3(
			map(abs(joyX) < joystickMinThreshold ? 0 : joyX, -512, 512, -speed, speed),
			map(abs(joyY) < joystickMinThreshold ? 0 : joyY, -512, 512, -speed, speed),
			0
		));
	};
	currentPageLoop = loop;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Menu Common
bool prevButtonStates[] = {false, false, false, false, false};

void drawMenu(
	uint8_t menuPageTopIndex,
	uint8_t menuIndex,
	uint8_t menuItemCount,
	GetMenuItemFunc itemFunc
) {
	drawMenuStart(menuIndex - menuPageTopIndex);
	char buf[16];
	for (uint8_t i = menuPageTopIndex; i < menuPageTopIndex + MENU_ITEMS_PER_PAGE && i<menuItemCount; i++) {
		itemFunc(i, buf, sizeof(buf));
		drawMenuItem(i - menuPageTopIndex, buf);
	}
}

void handleMenuUpDown(
	uint8_t &menuPageTopIndex,
	uint8_t &menuIndex,
	uint8_t menuItemCount,
	GetMenuItemFunc itemFunc
) {
	bool posChanged = false;

	bool newButtonStates[4];
	newButtonStates[SWITCH_UP]    = Esplora.readButton(SWITCH_UP) == 0;
	newButtonStates[SWITCH_DOWN]  = Esplora.readButton(SWITCH_DOWN) == 0;

	if (prevButtonStates[SWITCH_UP] && !newButtonStates[SWITCH_UP]) {
		clearMenuCursor(menuIndex - menuPageTopIndex);
		menuIndex = menuIndex > 0 ? (menuIndex-1) : (menuItemCount-1);
		posChanged = true;
	} else if (prevButtonStates[SWITCH_DOWN] && !newButtonStates[SWITCH_DOWN]) {
		clearMenuCursor(menuIndex - menuPageTopIndex);
		menuIndex = (menuIndex + 1) % menuItemCount;
		posChanged = true;
	}

	if (posChanged) {
		bool redrawNeeded = false;

		if (menuIndex < menuPageTopIndex) {
			menuPageTopIndex = menuIndex;
			redrawNeeded = true;
		} else if (menuIndex >= menuPageTopIndex + MENU_ITEMS_PER_PAGE) {
			menuPageTopIndex = menuIndex - MENU_ITEMS_PER_PAGE + 1;
			redrawNeeded = true;
		}

		if (redrawNeeded) {
			drawMenu(
				menuPageTopIndex,
				menuIndex,
				menuItemCount,
				itemFunc
			);
		}

		drawMenuCursor(menuIndex - menuPageTopIndex);
	}

	prevButtonStates[SWITCH_UP] = newButtonStates[SWITCH_UP];
	prevButtonStates[SWITCH_DOWN] = newButtonStates[SWITCH_DOWN];
}

MenuButton checkOkBackButtons(bool requireTap) {
	MenuButton result = MENU_BUTTON_NONE;

	bool newButtonStates[4];
	newButtonStates[SWITCH_LEFT]  = Esplora.readButton(SWITCH_LEFT) == 0;
	newButtonStates[SWITCH_RIGHT] = Esplora.readButton(SWITCH_RIGHT) == 0;

	if (requireTap) {
		if (prevButtonStates[SWITCH_RIGHT] && !newButtonStates[SWITCH_RIGHT]) {
			result = MENU_BUTTON_ENTER;
		} else if (prevButtonStates[SWITCH_LEFT] && !newButtonStates[SWITCH_LEFT]) {
			result = MENU_BUTTON_BACK;
		}

		prevButtonStates[SWITCH_LEFT] = newButtonStates[SWITCH_LEFT];
		prevButtonStates[SWITCH_RIGHT] = newButtonStates[SWITCH_RIGHT];
	} else {
		if (newButtonStates[SWITCH_RIGHT]) {
			result = MENU_BUTTON_ENTER;
		} else if (newButtonStates[SWITCH_LEFT]) {
			result = MENU_BUTTON_BACK;
		}

		prevButtonStates[SWITCH_LEFT] = false;
		prevButtonStates[SWITCH_RIGHT] = false;
	}

	return result;
}


