#define LED 2


void setup() {
  pinMode(LED,OUTPUT);
  Serial2.begin(115200, SERIAL_8N1, 16, 17); // Inicia UART2 Rx=16 Tx=17
  delay(10);

}

void loop() {
  Serial2.write("Hola");
  delay(1000);
  digitalWrite(LED,HIGH);
  delay(1000);
  digitalWrite(LED,LOW);

}
