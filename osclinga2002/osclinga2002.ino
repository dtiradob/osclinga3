#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ds3231.h> //https://github.com/rodan/ds3231/tree/master

const char *ssid = "Plan Humboldt 2.4Ghz";
const char *password = "holaplan0!";
WiFiUDP Udp;
const unsigned int localPort = 9000;
OSCErrorCode error;
int estorbox_on = 0;
int run = 0;
int led1 = 0;
int led2 = 0;
int int1 = 0;
int int2 = 0;
int pwm = 0;
unsigned long previous_strobox = 0;
unsigned char frame[8];

const int buttonPin = 23;  //PIN BOTÓN MULTIMODO
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

int modBusPin = 14;

//-------------------------------------------------------

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
  Serial.println("inicio display");
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(100);
  display.clearDisplay();
  display.display();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("IP: ");
  display.setCursor(0, 25);
  display.print(WiFi.localIP());
  display.display();
  Serial.println("FIN display");
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

  Serial.println("FIN SETUP");
}

void loop() {

  switch (modox) {
    case 1:  //COREO MIRKO
      {
        unsigned long tiempo = millis() - pasado;
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("MODO: ");
        display.setCursor(55, 0);
        display.print(modox);
        display.setCursor(0, 25);
        display.print("tiempo: ");
        display.setCursor(0, 50);
        display.print(tiempo);
        display.display();

        if (tiempo < 420000) {
          preset1();
          estorbo(1, 40, 80, 60);
        }

        if ((tiempo > 420000) && (tiempo < 440000)) {
          estorbo(0, 20, 30, 30);
          preset2();
        }

        if ((tiempo > 440000) && (tiempo < 500000)) {
          delay((random(2000)) + 2000);
          preset3();
        }

        if ((tiempo > 500000) && (tiempo < 520000)) {
          preset20();
        }

        if ((tiempo > 520000) && (tiempo < 580000)) {
          delay((random(1000)) + 2000);
          preset4();
        }

        if ((tiempo > 580000) && (tiempo < 590000)) {
          preset5();
          estorbo(1, 40, 80, 60);
        }

        if ((tiempo > 590000) && (tiempo < 710000)) {
          estorbo(0, 20, 30, 30);
          preset6();
          estorbo2(1, 70, 60);
        }

        if ((tiempo > 710000) && (tiempo < 720000)) {
          estorbo2(0, 70, 60);
          preset50();
          estorbo(1, 40, 80, 60);
        }

        if ((tiempo > 720000) && (tiempo < 780000)) {
          apaga();
          preset7();
          estorback(1, 100, 2500, 100);
          estorbo(1, 40, 80, 60);
        }

        if ((tiempo > 780000) && (tiempo < 790000)) {
          estorback(0, 100, 2500, 100);
          preset500();
          estorbo(1, 40, 80, 60);
        }

        if ((tiempo > 790000) && (tiempo < 820000)) {
          estorbackfin(1, 100, 3000, 3000);
        }

        if (tiempo > 820000) {
          if (!wait) {
            estorbackfin(0, 100, 3000, 3000);
            preset0();
            wait = 1;
          }
        }

        if (wait) {
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
      }
      break;
  }
  buttonRead();
  //modBus_STATUS(frame, 2);
  modBus_callback();
}

void modBus_callback() {
  if (Serial2.available()) {
    //display.clearDisplay();
    while (Serial2.available()) {
      Serial.println(Serial2.read(),HEX);
      //int inByte = Serial2.read();
      //Serial.write(inByte);
      //display.setCursor(0, 0);
      //display.setTextSize(1);
      //display.print(Serial2.readString());
      //display.display();
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
            break;
          case 2:
            modox = 2;
            Serial.print("modo: ");
            Serial.print(modox);
            Serial.println(" OSC");

            display.clearDisplay();
            display.setCursor(0, 0);
            display.print("MODO: ");
            display.setCursor(55, 0);
            display.print(modox);
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
            display.setCursor(55, 0);
            display.print(modox);
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
  display.setTextSize(2);
  display.print("motor: ");
  display.setCursor(70, 0);
  display.print(label);
  display.setCursor(0, 40);
  display.print(dato / 10.);
  display.setCursor(80, 40);
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
}

void strobox() {
  unsigned long currentMillis = millis();

  if (run) {
    if (estorbox_on == 0) {
      if (currentMillis - previous_strobox >= int1) {
        previous = currentMillis;
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
  digitalWrite(modBusPin, HIGH);
  Serial2.write(frame, 8);
  Serial2.flush();
  digitalWrite(modBusPin, LOW);
}
//------------------------------------------------------------------------

//TORPEDO

//paket[8] = { Address, Function Code, Register HIGH Byte, Register LOW Byte, Param HIGH Byte, Param LOW Byte, CRC LOW Byte, CRC HIGH Byte }

//CRC-16/MODBUS  0x010600000000 -> 0xCA89
//               0x010600000001 -> 0x0A48

//uint8_t paket_STOP[8] = { 0x01, 0x06, 0x00, 0x00, 0x00, 0x00, 0x89, 0xCA };
//uint8_t paket_RUN[8]  = { 0x01, 0x06, 0x00, 0x00, 0x00, 0x01, 0x48, 0x0A };


//CRC-16/MODBUS   0x01060001015E -> 0x6258
//                0x010600010032 -> 0xDF59

//uint8_t paket_35Hz[8] = { 0x01, 0x06, 0x00, 0x01, 0x01, 0x5E, 0x58, 0x62 };
//uint8_t paket_5Hz[8] =  { 0x01, 0x06, 0x00, 0x01, 0x00, 0x32, 0x59, 0xDF };

//------------------------------------------------------------------------