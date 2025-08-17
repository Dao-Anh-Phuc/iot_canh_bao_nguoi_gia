#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Cấu hình WiFi
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// API Configuration
const char* API_BASE_URL = "https://your-iot-app.onrender.com"; // Thay bằng Render URL sau khi deploy
const char* DEVICE_ID = "ESP32_001"; // Unique device ID

// Button configuration
const int BUTTON_PIN = 2;
bool buttonPressed = false;
unsigned long lastButtonPress = 0;
const unsigned long DEBOUNCE_DELAY = 1000; // 1 second debounce

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

void loop() {
  // Check button press
  if (digitalRead(BUTTON_PIN) == LOW && !buttonPressed) {
    unsigned long currentTime = millis();
    if (currentTime - lastButtonPress > DEBOUNCE_DELAY) {
      buttonPressed = true;
      lastButtonPress = currentTime;
      Serial.println("Button pressed! Sending alert...");
      sendAlert();
    }
  } else if (digitalRead(BUTTON_PIN) == HIGH) {
    buttonPressed = false;
  }
  
  delay(100);
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