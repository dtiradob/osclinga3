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
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>


//----------------------IO-----------------------
#define ENC2_SW 2
#define PWM1 4
#define FAN_PWM 3
#define ENC1_CLK 13  // 12 in the schematic
#define ENC1_SW 12   // 13 in the schematic
#define MODBUS_DMX_REDE 14
#define MODE 15
#define UART2RX 16
#define UART2TX 17
#define A_MULTIPLEXER 18
#define ENC2_CLK 19
#define SDA 21
#define SCL 22
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

float tiemposCSV[100];
float F1CSV[100];
float F2CSV[100];
int pasosCSV = 0;
int pasoActual = 0;
int waitingCSV = 0;
unsigned long tiempoCSV = 0;
String presets[] = { "P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7" };
String csvName= "";
AsyncWebServer server(80);

//------------------ENCODER VARIABLES--------------
#define ROTARY_ENCODER_VCC_PIN -1
#define ROTARY_ENCODER_STEPS 1
//instead of changing here, rather change numbers above
AiEsp32RotaryEncoder rotaryEncoder1 = AiEsp32RotaryEncoder(ENC1_CLK, ENC1_DT, ENC1_SW, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);
AiEsp32RotaryEncoder rotaryEncoder2 = AiEsp32RotaryEncoder(ENC2_CLK, ENC2_DT, ENC2_SW, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);
float frecENC1 = 0.0;
float frecENC2 = 0.0;


//--------------WIFI-OSC VARIABLES---------------
const char *ssid = "Plan Humboldt 2.4Ghz";
const char *password = "holaplan0!";
// const char *ssid = "Guga 2.4GHz";
// const char *password = "marialuisa";
WiFiUDP Udp;
const unsigned int localPort = 9000;
OSCErrorCode error;
String localip = "";

//--------------LED VARIABLES--------------

int estorbox_on = 0;
int run = 0;
int led1 = 2;
int led2 = 3;
int led3 = 4;
int led4 = 5;
int int1 = 0;
int int2 = 0;
int pwm = 0;
unsigned long previous_strobox = 0;
int leds[4] = { 0, 0, 0, 0 };

//----------------------MOD BUTTON VARIABLES--------------
int buttonState = 1;
int selec = 0;
int apretado = 0;
unsigned long pushtime = 0;
unsigned long ahora;
unsigned long push;
int modox = 0;
int buttonPushCounter = 0;           // counter for the number of button presses
int lastButtonState = 1;             // previous state of the
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

//----------------------DISPLAY VARIABLES----------------
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire, -1);

String labels[4] = { "", "", "", "" };

//--------------------MIRKO VARIABLES-------------------

unsigned long previous = 0, previous2 = 0, previous_back = 0;
unsigned long previousMillis = 0, previousMillis2 = 0, previousMillis7 = 0, previousMillis8 = 0;
unsigned long pasado = 0;
int estorbo_on = 0, estorbo_on2 = 0, estorbo_on_back = 0;
int wait = 1, fin = 0, preset = 1, new_frec = 40;
int dia, militar;
struct ts t;


//--------------------------MODBUS VARIABLES-------------

unsigned char frame[8];
float frecuencias[2] = { 0.0, 0.0 };
bool motorStates[2] = { 0, 0 };
const int modbusDelay = 10;
unsigned long tiempo = 0;

//---------------FUNCIONES ENCODER----------------------
void rotary_loop() {
  //dont print anything unless value changed
  if (rotaryEncoder1.encoderChanged()) {
    Serial.print("Value E1: ");
    frecENC1 = rotaryEncoder1.readEncoder() / 10.;
    Serial.println(frecENC1);
    FREC(frame, 1, frecENC1);
  }
  if (rotaryEncoder1.isEncoderButtonClicked()) {
    static unsigned long lastTimePressed = 0;
    //ignore multiple press in that time milliseconds
    if (millis() - lastTimePressed < 500) {
      return;
    }
    lastTimePressed = millis();
    Serial.print("button1 pressed ");
    if (motorStates[0]) {
      STOP(frame, 1);
    } else {
      RUN(frame, 1);
    }
  }
  if (rotaryEncoder2.encoderChanged()) {
    Serial.print("Value E2: ");
    frecENC2 = rotaryEncoder2.readEncoder() / 10.;
    Serial.println(frecENC2);
    FREC(frame, 2, frecENC2);
  }
  if (rotaryEncoder2.isEncoderButtonClicked()) {
    static unsigned long lastTimePressed = 0;
    //ignore multiple press in that time milliseconds
    if (millis() - lastTimePressed < 500) {
      return;
    }
    lastTimePressed = millis();
    Serial.print("button2 pressed ");
    if (motorStates[1]) {
      STOP(frame, 2);
    } else {
      RUN(frame, 2);
    }
  }
}

void IRAM_ATTR readEncoderISR() {
  rotaryEncoder1.readEncoder_ISR();
  rotaryEncoder2.readEncoder_ISR();
}

//-------------------SETUP-------------------

void setup() {
  //-------------------SERIAL SETUP-------------------
  Serial.begin(115200);
  while (!Serial) { delay(100); }
  // ************************** MODBUS ****************************************
  Serial2.begin(115200, SERIAL_8N1, UART2RX, UART2TX);  // Inicia UART2 Rx=16 Tx=17

  //-------------------WIFI SETUP-------------------
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

  //-------------WEB y CSV SETUP----------
  if (!SPIFFS.begin(true)) {
    Serial.println("Error mounting SPIFFS");
    return;
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/save_csv", HTTP_POST, handleSaveCSV);
  server.on("/get_csv", HTTP_GET, handleGetCSV);
  server.on("/list_presets", HTTP_GET, handleListPresets);
  server.on("/save_selections", HTTP_POST, handleSaveSelections);
  server.on("/play_preset", HTTP_POST, handlePlayPreset);
  server.on("/load_selections", HTTP_GET, handleLoadCoreo);
  server.onNotFound(notFound);
  server.begin();

  //readCSV();  //LEER COREO

  //-------------------DISPLAY SETUP-------------------
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(100);
  display.clearDisplay();
  display.display();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.println(WiFi.localIP());
  localip = WiFi.localIP().toString();
  labels[0] = "IP: " + localip;

  //-------------------RTC SETUP-------------------
  DS3231_get(&t);
  militar = t.min + t.hour * 100;
  dia = t.mon + t.mday * 100;
  Serial.println("militar: ");
  Serial.println(militar);
  Serial.println("dia: ");
  Serial.println(dia);
  labels[1] = "date:" + String(t.mday) + "/" + String(t.mon) + " time:" + String(t.hour) + ":" + String(t.min);


  //-------------------LED SETUP-------------------
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

  //-------------------BUTTON SETUP-------------------
  pinMode(MODE, INPUT);

  //-------------------MODBUS SETUP-------------------
  //modBus pin
  pinMode(MODBUS_DMX_REDE, OUTPUT);
  digitalWrite(MODBUS_DMX_REDE, LOW);  //lo iniciamos en LOW, listo para leer

  // Multiplex
  pinMode(A_MULTIPLEXER, OUTPUT);
  digitalWrite(A_MULTIPLEXER, LOW);  // LOW ENVIAR MODBUS; HIGH ENVIAR DMX

  // Inicializar Motores
  stopAll();

  //--------------ENCODER SETUP------------------
  //we must initialize rotary encoder
  rotaryEncoder1.begin();
  rotaryEncoder2.begin();
  rotaryEncoder1.setup(readEncoderISR);
  rotaryEncoder2.setup(readEncoderISR);
  //set boundaries and if values should cycle or not
  //in this example we will set possible values between 0 and 1000;
  bool circleValues = false;
  rotaryEncoder1.setBoundaries(0, 330, circleValues);  //minValue, maxValue, circleValues true|false (when max go to min and vice versa)
  rotaryEncoder2.setBoundaries(0, 330, circleValues);  //minValue, maxValue, circleValues true|false (when max go to min and vice versa)
  rotaryEncoder1.setAcceleration(0);                   //or set the value - larger number = more accelearation; 0 or 1 means disabled acceleration
  rotaryEncoder2.setAcceleration(0);                   //or set the value - larger number = more accelearation; 0 or 1 means disabled acceleration



  Serial.println("FIN SETUP");
}

void loop() {
  switch (modox) {
    case 0:  //idle
      {
        labels[0] = "MODO: " + String(modox) + " IDLE";
        //String localip = WiFi.localIP().toString();
        labels[1] = "IP: " + localip;
        labels[2] = "date:" + String(t.mday) + "/" + String(t.mon) + " time:" + String(t.hour) + ":" + String(t.min);
        displayFrecs();
      }
      break;
    case 1:  //COREO MIRKO
      {
        tiempo = millis() - pasado;
        labels[0] = "MODO: " + String(modox) + " SEQ";
        labels[1] = "t:" + String(tiempo);
        displayFrecs();
        labels[2] = "";
        strobox();
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
            msg.dispatch("/leds", ledsOSC);
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
        rotary_loop();
        labels[0] = "MODO: " + String(modox) + " ENC";
        labels[1] = "";
        labels[2] = "";
        displayFrecs();
      }
      break;
    case 4:  // CSV
      {
        modoCSV();
        labels[0] = "MODO: " + String(modox) + " CSV";
        labels[1] = csvName + "| step: " + String(pasoActual);
        labels[2] = "t:" + String(int(tiemposCSV[pasoActual])) + " | dt:" + String(int(tiempoCSV));
        displayFrecs();
      }
      break;
  }
  ledsControl();
  buttonRead();
  // modBus_callback();
  agenda();
  printOLED();
}




//--------------BUTTON FUNCTION------------------
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
        Serial.print("buttonPushCounter: ");
        Serial.println(buttonPushCounter);
        switch (buttonPushCounter) {
          case 0:
            modox = 0;
            Serial.print("modo: ");
            Serial.print(modox);
            Serial.println(" IDLE");
            pasado = millis();
            stopAll();
            break;
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
            //buttonPushCounter = 0;
            stopAll();
            break;
          case 4:
            modox = 4;
            Serial.print("modo: ");
            Serial.print(modox);
            Serial.println(" CSV");
            buttonPushCounter = -1;
            stopAll();
            pasado = millis();
            pasoActual = 0;
            break;
        }
      }
    }
  }
  lastButtonState = reading;
}

//--------------LED STROBO FUNCTIONS------------------
void strobox() {
  Serial.println("Strobox");
  unsigned long currentMillis = millis();
  if (run) {
    Serial.println("RUN");

    if (estorbox_on == 0) {
      if (currentMillis - previous_strobox >= int1) {
        previous_strobox = currentMillis;
        if (led1 == 0) {
        } else {
          leds[led1 - 1] = pwm;
        }
        if (led2 == 0) {
        } else {
          leds[led2 - 1] = 0;
        }
        estorbox_on = 1;
      }
    } else {
      if (currentMillis - previous_strobox >= int2) {
        previous_strobox = currentMillis;
        if (led1 == 0) {
        } else {
          leds[led1 - 1] = 0;
        }
        if (led2 == 0) {
        } else {
          leds[led2 - 1] = pwm;
        }
        estorbox_on = 0;
      }
    }

  } else {
    // leds[led1] =  0;
    // leds[led2] =  0;
    Serial.println("NO RUN");

    estorbox_on = 0;
  }
}

void ledsControl() {
  for (int i = 0; i < 4; i++) {
    ledcWrite(i + 1, leds[i]);
  }
}

//--------------OSC HANDLE FUNCTIONS-----------------
void ledsOSC(OSCMessage &msg) {
  leds[msg.getInt(0) - 1] = msg.getInt(1);
  for (int i = 0; i < 4; i++) {
    Serial.print("led: ");
    Serial.print(i + 1);
    Serial.print(", pwm: ");
    Serial.println(leds[i]);
  }
}
void estorboOSC(OSCMessage &msg) {
  run = msg.getInt(0);
  led1 = msg.getInt(1);
  led2 = msg.getInt(2);
  int1 = msg.getInt(3);
  int2 = msg.getInt(4);
  pwm = msg.getInt(5);


  Serial.print("led1");
  Serial.println(led1);
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

//---------------MODBUS FUNCTIONS------------------
void stopAll() {
  STOP(frame, 1);
  STOP(frame, 2);
  FREC(frame, 1, 0);
  FREC(frame, 2, 0);
  for (int i = 0; i < 4; i++) {
    leds[i] = 0;
  }
  run = 0;
  int1 = 0;
  int2 = 0;
  pwm = 0;
}

void modBus_callback() {
  if (Serial2.available()) {
    while (Serial2.available()) {
      Serial.println(Serial2.read(), DEC);
    }
  }
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
  digitalWrite(A_MULTIPLEXER, LOW);
  digitalWrite(MODBUS_DMX_REDE, HIGH);
  Serial2.write(frame, 8);
  Serial2.flush();
  digitalWrite(MODBUS_DMX_REDE, LOW);
  delay(modbusDelay);
}

//--------------RTC FUNCTIONS------------------
void reloz() {
  if (modox != 4) {
    modox = 4;  //PASA A MODO CSV
  }
  preset = 1;
  fin = 0;
  //pasado = millis();
  wait = 0;

  Serial.print("modo: ");
  Serial.print(modox);
  Serial.println(" CSV");
  buttonPushCounter = -1;
  stopAll();
  pasado = millis();
  pasoActual = 0;
}

void agenda() {
  DS3231_get(&t);
  militar = t.min + t.hour * 100;
  dia = t.mon + t.mday * 100;
  /*
  Serial.print("dia: ");
  Serial.println(dia);
  Serial.print("militar: ");
  Serial.println(militar);
  */
  if ((dia == 903) || (dia == 1106) || (dia == 1806) || (dia == 2506) || (dia == 207) || (dia == 907) || (dia == 1607)) {
    if ((militar == 1200) || (militar == 1202) || (militar == 1203)) {
      if (wait == 1) {
        readCSV("coreo");
        reloz();
      }
    }
  } else {
    if ((militar == 1553) || (militar == 1555) || (militar == 1540)) {
      if (wait == 1) {
        readCSV("coreo");
        reloz();
      }
    }
  }
}

//--------------DISPLAY FUNCTIONS------------------
void printOLED() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(labels[0]);
  display.setCursor(0, 16);
  display.print(labels[1]);
  display.setCursor(0, 32);
  display.print(labels[2]);
  display.setCursor(0, 48);
  display.print(labels[3]);
  display.display();
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


//---------------WEB Y CSV FUNCTIONS-------------
void handleLoadCoreo(AsyncWebServerRequest *request) {
  File file = SPIFFS.open("/coreo_list.csv", "r");
  if (!file) {
    request->send(404, "text/plain", "File not found");
    return;
  }
  String csvContent = file.readString();
  file.close();
  request->send(200, "text/csv", csvContent);
}

void handlePlayPreset(AsyncWebServerRequest *request) {
  AsyncWebParameter *plainParam = request->getParam("plain", true);
  if (plainParam != nullptr) {
    String presetToPlay = plainParam->value();
    Serial.println("Received CSV data:");
    Serial.println(presetToPlay);
    readCSV(presetToPlay);
    reloz();
    request->send(200, "text/plain", "Playing: " + presetToPlay);
  } else {
    request->send(400, "text/plain", "Bad request: missing plain parameter");
  }
}

void handleSaveSelections(AsyncWebServerRequest *request) {
  AsyncWebParameter *plainParam = request->getParam("plain", true);
  if (plainParam != nullptr) {
    String csvData = plainParam->value();
    File file = SPIFFS.open("/coreo_list.csv", FILE_WRITE);
    if (!file) {
      request->send(500, "text/plain", "Error opening file for writing");
      return;
    }
    if (file.print(csvData)) {
      request->send(200, "text/plain", "CSV data saved successfully");
    } else {
      request->send(500, "text/plain", "Error writing to file");
    }
    file.close();
    // Re-open the file for reading
    file = SPIFFS.open("/coreo_list.csv", FILE_READ);
    if (!file) {
      Serial.println("Error opening file for reading");
      return;
    }

    // Print the content of the file
    Serial.println("Contents of coreo_list.csv:");
    while (file.available()) {
      Serial.write(file.read());
    }
    Serial.println();

    // Close the file again
    file.close();
    request->send(200, "text/plain", "CSV saved successfully");
  } else {
    request->send(400, "text/plain", "Bad request: missing parameters");
  }
  createCoreo();
  readCSV("coreo");
  reloz();
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void handleRoot(AsyncWebServerRequest *request) {
  request->send(SPIFFS, "/index.html", "text/html");
}
void handleSaveCSV(AsyncWebServerRequest *request) {
  if (request->hasParam("name", true)) {
    AsyncWebParameter *nameParam = request->getParam("name", true);
    String fileName = "/" + nameParam->value() + ".csv";

    Serial.println(request->params());
    // Read the CSV data from the request body
    String csvData = "";
    AsyncWebParameter *p;
    for (int i = 2; i < request->params(); i++) {
      p = request->getParam(i);
      int val = request->params() - 1;
      if (i == val) {
        csvData += p->value();
      } else {
        csvData += p->value() + "\n";
      }
    }


    if (!p->isPost()) {
      request->send(400, "text/plain", "Bad request");
      return;
    }

    // Open or create the file for writing
    File file = SPIFFS.open(fileName, FILE_WRITE);
    if (!file) {
      request->send(500, "text/plain", "Error opening file for writing");
      return;
    }

    // Write the CSV data to the file
    if (file.print(csvData)) {
      request->send(200, "text/plain", "CSV data saved successfully");
    } else {
      request->send(500, "text/plain", "Error writing to file");
    }
    file.close();

    // Re-open the file for reading
    file = SPIFFS.open(fileName, FILE_READ);
    if (!file) {
      Serial.println("Error opening file for reading");
      return;
    }

    // Print the content of the file
    Serial.println("Contents of form_data.csv:");
    while (file.available()) {
      Serial.write(file.read());
    }
    Serial.println();

    // Close the file again
    file.close();
    request->send(200, "text/plain", "CSV saved successfully");
  } else {
    request->send(400, "text/plain", "Bad request: missing parameters");
  }



  //readCSV();
}
void handleGetCSV(AsyncWebServerRequest *request) {
  if (request->hasParam("name")) {
    AsyncWebParameter *nameParam = request->getParam("name");
    String fileName = "/" + nameParam->value() + ".csv";
    File file = SPIFFS.open(fileName, "r");
    if (!file) {
      request->send(404, "text/plain", "File not found");
      return;
    }
    String csvContent = file.readString();
    file.close();
    request->send(200, "text/csv", csvContent);
  } else {
    request->send(400, "text/plain", "Bad request: missing parameters");
  }
}

void handleListPresets(AsyncWebServerRequest *request) {
  int presetsCount = sizeof(presets) / sizeof(presets[0]);
  String preset = "";
  for (int i = 0; i < presetsCount; i++) {
    preset += presets[i];
    if (i < presetsCount - 1) {  // Check if it's not the last preset
      preset += ",";             // Add a comma if it's not the last preset
    }
  }
  request->send(200, "text/csv", preset);
}

void createCoreo() {
  // Read the preset list file
  File presetListFile = SPIFFS.open("/coreo_list.csv", "r");
  if (!presetListFile) {
    Serial.println("Failed to open file");
    return;
  }
  String combinedCSV = "";

  while (presetListFile.available()) {
    String presetName = presetListFile.readStringUntil('\n');  // Read a line
    presetName.trim();
    String presetFileName = "/" + presetName + ".csv";
    Serial.println(presetFileName);
    if (SPIFFS.exists(presetFileName)) {
      File presetFile = SPIFFS.open(presetFileName, "r");
      if (presetFile) {
        String line;
        while (presetFile.available()) {
          line = presetFile.readStringUntil('\n');
          combinedCSV += line;
          combinedCSV += "\n";
        }
        presetFile.close();
      }
    }
  }
  presetListFile.close();
  
  File coreoFile = SPIFFS.open("/coreo.csv", "w");
  if (!coreoFile) {
    Serial.println("Failed to open file");
    return;
  }
  coreoFile.print(combinedCSV);
  coreoFile.close();

  File coreo = SPIFFS.open("/coreo.csv", FILE_READ);
  if (!coreo) {
    Serial.println("Error opening file for reading");
    return;
  }

  // // Print the content of the file
  // Serial.println("Contents of coreo.csv:");
  // while (coreo.available()) {
  //   Serial.write(coreo.read());
  // }
  // Serial.println();

  // // Close the file again
  // coreo.close();
}


void readCSV(String param) {
  csvName = param;
  String fileName = "/" + csvName + ".csv";
  File file = SPIFFS.open(fileName, "r");
  if (!file) {
    Serial.println("Failed to open file");
    return;
  }
  pasosCSV= 0;
  // Read each line of the CSV file
  while (file.available()) {
    String line = file.readStringUntil('\n');  // Read a line
    line.trim();                               // Remove leading and trailing whitespace
    
    // Split the line into fields using comma as delimiter
    int delimiterIndex = line.indexOf(',');
    String field1 = line.substring(0, delimiterIndex);
    line = line.substring(delimiterIndex + 1);  // Move to next field
    delimiterIndex = line.indexOf(',');
    String field2 = line.substring(0, delimiterIndex);
    line = line.substring(delimiterIndex + 1);  // Move to next field
    String field3 = line;

    // Store the fields in an array or process them as needed
    // Example: Store in an array
    String rowData[] = { field1, field2, field3 };

    // Print the fields
    Serial.print("Tiempo ");
    Serial.print(pasosCSV);
    Serial.print(": ");
    Serial.print(rowData[0]);
    Serial.print("  | ");
    tiemposCSV[pasosCSV] = rowData[0].toFloat() * 1000;
    Serial.println(tiemposCSV[pasosCSV]);

    Serial.print("F1 ");
    Serial.print(pasosCSV);
    Serial.print(": ");
    Serial.print(rowData[1]);
    Serial.print("  | ");
    F1CSV[pasosCSV] = rowData[1].toFloat();
    Serial.println(F1CSV[pasosCSV]);

    Serial.print("F2 ");
    Serial.print(pasosCSV);
    Serial.print(": ");
    Serial.print(rowData[2]);
    Serial.print("  | ");
    F2CSV[pasosCSV] = rowData[2].toFloat();
    Serial.println(F2CSV[pasosCSV]);

    pasosCSV++;
  }
}

//------------MODO CSV---------------
void modoCSV() {
  tiempoCSV = millis() - pasado;
  if (tiempoCSV >= tiemposCSV[pasoActual]) {  //termina el paso
    pasoActual++;
    pasado = millis();
    if (pasoActual == pasosCSV) {  //TERMINA EL CSV
      stopAll();
      Serial.println("FIN CSV");
      pasoActual = 0;
      modox = 0;
      pasado = millis();
      buttonPushCounter = -1;
      wait = 1;
    }
    waitingCSV = 0;
  } else {  //empieza el paso
    if (waitingCSV == 0) {
      if (F1CSV[pasoActual] == 0) {
        STOP(frame, 1);
        FREC(frame, 1, F1CSV[pasoActual]);
      } else {
        RUN(frame, 1);
        FREC(frame, 1, F1CSV[pasoActual]);
      }
      if (F2CSV[pasoActual] == 0) {
        STOP(frame, 2);
        FREC(frame, 2, F2CSV[pasoActual]);
      } else {
        RUN(frame, 2);
        FREC(frame, 2, F2CSV[pasoActual]);
      }
      waitingCSV = 1;
    }
  }
}