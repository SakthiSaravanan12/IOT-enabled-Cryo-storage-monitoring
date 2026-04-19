#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebServer.h> // This is now used!
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_MAX31865.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "LittleFS.h" 
#include "time.h"
#include <esp_task_wdt.h> 

/* ===================== CONFIGURATION ===================== */
#define TFT_CS    5
#define TFT_DC    2
#define TFT_RST   4
#define MAX_CS    15
#define LN2_PIN   14
#define BUZZER_PIN 25
#define WDT_TIMEOUT 15 

#define PAGE_DELAY 2000 

const char* WIFI_SSID = "CRYO"; 
const char* WIFI_PASS = "12345678";

#define BOTtoken "8732909483:AAGf2gee5pFBf_GeoSondfU37Xu42B-vZ74" 
#define CHAT_ID  "5968826149" 

/* ===================== OBJECTS & VARS ===================== */
WebServer server(80); // <--- ADDED FOR IOT BROWSER ACCESS
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOTtoken, secured_client);
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
Adafruit_MAX31865 rtd = Adafruit_MAX31865(MAX_CS);

float temperatureC = -196.0, lastTempC = -196.0, stability = 100.0;
int uiPage = 0;
uint32_t lastSensorMs = 0, lastPageMs = 0, heartbeatMs = 0, lastTelegramMs = 0;
bool heartState = false;

/* ===================== IOT WEB DASHBOARD ===================== */

void handleRoot() {
  bool ln2Low = (digitalRead(LN2_PIN) == LOW);
  String statusText = (temperatureC > -150.0 || ln2Low) ? "CRITICAL ALERT" : "SYSTEM SECURE";
  String color = (temperatureC > -150.0 || ln2Low) ? "#ff4d4d" : "#2ecc71";

  String html = "<html><head><meta http-equiv='refresh' content='5'>"; // Auto-refresh 5s
  html += "<style>body{font-family:sans-serif; background:#121212; color:white; text-align:center;}";
  html += ".card{background:#1e1e1e; padding:20px; border-radius:15px; display:inline-block; margin:10px; border:2px solid " + color + ";}";
  html += "h1{color:" + color + ";}</style></head><body>";
  html += "<h1>CRYO GUARDIAN IOT DASHBOARD</h1>";
  html += "<div class='card'><h2>Temperature</h2><p style='font-size:30px;'>" + String(temperatureC, 1) + " C</p></div>";
  html += "<div class='card'><h2>LN2 Level</h2><p style='font-size:30px;'>" + String(ln2Low ? "LOW" : "NORMAL") + "</p></div>";
  html += "<div class='card'><h2>System Status</h2><p style='font-size:30px; color:" + color + ";'>" + statusText + "</p></div>";
  html += "<p>Uptime: " + String(millis()/60000) + " minutes</p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

/* ===================== UI HELPERS ===================== */

void drawHeader(const char* title) {
  tft.fillScreen(ST77XX_BLACK);
  tft.fillRect(0, 0, 320, 35, 0x0010); 
  tft.setCursor(10, 10); tft.setTextColor(ST77XX_CYAN); tft.setTextSize(2);
  tft.print(title);
}

void drawFooter() {
  bool ln2Low = (digitalRead(LN2_PIN) == LOW);
  bool tempCrit = (temperatureC > -150.0);
  tft.drawFastHLine(0, 210, 320, 0x4444);
  tft.setTextSize(2);
  tft.setCursor(10, 220); tft.setTextColor(tempCrit ? ST77XX_RED : ST77XX_CYAN);
  tft.print("T:"); tft.print(temperatureC, 0); tft.print("C");
  tft.setCursor(160, 220); tft.setTextColor(ln2Low ? ST77XX_RED : ST77XX_GREEN);
  tft.print("LN2:"); tft.print(ln2Low ? "LOW!" : "OK");
}

void renderPage() {
  bool ln2Low = (digitalRead(LN2_PIN) == LOW);
  bool tempCrit = (temperatureC > -150.0);

  if (uiPage == 0) { 
    tft.fillScreen(ST77XX_BLACK);
    tft.drawRoundRect(10, 10, 300, 220, 15, ST77XX_CYAN);
    tft.setCursor(45, 80); tft.setTextSize(2); tft.setTextColor(ST77XX_WHITE);
    tft.print("WELCOME TO");
    tft.setCursor(35, 120); tft.setTextSize(3); tft.setTextColor(ST77XX_CYAN);
    tft.print("CRYO GUARDIAN");
  } 
  else if (uiPage == 1) { 
    drawHeader("LIVE MONITOR");
    tft.setCursor(20, 55); tft.setTextSize(2); tft.setTextColor(ST77XX_WHITE); tft.print("TEMP:");
    tft.setCursor(20, 85); tft.setTextSize(5); tft.setTextColor(ST77XX_CYAN);
    tft.print(temperatureC, 1); tft.print("C");
    tft.setCursor(20, 135); tft.setTextSize(2); tft.setTextColor(ST77XX_WHITE); tft.print("LN2 STATUS:");
    tft.setCursor(160, 135); tft.setTextSize(2); tft.setTextColor(ln2Low ? ST77XX_RED : ST77XX_GREEN);
    tft.print(ln2Low ? "LOW!!!" : "NORMAL");
    tft.setCursor(20, 175); tft.setTextSize(2); tft.setTextColor(0xAAAA); 
    tft.print("STABILITY: "); tft.print(stability, 0); tft.print("%");
  }
  else if (uiPage == 2) { 
    drawHeader("NETWORK STATUS");
    tft.setTextSize(2); tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(20, 60); tft.print("NET: ");
    if(WiFi.status() == WL_CONNECTED) { tft.setTextColor(ST77XX_GREEN); tft.print("ONLINE"); }
    else { tft.setTextColor(ST77XX_RED); tft.print("OFFLINE"); }
    tft.setCursor(20, 100); tft.setTextColor(ST77XX_WHITE); tft.print("IP: "); tft.setTextColor(ST77XX_CYAN); tft.print(WiFi.localIP().toString());
    tft.setCursor(20, 140); tft.setTextColor(ST77XX_WHITE); tft.print("UPTIME: "); tft.setTextColor(ST77XX_YELLOW); tft.print(millis()/60000); tft.print(" MIN");
    drawFooter(); 
  }
  else if (uiPage == 3) { 
    drawHeader("ALARM CONFIG");
    tft.setTextSize(2);
    tft.setCursor(20, 60); tft.setTextColor(ST77XX_WHITE); tft.print("SAFE MIN : -200.0 C");
    tft.setCursor(20, 100); tft.setTextColor(ST77XX_WHITE); tft.print("SAFE MAX : -150.0 C");
    tft.setCursor(20, 140); tft.setTextColor(ST77XX_WHITE); tft.print("STATUS   : ");
    if (tempCrit && ln2Low) { tft.setTextColor(ST77XX_RED); tft.print("CRITICAL!!"); }
    else if (tempCrit || ln2Low) { tft.setTextColor(0xFD20); tft.print("WARNING!"); }
    else { tft.setTextColor(ST77XX_GREEN); tft.print("SECURE"); }
    drawFooter(); 
  }
  else if (uiPage == 4) { 
    drawHeader("HISTORY LOGS");
    tft.setCursor(10, 50); tft.setTextSize(2); tft.setTextColor(ST77XX_YELLOW);
    if(LittleFS.exists("/logs.txt")){
      File f = LittleFS.open("/logs.txt", "r");
      int lines = 0;
      while(f.available() && lines < 7) { tft.println(f.readStringUntil('\n')); lines++; }
      f.close();
    } else { tft.print("NO LOGS FOUND"); }
  }
}

/* ===================== SYSTEM CORE ===================== */

void setup() {
  Serial.begin(115200);
  pinMode(LN2_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); 

  if(!LittleFS.begin(true)) { Serial.println("LittleFS Failed"); }
  
  esp_task_wdt_init(WDT_TIMEOUT, true); 
  esp_task_wdt_add(NULL);

  SPI.begin(18, 19, 23);
  tft.init(240, 320); tft.setRotation(1);
  rtd.begin(MAX31865_2WIRE);
  
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(50, 110); tft.setTextSize(2); tft.setTextColor(ST77XX_WHITE); tft.print("WIFI CONNECTING...");

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(500); esp_task_wdt_reset(); }
  
  // START THE WEB SERVER ROUTES
  server.on("/", handleRoot);
  server.begin();

  secured_client.setInsecure();
  configTime(19800, 0, "pool.ntp.org", "time.google.com");
  
  uint32_t startSync = millis();
  while (time(nullptr) < 24 * 3600 && millis() - startSync < 5000) { delay(500); esp_task_wdt_reset(); }
  
  temperatureC = rtd.temperature(100.0, 430.0);
  renderPage(); 
}

void loop() {
  esp_task_wdt_reset();
  server.handleClient(); // <--- CRITICAL: Listens for web browser requests

  uint32_t nowMs = millis();

  if (nowMs - heartbeatMs > 500) {
    heartbeatMs = nowMs;
    heartState = !heartState;
    tft.fillCircle(310, 15, 3, heartState ? ST77XX_RED : ST77XX_BLACK);
  }

  if (nowMs - lastSensorMs >= 2000) {
    lastSensorMs = nowMs;
    lastTempC = temperatureC;
    temperatureC = rtd.temperature(100.0, 430.0);
    stability = constrain(100.0 - (abs(temperatureC - lastTempC) * 150.0), 0, 100);
  }

  bool ln2Low = (digitalRead(LN2_PIN) == LOW); 
  bool tempCrit = (temperatureC > -150.0);

  if (tempCrit && ln2Low) digitalWrite(BUZZER_PIN, HIGH); 
  else if (tempCrit || ln2Low) digitalWrite(BUZZER_PIN, (nowMs % 400 < 200) ? HIGH : LOW); 
  else digitalWrite(BUZZER_PIN, LOW);

  if ((tempCrit || ln2Low) && (nowMs - lastTelegramMs > 60000)) {
      esp_task_wdt_reset();
      String msg = "❄️ CRYO ALARM!\nTemp: " + String(temperatureC, 1) + "C\nLN2: " + (ln2Low ? "LOW" : "NORMAL");
      bot.sendMessage(CHAT_ID, msg, ""); 
      lastTelegramMs = nowMs;
  }

  if (nowMs - lastPageMs >= PAGE_DELAY) {
    lastPageMs = nowMs;
    uiPage = (uiPage + 1) % 5;
    renderPage();
  }
}
