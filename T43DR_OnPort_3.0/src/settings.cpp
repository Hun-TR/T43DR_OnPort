#include "settings.h"
#include <ETH.h>
#include <Preferences.h>
#include "log_system.h"

WebServer server(80);

const IPAddress local_IP(192, 168, 1, 160);
const IPAddress gateway(192, 168, 1, 1);
const IPAddress subnet(255, 255, 255, 0);
const IPAddress primaryDNS(8, 8, 8, 8);

String validUsername = "admin";
String validPassword = "1234";
bool isLoggedIn = false;
unsigned long sessionStartTime = 0;
const unsigned long SESSION_TIMEOUT = 30 * 60 * 1000;

long currentBaudRate = 115200;

void initPreferences() {
  Preferences preferences;
  preferences.begin("uart-config", false);
  currentBaudRate = preferences.getLong("baudrate", 115200);
  preferences.end();
}

void initEthernet() {
  ETH.begin(1, 16, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO17_OUT);
  if (!ETH.config(local_IP, gateway, subnet, primaryDNS)) {
    addLog("Statik IP atanamadı!");
  } else {
    addLog("Statik IP atandı: " + local_IP.toString());
  }
}
