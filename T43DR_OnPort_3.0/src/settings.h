#pragma once
#include <Arduino.h>
#include <WebServer.h>

extern WebServer server;

extern const IPAddress local_IP;
extern const IPAddress gateway;
extern const IPAddress subnet;
extern const IPAddress primaryDNS;

extern String validUsername;
extern String validPassword;
extern bool isLoggedIn;
extern unsigned long sessionStartTime;
extern const unsigned long SESSION_TIMEOUT;

extern bool ntpConfigured;
extern long currentBaudRate;

void initPreferences();
void initEthernet();
