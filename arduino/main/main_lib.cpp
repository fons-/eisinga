#if ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "main_lib.h"
#include <LiquidCrystal.h>
#include "Wire.h"

void serialSetup() {
	Serial.begin(9600);
	Serial.println("Hello World");
}

void lcdSetup() {
	pinMode(13, OUTPUT);
	lcd.begin(16, 2);
	lcd.print("Hello world");
	delay(1000);
}

void sensorSetup() {
	Wire.begin();
	
	#ifdef DEBUG
	Serial.begin(9600);
	Serial.println("Initializing I2C devices...");
	#endif
	
	accel.initialize();
	mag.initialize();
	
	#ifdef DEBUG
	Serial.println("Testing device connections...");
	Serial.println(accel.testConnection() ? "ADXL345 connection successful" : "ADXL345 connection failed");
	Serial.println(mag.testConnection() ? "HMC5883L connection successful" : "HMC5883 connection failed");
	#endif
	
	pinMode(LED_PIN, OUTPUT);
	pinMode(BTN_PIN, INPUT);
	
	accel.setRange(3);
	accel.setFullResolution(255);
	accel.setOffset(0, 0, 0);
}

void calibrate() {
	lcd.setCursor(0, 0);
	lcd.print("Flat surface");
	lcd.setCursor(0, 1);
	lcd.print("facing North");
	
	#ifdef DEBUG
	Serial.println("Calibration: flat surface pointing North");
	#endif
	
	pause();
	
	accel.getAcceleration(&rawx, &rawy, &rawz);
	delay(10);
	accel.getAcceleration(&rawx, &rawy, &rawz);
	xOffsetA = -rawx;
	yOffsetA = -rawy;
	zOffsetA = zScaleA - rawz;
	
	#ifdef DEBUG
	Serial.print("A:\t");
	Serial.print(rawx); Serial.print(",\t");
	Serial.print(rawy); Serial.print(",\t");
	Serial.print(rawz); Serial.println("");
	#endif
	
	mag.getHeading(&rawx, &rawy, &rawz);
	delay(10);
	mag.getHeading(&rawx, &rawy, &rawz);
	delay(10);
	mag.getHeading(&rawx, &rawy, &rawz);
	delay(10);
	mag.getHeading(&rawx, &rawy, &rawz);
	xOffsetM = (mIntensityY / mGain) - rawx;
	yOffsetM = (mIntensityX / mGain) - rawy;
	zOffsetM = (mIntensityZ / mGain) - rawz;
	
	#ifdef DEBUG
	Serial.print("M:\t");
	Serial.print(rawx); Serial.print(",\t");
	Serial.print(rawy); Serial.print(",\t");
	Serial.print(rawz); Serial.println("");
	Serial.println("");
	Serial.print("A:\t");
	Serial.print(xOffsetA); Serial.print(",\t");
	Serial.print(yOffsetA); Serial.print(",\t");
	Serial.print(zOffsetA); Serial.println("");
	Serial.print("M:\t");
	Serial.print(xOffsetM); Serial.print(",\t");
	Serial.print(yOffsetM); Serial.print(",\t");
	Serial.print(zOffsetM); Serial.println("");
	Serial.println("Calibration complete");
	delay(1000);
	pause();
	#endif
}

void pause() {
	while(digitalRead(BTN_PIN) == LOW) {
		delay(10);
	}
}

void readData() {
	mag.getHeading(&rawx, &rawy, &rawz);
	x = rawx + xOffsetM;
	y = rawy + yOffsetM;
	z = rawz + zOffsetM;
	roll = atan2(x, y) * 57.29577951308233;
	
	#ifdef DEBUG
	Serial.print(x); Serial.print(",\t");
	Serial.print(y); Serial.print(",\t");
	Serial.print(z); Serial.print(",\t");
	Serial.println(roll);
	#endif
	
	lcd.setCursor(0,1);
	lcd.print(roll);
	
	delay(50);
	
	blinkState = !blinkState;
	digitalWrite(LED_PIN, blinkState);
}
