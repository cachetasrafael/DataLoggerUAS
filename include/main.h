#ifndef __MAIN_H__
#define __MAIN_H__

/* include statements */
#include <WiFi.h>
#include <NTPClient.h>
#include <RTClib.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <Arduino.h>
#include "FS.h"
#include "SPI.h"
#include "SD.h"
#include "readpwm.h"

/* macro definitions */
//OFFSET SERVER NTP
#define TIME_OFFSET 0 // Eastern Time (US & Canada)

//WiFi SSID and PW
#define WIFI_SSID "NOS_Internet_E345"
#define WIFI_PASSWORD "11070017"

//Pins
const int SD_CS = 5; // chip select pin for SD card
const int PWM_pin=25; // button pin
const int PWM_LED = 32; // led pwm activation
const int WIFI_LED = 33; // led wifi connection
const int SD_LED = 27; // led create file on sd card or append

/* struct/enum/union definitions */

//Array for PWM signal name
extern char pwmNAME[2][50];

extern int ledState;
extern int flagNTPTime;

extern unsigned long previousMillis, interval;

/* function prototypes */
void getTIME();
void appendFile(fs::FS &fs, const char * path, const char * message);
void writeFile(fs::FS &fs, const char * path, const char * message);
#endif /* __MAIN_H__ */