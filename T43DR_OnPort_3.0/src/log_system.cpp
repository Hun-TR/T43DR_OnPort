#include "log_system.h"
#include <time.h>

// Log yapısı
struct LogEntry {
  String timestamp;
  String message;
  LogLevel level;
  String source;
  unsigned long millis_time;
};

// Genişletilmiş log dizisi
LogEntry logs[50]; // 10'dan 50'ye çıkardık
int logIndex = 0;
int totalLogs = 0;

void initLogSystem() {
  for (int i = 0; i < 50; i++) {
    logs[i].timestamp = "";
    logs[i].message = "";
    logs[i].level = INFO;
    logs[i].source = "";
    logs[i].millis_time = 0;
  }
  logIndex = 0;
  totalLogs = 0;
  
  // Sistem başlatma logu
  addLog("🚀 Log sistemi başlatıldı", INFO, "SYSTEM");
}

// Geliştirilmiş tarih/saat formatı
String getFormattedTimestamp() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%d.%m.%Y %H:%M:%S", &timeinfo);
    return String(buffer);
  } else {
    // NTP yoksa millis kullan
    unsigned long seconds = millis() / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    seconds %= 60;
    minutes %= 60;
    hours %= 24;
    
    char buffer[16];
    sprintf(buffer, "%02lu:%02lu:%02lu", hours, minutes, seconds);
    return String(buffer);
  }
}

// Ana log fonksiyonu - overload edilmiş
void addLog(const String& msg, LogLevel level, const String& source) {
  String timestamp = getFormattedTimestamp();
  
  logs[logIndex].timestamp = timestamp;
  logs[logIndex].message = msg;
  logs[logIndex].level = level;
  logs[logIndex].source = source;
  logs[logIndex].millis_time = millis();
  
  logIndex = (logIndex + 1) % 50;
  if (totalLogs < 50) totalLogs++;
  
  // Serial çıktısı
  String levelStr = "";
  switch(level) {
    case ERROR: levelStr = "❌ ERROR"; break;
    case WARN:  levelStr = "⚠️  WARN"; break;
    case INFO:  levelStr = "ℹ️  INFO"; break;
    case DEBUG: levelStr = "🔧 DEBUG"; break;
    case SUCCESS: levelStr = "✅ SUCCESS"; break;
  }
  
  Serial.println("[" + timestamp + "] " + levelStr + " [" + source + "] " + msg);
}

/*// Geriye uyumluluk için eski fonksiyon
void addLog(const String& msg) {
  addLog(msg, INFO, "SYSTEM");
}*/

// Log seviyesi string'e çevirme
String logLevelToString(LogLevel level) {
  switch(level) {
    case ERROR: return "ERROR";
    case WARN: return "WARN";
    case INFO: return "INFO";
    case DEBUG: return "DEBUG";
    case SUCCESS: return "SUCCESS";
    default: return "UNKNOWN";
  }
}

// Log seviyesi CSS class'ına çevirme
String logLevelToClass(LogLevel level) {
  switch(level) {
    case ERROR: return "log-error";
    case WARN: return "log-warning";
    case INFO: return "log-info";
    case DEBUG: return "log-debug";
    case SUCCESS: return "log-success";
    default: return "log-info";
  }
}

// Log seviyesi emojiye çevirme
String logLevelToEmoji(LogLevel level) {
  switch(level) {
    case ERROR: return "❌";
    case WARN: return "⚠️";
    case INFO: return "ℹ️";
    case DEBUG: return "🔧";
    case SUCCESS: return "✅";
    default: return "📝";
  }
}

// Geliştirilmiş HTML log üretimi
String getLogHTML() {
  if (totalLogs == 0) {
    return R"rawliteral(
      <div class="no-logs">
        <div class="no-logs-icon">📝</div>
        <h3>Henüz log kaydı bulunmuyor</h3>
        <p>Sistem henüz yeni başlatıldı veya loglar temizlendi.</p>
      </div>
    )rawliteral";
  }

  String html = R"rawliteral(
    <div class="log-container">
      <div class="log-header">
        <div class="log-stats">
          <span class="stat-item">📊 Toplam: )rawliteral" + String(totalLogs) + R"rawliteral(</span>
          <span class="stat-item">🕐 Son Güncelleme: )rawliteral" + getFormattedTimestamp() + R"rawliteral(</span>
        </div>
        <div class="log-controls">
          <button onclick="refreshLogs()" class="btn-small">🔄 Yenile</button>
          <button onclick="clearLogs()" class="btn-small btn-danger">🗑️ Temizle</button>
          <button onclick="exportLogs()" class="btn-small">📥 İndir</button>
        </div>
      </div>
      
      <div class="log-filters">
        <button onclick="filterLogs('all')" class="filter-btn active" data-filter="all">🌐 Tümü</button>
        <button onclick="filterLogs('error')" class="filter-btn" data-filter="error">❌ Hatalar</button>
        <button onclick="filterLogs('warn')" class="filter-btn" data-filter="warn">⚠️ Uyarılar</button>
        <button onclick="filterLogs('info')" class="filter-btn" data-filter="info">ℹ️ Bilgi</button>
        <button onclick="filterLogs('success')" class="filter-btn" data-filter="success">✅ Başarılı</button>
      </div>
      
      <div class="log-entries" id="logEntries">
  )rawliteral";
  
  // Logları tersten sırala (en yeni üstte)
  for (int i = totalLogs - 1; i >= 0; i--) {
    int idx = (logIndex - 1 - i + 50) % 50;
    if (logs[idx].message.length() > 0) {
      String relativeTime = getRelativeTime(logs[idx].millis_time);
      String levelClass = logLevelToClass(logs[idx].level);
      String levelStr = logLevelToString(logs[idx].level);
      levelStr.toLowerCase();
      
      html += R"rawliteral(
        <div class="log-entry )rawliteral" + levelClass  + R"rawliteral(" data-level=")rawliteral" + levelStr + R"rawliteral(">
          <div class="log-icon">)rawliteral" + logLevelToEmoji(logs[idx].level) + R"rawliteral(</div>
          <div class="log-content">
            <div class="log-header-entry">
              <span class="log-level">)rawliteral" + levelClass + R"rawliteral(</span>
              <span class="log-source">[)rawliteral" + logs[idx].source + R"rawliteral(]</span>
              <span class="log-time" title=")rawliteral" + logs[idx].timestamp + R"rawliteral(">)rawliteral" + relativeTime + R"rawliteral(</span>
            </div>
            <div class="log-message">)rawliteral" + logs[idx].message + R"rawliteral(</div>
          </div>
        </div>
      )rawliteral";
    }
  }
  
  html += R"rawliteral(
      </div>
    </div>
    
    <style>
      .log-container {
        background: white;
        border-radius: 12px;
        overflow: hidden;
        box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
      }
      
      .log-header {
        background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
        color: white;
        padding: 20px;
        display: flex;
        justify-content: space-between;
        align-items: center;
        flex-wrap: wrap;
      }
      
      .log-stats {
        display: flex;
        gap: 20px;
      }
      
      .stat-item {
        font-size: 0.9em;
        opacity: 0.9;
      }
      
      .log-controls {
        display: flex;
        gap: 10px;
      }
      
      .btn-small {
        padding: 8px 15px;
        border: none;
        border-radius: 6px;
        background: rgba(255, 255, 255, 0.2);
        color: white;
        cursor: pointer;
        font-size: 0.8em;
        transition: all 0.3s;
      }
      
      .btn-small:hover {
        background: rgba(255, 255, 255, 0.3);
        transform: translateY(-1px);
      }
      
      .btn-danger {
        background: rgba(220, 53, 69, 0.8) !important;
      }
      
      .btn-danger:hover {
        background: rgba(220, 53, 69, 1) !important;
      }
      
      .log-filters {
        padding: 15px 20px;
        background: #f8f9fa;
        border-bottom: 1px solid #dee2e6;
        display: flex;
        gap: 10px;
        flex-wrap: wrap;
      }
      
      .filter-btn {
        padding: 8px 16px;
        border: 2px solid transparent;
        border-radius: 20px;
        background: white;
        cursor: pointer;
        font-size: 0.85em;
        transition: all 0.3s;
      }
      
      .filter-btn:hover {
        transform: translateY(-1px);
        box-shadow: 0 2px 4px rgba(0,0,0,0.1);
      }
      
      .filter-btn.active {
        background: #667eea;
        color: white;
        border-color: #667eea;
      }
      
      .log-entries {
        max-height: 600px;
        overflow-y: auto;
        padding: 20px;
      }
      
      .log-entry {
        display: flex;
        align-items: flex-start;
        padding: 15px;
        margin-bottom: 10px;
        border-radius: 8px;
        border-left: 4px solid #ccc;
        background: #f8f9fa;
        transition: all 0.3s;
      }
      
      .log-entry:hover {
        transform: translateX(5px);
        box-shadow: 0 2px 8px rgba(0,0,0,0.1);
      }
      
      .log-error {
        border-left-color: #dc3545;
        background: #fff5f5;
      }
      
      .log-warning {
        border-left-color: #ffc107;
        background: #fffbf0;
      }
      
      .log-info {
        border-left-color: #17a2b8;
        background: #f0f8ff;
      }
      
      .log-success {
        border-left-color: #28a745;
        background: #f0fff4;
      }
      
      .log-debug {
        border-left-color: #6c757d;
        background: #f8f9fa;
      }
      
      .log-icon {
        font-size: 1.2em;
        margin-right: 12px;
        margin-top: 2px;
      }
      
      .log-content {
        flex: 1;
      }
      
      .log-header-entry {
        display: flex;
        align-items: center;
        gap: 10px;
        margin-bottom: 5px;
        font-size: 0.85em;
      }
      
      .log-level {
        font-weight: 600;
        padding: 2px 8px;
        border-radius: 12px;
        background: rgba(0,0,0,0.1);
      }
      
      .log-source {
        color: #6c757d;
        font-family: monospace;
      }
      
      .log-time {
        color: #6c757d;
        margin-left: auto;
      }
      
      .log-message {
        font-size: 0.95em;
        line-height: 1.4;
        word-break: break-word;
      }
      
      .no-logs {
        text-align: center;
        padding: 60px 20px;
        color: #6c757d;
      }
      
      .no-logs-icon {
        font-size: 4em;
        margin-bottom: 20px;
      }
      
      .no-logs h3 {
        margin-bottom: 10px;
        color: #495057;
      }
      
      /* Responsive */
      @media (max-width: 768px) {
        .log-header {
          flex-direction: column;
          gap: 15px;
          text-align: center;
        }
        
        .log-stats {
          flex-direction: column;
          gap: 10px;
        }
        
        .log-filters {
          justify-content: center;
        }
        
        .log-header-entry {
          flex-direction: column;
          align-items: flex-start;
          gap: 5px;
        }
        
        .log-time {
          margin-left: 0;
        }
      }
    </style>
    
    <script>
      // Log filtreleme
      function filterLogs(level) {
        const entries = document.querySelectorAll('.log-entry');
        const buttons = document.querySelectorAll('.filter-btn');
        
        // Buton durumlarını güncelle
        buttons.forEach(btn => {
          btn.classList.remove('active');
          if (btn.dataset.filter === level) {
            btn.classList.add('active');
          }
        });
        
        // Log girişlerini filtrele
        entries.forEach(entry => {
          if (level === 'all' || entry.dataset.level === level) {
            entry.style.display = 'flex';
          } else {
            entry.style.display = 'none';
          }
        });
      }
      
      // Sayfayı yenile
      function refreshLogs() {
        location.reload();
      }
      
      // Logları temizle
      function clearLogs() {
        if (confirm('Tüm log kayıtları silinecek. Emin misiniz?')) {
          fetch('/log/clear', { method: 'POST' })
            .then(() => location.reload())
            .catch(() => alert('Log temizleme başarısız!'));
        }
      }
      
      // Logları indir
      function exportLogs() {
        const entries = document.querySelectorAll('.log-entry:not([style*="display: none"])');
        let logText = 'TEİAŞ EKLİM - Log Kayıtları\\n';
        logText += '='.repeat(50) + '\\n';
        logText += 'Dışa Aktarma Zamanı: ' + new Date().toLocaleString('tr-TR') + '\\n\\n';
        
        entries.forEach(entry => {
          const level = entry.querySelector('.log-level').textContent;
          const source = entry.querySelector('.log-source').textContent;
          const time = entry.querySelector('.log-time').getAttribute('title');
          const message = entry.querySelector('.log-message').textContent;
          
          logText += `[${time}] ${level} ${source} ${message}\\n`;
        });
        
        const blob = new Blob([logText], { type: 'text/plain;charset=utf-8' });
        const url = window.URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = 'teias-eklim-logs-' + new Date().toISOString().split('T')[0] + '.txt';
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        window.URL.revokeObjectURL(url);
      }
      
      // Otomatik yenileme (60 saniye)
      setInterval(() => {
        refreshLogs();
      }, 60000);
    </script>
  )rawliteral";
  
  return html;
}

// Göreceli zaman hesaplama
String getRelativeTime(unsigned long logTime) {
  unsigned long diff = millis() - logTime;
  unsigned long seconds = diff / 1000;
  
  if (seconds < 60) {
    return String(seconds) + " saniye önce";
  } else if (seconds < 3600) {
    return String(seconds / 60) + " dakika önce";
  } else if (seconds < 86400) {
    return String(seconds / 3600) + " saat önce";
  } else {
    return String(seconds / 86400) + " gün önce";
  }
}

// Log temizleme fonksiyonu
void clearLogs() {
  for (int i = 0; i < 50; i++) {
    logs[i].timestamp = "";
    logs[i].message = "";
    logs[i].level = INFO;
    logs[i].source = "";
    logs[i].millis_time = 0;
  }
  logIndex = 0;
  totalLogs = 0;
  
  addLog("🧹 Log kayıtları temizlendi", INFO, "SYSTEM");
}

// Log istatistikleri
struct LogStats {
  int errorCount = 0;
  int warnCount = 0;
  int infoCount = 0;
  int debugCount = 0;
  int successCount = 0;
};

LogStats getLogStats() {
  LogStats stats;
  for (int i = 0; i < totalLogs; i++) {
    int idx = (logIndex - 1 - i + 50) % 50;
    switch(logs[idx].level) {
      case ERROR: stats.errorCount++; break;
      case WARN: stats.warnCount++; break;
      case INFO: stats.infoCount++; break;
      case DEBUG: stats.debugCount++; break;
      case SUCCESS: stats.successCount++; break;
    }
  }
  return stats;
}