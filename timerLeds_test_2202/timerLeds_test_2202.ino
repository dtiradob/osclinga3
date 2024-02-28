int ledID = 0;
int stateLed = 0;
int pwmLed = 0;
int tLedOn = 0;
int tLedOff = 0;

const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];  // temporary array for use when parsing
int integerFromPC[5] = { 0, 0, 0, 0, 0 };

boolean newData = false;

void setup() {
  Serial.begin(9600);
  Serial.println("Enter data in this style <ID,state,pwm,tON,tOFF>  ");
}

void loop() {
  recvWithStartEndMarkers();
  if (newData == true) {
    strcpy(tempChars, receivedChars);
    // this temporary copy is necessary to protect the original data
    //   because strtok() used in parseData() replaces the commas with \0
    parseData();
    showParsedData();
    newData = false;
  }

  ledUpdate();
}

//============

void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      } else {
        receivedChars[ndx] = '\0';  // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

//============

void parseData() {  // split the data into its parts

  char* strtokIndx;  // this is used by strtok() as an index

  strtokIndx = strtok(tempChars, ",");  // get the first part - the string
  ledID = atoi(strtokIndx);             // copy it to messageFromPC
  strtokIndx = strtok(NULL, ",");       // this continues where the previous call left off
  stateLed = atoi(strtokIndx);          // convert this part to an integer
  strtokIndx = strtok(NULL, ",");       // this continues where the previous call left off
  pwmLed = atoi(strtokIndx);            // convert this part to an integer
  strtokIndx = strtok(NULL, ",");       // this continues where the previous call left off
  tLedOn = atoi(strtokIndx);            // convert this part to an integer
  strtokIndx = strtok(NULL, ",");       // this continues where the previous call left off
  tLedOff = atoi(strtokIndx);           // convert this part to an integer
}

//============

void showParsedData() {
    Serial.print("ledID ");
    Serial.println(ledID);
    Serial.print("stateLed ");
    Serial.println(stateLed);
    Serial.print("pwmLed ");
    Serial.println(pwmLed);
    Serial.print("tLedOn ");
    Serial.println(tLedOn);
    Serial.print("tLedOff ");
    Serial.println(tLedOff);
}

//============

void ledUpdate() {
}