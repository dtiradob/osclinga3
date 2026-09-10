#ifndef OSCLINGA_GLOBALS_H
#define OSCLINGA_GLOBALS_H

// Shared includes, pin map, globals and cross-file prototypes for the three
// "sketch tabs" (osclinga_4.ino / coreo_mirko.ino / func_mirko.ino).
// Using extern declarations here (instead of relying on Arduino IDE's
// implicit tab-concatenation order) keeps the build correct no matter which
// order the build tool compiles/merges the .ino files in.

#include "Arduino.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>
#include <Adafruit_GFX.h>
#define SSD1306_NO_SPLASH  // skip the Adafruit boot logo
#include <Adafruit_SSD1306.h>
#include <ds3231.h>  //https://github.com/rodan/ds3231
#include "AiEsp32RotaryEncoder.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

// ---- Hardware profile for THIS build ----
// This src/ tree targets a plain ESP32 dev board + SSD1306 OLED bring-up
// test (no DS3231 RTC, no rotary encoders, no MAX3485 RS485 transceiver
// physically wired). Set each flag to 1 only once that specific piece of
// hardware is actually attached (or build with the pcb_board1/pcb_board2
// PlatformIO environments, which already do this via build_flags).
#ifndef HAVE_RTC
#define HAVE_RTC 0
#endif
#ifndef HAVE_ENCODERS
#define HAVE_ENCODERS 0
#endif
#ifndef HAVE_MODBUS
#define HAVE_MODBUS 0
#endif

//----------------------IO-----------------------
#define ENC2_SW 2
#define PWM1 4
// FAN_PWM shares GPIO3 with U0RXD (USB/Serial RX). Do NOT drive PWM on this
// pin while Serial is in use for flashing/logging - it will corrupt the
// console. Not currently used by firmware; needs a PCB rework/free GPIO
// before the fan output can actually be implemented.
#define FAN_PWM 3
// NOTE: GPIO12 (MTDI) is an ESP32 boot-strapping pin (flash voltage select).
// Confirmed via schematic: GPIO12 is ENC1_CLK, and is annotated there as
// "boot fails if pulled high". The rotary encoder module's pull-up holds
// CLK HIGH at rest, so at power-on/reset the chip can sample GPIO12 HIGH,
// select the wrong flash voltage, and fail to boot entirely. This is the
// confirmed root cause of a PCB-mounted ESP32 that "never boots again"
// after being wired up. GPIO2 (ENC2_SW) is a secondary, lower-severity risk
// - the schematic notes it must be floating or LOW at boot too.
#define ENC1_CLK 12
#define ENC1_SW 13
#define MODBUS_DMX_REDE 14
#define MODE 15
#define UART2RX 16
#define UART2TX 17
#define A_MULTIPLEXER 18
#define ENC2_CLK 19
// SDA/SCL (GPIO21/22) are the ESP32's default Wire pins already - no need to
// redefine the built-in SDA/SCL macros.
#define ENC2_DT 23
#define PWM3 25
#define ENC1_DT 26
#define PWM4 27
#define PWM2 32
#define ULT_TRIG 33
#define ULT_ECHO 34
#define PIR_INPUT 35

#define LEFT 1
#define BACK 2
#define FRONT 3
#define RIGHT 4

//------------------WEB Y CSV VARIABLES--------------
#define MAXSTEPS 999
extern float tiemposCSV[MAXSTEPS];
extern float F1CSV[MAXSTEPS];
extern float F2CSV[MAXSTEPS];
extern int pasosCSV;
extern int pasoActual;
extern int waitingCSV;
extern unsigned long tiempoCSV;
extern String presets[8];
extern String csvName;
extern AsyncWebServer server;

//------------------ENCODER VARIABLES--------------
extern AiEsp32RotaryEncoder rotaryEncoder1;
extern AiEsp32RotaryEncoder rotaryEncoder2;
extern float frecENC1;
extern float frecENC2;

//--------------WIFI-OSC VARIABLES---------------
#ifndef AP
#define AP 1  // 1:AP | 0:client
#endif
// Change BOARD_ID per physical board (e.g. via platformio.ini build_flags
// -D BOARD_ID=n) so each one gets its own SSID ("osclinga<BOARD_ID>") and
// its own AP gateway IP (192.168.<BOARD_ID>.1), so multiple boards running
// at once don't collide on SSID or IP.
#ifndef BOARD_ID
#define BOARD_ID 1
#endif
extern WiFiUDP Udp;
extern OSCErrorCode error;
extern String localip;

//--------------FIRMWARE VERSION (splash screen)---------------
#define FW_VERSION "4.0"
#define FW_BUILD_DATE __DATE__

//--------------LED VARIABLES--------------
extern int estorbox_on;
extern int run;
extern int led1;
extern int led2;
extern int led3;
extern int led4;
extern int int1;
extern int int2;
extern int pwm;
extern unsigned long previous_strobox;
extern int leds[4];

//----------------------MOD BUTTON VARIABLES--------------
extern int buttonState;
extern int selec;
extern int apretado;
extern unsigned long pushtime;
extern unsigned long ahora;
extern unsigned long push;
extern int modox;
extern int buttonPushCounter;
extern int lastButtonState;
extern unsigned long lastDebounceTime;
extern unsigned long debounceDelay;

//----------------------DISPLAY VARIABLES----------------
extern Adafruit_SSD1306 display;
extern String labels[4];

//--------------------MIRKO VARIABLES-------------------
extern unsigned long previous, previous2, previous_back;
extern unsigned long previousMillis, previousMillis2, previousMillis7, previousMillis8;
extern unsigned long pasado;
extern int estorbo_on, estorbo_on2, estorbo_on_back;
extern int wait, fin, preset, new_frec;
extern int dia, militar;
extern struct ts t;

//--------------------------MODBUS VARIABLES-------------
extern unsigned char frame[8];
extern float frecuencias[2];
extern bool motorStates[2];
extern const int modbusDelay;
extern unsigned long tiempo;

//--------------------------CROSS-FILE FUNCTION PROTOTYPES-------------
// osclinga_4.ino
void rotary_loop();
void IRAM_ATTR readEncoderISR();
void buttonRead();
void strobox();
void ledsControl();
void ledsOSC(OSCMessage &msg);
void estorboOSC(OSCMessage &msg);
void motoresOSC(OSCMessage &msg);
void stopAll();
void modBus_callback();
void STOP(unsigned char *frame, int address);
void RUN(unsigned char *frame, int address);
void FREC(unsigned char *frame, int address, float frecuencia);
void modBus_STATUS(unsigned char *frame, int address);
void CRC(unsigned char *frame);
void sendModBus(unsigned char *frame);
void syncClock();
void showSplash();
void reloz();
void agenda();
void printOLED();
void displayFrecs();
void handleLoadCoreo(AsyncWebServerRequest *request);
void handlePlayPreset(AsyncWebServerRequest *request);
void handleSaveSelections(AsyncWebServerRequest *request);
void notFound(AsyncWebServerRequest *request);
void handleRoot(AsyncWebServerRequest *request);
void handleSaveCSV(AsyncWebServerRequest *request);
void handleGetCSV(AsyncWebServerRequest *request);
void handleListPresets(AsyncWebServerRequest *request);
void createCoreo();
void readCSV(String param);
void modoCSV();
bool isValidPresetName(const String &name);

// coreo_mirko.ino
void coreoMirko();

// func_mirko.ino
void resetChoreoTimers();
void apaga();
void preset0();
void preset1();
void preset2();
void preset20();
void preset3();
void preset4();
void preset5();
void preset50();
void preset500();
void preset6();
void preset7();
void estorback(int runh, int led1h, int led2h, int pwmh, int int1h, int int2h);
void estorbackfin(int run, int pwm, int int1, int int2);
void estorbo(int runh, int led1h, int led2h, int pwmh, int int1h, int int2h);
void estorbo2(int runh, int led1h, int led2h, int pwmh, int int1h, int int2h);

#endif  // OSCLINGA_GLOBALS_H
