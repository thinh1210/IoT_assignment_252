#ifndef INDEX_HTML_H
#define INDEX_HTML_H

#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>YOLO UNO Smart Home</title>
  <link rel="stylesheet" href="styles.css">
  <script src="https://cdn.jsdelivr.net/npm/raphael@2.3.0/raphael.min.js"></script>
  <script src="https://cdn.jsdelivr.net/npm/justgage@1.3.5/justgage.min.js"></script>
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.0/css/all.min.css">
</head>
<body>
  <div class="sidebar">
    <div class="logo-container">
      <h2 style="color: #2294F2; margin: 0;">YOLO UNO</h2>
    </div>
    <div class="nav-item active" onclick="showSection('home', event)">🏠 <span>Trang chủ</span></div>
    <div class="nav-item" onclick="showSection('device', event)">⚡ <span>Thiết bị</span></div>
    <div class="nav-item" onclick="showSection('settings', event)">⚙️ <span>Cài đặt</span></div>
  </div>

  <div class="main-content">
    <!-- HOME -->
    <div id="home" class="section">
      <header>
        <h1>Giám sát môi trường</h1>
        <p>Dữ liệu thời gian thực từ cảm biến</p>
      </header>
      <div class="gauges-container">
        <div class="gauge-card"><div id="gauge_temp"></div></div>
        <div class="gauge-card"><div id="gauge_humi"></div></div>
      </div>
    </div>

    <!-- DEVICE -->
    <div id="device" class="section" style="display:none;">
      <header>
        <h1>Điều khiển GPIO</h1>
        <p>Quản lý các chân relay/đèn</p>
      </header>
      <div class="device-list" id="relayContainer"></div>
      <button class="add-relay-btn" onclick="openAddRelayDialog()"><i class="fa fa-plus"></i></button>
    </div>

    <!-- SETTINGS -->
    <div id="settings" class="section" style="display:none;">
      <div class="settings-card">
        <h2>Cấu hình hệ thống</h2>
        <form id="settingsForm">
          <div class="input-group"><i class="fa fa-wifi"></i><input type="text" id="ssid" placeholder="SSID"></div>
          <div class="input-group"><i class="fa fa-lock"></i><input type="password" id="password" placeholder="Pass"></div>
          <div class="input-group"><i class="fa fa-key"></i><input type="text" id="token" placeholder="Token"></div>
          <div class="input-group"><i class="fa fa-server"></i><input type="text" id="server" placeholder="Server"></div>
          <div class="input-group"><i class="fa fa-plug"></i><input type="number" id="port" placeholder="Port"></div>
          <button type="submit" class="btn-save">LƯU CẤU HÌNH</button>
        </form>
      </div>
    </div>
  </div>

  <div id="addRelayDialog" class="dialog-overlay" style="display:none;">
    <div class="dialog">
      <h3>Thêm thiết bị</h3>
      <input type="text" id="relayName" placeholder="Tên (VD: Đèn ngủ)" style="width:100%; margin-bottom:10px;">
      <input type="number" id="relayGPIO" placeholder="GPIO" style="width:100%; margin-bottom:10px;">
      <div class="dialog-buttons">
        <button class="confirm" onclick="saveRelay()">Lưu</button>
        <button class="cancel" onclick="document.getElementById('addRelayDialog').style.display='none'">Hủy</button>
      </div>
    </div>
  </div>

  <div id="confirmDeleteDialog" class="dialog-overlay" style="display:none;">
    <div class="dialog">
      <h3>Xóa thiết bị?</h3>
      <div class="dialog-buttons">
        <button class="danger" onclick="confirmDelete()">Xóa</button>
        <button class="cancel" onclick="document.getElementById('confirmDeleteDialog').style.display='none'">Hủy</button>
      </div>
    </div>
  </div>

  <script src="script.js"></script>
</body>
</html>
)rawliteral";

#endif
