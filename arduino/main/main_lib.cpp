#if ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "main_lib.h"
#include <LiquidCrystal.h>
#include "Wire.h"

void serialSetup() {
	#ifdef DEBUG
	Serial.begin(9600);
	Serial.println("Hello World");
	#endif
	Wire.begin();
}

void rtcSetup() {
	#ifdef DEBUG
	Serial.println("Initializing RTC connection...");
	#endif
	rtc.initialize();
	rtc.setClockRunning(true);
	#ifdef DEBUG
	Serial.println(rtc.testConnection() ? "DS1307 connection successful" : "DS1307 connection failed");
	#ifdef SETTIME
	Serial.setTimeout(20000);// 20 sec
	Serial.println("Unix time in s:");
	Serial.flush();
	while(Serial.available() < 10); {
		delay(10);
	}
	char inputArray[10];
	uint32_t inputT = 0;
	Serial.readBytes(inputArray, 10);
	for(int i = 0; i < 10; i++) {
		inputT += inputArray[i] - '0';
		if(i != 9) {
			inputT *= 10;
		}
	}
	Serial.println(inputT);
	rtc.setDateTime(DateTime(inputT));
	delay(100);
	/*Serial.println(rtc.getYear());
	Serial.println(rtc.getMonth());
	Serial.println(rtc.getDay());
	Serial.println(rtc.getHours24());
	Serial.println(rtc.getMinutes());*/
	#endif
	Serial.println("Current epoch:");
	Serial.println(rtc.getDateTime().unixtime());
	#endif
}

void lcdSetup() {
	pinMode(13, OUTPUT);
	lcd.begin(16, 2);
	lcd.print("Hello world");
	delay(1000);
}

void sensorSetup() {
	
	#ifdef DEBUG
	Serial.println("Initializing other I2C devices...");
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
	/*XYZ pm[4];
	XYZ pa[4];
	for(unsigned char n = '1'; n < '5'; n++) {
		displayRetrieveRaw(n, &pa[n - '1'], &pm[n - '1']);
	}
	//aOffset = FindCenter(pa);
	aOffset = XYZ(-135, -239, 1367);
	mOffset = FindCenter(pm);*/
	
	delay(200);
	
	double xtx[4][4];
	double xty[4][4];
	
	for(int ix = 0; ix < 4; ix++) {
		for(int iy = 0; iy < 4; iy++) {
			xtx[ix][iy] = xty[ix][iy] = 0.0;
		}
	}
	
	while(digitalRead(BTN_PIN) == LOW) {
		retrieveRaw(&mData);
		
		
		
		delay(100);
	}
	
	
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print(aOffset.x);
	
	delay(200);
	pause();
	
	
	#ifdef DEBUG
	Serial.println("Calibration complete: ");
	Serial.print("aOff: ");
	Serial.print(aOffset.x); Serial.print(", ");
	Serial.print(aOffset.y); Serial.print(", ");
	Serial.println(aOffset.z);
	Serial.print("mOff: ");
	Serial.print(mOffset.x); Serial.print(", ");
	Serial.print(mOffset.y); Serial.print(", ");
	Serial.println(mOffset.z);
	#endif
}

void retrieveRaw(XYZ *aData, XYZ *mData) {
	accel.getAcceleration(&rawx, &rawy, &rawz);
	int32_t xAccum, yAccum, zAccum;
	xAccum = yAccum = zAccum = 0;
	for(int i = 0; i < 10; i++) {
		delay(10);
		accel.getAcceleration(&rawx, &rawy, &rawz);
		xAccum += rawx;
		yAccum += rawy;
		zAccum += rawz;
	}
	*aData = XYZ(double(xAccum)*0.1, double(yAccum)*0.1, double(zAccum)*0.110714);
	
	xAccum = yAccum = zAccum = 0;
	
	mag.getHeading(&rawx, &rawy, &rawz);
	for(int i = 0; i < 10; i++) {
		delay(10);
		mag.getHeading(&rawx, &rawy, &rawz);
		xAccum += rawx;
		yAccum += rawy;
		zAccum += rawz;
	}
	*mData = XYZ(double(xAccum)/10.0, double(yAccum)/10.0, double(zAccum)/10.0);
}

void retrieveRaw(XYZ *mData) {
	int32_t xAccum, yAccum, zAccum;
	xAccum = yAccum = zAccum = 0;
	mag.getHeading(&rawx, &rawy, &rawz);
	for(int i = 0; i < 10; i++) {
		delay(10);
		mag.getHeading(&rawx, &rawy, &rawz);
		xAccum += rawx;
		yAccum += rawy;
		zAccum += rawz;
	}
	*mData = XYZ(double(xAccum)/10.0, double(yAccum)/10.0, double(zAccum)/10.0);
}

void displayRetrieveRaw(char n, XYZ *aData, XYZ *mData) {
	lcd.setCursor(0, 0);
	lcd.print(n);
	lcd.print("/4");
	pause();
	lcd.clear();
	delay(100);
	retrieveRaw(aData, mData);
}

void outputSerialData() {
	retrieveRaw(&aData, &mData);
	
	byte a[4];
	int32_t n;
	
	n = int32_t(mData.x);
	memcpy(a, &n, sizeof(n));
	Serial.write(a, 4);
	n = int32_t(mData.y);
	memcpy(a, &n, sizeof(n));
	Serial.write(a, 4);
	n = int32_t(mData.z);
	memcpy(a, &n, sizeof(n));
	Serial.write(a, 4);
	
	Serial.write(255);
	Serial.write(255);
	Serial.write(255);
}

void pause() {
	while(digitalRead(BTN_PIN) == LOW) {
		delay(10);
	}
}

void updateRtc() {
	lst = 18.697374558 + 24.06570982441908 * rtc.getDateTime().dayFrac();
}

void readData() {
	retrieveRaw(&aData, &mData);
	
	aData = XYZ::Subtract(aData, aOffset);
	mData = XYZ::Subtract(mData, mOffset);
	
	temp = mData.x;
	mData.x = -mData.y;
	mData.y = -temp;
	
	// Scale data to unit length?
	
	roll = atan2(aData.x, aData.z);
	
	temp = sqrt(aData.x * aData.x + aData.y * aData.y + aData.z * aData.z);
	xzlength = sqrt(aData.x * aData.x + aData.z * aData.z);
	
	pitch = atan(-aData.y / xzlength);
	
	spitch = -aData.y / temp;
	cpitch = xzlength / temp;
	
	sroll = aData.x / xzlength;
	croll = aData.z / xzlength;
	
	yaw = atan2(
		(mData.z * sroll) - (mData.y * croll), 
		(mData.x * cpitch) + (mData.y * spitch * sroll) + (mData.z * spitch * croll));
	
	dec = asin(spitch * sin(51.643 / 57.29577951308233) + cpitch * cos(51.643 / 57.29577951308233) * cos(yaw));
	ra = lst - asin(-sin(yaw) * cpitch / cos(dec));
	
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print(yaw * 57.29577951308233);
	lcd.setCursor(8,0);
	lcd.print(",");
	lcd.print(pitch * 57.29577951308233);
	lcd.setCursor(0,1);
	lcd.print(roll * 57.29577951308233);
	lcd.setCursor(8,1);
	lcd.print(",");
	lcd.print(dec * 57.29577951308233);
	
	delay(500);
	
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print(mData.x);
	lcd.setCursor(8,0);
	lcd.print(",");
	lcd.print(mData.y);
	lcd.setCursor(0,1);
	lcd.print(mData.z);
	
	delay(400);
	
	blinkState = !blinkState;
	digitalWrite(LED_PIN, blinkState);
}
