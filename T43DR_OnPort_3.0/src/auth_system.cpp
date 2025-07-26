#include "auth_system.h"
#include "settings.h"
#include "html_utils.h"
#include "log_system.h"
#include <WebServer.h>

extern WebServer server;

bool checkSession() {
  if (!isLoggedIn) return false;
  if (millis() - sessionStartTime > SESSION_TIMEOUT) {
    isLoggedIn = false;
    addLog("Oturum zaman aşımı");
    return false;
  }
  return true;
}

String generateLoginPage(bool hasError = false, String errorMessage = "") {
  String errorDiv = "";
  if (hasError) {
    errorDiv = R"rawliteral(
      <div class="error-message" style="
        background: #fee; 
        color: #c33; 
        padding: 15px; 
        border-radius: 8px; 
        margin-bottom: 20px; 
        border-left: 4px solid #c33;
        animation: shake 0.5s;">
        <i style="font-size: 1.2em;">⚠️</i> )rawliteral" + errorMessage + R"rawliteral(
      </div>
    )rawliteral";
  }

  return R"rawliteral(
<!DOCTYPE html>
<html lang="tr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>TEİAŞ EKLİM - Giriş</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }
        
        .login-container {
            background: rgba(255, 255, 255, 0.95);
            backdrop-filter: blur(10px);
            border-radius: 20px;
            padding: 40px;
            box-shadow: 0 20px 40px rgba(0, 0, 0, 0.1);
            width: 100%;
            max-width: 400px;
            text-align: center;
            border: 1px solid rgba(255, 255, 255, 0.2);
        }
        
        .logo {
            font-size: 3em;
            margin-bottom: 10px;
            background: linear-gradient(45deg, #667eea, #764ba2);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            background-clip: text;
        }
        
        .company-name {
            font-size: 1.8em;
            color: #333;
            margin-bottom: 5px;
            font-weight: 600;
        }
        
        .subtitle {
            color: #666;
            margin-bottom: 30px;
            font-size: 0.9em;
        }
        
        .form-group {
            margin-bottom: 20px;
            text-align: left;
        }
        
        .form-group label {
            display: block;
            margin-bottom: 8px;
            color: #555;
            font-weight: 500;
            font-size: 0.9em;
        }
        
        .input-wrapper {
            position: relative;
        }
        
        .input-wrapper i {
            position: absolute;
            left: 15px;
            top: 50%;
            transform: translateY(-50%);
            color: #999;
            font-size: 1.1em;
        }
        
        input[type="text"], input[type="password"] {
            width: 100%;
            padding: 15px 15px 15px 45px;
            border: 2px solid #e1e1e1;
            border-radius: 10px;
            font-size: 1em;
            transition: all 0.3s ease;
            background: #fafafa;
        }
        
        input[type="text"]:focus, input[type="password"]:focus {
            outline: none;
            border-color: #667eea;
            background: white;
            box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
        }
        
        .login-btn {
            width: 100%;
            padding: 15px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            border-radius: 10px;
            font-size: 1.1em;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
            margin-top: 10px;
            position: relative;
            overflow: hidden;
        }
        
        .login-btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 10px 20px rgba(102, 126, 234, 0.3);
        }
        
        .login-btn:active {
            transform: translateY(0);
        }
        
        .login-btn:disabled {
            opacity: 0.7;
            cursor: not-allowed;
            transform: none;
        }
        
        .system-info {
            margin-top: 30px;
            padding-top: 20px;
            border-top: 1px solid #eee;
            font-size: 0.8em;
            color: #999;
        }
        
        .system-info div {
            margin: 5px 0;
        }
        
        .loading {
            display: none;
            margin-top: 15px;
        }
        
        .loading-spinner {
            border: 3px solid #f3f3f3;
            border-top: 3px solid #667eea;
            border-radius: 50%;
            width: 30px;
            height: 30px;
            animation: spin 1s linear infinite;
            margin: 0 auto;
        }
        
        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }
        
        @keyframes shake {
            0%, 20%, 40%, 60%, 80% { transform: translateX(0); }
            10%, 30%, 50%, 70%, 90% { transform: translateX(-5px); }
        }
        
        .error-message {
            animation: slideDown 0.3s ease-out;
        }
        
        @keyframes slideDown {
            from { opacity: 0; transform: translateY(-10px); }
            to { opacity: 1; transform: translateY(0); }
        }
        
        /* Responsive tasarım */
        @media (max-width: 480px) {
            .login-container {
                padding: 30px 20px;
                margin: 10px;
            }
            
            .company-name {
                font-size: 1.5em;
            }
            
            .logo {
                font-size: 2.5em;
            }
        }
        
        /* Koyu tema desteği */
        @media (prefers-color-scheme: dark) {
            .login-container {
                background: rgba(30, 30, 30, 0.95);
                color: #e1e1e1;
            }
            
            .company-name {
                color: #e1e1e1;
            }
            
            .subtitle {
                color: #b1b1b1;
            }
            
            .form-group label {
                color: #c1c1c1;
            }
            
            input[type="text"], input[type="password"] {
                background: #2a2a2a;
                border-color: #444;
                color: #e1e1e1;
            }
            
            input[type="text"]:focus, input[type="password"]:focus {
                background: #333;
                border-color: #667eea;
            }
        }
    </style>
</head>
<body>
    <div class="login-container">
        <div class="logo">🔐</div>
        <h1 class="company-name">TEİAŞ EKLİM</h1>
        <p class="subtitle">Sistem Yönetim Paneli</p>
        
        )rawliteral" + errorDiv + R"rawliteral(
        
        <form method="POST" id="loginForm">
            <div class="form-group">
                <label for="username">👤 Kullanıcı Adı</label>
                <div class="input-wrapper">
                    <i>👤</i>
                    <input type="text" id="username" name="username" placeholder="Kullanıcı adınızı giriniz" required autocomplete="username">
                </div>
            </div>
            
            <div class="form-group">
                <label for="password">🔒 Şifre</label>
                <div class="input-wrapper">
                    <i>🔒</i>
                    <input type="password" id="password" name="password" placeholder="Şifrenizi giriniz" required autocomplete="current-password">
                </div>
            </div>
            
            <button type="submit" class="login-btn" id="loginBtn">
                <span id="btnText">🚀 Giriş Yap</span>
            </button>
            
            <div class="loading" id="loading">
                <div class="loading-spinner"></div>
                <p>Giriş yapılıyor...</p>
            </div>
        </form>
        
        <div class="system-info">
            <div><strong>📡 Sistem:</strong> ESP32 Web Yönetimi</div>
            <div><strong>🌐 IP:</strong> <span id="deviceIP">Yükleniyor...</span></div>
            <div><strong>⏰ Zaman:</strong> <span id="currentTime">Yükleniyor...</span></div>
        </div>
    </div>

    <script>
        // Form gönderimi animasyonu
        document.getElementById('loginForm').addEventListener('submit', function(e) {
            const btn = document.getElementById('loginBtn');
            const loading = document.getElementById('loading');
            const btnText = document.getElementById('btnText');
            
            btn.disabled = true;
            btnText.textContent = '⏳ Kontrol ediliyor...';
            loading.style.display = 'block';
            
            // Gerçek gönderimden 1 saniye sonra
            setTimeout(() => {
                this.submit();
            }, 500);
        });
        
        // Enter tuşu ile giriş
        document.addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                document.getElementById('loginForm').submit();
            }
        });
        
        // Saati güncelle
        function updateTime() {
            const now = new Date();
            const timeStr = now.toLocaleString('tr-TR', {
                day: '2-digit',
                month: '2-digit', 
                year: 'numeric',
                hour: '2-digit',
                minute: '2-digit',
                second: '2-digit'
            });
            document.getElementById('currentTime').textContent = timeStr;
        }
        
        // IP adresini al (basit bir yaklaşım)
        function updateIP() {
            document.getElementById('deviceIP').textContent = window.location.hostname;
        }
        
        // Sayfa yüklendiğinde
        window.onload = function() {
            updateTime();
            updateIP();
            setInterval(updateTime, 1000); // Her saniye güncelle
            
            // Kullanıcı adı alanına odaklan
            document.getElementById('username').focus();
        };
        
        // Otomatik form temizleme (hata durumunda)
        if (window.location.search.includes('error')) {
            setTimeout(() => {
                document.getElementById('username').value = '';
                document.getElementById('password').value = '';
                document.getElementById('username').focus();
            }, 2000);
        }
    </script>
</body>
</html>
)rawliteral";
}

void handleLogin() {
  if (server.method() == HTTP_POST) {
    String u = server.arg("username");
    String p = server.arg("password");
    
    // Basit rate limiting (son 10 saniyede 5'ten fazla deneme)
    static unsigned long lastAttempt = 0;
    static int attemptCount = 0;
    
    if (millis() - lastAttempt < 10000) {
      attemptCount++;
      if (attemptCount > 5) {
        addLog("Çok fazla giriş denemesi: " + server.client().remoteIP().toString());
        server.send(429, "text/html", generateLoginPage(true, "Çok fazla deneme! 10 saniye bekleyiniz."));
        return;
      }
    } else {
      attemptCount = 0;
    }
    lastAttempt = millis();
    
    // Input validasyonu
    if (u.length() < 1 || p.length() < 1) {
      addLog("Boş giriş denemesi");
      server.send(400, "text/html", generateLoginPage(true, "Kullanıcı adı ve şifre boş olamaz!"));
      return;
    }
    
    // Giriş kontrolü
    if (u == validUsername && p == validPassword) {
      isLoggedIn = true;
      sessionStartTime = millis();
      attemptCount = 0; // Başarılı girişte sayacı sıfırla
      
      addLog("✅ Başarılı giriş: " + u + " - IP: " + server.client().remoteIP().toString());
      
      // Cookie ayarla (opsiyonel)
      server.sendHeader("Set-Cookie", "session=active; Path=/; HttpOnly");
      server.sendHeader("Location", "/");
      server.send(302, "text/plain", "Yönlendiriliyor...");
    } else {
      addLog("❌ Başarısız giriş denemesi - Kullanıcı: " + u + " - IP: " + server.client().remoteIP().toString());
      
      // Güvenlik için hangi alanın yanlış olduğunu belirtmiyoruz
      server.send(401, "text/html", generateLoginPage(true, "Kullanıcı adı veya şifre hatalı!"));
    }
  } else {
    // GET isteği - login sayfasını göster
    server.send(200, "text/html; charset=utf-8", generateLoginPage());
  }
}

void handleLogout() {
  // Session'ı temizle
  isLoggedIn = false;
  sessionStartTime = 0;
  
  String logoutIP = server.client().remoteIP().toString();
  addLog("🚪 Çıkış yapıldı - IP: " + logoutIP);
  
  // Cookie'yi temizle
  server.sendHeader("Set-Cookie", "session=; Path=/; HttpOnly; Expires=Thu, 01 Jan 1970 00:00:00 GMT");
  server.sendHeader("Location", "/login");
  server.send(302, "text/html", R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
        <meta charset="UTF-8">
        <title>Çıkış Yapılıyor...</title>
        <style>
            body { 
                font-family: Arial, sans-serif; 
                text-align: center; 
                padding: 50px;
                background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
                color: white;
            }
            .logout-message {
                background: rgba(255,255,255,0.1);
                padding: 30px;
                border-radius: 15px;
                display: inline-block;
                backdrop-filter: blur(10px);
            }
        </style>
        <script>
            setTimeout(function() {
                window.location.href = '/login';
            }, 2000);
        </script>
    </head>
    <body>
        <div class="logout-message">
            <h2>👋 Başarıyla çıkış yaptınız!</h2>
            <p>2 saniye içinde giriş sayfasına yönlendirileceksiniz...</p>
            <div style="margin-top: 20px;">
                <div style="display: inline-block; width: 20px; height: 20px; border: 2px solid rgba(255,255,255,0.3); border-top: 2px solid white; border-radius: 50%; animation: spin 1s linear infinite;"></div>
            </div>
        </div>
        <style>
            @keyframes spin {
                0% { transform: rotate(0deg); }
                100% { transform: rotate(360deg); }
            }
        </style>
    </body>
    </html>
  )rawliteral");
}