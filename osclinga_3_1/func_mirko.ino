

//------------------------------------------------------------------------

void apaga() {

  if (!fin) {
    estorbo2(0, LEFT, RIGHT, 70, 60, 60);
    //estorbo2(0, 70, 60);
    delay(1000);
    for (int i = 0; i < 4; i++) {
      leds[i] = 0;
    }
    delay(3000);
    for (int i = 0; i < 4; i++) {
      leds[i] = 100;
    }
    delay(80);
    for (int i = 0; i < 4; i++) {
      leds[i] = 0;
    }
    delay(2000);
    fin = 1;
  }
}

//------------------------------------------------------------------------

void preset0() {


  STOP(frame, 1);

  STOP(frame, 2);

  for (int i = 0; i < 4; i++) {
    leds[i] = 0;
  }

}

//------------------------------------------------------------------------

void preset1() {

  if (preset == 1) {
    RUN(frame, 1);

    FREC(frame, 1, 8);

    RUN(frame, 2);

    FREC(frame, 2, 8);

    preset = 2;
  }
}

//------------------------------------------------------------------------

void preset2() {

  if (preset == 2) {
    FREC(frame, 1, 1);
    //10
    FREC(frame, 2, 1);
    delay(7000);  //7010
    FREC(frame, 2, 30);
    delay(1000);
    leds[BACK-1] = 100; 
    //ledcWrite(3, 100);
    delay(50);
    leds[BACK-1] = 0; 
    //ledcWrite(3, 0);
    leds[RIGHT-1] = 100; 
    //ledcWrite(4, 100);
    delay(2000);
    leds[RIGHT-1] = 0;
    //ledcWrite(4, 0);
    FREC(frame, 2, 1);
    delay(2940);
    FREC(frame, 1, 30);
    delay(1000);
    leds[BACK-1] = 100;
    //ledcWrite(3, 100);
    delay(50);
    leds[BACK-1] = 0;
    //ledcWrite(3, 0);
    leds[LEFT-1] = 100;
    //ledcWrite(1, 100);
    FREC(frame, 1, 1);
    delay(500);
    leds[LEFT-1] = 0;
    //ledcWrite(1, 0);
    FREC(frame, 2, 33);
    delay(1000);
    leds[BACK-1] = 100;
    //ledcWrite(3, 100);
    delay(50);
    leds[BACK-1] = 0;
    //ledcWrite(3, 0);
    leds[RIGHT-1] = 100;
    //ledcWrite(4, 100);
    delay(2000);
    leds[RIGHT-1] = 0;
    //ledcWrite(4, 0);
    FREC(frame, 2, 1);
    preset = 3;
  }
}

//------------------------------------------------------------------------

void preset20() {

  if (preset == 20) {
    FREC(frame, 1, 1);
    FREC(frame, 2, 1);
    delay(7000);
    FREC(frame, 1, 30);
    delay(1000);
    leds[BACK-1] =100;
    //ledcWrite(3, 100);
    delay(50);
    leds[BACK-1] =0;
    //ledcWrite(3, 0);
    leds[LEFT-1] =100;
    //ledcWrite(1, 100);
    FREC(frame, 1, 1);
    delay(500);
    leds[LEFT-1] =0;
    //ledcWrite(1, 0);
    delay(2940);
    FREC(frame, 2, 33);
    delay(1000);
    leds[BACK-1] =100;
    //ledcWrite(3, 100);
    delay(50);
    leds[BACK-1] =0;
    //ledcWrite(3, 0);
    leds[RIGHT-1] =100;
    //ledcWrite(4, 100);
    delay(2000);
    leds[RIGHT-1] =0;
    //ledcWrite(4, 0);
    FREC(frame, 2, 1);
    delay(1000);
    FREC(frame, 1, 30);
    delay(1000);
    leds[BACK-1] =100;
    //ledcWrite(3, 100);
    delay(50);
    leds[BACK-1] =0;
    //ledcWrite(3, 0);
    leds[LEFT-1] =100;
    //ledcWrite(1, 100);
    FREC(frame, 1, 1);
    delay(500);
    leds[LEFT-1] =0;
    //ledcWrite(1, 0);
    preset = 4;
  }
}

//------------------------------------------------------------------------

void preset3() {

  int zap = random(2);
  if (zap == 0) {
    FREC(frame, 1, (random(13) + 15));
    delay(2500);
    FREC(frame, 1, 14);
  }
  if (zap == 1) {
    FREC(frame, 2, (random(13) + 11));
    delay(2500);
    FREC(frame, 2, 7);
  }
  preset = 20;
}

//------------------------------------------------------------------------

void preset4() {

  int randomcito;
  int zap = random(2);
  if (zap == 0) {
    randomcito = (random(14) + 15);
    FREC(frame, 1, randomcito);
    if (randomcito >= 22) {
      leds[BACK-1] =100;
      //ledcWrite(3, 100);
      delay(50);
      leds[BACK-1] =0;
      //ledcWrite(3, 0);
      leds[LEFT-1] =100;
      //ledcWrite(1, 100);
      delay(2000);
      FREC(frame, 1, 12);
      delay(500);
      leds[LEFT-1] =0;
      //ledcWrite(1, 0);
    }
  }

  if (zap == 1) {
    randomcito = (random(13) + 15);
    FREC(frame, 2, randomcito);
    if (randomcito >= 22) {
      leds[BACK-1] =100;
      //ledcWrite(3, 100);
      delay(50);
      leds[BACK-1] =0;
      //ledcWrite(3, 0);
      leds[RIGHT-1] =100;
      //ledcWrite(4, 100);
      delay(2000);
      FREC(frame, 2, 8);
      delay(500);
      leds[RIGHT-1] =0;
      //ledcWrite(4, 0);
    }
  }
}

//------------------------------------------------------------------------

void preset5() {


  FREC(frame, 1, 10);

  FREC(frame, 2, 10);

  preset = 6;
}

//------------------------------------------------------------------------

void preset50() {


  FREC(frame, 1, 10);

  FREC(frame, 2, 10);

  preset = 7;
}

//------------------------------------------------------------------------

void preset500() {


  FREC(frame, 1, 10);

  FREC(frame, 2, 10);

  preset = 8;
}

//------------------------------------------------------------------------

void preset6() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 5000) {
    previousMillis = currentMillis;
    FREC(frame, 1, (random(17) + 16));
  }

  if (currentMillis - previousMillis2 >= 6000) {
    previousMillis2 = currentMillis;
    FREC(frame, 2, (random(30) + 10));
  }
}

//------------------------------------------------------------------------

void preset7() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis7 >= 5000) {
    previousMillis7 = currentMillis;
    FREC(frame, 1, (random(17) + 16));
  }

  if (currentMillis - previousMillis8 >= 6000) {
    previousMillis8 = currentMillis;
    FREC(frame, 2, (random(30) + 10));
  }
}

//------------------------------------------------------------------------
//void estorback(int run, int pwm, int int1, int int2) {
void estorback(int runh, int led1h, int led2h, int pwmh, int int1h, int int2h) {
  run = runh;
  led1 = led1h;
  led2 = led2h;
  int1 = int1h;
  int2 = int2h;
  pwm = pwmh;
  /*
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
  */
}

//------------------------------------------------------------------------

void estorbackfin(int run, int pwm, int int1, int int2) {

  unsigned long currentMillis = millis();

  if (run) {
    if (estorbo_on_back == 0) {
      if (currentMillis - previous_back >= (int1 + random(1000))) {
        previous_back = currentMillis;
        FREC(frame, 1, 30);
        delay(1000);
        leds[FRONT - 1] = pwm;
        //ledcWrite(2, pwm);
        estorbo_on_back = 1;
      }
    } else {
      if (currentMillis - previous_back >= int2) {
        previous_back = currentMillis;
        leds[FRONT - 1] = 0;
        //(2, 0);
        FREC(frame, 1, 1);
        estorbo_on_back = 0;
      }
    }
  } else {
    leds[FRONT-1] =0;
    //ledcWrite(2, 0);
    estorbo_on_back = 0;
  }
}

//------------------------------------------------------------------------

//void estorbo(int run, int pwm, int int1, int int2) {
void estorbo(int runh, int led1h, int led2h, int pwmh, int int1h, int int2h) {
  run = runh;
  led1 = led1h;
  led2 = led2h;
  int1 = int1h;
  int2 = int2h;
  pwm = pwmh;

  // unsigned long currentMillis = millis();

  // if (run) {
  //   if (estorbo_on == 0) {
  //     if (currentMillis - previous >= int1) {
  //       previous = currentMillis;
  //       ledcWrite(3, pwm);
  //       estorbo_on = 1;
  //     }
  //   } else {
  //     if (currentMillis - previous >= int2) {
  //       previous = currentMillis;
  //       ledcWrite(3, 0);
  //       estorbo_on = 0;
  //     }
  //   }
  // } else {
  //   ledcWrite(3, 0);
  //   estorbo_on = 0;
  // }
}

//------------------------------------------------------------------------

//void estorbo2(int run, int pwm, int int1) {
void estorbo2(int runh, int led1h, int led2h, int pwmh, int int1h, int int2h) {
  run = runh;
  led1 = led1h;
  led2 = led2h;
  int1 = int1h;
  int2 = int2h;
  pwm = pwmh;
  /*
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
  */
}

//------------------------------------------------------------------------

// Protocol                 : Modbus RTU
// Error check              : CRC
// Baud rate                : 9600bps, 19200bps, 38400bps, 57600bps, 115200bps(default)
// Data format              : 1 start bit, 8 data bits, 1 stop bits, no parity
// Physical signal          : RS 485 (2-wire)
// User interface           : RJ45
// Supported Function Codes : 03 Read Multiple Holding Registers
//                            06 Write Single Holding Register
//                            16 Write Multiple Holding Registers (Supported for registers 1-4 only)

// P-36 Serial Communications Configuration
//      Index 1 : Address = 1 (0x01)
//      Index 2 : Baud Rate = 115.2kbps

// Function Code.  : (0x06) 06

// Register Number : (0x00 0x00) 1
// Parameter       : (0x00 0x00) Stop
//                 : (0x00 0x01) Run

// Register Number : (0x00 0x01) 2
// Parameter       : (0x00 0x00 - 0x13 0x88) Frequency x10 (0 - 500Hz)

//------------------------------------------------------------------------
/*
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
*/
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