
// *****************************[2021] mirko.petrovich@gmail.com********************************************************************************
//
// Protocol                 : Modbus RTU
// Error check              : CRC
// Baud rate                : 9600bps, 19200bps, 38400bps, 57600bps, 115200bps(default)
// Data format              : 1 start bit, 8 data bits, 1 stop bits, no parity
// Physical signal          : RS 485 (2-wire)
// User interface           : RJ45
// Supported Function Codes : 03 Read Multiple Holding Registers
//                            06 Write Single Holding Register
//                            16 Write Multiple Holding Registers (Supported for registers 1-4 only)


// P-12
// 3 : Modbus Control
// 4:  Modbus Control con Rampas


// P-36 Serial Communications Configuration
//      Index 1 : Address = 1 (0x01)
//      Index 2 : Baud Rate = 115.2kbps

// Function Code : 06 (0x06)

// Register Number : (0x00 0x00) 1 !!!!!
// Function        : (0x00 0x00) Stop
//                 : (0x00 0x01) Run

// Register Number : (0x00 0x01) 2
// Function        : (0x00 0x00 - 0x13 0x88) Frequency x10 (0 - 500Hz)


//frame[8] = { Address, Function Code, Register HIGH Byte, Register LOW Byte, Param HIGH Byte, Param LOW Byte, CRC LOW Byte, CRC HIGH Byte }

//CRC-16/MODBUS  0x010600000000 -> 0xCA89
//               0x010600000001 -> 0x0A48
//               0x01060001015E -> 0x6258
//               0x010600010032 -> 0xDF59
//
// *********************************************************************************************************************************************

#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>
#include <Wire.h>
#include <ds3231.h>

const char ssid[] = "ppql";          // your network SSID (name)
const char password[] = "12345678";                    // your network password
const uint8_t ADDRESS = 0x01;
const uint8_t ADDRESS2 = 0x02;
const uint8_t FUNCTION = 0x06;
unsigned char frame[8] = { ADDRESS, FUNCTION, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char frame2[8] = { ADDRESS2, FUNCTION, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
struct ts t;               //REAL TIME CLOCK
const int buttonPin = 2;  //PIN BOTÓN MULTIMODO
int buttonState = 0;       //ESTADO BOTÓN MULTIMODO
WiFiUDP Udp;   // A UDP instance to let us send and receive packets over UDP
const unsigned int localPort = 8888;        // local port to listen for UDP packets (here's where we send the packets)
OSCErrorCode error;
int frecuencia_1;
int frecuencia_2;
int encendido;
int encendido2;
int duty_1 = 0;
int duty_2 = 0;
int duty_3 = 0;
int duty_4 = 0;
unsigned long previousMillis_1 = 0;
unsigned long previousMillis_2 = 0;
unsigned long previousMillis_3 = 0;
unsigned long previousMillis_4 = 0;

unsigned long previous = 0;


int interval1A = 500;
int interval2A = 100;
int interval1B = 500;
int interval2B = 100;
int interval1C = 500;
int interval2C = 100;
int interval1D = 500;
int interval2D = 100;

int strobe_on1 = 0;
int strobe_on2 = 0;
int strobe_on3 = 0;
int strobe_on4 = 0;
int strobe_run1 = 0;
int strobe_run2 = 0;
int strobe_run3 = 0;
int strobe_run4 = 0;

int estorbo_on = 0;

unsigned long tiempo;
int flag_on1 = 0;
int sustain1;
int pwm1 = 0;
int flag_on2 = 0;
int sustain2;
int pwm2 = 0;
int flag_on3 = 0;
int sustain3;
int pwm3 = 0;
int flag_on4 = 0;
int sustain4;
int pwm4 = 0;

int selec = 0;
int apretado = 0;
unsigned long pushtime = 0;
unsigned long ahora;
unsigned long push;
int modox = 0;

int reset1 = 0;
int preset = 0;

int activa_estrobo_3 = 0;









// *******************************************************************************************

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(2, OUTPUT);
  // ********************** RTC **************************
  Wire.begin();
  // ********************** PWM **************************
  ledcAttachPin(27, 1);
  ledcAttachPin(4, 2);
  ledcAttachPin(32, 3);
  ledcAttachPin(25, 4);
  ledcSetup(1, 1000, 8);
  ledcSetup(2, 1000, 8);
  ledcSetup(3, 1000, 8);
  ledcSetup(4, 1000, 8);

  Serial.begin(115200);

  // ************************** MODBUS ***************************************
  Serial2.begin(115200, SERIAL_8N1, 16, 17); // Inicia UART2 Rx=16 Tx=17
  // ************************ ACCESS POINT ************************************
  // Connect to WiFi network
  delay(10);

  WiFi.mode(WIFI_AP);
  while (!WiFi.softAP(ssid, password))
  {
    Serial.println(".");
    delay(100);
  }
  Serial.print("Iniciado AP ");
  Serial.println(ssid);
  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP());

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
}

// ********************************************** LOOP *******************************************************
void loop() {

  /*buttonState = digitalRead(buttonPin);
  if (buttonState == LOW) {
    // turn LED on:
    digitalWrite(2, HIGH);
  } else {
    // turn LED off:
    digitalWrite(2, LOW);
  }*/

    buttonState = digitalRead(buttonPin); //lee botón
  if (buttonState == HIGH) {    //si está apretado
   if(!apretado)                //y no estaba apretado
   {
    pushtime = millis();        //guarda tiempo en pushtime
   apretado = 1;                //registra que está apretado
   selec = 1;                   //indica que se apretó una vez
   }
    
  }
  
  else {
   
  apretado = 0;   //registra que no está apretado
  }


ahora = millis();   //tiempo actual
push = ahora-pushtime;  //tiempo que ha pasado desde que se apretó

if ((!apretado)&&(selec)&&(push<1000)) {  //si no está apretado pero se apretó hace menos de 1 seg
  digitalWrite(LED_BUILTIN, HIGH) ;
  //modox = 0;
  selec = 0;  // reset
}

if ((!apretado)&&(selec)&&(push>=2000)&&(push<5000)) {
  digitalWrite(LED_BUILTIN, LOW) ;
  //modox = 1;
  selec = 0;
}

if ((!apretado)&&(selec)&&(push>=5000)) {
  digitalWrite(LED_BUILTIN, HIGH) ;
  delay(300);
  digitalWrite(LED_BUILTIN, LOW) ;
  delay(300);
  digitalWrite(LED_BUILTIN, HIGH) ;
  delay(300);
  digitalWrite(LED_BUILTIN, LOW) ;
  
  
  selec = 0;

}

  DS3231_get(&t);
  int minutos = t.min;
  int horas = t.hour;
  int hora = horas * 100 + minutos;
  // if (hora==1651) RUN();
  // if (hora==1652) STOP();


  OSCMessage msg;
  int size = Udp.parsePacket();

  if (size > 0) {
    while (size--) {
      msg.fill(Udp.read());
    }
    if (!msg.hasError()) {
      msg.dispatch("/motor", motores);
      msg.dispatch("/estrobo_1", estrobo1);
      msg.dispatch("/estrobo_2", estrobo2);
      //msg.dispatch("/estrobo_3", estrobo3);
      msg.dispatch("/prende_1", prende1);
      msg.dispatch("/modo", modo_luces);
      msg.dispatch("/preset", preset0);
    }
    else
    {
      error = msg.getError();
      Serial.print("error: ");
      Serial.println(error);
    }
  }


//strobox3();

 if (preset==0) 
  {
  if (modox == 0) 
    {
    strobox1();
    strobox2();
    strobox3();
    }
else
    {
  sec1();
    }

  }
  
if (preset==1) {
  


  estorbo(120,12);
 // preset1();
 
  
  
  /* strobe_run3 = 1;
  interval1C = 120;
  interval2C = 12;
  duty_3 = 100;
  strobox3() ; */
  
}




}



// *************************** FUNCIONES ***************************************************


void preset0(OSCMessage &msg) {
  preset = msg.getInt(0);
}

void preset1() {

  if (!reset1) 
  {
  RUN();
  delay(10);
  RUN2();
  delay(10);
  FREC(8);
  delay(10);
  FREC2(8);
  
  reset1 = 1;
  }
}

void modo_luces(OSCMessage &msg) {
  modox = msg.getInt(0);
}

void prende1(OSCMessage &msg) {
  sustain1 = msg.getInt(0);
  pwm1 = msg.getInt(1);
  sustain2 = msg.getInt(2);
  pwm2 = msg.getInt(3);
  sustain3 = msg.getInt(4);
  pwm3 = msg.getInt(5);
  sustain4 = msg.getInt(6);
  pwm4 = msg.getInt(7);

  if (modox==1) {
  ledcWrite(1, pwm1);
  ledcWrite(2, pwm2);
  ledcWrite(3, pwm3);
  ledcWrite(4, pwm4);
  tiempo = millis();
  flag_on1 = 1;
  flag_on2 = 1;
  flag_on3 = 1;
  flag_on4 = 1;
  }

  else

  {
  ledcWrite(1, 0);
  ledcWrite(2, 0);
  ledcWrite(3, 0);
  ledcWrite(4, 0);
  flag_on1 = 0;
  flag_on2 = 0;
  flag_on3 = 0;
  flag_on4 = 0;
  }


}



void sec1() {

  unsigned long now = millis();
  if (flag_on1)
  {
    if (now - tiempo >= sustain1)
    {
      ledcWrite(1, 0);
      flag_on1 = 0;
    }
  }

  if (flag_on2)
  {
    if (now - tiempo >= sustain2)
    {
      ledcWrite(2, 0);
      flag_on2 = 0;
    }
  }

  if (flag_on3)
  {
    if (now - tiempo >= sustain3)
    {
      ledcWrite(3, 0);
      flag_on3 = 0;
    }
  }

  if (flag_on4)
  {
    if (now - tiempo >= sustain4)
    {
      ledcWrite(4, 0);
      flag_on4 = 0;
    }
  }

}


void motores(OSCMessage &msg) {
 
  encendido = msg.getInt(0);
  frecuencia_1 = msg.getInt(1);
  encendido2 = msg.getInt(2);
  frecuencia_2 = msg.getInt(3);
  

   FREC(frecuencia_1 / 10.);

   delay(10);
   FREC2(frecuencia_2 / 10.);

   delay(10);
  
  if (encendido == 1) 
  { 
    RUN();
  }
  else 
  {
    STOP();
  }

  delay(10);
  
  if (encendido2 == 1) 
  {
    RUN2();
  }
  else 
  {
    STOP2();
  }
}



void duty2(OSCMessage &msg) {
  duty_2 = msg.getInt(0);
}

void duty3(OSCMessage &msg) {
  duty_3 = msg.getInt(0);
}

void duty4(OSCMessage &msg) {
  duty_4 = msg.getInt(0);
}

void strobox1() {

  unsigned long currentMillis = millis();

  if (strobe_run1)
  {
    if (strobe_on1 == 0)
    {
      if (currentMillis - previousMillis_1 >= interval1A)
      {
        previousMillis_1 = currentMillis;
        ledcWrite(1, duty_1);
        ledcWrite(4, 0);
        strobe_on1 = 1;
      }
    }
    else
    {
      if (currentMillis - previousMillis_1 >= interval2A)
      {
        previousMillis_1 = currentMillis;
        ledcWrite(1, 0);
        ledcWrite(4, duty_1);
        strobe_on1 = 0;
      }
    }
  }
  else
  {
    ledcWrite(1, 0);
    ledcWrite(4,0);
    strobe_on1 = 0;
  }
}

void strobox2() {

  unsigned long currentMillis = millis();

  if (strobe_run2)
  {
    if (strobe_on2 == 0)
    {
      if (currentMillis - previousMillis_2 >= interval1B)
      {
        previousMillis_2 = currentMillis;
        ledcWrite(2, duty_2);
        strobe_on2 = 1;
      }
    }
    else
    {
      if (currentMillis - previousMillis_2 >= interval2B)
      {
        previousMillis_2 = currentMillis;
        ledcWrite(2, 0);
        strobe_on2 = 0;
      }
    }
  }
  else
  {
    ledcWrite(2, 0);
    strobe_on2 = 0;
  }
}

 void strobox3() {

  unsigned long currentMillis = millis();

  if (strobe_run3)
  {
    if (strobe_on3 == 0)
    {
      if (currentMillis - previousMillis_3 >= interval1C)
      {
        previousMillis_3 = currentMillis;
        ledcWrite(3, duty_3);
        strobe_on3 = 1;
      }
    }
    else
    {
      if (currentMillis - previousMillis_3 >= interval2C)
      {
        previousMillis_3 = currentMillis;
        ledcWrite(3, 0);
        strobe_on3 = 0;
      }
    }
  }
  else
  {
    ledcWrite(3, 0);
    strobe_on3 = 0;
  }
} 

 void estorbo(int intA, int intB) {

  unsigned long currentMillis = millis();

  if (1)
  {
    if (estorbo_on == 0)
    {
      if (currentMillis - previous >= intA)
      {
        previous = currentMillis;
        ledcWrite(3, 100);
        estorbo_on = 1;
      }
    }
    else
    {
      if (currentMillis - previous >= intB)
      {
        previous = currentMillis;
        ledcWrite(3, 0);
        estorbo_on = 0;
      }
    }
  }
  else
  {
    ledcWrite(3, 0);
    estorbo_on = 0;
  }
} 

/*void strobox4() {

  unsigned long currentMillis = millis();

  if (strobe_run4)
  {
    if (strobe_on4 == 0)
    {
      if (currentMillis - previousMillis_4 >= interval1D)
      {
        previousMillis_4 = currentMillis;
        ledcWrite(4, duty_4);
        strobe_on4 = 1;
      }
    }
    else
    {
      if (currentMillis - previousMillis_4 >= interval2D)
      {
        previousMillis_4 = currentMillis;
        ledcWrite(4, 0);
        strobe_on4 = 0;
      }
    }
  }
  else
  {
    ledcWrite(4, 0);
    strobe_on4 = 0;
  }
}*/

// ************************************** OSC ESTROBOS ******************************************************

void estrobo1(OSCMessage &msg) {
  strobe_run1 = msg.getInt(0);
  interval1A = msg.getInt(1);
  interval2A = msg.getInt(2);
  duty_1 = msg.getInt(3);
}

void estrobo2(OSCMessage &msg) {
  strobe_run2 = msg.getInt(0);
  interval1B = msg.getInt(1);
  interval2B = msg.getInt(2);
  duty_2 = msg.getInt(3);
}
void estrobo3(OSCMessage &msg) {
  strobe_run3 = msg.getInt(0);
  interval1C = msg.getInt(1);
  interval2C = msg.getInt(2);
  duty_3 = msg.getInt(3);
}



// ************************************** NO TOCAR ESTE CÓDIGO *****************************************************************
void STOP() {
  frame[3] = 0x00;
  frame[4] = 0x00;
  frame[5] = 0x00;
  CRC();
  Serial2.write(frame, 8);
}

void RUN() {
  frame[3] = 0x00;
  frame[4] = 0x00;
  frame[5] = 0x01;
  CRC();
  Serial2.write(frame, 8);
}

void FREC(float frecuencia) {
  int frec_int = frecuencia * 10.;
  if (frec_int < 0) frec_int += 0xFFFF + 1;
  frame[3] = 0x01;
  frame[4] = (frec_int >> 8) & 0xFF;
  frame[5] = frec_int & 0xFF;
  CRC();
  Serial2.write(frame, 8);
}

void STOP2() {
  frame2[3] = 0x00;
  frame2[4] = 0x00;
  frame2[5] = 0x00;
  CRC2();
  Serial2.write(frame2, 8);
}

void RUN2() {
  frame2[3] = 0x00;
  frame2[4] = 0x00;
  frame2[5] = 0x01;
  CRC2();
  Serial2.write(frame2, 8);
}

void FREC2(float frecuencia) {
  int frec_int = frecuencia * 10.;
  if (frec_int < 0) frec_int += 0xFFFF + 1;
  frame2[3] = 0x01;
  frame2[4] = (frec_int >> 8) & 0xFF;
  frame2[5] = frec_int & 0xFF;
  CRC2();
  Serial2.write(frame2, 8);
}

void CRC()
{
  unsigned int temp, temp2, flag;
  temp = 0xFFFF;
  for (unsigned char i = 0; i < 6; i++)
  {
    temp = temp ^ frame[i];
    for (unsigned char j = 1; j <= 8 ; j++)
    {
      flag = temp & 0x0001;
      temp >>= 1;
      if (flag)
        temp ^= 0xA001;
    }
  }

  frame[6] = temp & 0xFF;
  frame[7] = (temp >> 8) & 0xFF;
}

void CRC2()
{
  unsigned int temp, temp2, flag;
  temp = 0xFFFF;
  for (unsigned char i = 0; i < 6; i++)
  {
    temp = temp ^ frame2[i];
    for (unsigned char j = 1; j <= 8 ; j++)
    {
      flag = temp & 0x0001;
      temp >>= 1;
      if (flag)
        temp ^= 0xA001;
    }
  }

  frame2[6] = temp & 0xFF;
  frame2[7] = (temp >> 8) & 0xFF;
}
