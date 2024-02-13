#define LED 2
#define EN14 14

unsigned char frame[8];
int estorbo_on = 0, estorbo_on2 = 0, estorbo_on_back = 0;
unsigned long previous = 0, previous2 = 0, previous_back = 0;
int sendDelay = 50;

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(EN14, OUTPUT);
  digitalWrite(EN14, HIGH);
  Serial2.begin(115200, SERIAL_8N1, 16, 17);  // Inicia UART2 Rx=16 Tx=17
  Serial.begin(115200);
  delay(10);

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
}

void loop() {
  
  while (Serial.available()) {
    int inByte = Serial.read();
    Serial2.write(inByte);
  }
  

  estorbo(1,255,50,80); // (run, pwm, offTime, onTime)
  estorbo2(1,255,30);
  RUN(frame, 1);
  delay(sendDelay);
  FREC(frame,1,10);
 // Serial2.write("HOLA");
  delay(1000);
  STOP(frame, 1);
  delay(sendDelay);
  FREC(frame,1,5);
 //Serial2.write("CHAU");
  estorbo(0,255,50,80); // (run, pwm, offTime, onTime)
  estorbo2(0,255,30);
  delay(1000);
}

//------------------------------------------------------------------------

void estorback(int run, int pwm, int int1, int int2) {

  unsigned long currentMillis = millis();

  if (run) {
    if (estorbo_on_back == 0) {
      if (currentMillis - previous_back >= (int1 + random(1000))) {
        previous_back = currentMillis;
        ledcWrite(2, pwm);
        estorbo_on_back = 1;
      }
    } else {
      if (currentMillis - previous_back >= int2) {
        previous_back = currentMillis;
        ledcWrite(2, 0);
        estorbo_on_back = 0;
      }
    }
  } else {
    ledcWrite(2, 0);
    estorbo_on_back = 0;
  }
}

//------------------------------------------------------------------------

void estorbackfin(int run, int pwm, int int1, int int2) {

  unsigned long currentMillis = millis();

  if (run) {
    if (estorbo_on_back == 0) {
      if (currentMillis - previous_back >= (int1 + random(1000))) {
        previous_back = currentMillis;
        FREC(frame, 30, 1);
        delay(1000);
        ledcWrite(2, pwm);
        estorbo_on_back = 1;
      }
    } else {
      if (currentMillis - previous_back >= int2) {
        previous_back = currentMillis;
        ledcWrite(2, 0);
        FREC(frame, 1, 1);
        estorbo_on_back = 0;
      }
    }
  } else {
    ledcWrite(2, 0);
    estorbo_on_back = 0;
  }
}

//------------------------------------------------------------------------

void estorbo(int run, int pwm, int int1, int int2) {

  unsigned long currentMillis = millis();

  if (run) {
    if (estorbo_on == 0) {
      if (currentMillis - previous >= int1) {
        previous = currentMillis;
        ledcWrite(3, pwm);
        estorbo_on = 1;
      }
    } else {
      if (currentMillis - previous >= int2) {
        previous = currentMillis;
        ledcWrite(3, 0);
        estorbo_on = 0;
      }
    }
  } else {
    ledcWrite(3, 0);
    estorbo_on = 0;
  }
}

//------------------------------------------------------------------------

void estorbo2(int run, int pwm, int int1) {

  unsigned long currentMillis = millis();

  if (run) {
    if (estorbo_on2 == 0) {
      if (currentMillis - previous2 >= int1) {
        previous2 = currentMillis;
        ledcWrite(1, pwm);
        ledcWrite(4, 0);
        estorbo_on2 = 1;
      }
    } else {
      if (currentMillis - previous2 >= int1) {
        previous2 = currentMillis;
        ledcWrite(1, 0);
        ledcWrite(4, pwm);
        estorbo_on2 = 0;
      }
    }
  } else {
    ledcWrite(1, 0);
    ledcWrite(4, 0);
    estorbo_on2 = 0;
  }
}

void STOP(unsigned char *frame, int address) {
    
  frame[0] = address;    // Address
  frame[1] = 0x06;       // Function Code
  frame[2] = 0x00;       // Register HIGH Byte
  frame[3] = 0x00;       // Register LOW Byte
  frame[4] = 0x00;       // Param HIGH Byte
  frame[5] = 0x00;       // Param LOW Byte
  CRC(frame);
  Serial2.write(frame,8);
}

void RUN(unsigned char *frame, int address) {
    
  frame[0] = address;    
  frame[1] = 0x06;      
  frame[2] = 0x00;       
  frame[3] = 0x00;
  frame[4] = 0x00;       
  frame[5] = 0x01;
  CRC(frame);
  Serial2.write(frame,8);
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
  Serial2.write(frame,8);
}

void CRC(unsigned char *frame) {
    
  unsigned int temp, flag;
  temp = 0xFFFF;

  for (int i = 0; i < 6; i++) {
    temp ^=frame[i];
    for (int j = 1; j <= 8 ; j++) {
      flag = temp & 0x0001;
      temp >>= 1;
      if (flag) temp ^= 0xA001;
    }
  }
  frame[6] = temp & 0xFF;         // CRC LOW Byte
  frame[7] = (temp >> 8) & 0xFF;  // CRC HIGH Byte  
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