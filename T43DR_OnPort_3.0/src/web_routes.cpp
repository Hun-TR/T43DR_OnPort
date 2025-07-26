#include "web_routes.h"
#include "settings.h"
#include "log_system.h"
#include "html_utils.h"
#include "auth_system.h"
#include "uart_handler.h"
#include "ntp_handler.h"

#include <ETH.h>
#include <Preferences.h>
#include <time.h>

extern WebServer server;

// === Tarih/Saat Formatlama Fonksiyonları ===
inline String getFormattedDateTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "NTP Bağlantısı Yok";
  }
  
  char buffer[64];
  strftime(buffer, sizeof(buffer), "%d.%m.%Y %H:%M:%S", &timeinfo);
  return String(buffer);
}

String getFormattedDate() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Bilinmiyor";
  }
  
  char buffer[32];
  strftime(buffer, sizeof(buffer), "%d.%m.%Y", &timeinfo);
  return String(buffer);
}

String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Bilinmiyor";
  }
  
  char buffer[16];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
  return String(buffer);
}

String getNTPStatus() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "<span style='color: red;'>❌ Bağlantısız</span>";
  }
  return "<span style='color: green;'>✅ Senkronize</span>";
}

String getUptime() {
  unsigned long seconds = millis() / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;
  
  seconds %= 60;
  minutes %= 60;
  hours %= 24;
  
  String uptime = "";
  if (days > 0) uptime += String(days) + " gün ";
  if (hours > 0) uptime += String(hours) + " saat ";
  if (minutes > 0) uptime += String(minutes) + " dakika ";
  uptime += String(seconds) + " saniye";
  
  return uptime;
}

// === Geliştirilmiş Ana Sayfa ===
void handleRoot() {
  if (!checkSession()) {
    server.sendHeader("Location", "/login");
    server.send(302);
    return;
  }

   // Arka porttan gelen verileri oku
  processReceivedData();

  String currentDateTime = getFormattedDateTime();
  String currentDate = getFormattedDate();
  String currentTime = getFormattedTime();
  String uptime = getUptime();
  
  // NTP durumunu kontrol et
  String ntpStatus;
  if (isTimeDataValid()) {
    unsigned long lastUpdate = (millis() - getLastUpdateTime()) / 1000;
    ntpStatus = "<span style='color: green;'>✅ Aktif (Son güncelleme: " + String(lastUpdate) + "s önce)</span>";
  } else {
    ntpStatus = "<span style='color: red;'>❌ Veri Alınamıyor</span>";
  }

  String content = R"rawliteral(
    <div class="datetime-header" style="background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); 
         color: white; padding: 20px; border-radius: 10px; margin-bottom: 20px; text-align: center;">
      <h2 style="margin: 0; font-size: 2em;" id="currentDateTime">📅 )rawliteral" + currentDateTime + R"rawliteral(</h2>
      <p style="margin: 5px 0 0 0; opacity: 0.9;">Arka Port NTP Senkronize Verisi</p>
      <div style="font-size: 0.9em; opacity: 0.8; margin-top: 10px;">
        <span>🔄 UART ile Gerçek Zamanlı Veri</span>
      </div>
    </div>

    <!-- Geliştirici Bilgileri -->
    <div class="developer-info" style="
      margin-top: 40px; 
      padding: 25px; 
      background: linear-gradient(135deg, #f8f9fa 0%, #e9ecef 100%);
      border-radius: 15px; 
      text-align: center;
      border: 1px solid #dee2e6;
      box-shadow: 0 2px 10px rgba(0,0,0,0.05);">
      
      <div style="margin-bottom: 15px;">
        <span style="
          font-size: 2em; 
          margin-bottom: 10px; 
          display: block;
          background: linear-gradient(45deg, #667eea, #764ba2);
          -webkit-background-clip: text;
          -webkit-text-fill-color: transparent;
          background-clip: text;">
          👨‍💻
        </span>
      </div>
      
      <h4 style="
        color: #495057; 
        margin: 0 0 15px 0; 
        font-size: 1.1em;
        font-weight: 600;">
        🛠️ Sistem Geliştiricileri
      </h4>
      
      <div style="
        display: flex; 
        justify-content: center; 
        gap: 30px; 
        flex-wrap: wrap;
        margin-bottom: 15px;">
        
        <div class="developer-card" style="
          background: white;
          padding: 15px 20px;
          border-radius: 10px;
          box-shadow: 0 2px 8px rgba(0,0,0,0.1);
          transition: transform 0.3s ease, box-shadow 0.3s ease;
          min-width: 180px;
          border-left: 4px solid #667eea;">
          <div style="font-size: 1.2em; margin-bottom: 5px;">🚀</div>
          <div style="font-weight: 600; color: #495057; font-size: 0.95em;">Mehmet DEMİRBİLEK</div>
          <div style="color: #6c757d; font-size: 0.8em; margin-top: 3px;">Senior Developer</div>
        </div>
        
        <div class="developer-card" style="
          background: white;
          padding: 15px 20px;
          border-radius: 10px;
          box-shadow: 0 2px 8px rgba(0,0,0,0.1);
          transition: transform 0.3s ease, box-shadow 0.3s ease;
          min-width: 180px;
          border-left: 4px solid #764ba2;">
          <div style="font-size: 1.2em; margin-bottom: 5px;">⚡</div>
          <div style="font-weight: 600; color: #495057; font-size: 0.95em;">Hüseyin ÇİFTCİ</div>
          <div style="color: #6c757d; font-size: 0.8em; margin-top: 3px;">System Architect</div>
        </div>
      </div>
      
      <div style="color: #6c757d; font-size: 0.85em; margin-top: 15px;">
        <span style="margin-right: 15px;">🏢 TEİAŞ EKLİM</span>
        <span style="margin-right: 15px;">📅 2025</span>
        <span>⚙️ ESP32 Platform</span>
      </div>
    </div>

    <div class="status-grid" style="display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; margin-top: 20px;">
      
      <div class="status-item" style="background: #f8f9fa; padding: 20px; border-radius: 10px; border-left: 4px solid #28a745;">
        <h3 style="color: #28a745; margin-top: 0;">🕐 Zaman Bilgileri</h3>
        <p><strong>Tarih:</strong> <span id="currentDate">)rawliteral" + currentDate + R"rawliteral(</span></p>
        <p><strong>Saat:</strong> <span id="currentTime">)rawliteral" + currentTime + R"rawliteral(</span></p>
        <p><strong>Veri Durumu:</strong> <span id="dataStatus">)rawliteral" + ntpStatus + R"rawliteral(</span></p>
        <p><strong>Zaman Dilimi:</strong> UTC+3 (Türkiye)</p>
        <p><strong>Veri Kaynağı:</strong> Arka Port (SEL2488)</p>
      </div>

      <div class="status-item" style="background: #f8f9fa; padding: 20px; border-radius: 10px; border-left: 4px solid #007bff;">
        <h3 style="color: #007bff; margin-top: 0;">💻 Cihaz Bilgileri</h3>
        <p><strong>IP Adresi:</strong> )rawliteral" + local_IP.toString() + R"rawliteral(</p>
        <p><strong>Kart Türü:</strong> ÖN PORT (Web Arayüzü)</p>
        <p><strong>Üretici:</strong> TEİAŞ EKLİM</p>
        <p><strong>Çalışma Süresi:</strong> <span class="uptime-display">)rawliteral" + uptime + R"rawliteral(</span></p>
        <p><strong>UART Baud:</strong> )rawliteral" + String(currentBaudRate) + R"rawliteral( bps</p>
      </div>
      
      <div class="status-item" style="background: #f8f9fa; padding: 20px; border-radius: 10px; border-left: 4px solid #ffc107;">
        <h3 style="color: #ffc107; margin-top: 0;">🌐 Sistem Durumu</h3>
        <p><strong>Ethernet:</strong> <span class="ethernet-status">)rawliteral" + (ETH.linkUp() ? "<span style='color: green;'>✅ Bağlı</span>" : "<span style='color: red;'>❌ Bağlantısız</span>") + R"rawliteral(</span></p>
        <p><strong>NTP Yapılandırma:</strong> <span class="ntp-config-status">)rawliteral" + (ntpConfigured ? "<span style='color: green;'>✅ Yapılandırıldı</span>" : "<span style='color: orange;'>⚠️ Varsayılan</span>") + R"rawliteral(</span></p>
        <p><strong>Arka Port Bağlantı:</strong> <span id="backendStatus">)rawliteral" + (isTimeDataValid() ? "<span style='color: green;'>✅ Aktif</span>" : "<span style='color: red;'>❌ Bağlantısız</span>") + R"rawliteral(</span></p>
        <p><strong>Aktif NTP:</strong> )rawliteral" + String(ntpConfig.ntpServer1) + R"rawliteral(</p>
      </div>
    </div>

    <script>
      // Gerçek zamanlı güncelleme (her 2 saniye)
      function updateStatus() {
        fetch('/api/status')
          .then(response => response.json())
          .then(data => {
            document.getElementById('currentDateTime').innerHTML = '📅 ' + data.datetime;
            document.getElementById('currentDate').textContent = data.date;
            document.getElementById('currentTime').textContent = data.time;
            document.getElementById('dataStatus').innerHTML = data.ntpStatus;
            document.getElementById('backendStatus').innerHTML = data.backendStatus;
          })
          .catch(error => {
            console.error('Durum güncellenemedi:', error);
          });
      }
      
      function updateUptime() {
        const uptime = Date.now() - )rawliteral" + String(millis()) + R"rawliteral(;
        const seconds = Math.floor(uptime / 1000) % 60;
        const minutes = Math.floor(uptime / (1000 * 60)) % 60;
        const hours = Math.floor(uptime / (1000 * 60 * 60)) % 24;
        const days = Math.floor(uptime / (1000 * 60 * 60 * 24));
        
        let uptimeStr = '';
        if (days > 0) uptimeStr += days + ' gün ';
        if (hours > 0) uptimeStr += hours + ' saat ';
        if (minutes > 0) uptimeStr += minutes + ' dakika ';
        uptimeStr += seconds + ' saniye';
        
        const uptimeElements = document.querySelectorAll('.uptime-display');
        uptimeElements.forEach(el => el.textContent = uptimeStr);
      }
      
      // Başlangıç
      document.addEventListener('DOMContentLoaded', function() {
        updateStatus();
        updateUptime();
        
        // Periyodik güncellemeler
        setInterval(updateStatus, 2000);    // Her 2 saniye veri durumu
        setInterval(updateUptime, 1000);    // Her saniye uptime
      });
    </script>
  )rawliteral";

  server.send(200, "text/html; charset=utf-8", generatePage("Ana Sayfa", content));
}

// NTP ayarları sayfası güncelleme
void handleNtpPage() {
  if (!checkSession()) {
    server.sendHeader("Location", "/login");
    server.send(302);
    return;
  }

  String currentDateTime = getCurrentDateTime();
  String ntpStatus = isTimeDataValid() ? 
    "<span style='color: green;'>✅ Veri Alınıyor</span>" : 
    "<span style='color: red;'>❌ Veri Alınamıyor</span>";

  String content = R"rawliteral(
    <div class="ntp-status" style="background: #e3f2fd; padding: 15px; border-radius: 8px; margin-bottom: 20px;">
      <h3 style="margin-top: 0; color: #1976d2;">📡 Mevcut NTP Durumu</h3>
      <p><strong>Arka Porttan Alınan Zaman:</strong> )rawliteral" + currentDateTime + R"rawliteral(</p>
      <p><strong>Veri Durumu:</strong> )rawliteral" + ntpStatus + R"rawliteral(</p>
      <p><strong>Mevcut NTP-1:</strong> )rawliteral" + String(ntpConfig.ntpServer1) + R"rawliteral(</p>
      <p><strong>Mevcut NTP-2:</strong> )rawliteral" + String(ntpConfig.ntpServer2) + R"rawliteral(</p>
    </div>

    <form method="POST" action="/ntp">
      <div class="form-group">
        <label for="ntp1IP">Birincil NTP Sunucusu:</label>
        <input type="text" id="ntp1IP" name="ntp1IP" value=")rawliteral" + String(ntpConfig.ntpServer1) + R"rawliteral(" 
               placeholder="Örn: 192.168.1.180" required>
        <small>SEL2488 NTP sunucu IP adresi</small>
      </div>
      
      <div class="form-group">
        <label for="ntp2IP">Yedek NTP Sunucusu:</label>
        <input type="text" id="ntp2IP" name="ntp2IP" value=")rawliteral" + String(ntpConfig.ntpServer2) + R"rawliteral(" 
               placeholder="Örn: 192.168.2.180" required>
        <small>İkinci NTP sunucu IP adresi</small>
      </div>
      
      <div class="form-group">
        <label for="timezone">Saat Dilimi:</label>
        <select name="timezone" id="timezone">
          <option value="0" )rawliteral" + (ntpConfig.timezone == 0 ? "selected" : "") + R"rawliteral(>UTC+0 (GMT)</option>
          <option value="1" )rawliteral" + (ntpConfig.timezone == 1 ? "selected" : "") + R"rawliteral(>UTC+1</option>
          <option value="2" )rawliteral" + (ntpConfig.timezone == 2 ? "selected" : "") + R"rawliteral(>UTC+2</option>
          <option value="3" )rawliteral" + (ntpConfig.timezone == 3 ? "selected" : "") + R"rawliteral(>UTC+3 (Türkiye)</option>
          <option value="4" )rawliteral" + (ntpConfig.timezone == 4 ? "selected" : "") + R"rawliteral(>UTC+4</option>
        </select>
      </div>
      
      <button type="submit" class="btn" style="background: #28a745;">🔄 NTP Ayarlarını Arka Porta Gönder</button>
    </form>

    <div style="margin-top: 20px; padding: 15px; background: #fff3cd; border-radius: 8px;">
      <h4 style="margin-top: 0; color: #856404;">💡 Bilgi</h4>
      <p>Bu ayarlar arka porttaki NTP kartına UART ile gönderilecek ve SEL2488 sunucusundan zaman senkronizasyonu yapılacaktır.</p>
      <p>Değişiklikler anında uygulanır, sistem yeniden başlatmaya gerek yoktur.</p>
    </div>
  )rawliteral";

  server.send(200, "text/html", generatePage("NTP Ayarları", content));
}

void handleNtpSave() {
  if (!checkSession()) {
    server.sendHeader("Location", "/login");
    server.send(302);
    return;
  }

  String ntp1 = server.arg("ntp1IP");
  String ntp2 = server.arg("ntp2IP");
  int timezone = server.arg("timezone").toInt();
  
  // Basit IP validasyonu
  if (ntp1.length() < 7 || ntp1.length() > 253 || ntp2.length() < 7 || ntp2.length() > 253) {
    String errorContent = "<p style='color: red;'>❌ Geçersiz NTP sunucu adresleri!</p><a href='/ntp' class='btn'>Geri Dön</a>";
    server.send(400, "text/html", generatePage("Hata", errorContent));
    return;
  }

  if (timezone < 0 || timezone > 12) {
    String errorContent = "<p style='color: red;'>❌ Geçersiz saat dilimi!</p><a href='/ntp' class='btn'>Geri Dön</a>";
    server.send(400, "text/html", generatePage("Hata", errorContent));
    return;
  }

  // NTP ayarlarını kaydet ve arka porta gönder
  saveNTPSettings(ntp1, ntp2, timezone);
  
  String successContent = R"rawliteral(
    <div style="text-align: center; padding: 20px;">
      <h3 style="color: green;">✅ NTP Ayarları Başarıyla Güncellendi</h3>
      <p><strong>Birincil NTP:</strong> )rawliteral" + ntp1 + R"rawliteral(</p>
      <p><strong>Yedek NTP:</strong> )rawliteral" + ntp2 + R"rawliteral(</p>
      <p><strong>Saat Dilimi:</strong> UTC+)rawliteral" + String(timezone) + R"rawliteral(</p>
      <p>Ayarlar arka porta UART ile gönderildi.</p>
      <div style="margin-top: 20px;">
        <a href="/" class="btn">Ana Sayfaya Dön</a>
        <a href="/ntp" class="btn" style="background: #6c757d;">NTP Ayarları</a>
      </div>
    </div>
    <script>
      // 5 saniye sonra ana sayfaya yönlendir
      setTimeout(function() {
        window.location.href = '/';
      }, 5000);
    </script>
  )rawliteral";
  
  server.send(200, "text/html", generatePage("NTP Güncellemesi", successContent));
}

void handleStatusAPI() {
  processReceivedData(); // En güncel veriyi al
  
  String ntpStatus;
  String backendStatus;
  
  if (isTimeDataValid()) {
    unsigned long lastUpdate = (millis() - getLastUpdateTime()) / 1000;
    ntpStatus = "<span style='color: green;'>✅ Aktif (Son güncelleme: " + String(lastUpdate) + "s önce)</span>";
    backendStatus = "<span style='color: green;'>✅ Aktif</span>";
  } else {
    ntpStatus = "<span style='color: red;'>❌ Veri Alınamıyor</span>";
    backendStatus = "<span style='color: red;'>❌ Bağlantısız</span>";
  }
  
  String json = "{";
  json += "\"datetime\":\"" + getCurrentDateTime() + "\",";
  json += "\"date\":\"" + getCurrentDate() + "\",";
  json += "\"time\":\"" + getCurrentTime() + "\",";
  json += "\"ntpStatus\":\"" + ntpStatus + "\",";
  json += "\"backendStatus\":\"" + backendStatus + "\"";
  json += "}";
  
  server.send(200, "application/json", json);
}

// === Diğer fonksiyonlar aynı kalıyor ===
void handleBaudratePage() {
  if (!checkSession()) {
    server.sendHeader("Location", "/login");
    server.send(302);
    return;
  }

  String content = R"rawliteral(
    <form method="POST" action="/baudrate">
      <div class="form-group">
        <label for="baud">UART Baud Rate Seç:</label>
        <select name="baud" id="baud">
          <option value="9600">9600</option>
          <option value="19200">19200</option>
          <option value="38400">38400</option>
          <option value="57600">57600</option>
          <option value="115200" selected>115200</option>
        </select>
      </div>
      <button class="btn" type="submit">💾 Kaydet</button>
    </form>
  )rawliteral";

  server.send(200, "text/html", generatePage("BaudRate Ayarı", content));
}

void handleBaudrateSave() {
  if (!checkSession()) {
    server.sendHeader("Location", "/login");
    server.send(302);
    return;
  }

  long baud = server.arg("baud").toInt();
  
  // Baudrate validasyonu
  if (baud != 9600 && baud != 19200 && baud != 38400 && baud != 57600 && baud != 115200) {
    String errorContent = "<p style='color: red;'>❌ Geçersiz baudrate değeri!</p><a href='/baudrate' class='btn'>Geri Dön</a>";
    server.send(400, "text/html", generatePage("Hata", errorContent));
    return;
  }
  
  Preferences preferences;
  if (!preferences.begin("uart-config", false)) {
    addLog("HATA: Preferences açılamadı");
    server.sendHeader("Location", "/baudrate");
    server.send(302);
    return;
  }
  
  preferences.putLong("baudrate", baud);
  preferences.end();
  currentBaudRate = baud;
  addLog("Yeni baudrate ayarlandı: " + String(baud));
  
  server.sendHeader("Location", "/baudrate");
  server.send(302);
}

void handleLogPage() {
  if (!checkSession()) {
    server.sendHeader("Location", "/login");
    server.send(302);
    return;
  }

  String currentTime = getFormattedDateTime();
  String content = R"rawliteral(
    <div style="margin-bottom: 20px; padding: 10px; background: #f8f9fa; border-radius: 5px;">
      <strong>📄 Log Görüntüleme Zamanı:</strong> )rawliteral" + currentTime + R"rawliteral(
    </div>
  )rawliteral" + getLogHTML();
  
  server.send(200, "text/html", generatePage("Log Kayıtları", content));
}

void handleFaultPage() {
  if (!checkSession()) {
    server.sendHeader("Location", "/login");
    server.send(302);
    return;
  }

  String currentTime = getFormattedDateTime();
  String content = R"rawliteral(
    <div style="margin-bottom: 20px; padding: 10px; background: #fff3cd; border-radius: 5px;">
      <strong>🛠️ Arıza Sorgu Zamanı:</strong> )rawliteral" + currentTime + R"rawliteral(
    </div>
    
    <form action="/fault/first" method="POST">
      <button class="btn" type="submit">🛠 İlk Arıza Kaydı</button>
    </form>
    <form action="/fault/next" method="POST" style="margin-top:20px;">
      <button class="btn" type="submit">➡ Sonraki Arıza</button>
    </form>
    <div style="margin-top:30px;">
      <h3>📋 Son Gelen Cevap:</h3>
      <div class="log-entry">)rawliteral" + getLastFaultResponse() + R"rawliteral(</div>
    </div>
  )rawliteral";

  server.send(200, "text/html", generatePage("Arıza Kayıtları", content));
}

void handleFaultFirst() {
  requestFirstFault();
  server.sendHeader("Location", "/fault");
  server.send(302);
}

void handleFaultNext() {
  requestNextFault();
  server.sendHeader("Location", "/fault");
  server.send(302);
}

void handleAccountPage() {
  if (!checkSession()) {
    server.sendHeader("Location", "/login");
    server.send(302);
    return;
  }

  String content = R"rawliteral(
    <form method="POST" action="/account">
      <div class="form-group">
        <label>Yeni Kullanıcı Adı:</label>
        <input type="text" name="username" value=")rawliteral" + validUsername + R"rawliteral(" required>
      </div>
      <div class="form-group">
        <label>Yeni Şifre:</label>
        <input type="password" name="password" required>
      </div>
      <button class="btn" type="submit">💾 Kaydet</button>
    </form>
  )rawliteral";

  server.send(200, "text/html", generatePage("Hesap Ayarları", content));
}

void handleAccountSave() {
  if (!checkSession()) {
    server.sendHeader("Location", "/login");
    server.send(302);
    return;
  }

  String newUsername = server.arg("username");
  String newPassword = server.arg("password");
  
  // Basit validasyon
  if (newUsername.length() < 3 || newPassword.length() < 4) {
    String errorContent = "<p style='color: red;'>❌ Kullanıcı adı en az 3, şifre en az 4 karakter olmalı!</p><a href='/account' class='btn'>Geri Dön</a>";
    server.send(400, "text/html", generatePage("Hata", errorContent));
    return;
  }
  
  validUsername = newUsername;
  validPassword = newPassword;
  addLog("Kullanıcı bilgileri güncellendi: " + newUsername);

  String content = R"rawliteral(
    <div style="text-align: center; padding: 20px;">
      <p class='success-message' style="color: green; font-size: 1.2em;">✅ Hesap bilgileri başarıyla güncellendi!</p>
      <p><strong>Güncelleme Zamanı:</strong> )rawliteral" + getFormattedDateTime() + R"rawliteral(</p>
      <a href='/' class='btn'>Ana Sayfa</a>
    </div>
  )rawliteral";
  
  server.send(200, "text/html", generatePage("Güncelleme Başarılı", content));
}

// === Tüm Rotalar ===
void setupWebRoutes() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/login", HTTP_GET, handleLogin);
  server.on("/login", HTTP_POST, handleLogin);
  server.on("/logout", HTTP_GET, handleLogout);

  server.on("/ntp", HTTP_GET, handleNtpPage);
  server.on("/ntp", HTTP_POST, handleNtpSave);

  server.on("/baudrate", HTTP_GET, handleBaudratePage);
  server.on("/baudrate", HTTP_POST, handleBaudrateSave);

  server.on("/log", HTTP_GET, handleLogPage);

  server.on("/fault", HTTP_GET, handleFaultPage);
  server.on("/fault/first", HTTP_POST, handleFaultFirst);
  server.on("/fault/next", HTTP_POST, handleFaultNext);

  server.on("/account", HTTP_GET, handleAccountPage);
  server.on("/account", HTTP_POST, handleAccountSave);

   // Yeni API endpoint
  server.on("/api/status", HTTP_GET, handleStatusAPI);
}