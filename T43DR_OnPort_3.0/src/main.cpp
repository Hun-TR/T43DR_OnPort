#include <Arduino.h>
#include <WebServer.h>

#include "settings.h"
#include "log_system.h"
#include "html_utils.h"
#include "auth_system.h"
#include "uart_handler.h"
#include "ntp_handler.h"
#include "web_routes.h"

extern bool ntpConfigured;
extern String getCurrentDateTime();

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("ESP32 başlatılıyor...");

  initLogSystem();
  initPreferences();
  initUART();
  initEthernet();
  initNTPHandler();
  setupWebRoutes();

  server.begin();
  addLog("Web sunucusu başlatıldı.");
}

void loop() {
  server.handleClient();

  // Arka porttan gelen verileri sürekli oku
  processReceivedData();
  
  // ... diğer loop kodları ...
  
  delay(10); // CPU'ya nefes verme

  static unsigned long lastLog = 0;
  if (millis() - lastLog > 600000) {
    addLog("Sistem çalışıyor - uptime: " + String(millis() / 1000) + " sn");
    lastLog = millis();
  }
}
