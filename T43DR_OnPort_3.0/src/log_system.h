#pragma once
#ifndef LOG_SYSTEM_H
#define LOG_SYSTEM_H

#include <Arduino.h>

// Log seviyeleri
enum LogLevel {
  ERROR = 0,
  WARN = 1,
  INFO = 2,
  DEBUG = 3,
  SUCCESS = 4
};

// Log istatistikleri yapısı
struct LogStats;
// Log entry yapısı
struct LogEntry;

// Ana fonksiyonlar
void initLogSystem();
void addLog(const String& msg, LogLevel level = INFO, const String& source = "SYSTEM");
//void addLog(const String& msg); // Geriye uyumluluk
String getLogHTML();
void clearLogs();

// Yardımcı fonksiyonlar
String getFormattedTimestamp();
String getRelativeTime(unsigned long logTime);
String logLevelToString(LogLevel level);
String logLevelToClass(LogLevel level);
String logLevelToEmoji(LogLevel level);
LogStats getLogStats();

// Makrolar - kolay kullanım için
#define LOG_ERROR(msg, source) addLog(msg, ERROR, source)
#define LOG_WARN(msg, source) addLog(msg, WARN, source)
#define LOG_INFO(msg, source) addLog(msg, INFO, source)
#define LOG_DEBUG(msg, source) addLog(msg, DEBUG, source)
#define LOG_SUCCESS(msg, source) addLog(msg, SUCCESS, source)

// Global değişkenler
extern LogEntry logs[50];
extern int logIndex;
extern int totalLogs;

#endif // LOG_SYSTEM_H
