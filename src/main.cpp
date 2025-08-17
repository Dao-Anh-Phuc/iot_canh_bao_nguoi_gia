#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Cấu hình WiFi
const char* ssid = "Tang 2";
const char* password = "88888888";

// API Configuration
const char* API_BASE_URL = "https://iot-backend-346j.onrender.com";
const char* DEVICE_ID = "esp32_001"; // Unique device ID

// Button configuration
const int BUTTON_PIN = 2;
bool buttonPressed = false;
unsigned long lastButtonPress = 0;
const unsigned long DEBOUNCE_DELAY = 1000; // 1 second debounce
void sendAlert();
void checkCommandFromBackend();
void setup() {
  Serial.begin(115200);
  
  // Setup button
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

unsigned long lastCommandCheck = 0;
const unsigned long COMMAND_CHECK_INTERVAL = 5000; // 5 giây

void loop() {
  // Test: gửi alert khi nhập ký tự '1' từ Serial
  if (Serial.available()) {
    char c = Serial.read();
    if (c == '1') {
      Serial.println("Serial input '1' detected! Sending alert...");
      sendAlert();
    }
  }

  // Poll lệnh từ backend mỗi 5 giây
  if (millis() - lastCommandCheck > COMMAND_CHECK_INTERVAL) {
    lastCommandCheck = millis();
    checkCommandFromBackend();
  }
  delay(100);
}
void checkCommandFromBackend() {
  HTTPClient http;
  String url = String(API_BASE_URL) + "/auth/devices/" + DEVICE_ID + "/command";
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (!error) {
      String command = doc["command"].as<String>();
      Serial.println("Received command: " + command);
      // Thực hiện lệnh nếu cần
      if (command == "call_user") {
        sendAlert();
      }
    } else {
      Serial.println("JSON parsing failed (command)");
    }
  } else {
    Serial.printf("HTTP GET command failed, code: %d\n", httpCode);
  }
  http.end();
}

String getPhoneNumber() {
  HTTPClient http;
  String url = String(API_BASE_URL) + "/auth/devices/" + DEVICE_ID + "/phone";
  
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  
  int httpCode = http.GET();
  String phoneNumber = "";
  
  if (httpCode == 200) {
    String payload = http.getString();
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error) {
      phoneNumber = doc["phone_number"].as<String>();
      Serial.println("Phone number retrieved: " + phoneNumber);
    } else {
      Serial.println("JSON parsing failed");
    }
  } else {
    Serial.printf("HTTP GET failed, code: %d\n", httpCode);
  }
  
  http.end();
  return phoneNumber;
}

void sendAlert() {
  String phoneNumber = getPhoneNumber();
  
  if (phoneNumber.length() > 0) {
    Serial.println("Sending SMS to: " + phoneNumber);
    // TODO: Implement SMS sending logic
    // Could use Twilio API, local SMS gateway, etc.
    
    // Log alert to backend
    HTTPClient http;
    String url = String(API_BASE_URL) + "/auth/alerts";
    
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    
    JsonDocument alertDoc;
    alertDoc["device_id"] = DEVICE_ID;
    alertDoc["alert_type"] = "emergency_button";
    alertDoc["message"] = "Emergency button pressed!";
    alertDoc["phone_number"] = phoneNumber;
    
    String alertJson;
    serializeJson(alertDoc, alertJson);
    
    int httpCode = http.POST(alertJson);
    if (httpCode == 200 || httpCode == 201) {
      Serial.println("Alert logged successfully");
    } else {
      Serial.printf("Failed to log alert, code: %d\n", httpCode);
    }
    
    http.end();
  } else {
    Serial.println("No phone number configured for device");
  }
}