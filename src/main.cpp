// ...includes...
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <Preferences.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>

Preferences preferences;

// Lấy số điện thoại đã lưu từ Preferences
String getPhoneNumber() {
  preferences.begin("wifi", true);
  String phone = preferences.getString("phone_number", "");
  preferences.end();
  return phone;
}
AsyncWebServer server(80);
DNSServer dnsServer;

static const char* AP_SSID = "ESP32-Config";
static const char* AP_PASS = "12345678";
static const uint32_t WIFI_CONNECT_TIMEOUT_MS = 15000; // 15s
static const byte DNS_PORT = 53;

bool wifiJustConnected = false;
unsigned long wifiConnectedTime = 0;

// API Configuration
const char* API_BASE_URL = "https://iot-backend-346j.onrender.com";
const char* DEVICE_ID = "esp32_001"; // Unique device ID cố định cho sản phẩm

// Button configuration
const int BUTTON_PIN = 2;
bool buttonPressed = false;
unsigned long lastButtonPress = 0;
const unsigned long DEBOUNCE_DELAY = 1000; // 1 second debounce
String savedPhoneNumber = ""; // Biến lưu số điện thoại
void sendAlert();
void checkCommandFromBackend();

void startAP();
void startWebServer();
bool tryConnectSavedWiFi();

void setup() {
  // Đọc số điện thoại đã lưu (nếu có)
  preferences.begin("wifi", true);
  savedPhoneNumber = preferences.getString("phone_number", "");
  preferences.end();
  if (!savedPhoneNumber.isEmpty()) {
    Serial.printf("[PHONE] Số điện thoại đã lưu: %s\n", savedPhoneNumber.c_str());
  }
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Khởi tạo SPIFFS cho giao diện cấu hình
  if (!SPIFFS.begin(true)) {
    Serial.println("[SPIFFS] Mount failed. Formatting...");
    if (!SPIFFS.begin(true)) {
      Serial.println("[SPIFFS] Format failed! Cannot continue.");
      while(1) delay(1000);
    }
  }
  Serial.println("[SPIFFS] Mounted successfully");

  // DEVICE_ID cố định, không lấy từ Preferences
  Serial.printf("[DEVICE_ID] Sử dụng device_id cố định: %s\n", DEVICE_ID);

  // Luôn khởi động AP mode trước
  Serial.println("[WiFi] Starting Access Point mode...");
  startAP();

  // Thử kết nối WiFi đã lưu (sẽ chuyển sang AP+STA nếu thành công)
  if (tryConnectSavedWiFi()) {
    Serial.println("[WiFi] ✓ Connected to saved WiFi while maintaining AP mode");
  } else {
    Serial.println("[WiFi] No valid WiFi credentials or connection failed");
    Serial.println("[WiFi] Continuing in AP-only mode");
  }

  // Khởi động web server cấu hình
  startWebServer();

  Serial.println("=== Setup Complete ===");
  Serial.printf("[INFO] Free heap: %d bytes\n", ESP.getFreeHeap());
}

unsigned long lastCommandCheck = 0;
const unsigned long COMMAND_CHECK_INTERVAL = 5000; // 5 giây


void loop() {
  // Xử lý DNS requests cho captive portal (chỉ khi ở chế độ AP)
  if (WiFi.getMode() & WIFI_AP) {
    dnsServer.processNextRequest();
  }

  // Kiểm tra trạng thái WiFi định kỳ
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 30000) { // Kiểm tra mỗi 30s
    lastCheck = millis();
    if (WiFi.getMode() == WIFI_STA && WiFi.status() != WL_CONNECTED) {
      Serial.println("[WiFi] STA disconnected, switching to AP mode");
      startAP();
    }
  }

  // Nếu đã kết nối WiFi nhà thì chạy logic backend
  if (WiFi.status() == WL_CONNECTED) {
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
  }
  delay(100);
}
// ----------- Tích hợp các hàm cấu hình WiFi từ main_wifi.cpp -----------
bool tryConnectSavedWiFi() {
  preferences.begin("wifi", true);
  String ssid = preferences.getString("ssid", "");
  String pass = preferences.getString("pass", "");
  preferences.end();

  if (ssid.isEmpty()) {
    Serial.println("[WiFi] No SSID found in preferences");
    return false;
  }

  Serial.printf("[WiFi] Attempting to connect to: %s\n", ssid.c_str());
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());

  Serial.print("[WiFi] Connecting");
  uint32_t startTime = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < WIFI_CONNECT_TIMEOUT_MS) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[WiFi] ✓ Connected successfully!");
    Serial.printf("[WiFi] IP Address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("[WiFi] Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("[WiFi] DNS: %s\n", WiFi.dnsIP().toString().c_str());
    Serial.printf("[WiFi] RSSI: %d dBm\n", WiFi.RSSI());
    wifiJustConnected = true;
    wifiConnectedTime = millis();
    return true;
  } else {
    Serial.printf("[WiFi] ✗ Connection failed (Status: %d)\n", WiFi.status());
    WiFi.disconnect(true);
    return false;
  }
}

void startAP() {
  WiFi.mode(WIFI_AP_STA);
  IPAddress local_IP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  Serial.println("[AP] Trying to start Access Point on channel 1...");
  bool apStarted = WiFi.softAP(AP_SSID, AP_PASS, 1, 0, 8); // channel 1, max_connections=8
  if (!apStarted) {
    Serial.println("[AP] ✗ Failed to start AP on channel 1. Trying channel 6...");
    apStarted = WiFi.softAP(AP_SSID, AP_PASS, 6, 0, 8); // channel 6
  }
  if (apStarted) {
    Serial.println("[AP] ✓ Access Point started successfully!");
    Serial.printf("[AP] SSID: %s\n", AP_SSID);
    Serial.printf("[AP] Password: %s\n", AP_PASS);
    Serial.printf("[AP] IP Address: %s\n", WiFi.softAPIP().toString().c_str());
    Serial.printf("[AP] MAC Address: %s\n", WiFi.softAPmacAddress().c_str());
    Serial.printf("[AP] Channel: %d\n", WiFi.channel());
    Serial.printf("[AP] Max Connections: %d\n", WiFi.softAPgetStationNum());
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
    Serial.println("[DNS] Captive portal DNS started");
  } else {
    Serial.println("[AP] ✗ Failed to start Access Point on all channels! Please check for hardware issues or try restarting.");
  }
}

void startWebServer() {
  // API: Lưu số điện thoại từ giao diện
  server.on("/phone", HTTP_POST, [](AsyncWebServerRequest *request){
    String phone = "";
    if (request->hasParam("phone", true)) {
      phone = request->getParam("phone", true)->value();
      phone.trim();
    }
    if (phone.isEmpty()) {
      request->send(400, "application/json", "{\"ok\":false,\"msg\":\"Số điện thoại không được để trống\"}");
      return;
    }
    preferences.begin("wifi", false);
    preferences.putString("phone_number", phone);
    preferences.end();
    savedPhoneNumber = phone;
    Serial.printf("[PHONE] Số điện thoại đã thay đổi thành: %s\n", savedPhoneNumber.c_str());
    request->send(200, "application/json", "{\"ok\":true,\"msg\":\"Đã lưu số điện thoại\"}");
  });
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String clientIP = request->client()->remoteIP().toString();
    IPAddress staIP = WiFi.localIP();
    IPAddress apIP = WiFi.softAPIP();
    if (WiFi.status() == WL_CONNECTED && (clientIP.startsWith(staIP.toString().substring(0, staIP.toString().lastIndexOf('.'))))) {
      request->send(SPIFFS, "/dashboard.html", "text/html");
    } else if (WiFi.status() == WL_CONNECTED && request->host() == staIP.toString()) {
      request->send(SPIFFS, "/dashboard.html", "text/html");
    } else {
      request->send(SPIFFS, "/index.html", "text/html");
    }
  });
  server.serveStatic("/static/", SPIFFS, "/").setCacheControl("max-age=86400");
  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request){
    Serial.println("[API] Received WiFi configuration");
  String ssid, pass;
    if (request->hasParam("ssid", true)) {
      ssid = request->getParam("ssid", true)->value();
      ssid.trim();
    }
    if (request->hasParam("pass", true)) {
      pass = request->getParam("pass", true)->value();
    }
    if (ssid.isEmpty()) {
      Serial.println("[API] ✗ Empty SSID provided");
      request->send(400, "application/json", "{\"ok\":false,\"msg\":\"SSID không được để trống\"}");
      return;
    }
    if (ssid.length() > 32) {
      Serial.println("[API] ✗ SSID too long");
      request->send(400, "application/json", "{\"ok\":false,\"msg\":\"SSID quá dài (max 32 ký tự)\"}");
      return;
    }
    preferences.begin("wifi", false);
    preferences.putString("ssid", ssid);
    preferences.putString("pass", pass);
    preferences.putBool("saved", true);
    preferences.end();
    Serial.printf("[API] ✓ Saved WiFi config: SSID='%s', Password length=%d\n", ssid.c_str(), pass.length());
    request->send(200, "application/json", "{\"ok\":true,\"msg\":\"Đã lưu cấu hình. ESP32 sẽ khởi động lại...\"}");
    ESP.restart();
  });
  // ...existing code...

  // API: Quét mạng WiFi
  server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request){
    int scanResult = WiFi.scanComplete();
    if (scanResult == WIFI_SCAN_RUNNING) {
      request->send(200, "application/json", "{\"scanning\":true}");
      return;
    }
    if (scanResult == WIFI_SCAN_FAILED || scanResult == -2) {
      Serial.println("[SCAN] Starting WiFi scan...");
      WiFi.scanNetworks(true, true); // async=true, show_hidden=true
      request->send(200, "application/json", "{\"scanning\":true}");
      return;
    }
    Serial.printf("[SCAN] Found %d networks\n", scanResult);
    JsonDocument doc;
    JsonArray networks = doc["networks"].to<JsonArray>();
    for (int i = 0; i < scanResult; i++) {
      JsonObject network = networks.add<JsonObject>();
      String ssid = WiFi.SSID(i);
      network["ssid"] = ssid.isEmpty() ? "(Hidden Network)" : ssid;
      network["rssi"] = WiFi.RSSI(i);
      network["bssid"] = WiFi.BSSIDstr(i);
      network["channel"] = WiFi.channel(i);
      wifi_auth_mode_t authMode = WiFi.encryptionType(i);
      network["auth"] = (int)authMode;
      network["secure"] = (authMode != WIFI_AUTH_OPEN);
      network["hidden"] = ssid.isEmpty();
      String authName;
      switch(authMode) {
        case WIFI_AUTH_OPEN: authName = "Open"; break;
        case WIFI_AUTH_WEP: authName = "WEP"; break;
        case WIFI_AUTH_WPA_PSK: authName = "WPA"; break;
        case WIFI_AUTH_WPA2_PSK: authName = "WPA2"; break;
        case WIFI_AUTH_WPA_WPA2_PSK: authName = "WPA/WPA2"; break;
        case WIFI_AUTH_WPA2_ENTERPRISE: authName = "WPA2-Enterprise"; break;
        case WIFI_AUTH_WPA3_PSK: authName = "WPA3"; break;
        case WIFI_AUTH_WPA2_WPA3_PSK: authName = "WPA2/WPA3"; break;
        default: authName = "Unknown"; break;
      }
      network["auth_name"] = authName;
    }
    WiFi.scanDelete();
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  server.onNotFound([](AsyncWebServerRequest *request){
    String path = request->url();
    if (path.endsWith("/")) path += "index.html";
    if (SPIFFS.exists(path)) {
      String contentType = "text/plain";
      if (path.endsWith(".html")) contentType = "text/html";
      else if (path.endsWith(".css")) contentType = "text/css";
      else if (path.endsWith(".js")) contentType = "application/javascript";
      else if (path.endsWith(".json")) contentType = "application/json";
      else if (path.endsWith(".ico")) contentType = "image/x-icon";
      request->send(SPIFFS, path, contentType);
    } else {
      if (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED) {
        request->send(SPIFFS, "/dashboard.html", "text/html");
      } else {
        request->send(SPIFFS, "/index.html", "text/html");
      }
    }
  });
  server.begin();
  Serial.println("[WEB] ✓ AsyncWebServer started on port 80");
}
void checkCommandFromBackend() {
  // DEVICE_ID cố định, luôn thực hiện checkCommandFromBackend
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

void sendAlert() {
  // In ra số điện thoại đã lưu khi gửi cảnh báo
  Serial.printf("[ALERT] Gửi cảnh báo tới số điện thoại: %s\n", savedPhoneNumber.c_str());
  // Nếu cần, thêm logic gửi cảnh báo tới backend hoặc thực hiện hành động khác ở đây
}
// Không lấy số điện thoại từ backend mỗi lần gửi alert nữa
// Số điện thoại sẽ được cập nhật khi nhận POST /phone từ giao diện cấu hình

// Không cần gửi cảnh báo, chỉ cần cập nhật biến và in ra khi nhận POST /phone
