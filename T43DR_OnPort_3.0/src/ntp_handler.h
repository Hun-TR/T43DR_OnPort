#pragma once

#ifndef NTP_HANDLER_H
#define NTP_HANDLER_H

#include <Arduino.h>

// Arka porttan gelen veri formatları
struct ReceivedTimeData {
  String date;      // DDMMYY formatında
  String time;      // HHMMSS formatında
  char dateChecksum;
  char timeChecksum;
  bool isValid;
  unsigned long lastUpdate;
};

// NTP Konfigürasyon yapısı (arka porta gönderilecek)
struct NTPConfig {
  char ntpServer1[64];
  char ntpServer2[64];
  int timezone;
  bool enabled;
  unsigned long lastSent;
};

// Global değişkenler
extern ReceivedTimeData receivedTime;
extern NTPConfig ntpConfig;
extern bool ntpConfigured;

// Fonksiyon bildirimleri
void initNTPHandler();
void saveNTPSettings(const String& server1, const String& server2, int timezone = 3);
bool loadNTPSettings();
void sendNTPConfigToBackend();
void processReceivedData();
bool validateChecksum(const String& data, char checksum, bool isTime = false);

// Zaman fonksiyonları
String getCurrentDateTime();
String getCurrentDate();  
String getCurrentTime();
String getFormattedDateTime();
bool isTimeDataValid();
unsigned long getLastUpdateTime();

// UART okuma fonksiyonları
void readBackendData();
void parseTimeData(const String& data);

#endif