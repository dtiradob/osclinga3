#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

const char *ssid = "Plan Humboldt 2.4Ghz";
const char *password = "holaplan0!";


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
  AsyncWebParameter* p;
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

void loop() {}
