#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
#include <ds3231.h>  //https://github.com/rodan/ds3231
//#include "AiEsp32RotaryEncoder.h"
#include "Arduino.h"

/// CODE TO TEST THE OLD PCB
//----------------------IO-----------------------

//#define ENC2_SW 2

//#define FAN_PWM 3
//#define ENC1_CLK 13  // 12 in the schematic
//#define ENC1_SW 12   // 13 in the schematic
//#define MODBUS_DMX_REDE 14
#define MODE 15
#define UART2RX 16
#define UART2TX 17
//#define A_MULTIPLEXER 18
//#define ENC2_CLK 19

#define SDA 21
#define SCL 22
//#define ENC2_DT 23

//#define ENC1_DT 26

#define ULT_TRIG 33
#define ULT_ECHO 35
#define PIR_INPUT 9

#define PWM1 4
#define PWM2 32
#define PWM3 25
#define PWM4 27

#define LEFT 1
#define BACK 2
#define FRONT 3
#define RIGHT 4



// //----------------------ENCODER-----------------------
// #define ROTARY_ENCODER_VCC_PIN -1
// #define ROTARY_ENCODER_STEPS 1

// //instead of changing here, rather change numbers above
// AiEsp32RotaryEncoder rotaryEncoder1 = AiEsp32RotaryEncoder(ENC1_CLK, ENC1_DT, ENC1_SW, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);
// AiEsp32RotaryEncoder rotaryEncoder2 = AiEsp32RotaryEncoder(ENC2_CLK, ENC2_DT, ENC2_SW, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);
// float frecENC1 = 0.0;
// float frecENC2 = 0.0;


//--------------WIFI - OSC---------------
const char *ssid = "Plan Humboldt 2.4Ghz";
const char *password = "holaplan0!";
WiFiUDP Udp;
const unsigned int localPort = 9000;
OSCErrorCode error;

//--------------LED VARIABLES--------------

int estorbox_on = 0;
int run = 0;
int led1 = 0;
int led2 = 0;
int led3 = 0;
int led4 = 0;
int int1 = 0;
int int2 = 0;
int pwm = 0;
unsigned long previous_strobox = 0;
int leds[4] = { 0, 0, 0, 0 };

//------------PUSH BUTTON VARIABLES-----------

unsigned char frame[8];
int buttonState = 0;
int selec = 0;
int apretado = 0;
unsigned long pushtime = 0;
unsigned long ahora;
unsigned long push;
int modox = 0;
int buttonPushCounter = 0;           // counter for the number of button presses
int lastButtonState = 0;             // previous state of the
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

//------------------MIRKO VARIABLES-------------------

unsigned long previous = 0, previous2 = 0, previous_back = 0;
unsigned long previousMillis = 0, previousMillis2 = 0, previousMillis7 = 0, previousMillis8 = 0;
unsigned long pasado = 0;
int estorbo_on = 0, estorbo_on2 = 0, estorbo_on_back = 0;
int wait = 0, fin = 0, preset = 1, new_frec = 40;
int dia, militar;
struct ts t;

//--------------------MODBUS VARIABLES---------------------

float frecuencias[2] = { 0.0, 0.0 };
bool motorStates[2] = { 0, 0 };
unsigned long tiempo = 0;
const int modbusDelay = 10;

//---------------------DISPLAY VARIABLES------------------------

// Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire, -1);

String labels[4] = { "", "", "", "" };


//---------------FUNCIONES ENCODER----------------------


// void rotary_loop() {
//   //dont print anything unless value changed
//   if (rotaryEncoder1.encoderChanged()) {
//     Serial.print("Value E1: ");
//     frecENC1 = rotaryEncoder1.readEncoder() / 10.;
//     Serial.println(frecENC1);
//     FREC(frame, 1, frecENC1);
//   }
//   if (rotaryEncoder1.isEncoderButtonClicked()) {
//     static unsigned long lastTimePressed = 0;
//     //ignore multiple press in that time milliseconds
//     if (millis() - lastTimePressed < 500) {
//       return;
//     }
//     lastTimePressed = millis();
//     Serial.print("button1 pressed ");
//     if(motorStates[0]){
//       STOP(frame,1);
//     }else {
//       RUN(frame,1);
//     }

//   }

//   if (rotaryEncoder2.encoderChanged()) {
//     Serial.print("Value E2: ");
//     frecENC2 = rotaryEncoder2.readEncoder() / 10.;
//     Serial.println(frecENC2);
//     FREC(frame, 2, frecENC2);
//   }
//   if (rotaryEncoder2.isEncoderButtonClicked()) {
//       static unsigned long lastTimePressed = 0;
//     //ignore multiple press in that time milliseconds
//     if (millis() - lastTimePressed < 500) {
//       return;
//     }
//     lastTimePressed = millis();
//     Serial.print("button2 pressed ");
//     if(motorStates[1]){
//       STOP(frame,2);
//     }else {
//       RUN(frame,2);
//     }
//   }
// }

// void IRAM_ATTR readEncoderISR() {
//   rotaryEncoder1.readEncoder_ISR();
//   rotaryEncoder2.readEncoder_ISR();
// }

//-----------------------------------------------------

void setup() {

  //--------------SERIAL SETUP-------------------
  Serial.begin(115200);
  while (!Serial) { delay(100); }
  // ************************** MODBUS ***************************************
  Serial2.begin(115200, SERIAL_8N1, UART2RX, UART2TX);  // Inicia UART2 Rx=16 Tx=17



  //-----------------WIFI SETUP--------------------------
  delay(10);
  Serial.println();
  Serial.println("******************************************************");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
#ifdef ESP32
  Serial.println(localPort);
#else
  Serial.println(Udp.localPort());
#endif

  //---------------DISPLAY SETUP-----------------------------
  // display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  // display.display();
  // delay(100);
  // display.clearDisplay();
  // display.display();
  // display.setTextColor(SSD1306_WHITE);
  // display.setTextSize(1);
  // display.println(WiFi.localIP());
  // String localip = WiFi.localIP().toString();
  // labels[0] = "IP: " + localip;

  //--------------RTC SETUP--------------------------
  DS3231_get(&t);
  militar = t.min + t.hour * 100;
  dia = t.mon + t.mday * 100;
  Serial.println("militar: ");
  Serial.println(militar);
  Serial.println("dia: ");
  Serial.println(dia);
  labels[1] = "date:" + String(t.mday) + "/" + String(t.mon) + " time:" + String(t.hour) + ":" + String(t.min);


  //--------------------LED SETUP------------------------
  // (pin, canal)
  ledcAttachPin(PWM1, LEFT);   // IZQUIERDA
  ledcAttachPin(PWM2, BACK);   // ATRÁS
  ledcAttachPin(PWM3, FRONT);  // ADELANTE
  ledcAttachPin(PWM4, RIGHT);  // DERECHA
  // (canal,frecuencia,resolución)
  ledcSetup(LEFT, 1000, 8);
  ledcSetup(BACK, 1000, 8);
  ledcSetup(FRONT, 1000, 8);
  ledcSetup(RIGHT, 1000, 8);

  // button
  pinMode(MODE, INPUT);

  // //modBus pin
  // pinMode(MODBUS_DMX_REDE, OUTPUT);
  // digitalWrite(MODBUS_DMX_REDE, LOW);  //lo iniciamos en LOW, listo para leer

  // // Multiplex
  // pinMode(A_MULTIPLEXER, OUTPUT);
  // digitalWrite(A_MULTIPLEXER, LOW);  // LOW ENVIAR MODBUS; HIGH ENVIAR DMX


  //--------------ENCODER SETUP------------------
  // //we must initialize rotary encoder
  // rotaryEncoder1.begin();
  // rotaryEncoder2.begin();
  // rotaryEncoder1.setup(readEncoderISR);
  // rotaryEncoder2.setup(readEncoderISR);

  // //set boundaries and if values should cycle or not
  // //in this example we will set possible values between 0 and 1000;
  // bool circleValues = false;
  // rotaryEncoder1.setBoundaries(0, 330, circleValues);  //minValue, maxValue, circleValues true|false (when max go to min and vice versa)
  // rotaryEncoder2.setBoundaries(0, 330, circleValues);  //minValue, maxValue, circleValues true|false (when max go to min and vice versa)
  // rotaryEncoder1.setAcceleration(0);                   //or set the value - larger number = more accelearation; 0 or 1 means disabled acceleration
  // rotaryEncoder2.setAcceleration(0);                   //or set the value - larger number = more accelearation; 0 or 1 means disabled acceleration

  // Inicializar Motores
  stopAll();
  Serial.println("FIN SETUP");
}

void loop() {

  switch (modox) {
    case 1:  //COREO MIRKO
      {
        tiempo = millis() - pasado;
        labels[0] = "MODO: " + String(modox) + " SEQ";
        labels[1] = "t:" + String(tiempo);
        displayFrecs();
        labels[2] = "";
        coreoMirko();
        strobox();
      }
      break;
    case 2:  //OSC
      {
        OSCMessage msg;
        int size = Udp.parsePacket();

        if (size > 0) {
          while (size--) {
            msg.fill(Udp.read());
          }
          if (!msg.hasError()) {
            msg.dispatch("/motor", motoresOSC);
            msg.dispatch("/estorbo", estorboOSC);
            msg.dispatch("/leds", ledsOSC);
            msg.dispatch("/status", statusOSC);
            msg.dispatch("/full", fullOSC);
            msg.dispatch("/full2", fullOSC2);
          } else {
            error = msg.getError();
            Serial.print("error: ");
            Serial.println(error);
          }
        }
        strobox();
        labels[0] = "MODO: " + String(modox) + " OSC";
        labels[1] = "";
        labels[2] = "";
        displayFrecs();
      }
      break;
    case 3:  // LIVE ENC
      {
        //rotary_loop();
        labels[0] = "MODO: " + String(modox) + " ENC";
        labels[1] = "";
        labels[2] = "";
        displayFrecs();
      }
      break;
  }
  ledsControl();
  buttonRead();
  //modBus_STATUS(frame, 2);
  //modBus_callback();
  agenda();
  //printOLED();
}

void stopAll() {
  STOP(frame, 1);
  STOP(frame, 2);
  FREC(frame, 1, 0);
  FREC(frame, 2, 0);

  //AGREGAR APAGADO DE LEDS?
}



void modBus_callback() {
  if (Serial2.available()) {
    while (Serial2.available()) {
      Serial.println(Serial2.read(), DEC);
    }
  }
}

void buttonRead() {
  int reading = digitalRead(MODE);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        buttonPushCounter++;
        //Serial.println(buttonPushCounter);
        switch (buttonPushCounter) {
          case 1:
            modox = 1;
            Serial.print("modo: ");
            Serial.print(modox);
            Serial.println(" coreo");
            pasado = millis();
            stopAll();
            break;
          case 2:
            modox = 2;
            Serial.print("modo: ");
            Serial.print(modox);
            Serial.println(" OSC");
            stopAll();
            break;
          case 3:
            modox = 3;
            Serial.print("modo: ");
            Serial.print(modox);
            Serial.println(" ENC");
            buttonPushCounter = 0;
            stopAll();
            break;
        }
      }
    }
  }
  lastButtonState = reading;
}

//--------------FUNCION DISPLAY--------------------
// void printOLED() {
//   display.clearDisplay();
//   display.setCursor(0, 0);
//   display.print(labels[0]);
//   display.setCursor(0, 16);
//   display.print(labels[1]);
//   display.setCursor(0, 32);
//   display.print(labels[2]);
//   display.setCursor(0, 48);
//   display.print(labels[3]);
//   display.display();
// }



void motoresOSC(OSCMessage &msg) {

  int id = msg.getInt(0);
  int on = msg.getInt(1);
  float freq = msg.getInt(2);

  if (on && motorStates[id - 1] == 0) {
    RUN(frame, id);
  } else if (!on && motorStates[id - 1] == 1) {
    STOP(frame, id);
  }
  Serial.print("id: ");
  Serial.print(id);
  Serial.print(", encendido: ");
  Serial.print(motorStates[id - 1]);
  Serial.print(", frequencia: ");
  Serial.println(freq / 10.);
  FREC(frame, id, freq / 10.);
}

void strobox() {
  unsigned long currentMillis = millis();
  if (run) {
    if (estorbox_on == 0) {
      if (currentMillis - previous_strobox >= int1) {
        previous_strobox = currentMillis;
        leds[led1 - 1] = pwm;
        leds[led2 - 1] = 0;
        estorbox_on = 1;
      }
    } else {
      if (currentMillis - previous_strobox >= int2) {
        previous_strobox = currentMillis;
        leds[led1 - 1] = 0;
        leds[led2 - 1] = pwm;
        estorbox_on = 0;
      }
    }
  } else {
    // leds[led1] =  0;
    // leds[led2] =  0;
    estorbox_on = 0;
  }
}


void ledsControl() {
  for (int i = 0; i < 4; i++) {
    ledcWrite(i + 1, leds[i]);
  }
}

void ledsOSC(OSCMessage &msg) {
  leds[msg.getInt(0) - 1] = msg.getInt(1);
  for (int i = 0; i < 4; i++) {
    Serial.print("led: ");
    Serial.print(i + 1);
    Serial.print(", pwm: ");
    Serial.println(leds[i]);
  }
}
void statusOSC(OSCMessage &msg) {
  int id = msg.getInt(0);

  frame[0] = 0x02;
  frame[1] = 0x03;
  frame[2] = 0x00;
  frame[3] = id;
  frame[4] = 0x00;
  frame[5] = 0x00;
  CRC(frame);

  sendModBus(frame);
}

void fullOSC(OSCMessage &msg) {
  Serial.println("Full");

  int addr = msg.getInt(0);
  int funct = msg.getInt(1);
  int RH = msg.getInt(2);
  int RL = msg.getInt(3);
  int PH = msg.getInt(4);
  int PL = msg.getInt(5);

  frame[0] = addr;   // Address
  frame[1] = funct;  // Function Code
  frame[2] = RH;     // Register HIGH Byte
  frame[3] = RL;     // Register LOW Byte
  frame[4] = PH;     // Param HIGH Byte
  frame[5] = PL;     // Param LOW Byte
  CRC(frame);

  sendModBus(frame);
}

void fullOSC2(OSCMessage &msg) {
  Serial.println("Full 2");

  int addr = msg.getInt(0);
  int funct = msg.getInt(1);
  int R = msg.getInt(2);
  int P = msg.getInt(3);
  frame[0] = addr;   // Address
  frame[1] = funct;  // Function Code
  frame[2] = (R >> 8) & 0xFF;
  frame[3] = R & 0xFF;
  frame[4] = (P >> 8) & 0xFF;
  frame[5] = P & 0xFF;

  CRC(frame);
  sendModBus(frame);
}

void estorboOSC(OSCMessage &msg) {
  run = msg.getInt(0);
  led1 = msg.getInt(1);
  led2 = msg.getInt(2);
  int1 = msg.getInt(3);
  int2 = msg.getInt(4);
  pwm = msg.getInt(5);

  switch (led1) {
    case 0:
      for (int i = 0; i < 4; i++) {
        leds[i] = 0;
      }
      break;
    case 1:
      leds[2] = 0;
      leds[3] = 0;
      leds[4] = 0;
      break;
    case 2:
      leds[1] = 0;
      leds[3] = 0;
      leds[4] = 0;
      break;
    case 3:
      leds[1] = 0;
      leds[2] = 0;
      leds[4] = 0;
      break;
  }
  switch (led2) {
    case 0:
      for (int i = 0; i < 4; i++) {
        leds[i] = 0;
      }
      break;
    case 1:
      leds[2] = 0;
      leds[3] = 0;
      leds[4] = 0;
      break;
    case 2:
      leds[1] = 0;
      leds[3] = 0;
      leds[4] = 0;
      break;
    case 3:
      leds[1] = 0;
      leds[2] = 0;
      leds[4] = 0;
      break;
  }
  Serial.print("run: ");
  Serial.print(run);
  Serial.print(", led1: ");
  Serial.print(led1);
  Serial.print(", led2: ");
  Serial.print(led2);
  Serial.print(", t1: ");
  Serial.print(int1);
  Serial.print(", t2: ");
  Serial.print(int2);
  Serial.print(", pwm: ");
  Serial.println(pwm);
}


void STOP(unsigned char *frame, int address) {
  motorStates[address - 1] = 0;
  frame[0] = address;  // Address
  frame[1] = 0x06;     // Function Code
  frame[2] = 0x00;     // Register HIGH Byte
  frame[3] = 0x00;     // Register LOW Byte
  frame[4] = 0x00;     // Param HIGH Byte
  frame[5] = 0x00;     // Param LOW Byte
  CRC(frame);
  sendModBus(frame);
}

void RUN(unsigned char *frame, int address) {
  motorStates[address - 1] = 1;
  frame[0] = address;
  frame[1] = 0x06;
  frame[2] = 0x00;
  frame[3] = 0x00;
  frame[4] = 0x00;
  frame[5] = 0x01;
  CRC(frame);
  sendModBus(frame);
}



void FREC(unsigned char *frame, int address, float frecuencia) {
  frecuencias[address - 1] = frecuencia;
  int frec_int = frecuencia * 10.;
  if (frec_int < 0) frec_int += 0xFFFF + 1;

  frame[0] = address;
  frame[1] = 0x06;
  frame[2] = 0x00;
  frame[3] = 0x01;
  frame[4] = (frec_int >> 8) & 0xFF;
  frame[5] = frec_int & 0xFF;
  CRC(frame);
  sendModBus(frame);
}

void modBus_STATUS(unsigned char *frame, int address) {
  frame[0] = address;
  frame[1] = 0x03;
  frame[2] = 0x00;
  frame[3] = 0x05;
  frame[4] = 0x00;
  frame[5] = 0x00;
  CRC(frame);
  sendModBus(frame);
}

void CRC(unsigned char *frame) {

  unsigned int temp, flag;
  temp = 0xFFFF;

  for (int i = 0; i < 6; i++) {
    temp ^= frame[i];
    for (int j = 1; j <= 8; j++) {
      flag = temp & 0x0001;
      temp >>= 1;
      if (flag) temp ^= 0xA001;
    }
  }
  frame[6] = temp & 0xFF;         // CRC LOW Byte
  frame[7] = (temp >> 8) & 0xFF;  // CRC HIGH Byte
}


void sendModBus(unsigned char *frame) {
  //digitalWrite(A_MULTIPLEXER, LOW);
  //digitalWrite(MODBUS_DMX_REDE, HIGH);
  Serial2.write(frame, 8);
  Serial2.flush();
  //digitalWrite(MODBUS_DMX_REDE, LOW);
  delay(modbusDelay);
}
//------------------------------------------------------------------------

void reloz() {
  if (modox > 1) {
    modox = 1;
  }
  preset = 1;
  fin = 0;
  pasado = millis();
  wait = 0;
}

void agenda() {
  DS3231_get(&t);
  militar = t.min + t.hour * 100;
  dia = t.mon + t.mday * 100;

  if ((dia == 406) || (dia == 1106) || (dia == 1806) || (dia == 2506) || (dia == 207) || (dia == 907) || (dia == 1607)) {
    if ((militar == 1440) || (militar == 1540) || (militar == 1640)) {
      reloz();
    }
  } else {
    if ((militar == 1140) || (militar == 1440) || (militar == 1540)) {
      reloz();
    }
  }
}

void displayFrecs() {
  String on1 = "S ";
  String on2 = "S ";
  if (motorStates[0]) {
    on1 = "R ";
  } else {
    on1 = "S ";
  }
  if (motorStates[1]) {
    on2 = "R ";
  } else {
    on2 = "S ";
  }
  labels[3] = on1 + String(frecuencias[0]) + "Hz | " + on2 + String(frecuencias[1]) + "Hz";
}
