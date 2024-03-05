#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

const char *ssid = "Plan Humboldt 2.4Ghz";
const char *password = "holaplan0!";


float tiemposCSV[50];
float F1CSV[50];
float F2CSV[50];
int pasosCSV = 0;

AsyncWebServer server(80);

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void handleRoot(AsyncWebServerRequest *request) {
  request->send(SPIFFS, "/index.html", "text/html");
}

void handleSaveCSV(AsyncWebServerRequest *request) {
  Serial.println(request->params());
  // Read the CSV data from the request body
  String csvData = "";
  AsyncWebParameter *p;
  for (int i = 0; i < request->params(); i++) {
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
  File file = SPIFFS.open("/form_data.csv", FILE_WRITE);
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
  file = SPIFFS.open("/form_data.csv", FILE_READ);
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
}

void handleGetCSV(AsyncWebServerRequest *request) {
  // Open the CSV file for reading
  File file = SPIFFS.open("/form_data.csv", FILE_READ);
  if (!file) {
    request->send(500, "text/plain", "Error opening file");
    return;
  }

  // Read the content of the CSV file
  String csvContent = "";
  while (file.available()) {
    csvContent += (char)file.read();
  }
  file.close();

  // Send the CSV content to the client
  request->send(200, "text/csv", csvContent);
}


void setup() {
  Serial.begin(115200);

  if (!SPIFFS.begin(true)) {
    Serial.println("Error mounting SPIFFS");
    return;
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  server.on("/", HTTP_GET, handleRoot);
  server.on("/save_csv", HTTP_POST, handleSaveCSV);
  server.on("/get_csv", HTTP_GET, handleGetCSV);

  server.onNotFound(notFound);

  server.begin();
}

void loop() {

  readCSV();
  delay(10000);
}

/*
float tiemposCSV[50];
float F1CSV[50];
float F2CSV[50];
int pasosCSV = 0;
*/

void readCSV() {
  // Open the CSV file
  File file = SPIFFS.open("/form_data.csv", "r");
  if (!file) {
    Serial.println("Failed to open file");
    return;
  }

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
    tiemposCSV[pasosCSV] =rowData[0].toFloat();
    Serial.println(tiemposCSV[pasosCSV]);

    Serial.print("F1 ");
    Serial.print(pasosCSV);
    Serial.print(": ");
    Serial.print(rowData[1]);
    Serial.print("  | ");
    F1CSV[pasosCSV] =rowData[1].toFloat();
    Serial.println(F1CSV[pasosCSV]);

    Serial.print("F2 ");
    Serial.print(pasosCSV);
    Serial.print(": ");
    Serial.print(rowData[2]);
    Serial.print("  | ");
    F2CSV[pasosCSV] =rowData[2].toFloat();
    Serial.println(F2CSV[pasosCSV]);

    /*
    Serial.print("Field 2: ");
    Serial.println(rowData[1]);
    F1CSV[pasosCSV] =rowData[1].toFLoat();
    Serial.print("Field 3: ");
    Serial.println(rowData[2]);
    F2CSV[pasosCSV] =rowData[2].toFLoat();
    Serial.println("------");
    */

    pasosCSV++;
  }
}
