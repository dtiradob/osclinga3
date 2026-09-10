#include "globals.h"

//------------------WEB Y CSV VARIABLES--------------
float tiemposCSV[MAXSTEPS];
float F1CSV[MAXSTEPS];
float F2CSV[MAXSTEPS];
int pasosCSV = 0;
int pasoActual = 0;
int waitingCSV = 0;
unsigned long tiempoCSV = 0;
String presets[] = { "P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7" };
String csvName = "";
AsyncWebServer server(80);

//------------------ENCODER VARIABLES--------------
#define ROTARY_ENCODER_VCC_PIN -1
#define ROTARY_ENCODER_STEPS 1
//instead of changing here, rather change numbers above
// ENC1/ENC2 pin groups swapped here vs. their #define names, and CLK/DT
// swapped within each, to match physical wiring and correct direction.
AiEsp32RotaryEncoder rotaryEncoder1 = AiEsp32RotaryEncoder(ENC2_DT, ENC2_CLK, ENC2_SW, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);
AiEsp32RotaryEncoder rotaryEncoder2 = AiEsp32RotaryEncoder(ENC1_DT, ENC1_CLK, ENC1_SW, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);
float frecENC1 = 0.0;
float frecENC2 = 0.0;


//--------------WIFI-OSC VARIABLES---------------
//const char *ssid = "LABDIE2G";
//const char *password = "electricidad";

// const char *ssid = "Plan Humboldt 2.4Ghz";
// const char *password = "holaplan0!";
//
//const char *ssid = "Guga 2.4GHz";
//const char *password = "marialuisa";
//
char ssid[16];  // built in setup() from BOARD_ID, e.g. "osclinga1"
const char *password = "p1c0p4lqu3l33";
// AP mode (1:AP | 0:client) is defined in globals.h, overridable via build_flags


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
int modox = 2;  // no physical mode button on this dev board - boot straight into OSC mode
int buttonPushCounter = 0;           // counter for the number of button presses
int lastButtonState = 1;             // previous state of the
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

//----------------------DISPLAY VARIABLES----------------
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire, -1);
// Tracks whether display.begin() actually found a panel, so a dead/missing
// OLED doesn't keep hammering the I2C bus with pointless writes every loop.
bool displayOK = false;
// Throttles OLED label rebuild + repaint to 10Hz - these run inside loop(),
// which otherwise spins as fast as possible and was rebuilding several
// String objects per iteration (heap churn -> long-run fragmentation risk).
#define DISPLAY_REFRESH_MS 100

String labels[4] = { "", "", "", "" };

#define MAX_CSV_UPLOAD_BYTES 32768

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
#if HAVE_ENCODERS
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
#endif  // HAVE_ENCODERS
}

void IRAM_ATTR readEncoderISR() {
#if HAVE_ENCODERS
  rotaryEncoder1.readEncoder_ISR();
  rotaryEncoder2.readEncoder_ISR();
#endif  // HAVE_ENCODERS
}

// Starts (or restarts) the board as a WiFi AP - used both for the normal
// AP==1 path and as a fallback if STA connect times out below.
void startAPMode() {
  Serial.print("Setting AP (Access Point)…");
  WiFi.mode(WIFI_AP);
  // Gives each BOARD_ID its own AP gateway IP (192.168.<BOARD_ID>.1)
  // so multiple boards running at once show up at different addresses.
  IPAddress apIP(192, 168, BOARD_ID, 1);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  if (!WiFi.softAP(ssid, password)) {
    Serial.println("softAP() failed to start!");
  }
  localip = WiFi.softAPIP().toString();
  Serial.print("AP IP address: ");
  Serial.println(localip);
}

//-------------------SETUP-------------------

void setup() {
  //-------------------SERIAL SETUP-------------------
  Serial.begin(115200);
  while (!Serial) { delay(100); }
#if HAVE_MODBUS
  // ************************** MODBUS ****************************************
  Serial2.begin(115200, SERIAL_8N1, UART2RX, UART2TX);  // Inicia UART2 Rx=16 Tx=17
#endif  // HAVE_MODBUS

  //-------------------WIFI SETUP-------------------
  delay(10);
  Serial.println();
  snprintf(ssid, sizeof(ssid), "osclinga%d", BOARD_ID);
  if (AP == 1) {
    startAPMode();
  } else {
    Serial.println("******************************************************");
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    // Bounded wait instead of an infinite loop - a wrong password/unreachable
    // router must not permanently hang setup() before anything else (OLED,
    // motors, web server) gets initialized. Falls back to AP mode instead.
    const unsigned long WIFI_CONNECT_TIMEOUT_MS = 20000;
    unsigned long wifiStart = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - wifiStart) < WIFI_CONNECT_TIMEOUT_MS) {
      delay(500);
      Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      localip = WiFi.localIP().toString();
      Serial.println(localip);
    } else {
      Serial.println("");
      Serial.println("WiFi STA connect timed out - falling back to AP mode");
      startAPMode();
    }
  }

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
#ifdef ESP32
  Serial.println(localPort);
#else
  Serial.println(Udp.localPort());
#endif

  //-------------WEB y CSV SETUP----------
  // Do NOT `return` here on failure - every SPIFFS.open() call downstream
  // already checks its own result and fails gracefully (404/500 replies,
  // "Failed to open file" logs). Bailing out of setup() early used to skip
  // display/RTC/LED/button/motor/encoder init entirely just because the
  // filesystem didn't mount, leaving the rest of the board half-initialized.
  if (!SPIFFS.begin(true)) {
    Serial.println("Error mounting SPIFFS - web UI/CSV presets unavailable, continuing boot");
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
  readCSV("coreo"); //LEER COREO

  //-------------------DISPLAY SETUP-------------------
  // display.begin() return value is unchecked upstream - log failures so a
  // silently-blank OLED (wrong address/wiring) is diagnosable from Serial.
  // Both SWITCHCAPVCC and EXTERNALVCC ACK fine over I2C but panel stays dark
  // on this board - ruled out VCC/charge-pump mode, likely a wiring/power or
  // panel-size issue instead.
  displayOK = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  if (!displayOK) {
    Serial.println("SSD1306 not found at 0x3C, trying 0x3D...");
    displayOK = display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
    if (displayOK) {
      Serial.println("SSD1306 found at 0x3D");
    } else {
      Serial.println("SSD1306 init FAILED at both 0x3C and 0x3D - check wiring/power");
    }
  } else {
    Serial.println("SSD1306 found at 0x3C");
  }
  showSplash();
  if (displayOK) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.println(localip);
  }
  labels[0] = "IP: " + localip;

  //-------------------RTC SETUP-------------------
  syncClock();
  militar = t.min + t.hour * 100;
  dia = t.mon + t.mday * 100;
  Serial.println("militar: ");
  Serial.println(militar);
  Serial.println("dia: ");
  Serial.println(dia);
  labels[1] = "date:" + String(t.mday) + "/" + String(t.mon) + " time:" + String(t.hour) + ":" + String(t.min);


  //-------------------LED SETUP-------------------
  // NOTE: the resolved arduino-esp32 core (via PlatformIO's espressif32
  // platform) is still on the 2.x line, which uses the legacy
  // channel-based ledc API (ledcAttachPin/ledcSetup), not the 3.x
  // ledcAttach(pin, freq, res) API. Keep this until the platform is
  // upgraded to arduino-esp32 3.x / ESP-IDF 5.
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
  pinMode(MODE, INPUT_PULLUP);

#if HAVE_MODBUS
  //-------------------MODBUS SETUP-------------------
  //modBus pin
  pinMode(MODBUS_DMX_REDE, OUTPUT);
  digitalWrite(MODBUS_DMX_REDE, LOW);  //lo iniciamos en LOW, listo para leer

  // Multiplex
  pinMode(A_MULTIPLEXER, OUTPUT);
  digitalWrite(A_MULTIPLEXER, LOW);  // LOW ENVIAR MODBUS; HIGH ENVIAR DMX
#endif  // HAVE_MODBUS

  // Inicializar Motores
  stopAll();

#if HAVE_ENCODERS
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
#endif  // HAVE_ENCODERS


  Serial.println("FIN SETUP");
}

void loop() {
  // OLED text only needs to change a few times a second for a human to read
  // it; gating the label rebuild + repaint here avoids rebuilding several
  // String objects and hitting the I2C bus on every single loop() iteration
  // (loop() itself has no delay in OSC/idle modes and can spin extremely
  // fast, so ungated this was a steady heap-fragmentation source on
  // long-running/unattended installs).
  unsigned long nowMs = millis();
  static unsigned long lastDisplayUpdate = 0;
  bool refreshDisplay = (nowMs - lastDisplayUpdate >= DISPLAY_REFRESH_MS);
  if (refreshDisplay) lastDisplayUpdate = nowMs;

  switch (modox) {
    case 0:  //idle
      {
        if (refreshDisplay) {
          labels[0] = "MODO: " + String(modox) + " IDLE";
          //String localip = WiFi.localIP().toString();
          labels[1] = "IP: " + localip;
          labels[2] = "date:" + String(t.mday) + "/" + String(t.mon) + " time:" + String(t.hour) + ":" + String(t.min);
          displayFrecs();
        }
      }
      break;
    case 1:  //COREO MIRKO
      {
        tiempo = millis() - pasado;
        if (refreshDisplay) {
          labels[0] = "MODO: " + String(modox) + " SEQ";
          labels[1] = "t:" + String(tiempo);
          displayFrecs();
          labels[2] = "";
        }
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
        if (refreshDisplay) {
          labels[0] = "MODO: " + String(modox) + " OSC";
          labels[1] = "SSID: " + String(ssid);
          labels[2] = "IP: " + localip;
          displayFrecs();
        }
      }
      break;
    case 3:  // LIVE ENC
      {
        rotary_loop();
        if (refreshDisplay) {
          labels[0] = "MODO: " + String(modox) + " ENC";
          labels[1] = "";
          labels[2] = "";
          displayFrecs();
        }
      }
      break;
    case 4:  // CSV
      {
        modoCSV();
        if (refreshDisplay) {
          labels[0] = "MODO: " + String(modox) + " CSV";
          labels[1] = csvName + "| step: " + String(pasoActual);
          labels[2] = "t:" + String(int(tiemposCSV[pasoActual])) + " | dt:" + String(int(tiempoCSV));
          displayFrecs();
        }
      }
      break;
  }
  ledsControl();
  buttonRead();
  // modBus_callback();
  agenda();
  if (refreshDisplay) printOLED();
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
  unsigned long currentMillis = millis();
  if (run) {
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
  int idx = msg.getInt(0) - 1;
  if (idx < 0 || idx > 3) {
    Serial.println("ledsOSC: index out of range, ignoring");
    return;
  }
  leds[idx] = msg.getInt(1);
  for (int i = 0; i < 4; i++) {
    Serial.print("led: ");
    Serial.print(i + 1);
    Serial.print(", pwm: ");
    Serial.println(leds[i]);
  }
}
void estorboOSC(OSCMessage &msg) {
  int newRun = msg.getInt(0);
  int newLed1 = msg.getInt(1);
  int newLed2 = msg.getInt(2);
  if (newLed1 < 0 || newLed1 > 4 || newLed2 < 0 || newLed2 > 4) {
    Serial.println("estorboOSC: led index out of range, ignoring");
    return;
  }
  run = newRun;
  led1 = newLed1;
  led2 = newLed2;
  int1 = msg.getInt(3);
  int2 = msg.getInt(4);
  pwm = msg.getInt(5);

  // Turn off every light that isn't one of the two selected strobe positions.
  for (int i = 0; i < 4; i++) {
    if ((i + 1) != led1 && (i + 1) != led2) {
      leds[i] = 0;
    }
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
  if (id != 1 && id != 2) {
    Serial.println("motoresOSC: invalid motor id, ignoring");
    return;
  }
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
#if HAVE_MODBUS
  if (Serial2.available()) {
    while (Serial2.available()) {
      Serial.println(Serial2.read(), DEC);
    }
  }
#endif  // HAVE_MODBUS
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
#if HAVE_MODBUS
  digitalWrite(A_MULTIPLEXER, LOW);
  digitalWrite(MODBUS_DMX_REDE, HIGH);
  Serial2.write(frame, 8);
  Serial2.flush();
  digitalWrite(MODBUS_DMX_REDE, LOW);
  delay(modbusDelay);
#else
  // No MAX3485 transceiver wired on this dev board - keep the logical
  // motorStates/frecuencias bookkeeping (done by callers) but skip the
  // actual RS485 write.
#endif  // HAVE_MODBUS
}

//--------------DISPLAY FUNCTIONS------------------
// This panel's top 16 rows are the physical yellow strip, rows 16-63 are
// blue - so the logo/icon lives up top and the motor-control graphic below.
static void drawMotorIcon(int cx, int cy, int r, float rotation) {
  display.drawCircle(cx, cy, r, SSD1306_WHITE);
  for (int i = 0; i < 3; i++) {
    float a = i * 2.0944f + rotation;  // 3 blades, 120 deg apart, spinning
    int x2 = cx + (int)((r - 1) * cos(a));
    int y2 = cy + (int)((r - 1) * sin(a));
    display.drawLine(cx, cy, x2, y2, SSD1306_WHITE);
  }
  display.fillCircle(cx, cy, 1, SSD1306_WHITE);
}

static void drawFrequencyWave(int x0, int y0, int w, int amplitude, float phaseOffset) {
  float phase = phaseOffset;
  int prevX = x0, prevY = y0;
  for (int x = 0; x < w; x++) {
    float freq = 0.05f + (float)x / w * 0.35f;  // chirp: freq rises left->right = "variable frequency"
    phase += freq;
    int y = y0 + (int)(amplitude * sin(phase));
    if (x > 0) display.drawLine(prevX, prevY, x0 + x, y, SSD1306_WHITE);
    prevX = x0 + x;
    prevY = y;
  }
}

void showSplash() {
  if (!displayOK) return;
  const unsigned long SPLASH_MS = 3000;
  const unsigned long FRAME_MS = 40;
  unsigned long start = millis();
  float animPhase = 0;

  while (millis() - start < SPLASH_MS) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    // Title owns the yellow strip; icon sits after it, never blocking the name.
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("OSCLINGA");
    drawMotorIcon(112, 8, 7, animPhase);

    // Scrolling phase makes the chirp wave "live", like a running VFD output.
    // Same animPhase drives both, so the blades spin at the wave's speed.
    drawFrequencyWave(0, 30, 128, 10, animPhase);
    animPhase += 0.35f;

    display.setTextSize(1);
    display.setCursor(0, 48);
    display.println("v" FW_VERSION);
    display.setCursor(0, 56);
    display.println(FW_BUILD_DATE);
    display.display();
    delay(FRAME_MS);
  }
}

//--------------RTC FUNCTIONS------------------
void syncClock() {
#if HAVE_RTC
  DS3231_get(&t);
#else
  // No DS3231 physically on the I2C bus for this dev board - fake a wall
  // clock from millis() so the display/scheduling code paths still run
  // without hanging or reading garbage off an absent device.
  unsigned long secs = millis() / 1000;
  t.sec = secs % 60;
  t.min = (secs / 60) % 60;
  t.hour = (secs / 3600) % 24;
  t.mday = 1;
  t.mon = 1;
  t.year = 2026;
#endif  // HAVE_RTC
}

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
  syncClock();
  militar = t.min + t.hour * 100;
  dia = t.mon + t.mday * 100;
  /*
  Serial.print("dia: ");
  Serial.println(dia);
  Serial.print("militar: ");
  Serial.println(militar);
  */
  if ((dia == 1703) || (dia == 2403) || (dia == 3103) || (dia == 704) || (dia == 1404) || (dia == 2104) || (dia == 2804) || (dia == 505) || (dia == 1205) || (dia == 1905) || (dia == 2605) || (dia == 206) || (dia == 906)) {
    //LOS DOMINGOS SON DEL SEÑOR, ALELUYA HERMANO
  } else {
    if ((militar == 900) || (militar == 1000) || (militar == 1100) || (militar == 1200) || (militar == 1300) || (militar == 1400) || (militar == 1500) || (militar == 1600) || (militar == 1700) || (militar == 1800) || (militar == 1900) || (militar == 2000) || (militar == 2100)) {
      if (wait == 1) {
        readCSV("coreo");
        reloz();
      }
    }
  }
}


//--------------DISPLAY FUNCTIONS------------------
void printOLED() {
  if (!displayOK) return;
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
  const AsyncWebParameter *plainParam = request->getParam("plain", true);
  if (plainParam != nullptr) {
    String presetToPlay = plainParam->value();
    if (!isValidPresetName(presetToPlay)) {
      request->send(400, "text/plain", "Bad request: invalid preset name");
      return;
    }
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
  const AsyncWebParameter *plainParam = request->getParam("plain", true);
  if (plainParam != nullptr) {
    String csvData = plainParam->value();
    if (csvData.length() > MAX_CSV_UPLOAD_BYTES) {
      request->send(413, "text/plain", "Payload too large");
      return;
    }
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

// Only allow simple, relative filenames (letters, digits, '_', '-') so that
// user-supplied preset/file names can never escape the SPIFFS root via '/'
// or '..', or overwrite arbitrary files.
bool isValidPresetName(const String &name) {
  if (name.length() == 0 || name.length() > 32) {
    return false;
  }
  for (unsigned int i = 0; i < name.length(); i++) {
    char c = name.charAt(i);
    if (!isalnum(c) && c != '_' && c != '-') {
      return false;
    }
  }
  return true;
}

void handleRoot(AsyncWebServerRequest *request) {
  request->send(SPIFFS, "/index.html", "text/html");
}
void handleSaveCSV(AsyncWebServerRequest *request) {
  if (request->hasParam("name", true)) {
    const AsyncWebParameter *nameParam = request->getParam("name", true);
    if (!isValidPresetName(nameParam->value())) {
      request->send(400, "text/plain", "Bad request: invalid name");
      return;
    }
    String fileName = "/" + nameParam->value() + ".csv";

    Serial.println(request->params());
    // Read the CSV data from the request body
    String csvData = "";
    if (!request->hasParam("plain", true) && request->params() <= 2) {
      request->send(400, "text/plain", "Bad request: missing CSV body");
      return;
    }
    for (int i = 2; i < request->params(); i++) {
      const AsyncWebParameter *p = request->getParam(i);
      int val = request->params() - 1;
      if (i == val) {
        csvData += p->value();
      } else {
        csvData += p->value() + "\n";
      }
      if (csvData.length() > MAX_CSV_UPLOAD_BYTES) {
        request->send(413, "text/plain", "Payload too large");
        return;
      }
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
    const AsyncWebParameter *nameParam = request->getParam("name");
    if (!isValidPresetName(nameParam->value())) {
      request->send(400, "text/plain", "Bad request: invalid name");
      return;
    }
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
  pasosCSV = 0;
  // Read each line of the CSV file
  while (file.available()) {
    if (pasosCSV >= MAXSTEPS) {
      Serial.println("readCSV: MAXSTEPS reached, truncating file");
      break;
    }
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
