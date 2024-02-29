#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ds3231.h>  //https://github.com/rodan/ds3231
#include "AiEsp32RotaryEncoder.h"
#include "Arduino.h"

//----------------------ENCODER-----------------------
#define ROTARY_ENCODER_A_PIN 13
#define ROTARY_ENCODER_B_PIN 26
#define ROTARY_ENCODER_BUTTON_PIN 12
#define ROTARY_ENCODER_VCC_PIN -1 
#define ROTARY_ENCODER_STEPS 1
//#define ROTARY_ENCODER_STEPS 2
//#define ROTARY_ENCODER_STEPS 4
//instead of changing here, rather change numbers above
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);
float frecENC = 0.0;

// WIFI - OSC

const char *ssid = "Plan Humboldt 2.4Ghz";
const char *password = "holaplan0!";
WiFiUDP Udp;
const unsigned int localPort = 9000;
OSCErrorCode error;
 //---

int estorbox_on = 0;
int run = 0;
int led1 = 0;
int led2 = 0;
int int1 = 0;
int int2 = 0;
int pwm = 0;
unsigned long previous_strobox = 0;
unsigned char frame[8];

const int buttonPin = 15;  //PIN BOTÓN MULTIMODO
int buttonState = 0;
int selec = 0;
int apretado = 0;
unsigned long pushtime = 0;
unsigned long ahora;
unsigned long push;
int modox = 0;
int buttonPushCounter = 0;  // counter for the number of button presses
int lastButtonState = 0;    // previous state of the
const int ledPin = 2;

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire, -1);

//--------------------variables mirko ---------------

unsigned long previous = 0, previous2 = 0, previous_back = 0;
unsigned long previousMillis = 0, previousMillis2 = 0, previousMillis7 = 0, previousMillis8 = 0;
unsigned long pasado = 0;
int estorbo_on = 0, estorbo_on2 = 0, estorbo_on_back = 0;
int wait = 0, fin = 0, preset = 1, new_frec = 40;
int dia, militar;
struct ts t;

//-----------------------------------------------

//---------VARIABLES MULTIPLEXOR----------------
const int aMultiplexPin = 18;


//-----------------------------------------------
//DS3231_init(DS3231_CONTROL_INTCN);

int modBusPin = 14;

//-------------------------------------------------------

float frecuencias[2] = { 0.0, 0.0 };
unsigned long tiempo = 0;

//---------------FUNCIONES ENCODER------------------
void rotary_onButtonClick() {
  static unsigned long lastTimePressed = 0;
  //ignore multiple press in that time milliseconds
  if (millis() - lastTimePressed < 500) {
    return;
  }
  lastTimePressed = millis();
  Serial.print("button pressed ");
  Serial.print(millis());
  Serial.println(" milliseconds after restart");
}

void rotary_loop() {
  //dont print anything unless value changed
  if (rotaryEncoder.encoderChanged()) {
    Serial.print("Value: ");
    frecENC = rotaryEncoder.readEncoder() / 10.;
    Serial.println(frecENC);
    FREC(frame, 2, frecENC);
  }
  if (rotaryEncoder.isEncoderButtonClicked()) {
    rotary_onButtonClick();
  }
}

void IRAM_ATTR readEncoderISR() {
  rotaryEncoder.readEncoder_ISR();
}

//-------------------------------------------------------------------

void setup() {

  Serial.begin(115200);
  while (!Serial) { delay(100); }
  // ************************** MODBUS ***************************************
  Serial2.begin(115200, SERIAL_8N1, 16, 17);  // Inicia UART2 Rx=16 Tx=17
  // ************************ ACCESS POINT ************************************
  // Connect to WiFi network
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
  // display

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(100);
  display.clearDisplay();
  display.display();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("IP: ");
  display.setCursor(18, 0);
  display.println(WiFi.localIP());

  //RTC
  DS3231_get(&t);
  militar = t.min + t.hour * 100;
  dia = t.mon + t.mday * 100;
  Serial.println("militar: ");
  Serial.println(militar);
  Serial.println("dia: ");
  Serial.println(dia);

  display.setCursor(0, 18);
  display.print(t.mday);
  display.setCursor(16, 18);
  display.print("/");
  display.setCursor(24, 18);
  display.print(t.mon);
  display.setCursor(40, 18);
  display.print(t.hour);
  display.setCursor(54, 18);
  display.print(":");
  display.setCursor(60, 18);
  display.print(t.min);

  display.display();

  // (pin, canal)
  ledcAttachPin(4, 1);   // IZQUIERDA
  ledcAttachPin(32, 2);  // ATRÁS
  ledcAttachPin(25, 3);  // ADELANTE
  ledcAttachPin(27, 4);  // DERECHA

  // (canal,frecuencia,resolución)
  ledcSetup(1, 1000, 8);
  ledcSetup(2, 1000, 8);
  ledcSetup(3, 1000, 8);
  ledcSetup(4, 1000, 8);

  // button
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  //modBus pin
  pinMode(modBusPin, OUTPUT);
  digitalWrite(modBusPin, LOW);  //lo iniciamos en LOW, listo para leer

  // Multiplex
  pinMode(aMultiplexPin, OUTPUT);
  digitalWrite(aMultiplexPin, LOW); // LOW ENVIAR MODBUS; HIGH ENVIAR DMX


  //--------------ENCODER------------------
  //we must initialize rotary encoder
  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);
  //set boundaries and if values should cycle or not
  //in this example we will set possible values between 0 and 1000;
  bool circleValues = false;
  rotaryEncoder.setBoundaries(0, 330, circleValues);  //minValue, maxValue, circleValues true|false (when max go to min and vice versa)

  //rotaryEncoder.disableAcceleration(); //acceleration is now enabled by default - disable if you dont need it
  rotaryEncoder.setAcceleration(250);  //or set the value - larger number = more accelearation; 0 or 1 means disabled acceleration

  Serial.println("FIN SETUP");
}

void loop() {

  switch (modox) {
    case 1:  //COREO MIRKO
      {
        tiempo = millis() - pasado;
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("MODO: ");
        display.setCursor(30, 0);
        display.print(modox);
        display.setCursor(0, 18);
        display.print("tiempo: ");
        display.setCursor(50, 18);
        display.print(tiempo);
        displayFrecs();
        display.display();
        coreoMirko();
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
      }
      break;
    case 3:  // LIVE ENC
      {
        rotary_loop();
      }
      break;
  }
  
  buttonRead();
  //modBus_STATUS(frame, 2);
  modBus_callback();
  agenda();

}

void modBus_callback() {
  if (Serial2.available()) {
    //display.clearDisplay();
    Serial.println("tu hermana");
    while (Serial2.available()) {
      Serial.println(Serial2.read(), DEC);

    }
  }
}

void buttonRead() {
  int reading = digitalRead(buttonPin);
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

            display.clearDisplay();
            display.setCursor(0, 0);
            display.print("MODO: ");
            display.setCursor(55, 0);
            display.print(modox);
            display.display();
            pasado = millis();
            break;
          case 2:
            modox = 2;
            //Serial.print("modo: ");
            //Serial.print(modox);
            //Serial.println(" OSC");

            display.clearDisplay();
            display.setCursor(0, 0);
            display.print("MODO: ");
            display.setCursor(30, 0);
            display.print(modox);
            display.setCursor(40, 0);
            display.print("OSC");
            display.display();
            break;
          case 3:
            modox = 3;
            Serial.print("modo: ");
            Serial.print(modox);
            Serial.println(" live enc");

            display.clearDisplay();
            display.setCursor(0, 0);
            display.print("MODO: ");
            display.setCursor(30, 0);
            display.print(modox);
            display.setCursor(40, 0);
            display.print("ENC");
            display.display();

            buttonPushCounter = 0;
            break;
        }
      }
    }
  }
  lastButtonState = reading;
}

void printDisplay(int label, int dato) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("motor: ");
  display.setCursor(40, 0);
  display.print(label);
  display.setCursor(0, 16);
  display.print(dato / 10.);
  display.setCursor(40, 16);
  display.print("Hz");

  display.display();
  //delay(100);
}


void motoresOSC(OSCMessage &msg) {

  int id = msg.getInt(0);
  int on = msg.getInt(1);
  float freq = msg.getInt(2);



  if (on) {
    RUN(frame, id);
    delay(10);
  } else {
    STOP(frame, id);
    delay(10);
  }

  FREC(frame, id, freq / 10.);
  delay(10);

  Serial.print("id: ");
  Serial.print(id);
  Serial.print(", encendido: ");
  Serial.print(on);
  Serial.print(", frequencia: ");
  Serial.println(freq / 10.);

  printDisplay(id, freq);
  displayFrecs();
  display.display();
}

void strobox() {
  unsigned long currentMillis = millis();
  if (run) {
    if (estorbox_on == 0) {
      if (currentMillis - previous_strobox >= int1) {
        previous_strobox = currentMillis;
        ledcWrite(led1, pwm);
        ledcWrite(led2, 0);
        estorbox_on = 1;
      }
    } else {
      if (currentMillis - previous_strobox >= int2) {
        previous_strobox = currentMillis;
        ledcWrite(led1, 0);
        ledcWrite(led2, pwm);
        estorbox_on = 0;
      }
    }
  } else {
    ledcWrite(led1, 0);
    ledcWrite(led2, 0);
    estorbox_on = 0;
  }
}

void statusOSC(OSCMessage &msg) {

  int id = msg.getInt(0);
  Serial.println("STATUS");
  //Serial.println(id);
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
  /*
  Serial.print("id: ");
  Serial.print(id);
  Serial.print(", encendido: ");
  Serial.print(on);
  Serial.print(", frequencia: ");
  Serial.println(freq / 10.);
*/
  //printDisplay(id, freq);
  //displayFrecs();
  //display.display();
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
  /*
  Serial.print("id: ");
  Serial.print(id);
  Serial.print(", encendido: ");
  Serial.print(on);
  Serial.print(", frequencia: ");
  Serial.println(freq / 10.);
*/
  //printDisplay(id, freq);
  //displayFrecs();
  //display.display();
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
      ledcWrite(1, 0);
      ledcWrite(2, 0);
      ledcWrite(3, 0);
      ledcWrite(4, 0);
      break;
    case 1:
      ledcWrite(2, 0);
      ledcWrite(3, 0);
      ledcWrite(4, 0);
      break;
    case 2:
      ledcWrite(1, 0);
      ledcWrite(3, 0);
      ledcWrite(4, 0);
      break;
    case 3:
      ledcWrite(1, 0);
      ledcWrite(2, 0);
      ledcWrite(4, 0);
      break;
  }
  switch (led2) {
    case 0:
      ledcWrite(1, 0);
      ledcWrite(2, 0);
      ledcWrite(3, 0);
      ledcWrite(4, 0);
      break;
    case 1:
      ledcWrite(2, 0);
      ledcWrite(3, 0);
      ledcWrite(4, 0);
      break;
    case 2:
      ledcWrite(1, 0);
      ledcWrite(3, 0);
      ledcWrite(4, 0);
      break;
    case 3:
      ledcWrite(1, 0);
      ledcWrite(2, 0);
      ledcWrite(4, 0);
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
  digitalWrite(aMultiplexPin, LOW);
  digitalWrite(modBusPin, HIGH);
  Serial2.write(frame, 8);
  Serial2.flush();
  digitalWrite(modBusPin, LOW);
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
  display.setCursor(0, 54);
  display.print("f1: ");
  display.setCursor(18, 54);
  display.print(frecuencias[0]);

  display.setCursor(45, 54);
  display.print("|");

  display.setCursor(52, 54);
  display.print("f2: ");
  display.setCursor(70, 54);
  display.print(frecuencias[1]);
}
