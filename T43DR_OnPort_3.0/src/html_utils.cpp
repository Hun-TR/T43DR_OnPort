#include "html_utils.h"

String generatePage(const String& title, const String& content, bool showNav) {
  String navigation = "";

  if (showNav) {
    navigation = R"rawliteral(
    <nav class="navbar">
      <div class="nav-container">
        <div class="nav-brand">TEİAŞ EKLİM Sistemi</div>
        <ul class="nav-menu">
          <li><a href="/" class="nav-link">Ana Sayfa</a></li>
          <li><a href="/ntp" class="nav-link">NTP Ayarları</a></li>
          <li><a href="/baudrate" class="nav-link">BaudRate</a></li>
          <li><a href="/log" class="nav-link">Log Kayıtları</a></li>
          <li><a href="/logout" class="nav-link logout">Çıkış</a></li>
        </ul>
      </div>
    </nav>
    )rawliteral";
  }

  return R"rawliteral(
<!DOCTYPE html>
<html lang="tr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>)rawliteral" + title + R"rawliteral(</title>
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
      color: #333;
    }
    
    .navbar {
      background: rgba(255, 255, 255, 0.95);
      backdrop-filter: blur(10px);
      box-shadow: 0 2px 20px rgba(0,0,0,0.1);
      position: sticky;
      top: 0;
      z-index: 1000;
    }
    
    .nav-container {
      max-width: 1200px;
      margin: 0 auto;
      padding: 0 20px;
      display: flex;
      justify-content: space-between;
      align-items: center;
      height: 70px;
    }
    
    .nav-brand {
      font-size: 1.5rem;
      font-weight: bold;
      color: #4a5568;
    }
    
    .nav-menu {
      display: flex;
      list-style: none;
      gap: 30px;
    }
    
    .nav-link {
      text-decoration: none;
      color: #4a5568;
      font-weight: 500;
      padding: 10px 15px;
      border-radius: 8px;
      transition: all 0.3s ease;
    }
    
    .nav-link:hover {
      background: #f7fafc;
      color: #2d3748;
      transform: translateY(-2px);
    }
    
    .nav-link.logout {
      background: #fed7d7;
      color: #c53030;
    }
    
    .nav-link.logout:hover {
      background: #feb2b2;
    }
    
    .container {
      max-width: 1200px;
      margin: 0 auto;
      padding: 40px 20px;
    }
    
    .card {
      background: rgba(255, 255, 255, 0.95);
      backdrop-filter: blur(10px);
      border-radius: 20px;
      padding: 40px;
      box-shadow: 0 20px 40px rgba(0,0,0,0.1);
      margin-bottom: 30px;
    }
    
    .card h1 {
      font-size: 2.5rem;
      margin-bottom: 30px;
      color: #2d3748;
      text-align: center;
    }
    
    .form-group {
      margin-bottom: 25px;
    }
    
    .form-group label {
      display: block;
      margin-bottom: 8px;
      font-weight: 500;
      color: #4a5568;
    }
    
    .form-group input {
      width: 100%;
      padding: 12px 16px;
      border: 2px solid #e2e8f0;
      border-radius: 10px;
      font-size: 16px;
      transition: all 0.3s ease;
    }
    
    .form-group select {
      width: 100%;
      padding: 12px 16px;
      border: 2px solid #e2e8f0;
      border-radius: 10px;
      font-size: 16px;
      background-color: white;
      transition: all 0.3s ease;
      cursor: pointer;
    }
    
    .form-group select:focus {
      outline: none;
      border-color: #667eea;
      box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
    }
    
    .form-group input:focus {
      outline: none;
      border-color: #667eea;
      box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
    }
    
    .btn {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
      border: none;
      padding: 12px 30px;
      border-radius: 10px;
      font-size: 16px;
      font-weight: 500;
      cursor: pointer;
      transition: all 0.3s ease;
      display: inline-block;
      text-decoration: none;
    }
    
    .btn:hover {
      transform: translateY(-2px);
      box-shadow: 0 10px 20px rgba(0,0,0,0.2);
    }
    
    .btn:disabled {
      opacity: 0.6;
      cursor: not-allowed;
      transform: none;
    }
    
    .status-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
      gap: 20px;
      margin-top: 30px;
    }
    
    .status-item {
      background: #f7fafc;
      padding: 20px;
      border-radius: 10px;
      border-left: 4px solid #667eea;
    }
    
    .status-item h3 {
      color: #2d3748;
      margin-bottom: 10px;
    }
    
    .status-item p {
      color: #4a5568;
      font-size: 1.1rem;
    }
    
    .log-entry {
      background: #f7fafc;
      padding: 15px;
      border-radius: 8px;
      margin-bottom: 10px;
      border-left: 3px solid #667eea;
      font-family: 'Courier New', monospace;
    }
    
    .success-message {
      background: #c6f6d5;
      color: #22543d;
      padding: 15px;
      border-radius: 8px;
      margin-bottom: 20px;
      border-left: 4px solid #38a169;
    }
    
    .error-message {
      background: #fed7d7;
      color: #c53030;
      padding: 15px;
      border-radius: 8px;
      margin-bottom: 20px;
      border-left: 4px solid #e53e3e;
    }
    
    @media (max-width: 768px) {
      .nav-container {
        flex-direction: column;
        height: auto;
        padding: 20px;
      }
      
      .nav-menu {
        margin-top: 20px;
        flex-wrap: wrap;
        justify-content: center;
      }
      
      .container {
        padding: 20px 10px;
      }
      
      .card {
        padding: 20px;
      }
      
      .card h1 {
        font-size: 2rem;
      }
    }
  </style>
</head>
<body>
)rawliteral" + navigation + R"rawliteral(
  <div class="container">
    <div class="card">
      <h1>)rawliteral" + title + R"rawliteral(</h1>
      )rawliteral" + content + R"rawliteral(
    </div>
  </div>
</body>
</html>
)rawliteral";
}
