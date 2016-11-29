#include <SPI.h>

#include "HumanInterface.h"

#include "PDQ_ST7735_config.h"
#include <PDQ_GFX.h>
#include <PDQ_ST7735.h>
#include <Esplora.h>
#include <EEPROM.h>
#include "Persistence.h"

PDQ_ST7735 tft;

int initialX, initialY;

void setup(void) {
	Serial.end();
	Serial.begin(115200);

	Serial1.begin(115200);
	Serial.println("Esplora CNC Init");

	setupLCD();
	setupEsploraHelper();
	setupEsploraCnc();

	//clearAllSaved();
}

void loop() {
	loopEsploraCnc();
	delay(10);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Menu Drawing Routines

const uint8_t charWidth = 10;
const uint8_t charHeight = 16;

void clearScreen() {
	tft.fillScreen(ST7735_BLACK);
}

void drawMenuStart(uint8_t cursorPos) {
	tft.fillScreen(ST7735_BLACK);
	tft.setTextSize(2);
	tft.setTextColor(ST7735_WHITE);
	tft.setTextWrap(false);

	drawMenuCursor(cursorPos);
}

void drawMenuItem(
	uint8_t index,
	const char* text
) {
	tft.setCursor(charWidth, index * charHeight);
	tft.print(text);
}


void drawCenteredText(
	uint16_t y,
	const char* text,
	uint8_t size
) {
	tft.setTextSize(size);
	tft.setCursor((140 - strlen(text)*5*size)/2, y);
	tft.print(text);
}

void updateText(
	uint16_t y,
	bool center,
	uint8_t size,
    const char* format,
    ...
) {
	tft.setTextSize(size);
	tft.fillRect(0, y, 160, charHeight, ST7735_BLACK);

	char buf[20];


	va_list(args);
	va_start(args, format);
	vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);

	tft.setCursor(center ? (140 - strlen(buf)*5*size)/2 : 0, y);
	tft.print(buf);
}

void drawMenuCursor(
	uint8_t index
) {
	tft.setTextSize(2);
	tft.setTextColor(ST7735_WHITE);
	tft.setTextWrap(false);

	tft.setCursor(0, index * charHeight);
	tft.print("> ");
}

void clearMenuCursor(
	uint8_t index
) {
	tft.fillRect(
		0, index * charHeight,
		charWidth, charHeight,
		ST7735_BLACK
	);
}

bool drawJoystickIndicator(
	uint8_t centerX,
	uint8_t centerY,
	uint8_t rOuter,
	uint8_t rInner
) {
	static int16_t prevIndicatorX = 5;
	static int16_t prevIndicatorY = -5;
	static int16_t prevSlider = 5;

	int8_t indicatorX = map(readJoystickX(), 512, -512, -rOuter + rInner, rOuter - rInner);
	int8_t indicatorY = map(readJoystickY(), -512, 512, -rOuter + rInner, rOuter - rInner);
	int16_t slider = 1024 - Esplora.readSlider();

	bool result = false;
	if (abs(slider - prevSlider) >= 2) {
		uint16_t sliderWidth = map(slider, 0, 1024, 0, rOuter*2-2);
		tft.fillRect(
			centerX - rOuter + 1,
			centerY + rOuter + 1,
			sliderWidth,
			rInner,
			ST7735_WHITE
		);

		tft.fillRect(
			centerX - rOuter + sliderWidth + 1,
			centerY + rOuter + 1,
			rOuter*2 - sliderWidth,
			rInner,
			ST7735_BLACK
		);

		prevSlider = slider;
		result = true;
	}

	if (abs(indicatorX - prevIndicatorX) >= 2 || abs(indicatorY - prevIndicatorY) >= 2) {
		tft.fillCircle(
			centerX + prevIndicatorX,
			centerY + prevIndicatorY,
			rInner,
			ST7735_BLACK
		);

		tft.drawRoundRect(
			centerX - rOuter,
			centerY - rOuter,
			rOuter*2,
			rOuter*2,
			rInner,
			ST7735_WHITE
		);

		tft.fillCircle(
			centerX + indicatorX,
			centerY + indicatorY,
			rInner,
			ST7735_WHITE
		);

		prevIndicatorX = indicatorX;
		prevIndicatorY = indicatorY;

		result = true;
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Esplora Helper Routines

int initialJoystickX, initialJoystickY;
void setupEsploraHelper() {
	initialJoystickY = Esplora.readJoystickY();
	initialJoystickX = Esplora.readJoystickX();
}

int readJoystickX() {
	return Esplora.readJoystickX() - initialJoystickX;
}

int readJoystickY() {
	return Esplora.readJoystickY() - initialJoystickY;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LCD Setup Routines

void setupLCD() {
	// Reset the LCD Controller
	pinMode(ST7735_RST_PIN, OUTPUT);
	digitalWrite(ST7735_RST_PIN, HIGH);
	delay(10);
	digitalWrite(ST7735_RST_PIN, LOW);
	delay(10);
	digitalWrite(ST7735_RST_PIN, HIGH);
	delay(10);

	tft.begin();

	// Set rotation for the Esplora
	tft.setRotation(3);
}

/*
 - New Position
   ~ Homing: Z,X,Y
   ~ XY Free Move
   Touchplate
    - Really?
    - Really Really?
 - Load Position
 - Free Jog
 - Spindle Control
 */