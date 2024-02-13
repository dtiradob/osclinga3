#define LED 2
#define EN14 14

void setup() {
  pinMode(LED,OUTPUT);
  pinMode(EN14,OUTPUT);
  digitalWrite(EN14, HIGH);
  Serial2.begin(115200, SERIAL_8N1, 16, 17); // Inicia UART2 Rx=16 Tx=17
  Serial.begin(115200);
  delay(10);
}

void loop() {
  while (Serial.available()) {
    int inByte = Serial.read();
    Serial2.write(inByte);
  }

}
