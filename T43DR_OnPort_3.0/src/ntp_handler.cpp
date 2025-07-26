#include "ntp_handler.h"
#include "uart_handler.h"
#include "log_system.h"
#include <Preferences.h>
#include <HardwareSerial.h>
#include <time.h>

// Global değişkenler
ReceivedTimeData receivedTime;
NTPConfig ntpConfig;
bool ntpConfigured = false;

// UART (GPIO2: TX, GPIO4: RX - Arka port ile haberleşme)
HardwareSerial backendSerial(2);

void initNTPHandler() {
  // UART başlatma (arka port ile haberleşme için)
  backendSerial.begin(115200, SERIAL_8N1, 4, 2); // RX:GPIO4, TX:GPIO2
  
  // Değişkenleri initialize et
  receivedTime.isValid = false;
  receivedTime.lastUpdate = 0;
  
  // Kayıtlı NTP ayarlarını yükle
  if (loadNTPSettings()) {
    ntpConfigured = true;
    addLog("NTP ayarları yüklendi: " + String(ntpConfig.ntpServer1) + ", " + String(ntpConfig.ntpServer2));
  } else {
    // Varsayılan ayarlar
    strcpy(ntpConfig.ntpServer1, "192.168.1.180");
    strcpy(ntpConfig.ntpServer2, "192.168.2.180");
    ntpConfig.timezone = 3;
    ntpConfig.enabled = true;
    ntpConfig.lastSent = 0;
    
    saveNTPSettings(ntpConfig.ntpServer1, ntpConfig.ntpServer2, ntpConfig.timezone);
    ntpConfigured = false;
  }
  
  addLog("NTP Handler başlatıldı (Frontend)");
  
  // Başlangıçta mevcut ayarları arka porta gönder
  sendNTPConfigToBackend();
}

bool loadNTPSettings() {
  Preferences preferences;
  preferences.begin("ntp-config", true);
  
  String server1 = preferences.getString("ntp_server1", "");
  String server2 = preferences.getString("ntp_server2", "");
  
  if (server1.length() == 0) {
    preferences.end();
    return false;
  }
  
  server1.toCharArray(ntpConfig.ntpServer1, sizeof(ntpConfig.ntpServer1));
  server2.toCharArray(ntpConfig.ntpServer2, sizeof(ntpConfig.ntpServer2));
  ntpConfig.timezone = preferences.getInt("timezone", 3);
  ntpConfig.enabled = preferences.getBool("enabled", true);
  
  preferences.end();
  return true;
}

void saveNTPSettings(const String& server1, const String& server2, int timezone) {
  Preferences preferences;
  preferences.begin("ntp-config", false);
  
  // Eski ayarları kontrol et
  String oldServer1 = preferences.getString("ntp_server1", "");
  String oldServer2 = preferences.getString("ntp_server2", "");
  int oldTimezone = preferences.getInt("timezone", 3);
  
  // Yeni ayarları kaydet
  preferences.putString("ntp_server1", server1);
  preferences.putString("ntp_server2", server2);
  preferences.putInt("timezone", timezone);
  preferences.putBool("enabled", true);
  preferences.end();
  
  // Global yapıyı güncelle
  server1.toCharArray(ntpConfig.ntpServer1, sizeof(ntpConfig.ntpServer1));
  server2.toCharArray(ntpConfig.ntpServer2, sizeof(ntpConfig.ntpServer2));
  ntpConfig.timezone = timezone;
  ntpConfig.enabled = true;
  
  addLog("NTP ayarları kaydedildi: " + server1 + ", " + server2 + " (UTC+" + String(timezone) + ")");
  
  // Ayarlarda değişiklik varsa arka porta gönder
  if (oldServer1 != server1 || oldServer2 != server2 || oldTimezone != timezone) {
    addLog("NTP ayarları değişti, arka porta gönderiliyor...");
    sendNTPConfigToBackend();
    ntpConfigured = true;
  }
}

void sendNTPConfigToBackend() {
  // Arka port kodundaki format: "NTP_UPDATE;server1;server2"
  String message = "NTP_UPDATE;" + String(ntpConfig.ntpServer1) + ";" + String(ntpConfig.ntpServer2);
  
  backendSerial.println(message);
  ntpConfig.lastSent = millis();
  
  addLog("Arka porta NTP ayarları gönderildi: " + message);
  
  // ACK bekleme (5 saniye timeout)
  unsigned long startTime = millis();
  while (millis() - startTime < 5000) {
    if (backendSerial.available()) {
      String response = backendSerial.readStringUntil('\n');
      response.trim();
      
      if (response == "ACK") {
        addLog("Arka porttan ACK alındı - NTP ayarları uygulandı");
        return;
      }
    }
    delay(10);
  }
  
  addLog("UYARI: Arka porttan ACK alınamadı");
}

void readBackendData() {
  static String dataBuffer = "";
  static unsigned long lastDataTime = 0;
  
  while (backendSerial.available()) {
    char c = backendSerial.read();
    
    if (c == '\n' || c == '\r') {
      if (dataBuffer.length() > 0) {
        parseTimeData(dataBuffer);
        dataBuffer = "";
        lastDataTime = millis();
      }
    } else {
      dataBuffer += c;
      
      // Buffer overflow koruması
      if (dataBuffer.length() > 50) {
        dataBuffer = "";
        addLog("UYARI: UART buffer overflow");
      }
    }
  }
  
  // Veri timeout kontrolü (30 saniye)
  if (receivedTime.isValid && (millis() - receivedTime.lastUpdate > 30000)) {
    receivedTime.isValid = false;
    addLog("UYARI: Arka porttan veri timeout");
  }
}

String formatDate(const String& dateStr) {
  if (dateStr.length() != 6) return "Geçersiz";
  
  String day = dateStr.substring(0, 2);
  String month = dateStr.substring(2, 4);
  String year = "20" + dateStr.substring(4, 6);
  
  return day + "." + month + "." + year;
}

String formatTime(const String& timeStr) {
  if (timeStr.length() != 6) return "Geçersiz";
  
  String hour = timeStr.substring(0, 2);
  String minute = timeStr.substring(2, 4);
  String second = timeStr.substring(4, 6);
  
  return hour + ":" + minute + ":" + second;
}

void parseTimeData(const String& data) {
  // Arka port kodundaki format kontrol et
  if (data.length() == 7) {
    String dataOnly = data.substring(0, 6);
    char checksum = data.charAt(6);
    
    // Tarih verisi (checksum büyük harf)
    if (checksum >= 'A' && checksum <= 'Z') {
      if (validateChecksum(dataOnly, checksum, false)) {
        receivedTime.date = dataOnly;
        receivedTime.dateChecksum = checksum;
        receivedTime.lastUpdate = millis();
        
        addLog("Tarih alındı: " + formatDate(dataOnly));
      } else {
        addLog("HATA: Tarih checksum hatası - " + data);
      }
    }
    // Saat verisi (checksum küçük harf)
    else if (checksum >= 'a' && checksum <= 'z') {
      if (validateChecksum(dataOnly, checksum, true)) {
        receivedTime.time = dataOnly;
        receivedTime.timeChecksum = checksum;
        receivedTime.lastUpdate = millis();
        receivedTime.isValid = true;
        
        addLog("Saat alındı: " + formatTime(dataOnly));
      } else {
        addLog("HATA: Saat checksum hatası - " + data);
      }
    }
  } else {
    addLog("UYARI: Geçersiz veri formatı - " + data);
  }
}

bool validateChecksum(const String& data, char checksum, bool isTime) {
  if (data.length() != 6) return false;
  
  uint8_t sum = 0;
  for (int i = 0; i < 6; i++) {
    if (data[i] >= '0' && data[i] <= '9') {
      sum += (data[i] - '0');
    }
  }
  
  uint8_t calculatedChecksum = sum % 10;
  char expectedChecksum;
  
  if (isTime) {
    expectedChecksum = 'a' + calculatedChecksum;
  } else {
    expectedChecksum = 'A' + calculatedChecksum;
  }
  
  return (checksum == expectedChecksum);
}



String getCurrentDateTime() {
  if (!receivedTime.isValid || receivedTime.date.length() != 6 || receivedTime.time.length() != 6) {
    return "Veri Bekleniyor";
  }
  
  return formatDate(receivedTime.date) + " " + formatTime(receivedTime.time);
}

String getCurrentDate() {
  if (!receivedTime.isValid || receivedTime.date.length() != 6) {
    return "Bilinmiyor";
  }
  return formatDate(receivedTime.date);
}

String getCurrentTime() {
  if (!receivedTime.isValid || receivedTime.time.length() != 6) {
    return "Bilinmiyor";
  }
  return formatTime(receivedTime.time);
}

String getFormattedDateTime() {
  return getCurrentDateTime();
}

bool isTimeDataValid() {
  return receivedTime.isValid && 
         (millis() - receivedTime.lastUpdate < 30000) &&
         receivedTime.date.length() == 6 && 
         receivedTime.time.length() == 6;
}

unsigned long getLastUpdateTime() {
  return receivedTime.lastUpdate;
}

void processReceivedData() {
  readBackendData();
}