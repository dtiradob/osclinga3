#define LED 2
#define EN14 14
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire, -1);

void setup() {
  pinMode(LED,OUTPUT);
  pinMode(EN14,OUTPUT);
  digitalWrite(EN14, LOW);
  Serial2.begin(115200, SERIAL_8N1, 16, 17); // Inicia UART2 Rx=16 Tx=17
  Serial.begin(115200);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(100);
  display.clearDisplay();
  display.display();
  display.setTextColor(WHITE);

}

void loop() {
  if (Serial2.available()){
    display.clearDisplay();
    while (Serial2.available()) {
    
    //int inByte = Serial2.read();
  
    //Serial.write(inByte);
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print(Serial2.readString());

    display.display();
    delay(100);

  }
  }
  

}
