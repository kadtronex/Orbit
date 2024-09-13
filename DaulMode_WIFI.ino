#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

// Create an instance of the web server (HTTP server) on port 80
WebServer server(80);

// WiFi Credentials
char* ssid = "ESP32_AP";           // Name of the Access Point
char* password = "12345678";       // Password for the Access Point
String localSSID = "";
String localPass = "";

// Preferences object to store WiFi credentials in non-volatile storage (like EEPROM)
Preferences preferences;

// HTML form to serve for user input of WiFi credentials
const char* webpage = R"=====(
<!DOCTYPE html>
<html>
<head>
<title>WiFi Setup</title>
</head>
<body>
<h1>WiFi Setup</h1>
<form action="/save">
  <label for="ssid">SSID:</label><br>
  <input type="text" id="ssid" name="ssid"><br><br>
  <label for="pass">Password:</label><br>
  <input type="password" id="pass" name="pass"><br><br>
  <input type="submit" value="Save and Restart">
</form>
</body>
</html>
)=====";

void handleRoot() {
  server.send(200, "text/html", webpage);
}

void handleSave() {
  localSSID = server.arg("ssid");
  localPass = server.arg("pass");

  // Save the credentials in Preferences (non-volatile storage)
  preferences.begin("WiFiCreds", false);
  preferences.putString("ssid", localSSID);
  preferences.putString("pass", localPass);
  preferences.end();

  server.send(200, "text/html", "<h1>Credentials Saved! Restarting...</h1>");
  
  delay(3000);
  ESP.restart();
}

void startAccessPoint() {
  // Start ESP32 in Access Point mode
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Access Point started. Connect to it and enter WiFi credentials: ");
  Serial.println(IP);

  // Handle the root URL (serves the HTML form)
  server.on("/", handleRoot);

  // Handle the "/save" URL (saves the credentials)
  server.on("/save", handleSave);

  // Start the web server
  server.begin();
}

void connectToWiFi() {
  // Retrieve saved credentials
  preferences.begin("WiFiCreds", true);
  String savedSSID = preferences.getString("ssid", "");
  String savedPass = preferences.getString("pass", "");
  preferences.end();

  // Check if credentials are saved
  if (savedSSID != "" && savedPass != "") {
    WiFi.begin(savedSSID.c_str(), savedPass.c_str());
    Serial.print("Connecting to WiFi: ");
    Serial.println(savedSSID);

    // Wait for connection
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }

    // Check if connected
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected to WiFi!");
      Serial.print("ESP32 IP Address: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("\nFailed to connect. Starting AP mode...");
      startAccessPoint();
    }
  } else {
    // No saved credentials, start AP mode
    startAccessPoint();
  }
}

void setup() {
  Serial.begin(115200);

  // Check if there are saved WiFi credentials and try to connect
  connectToWiFi();
}

void loop() {
  // Handle incoming web client requests
  server.handleClient();
}
