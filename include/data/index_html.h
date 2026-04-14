#ifndef INDEX_HTML_H
#define INDEX_HTML_H

#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>YOLO UNO Plant Care Portal</title>
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link href="https://fonts.googleapis.com/css2?family=Space+Grotesk:wght@500;700&family=Manrope:wght@400;500;700&display=swap" rel="stylesheet">
  <link rel="stylesheet" href="styles.css">
  <script src="https://cdn.jsdelivr.net/npm/raphael@2.3.0/raphael.min.js"></script>
  <script src="https://cdn.jsdelivr.net/npm/justgage@1.3.5/justgage.min.js"></script>
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.0/css/all.min.css">
</head>
<body>
  <div class="app-shell">
    <header class="topbar">
      <div class="brand-block">
        <p class="eyebrow">YOLO UNO / ACCESS POINT PORTAL</p>
        <h1>Plant Care Control Surface</h1>
        <p class="subtitle">
          Cổng cấu hình và điều khiển cục bộ cho cảm biến môi trường, relay, CoreIoT và manual control qua IR remote.
        </p>
      </div>

      <div class="header-panels">
        <div class="header-panel">
          <span class="panel-label">Portal Mode</span>
          <strong>ACCESS POINT</strong>
          <span class="panel-meta">Cấu hình thiết bị và kiểm tra relay tại chỗ</span>
        </div>
        <div class="header-panel">
          <span class="panel-label">WebSocket</span>
          <strong id="wsStateText">Đang kết nối...</strong>
          <span class="panel-meta" id="wsStateHint">Chờ portal đồng bộ dữ liệu</span>
        </div>
        <div class="header-panel">
          <span class="panel-label">Manual Remote</span>
          <strong>IR on GPIO6</strong>
          <span class="panel-meta">Remote cycle: NORMAL → AP → MANUAL</span>
        </div>
      </div>
    </header>

    <nav class="tabbar">
      <button class="nav-item active" onclick="showSection('home', event)">Tổng quan</button>
      <button class="nav-item" onclick="showSection('device', event)">Thiết bị</button>
      <button class="nav-item" onclick="showSection('settings', event)">Cấu hình</button>
    </nav>

    <main class="main-content">
      <section id="home" class="section">
        <div class="hero-grid">
          <div class="hero-card hero-primary">
            <div class="hero-copy">
              <span class="hero-chip">Live Telemetry</span>
              <h2>Giám sát nhiệt độ, độ ẩm và khuyến nghị TinyML</h2>
              <p>
                Portal AP này phản ánh dữ liệu cảm biến gần nhất, trạng thái relay và khuyến nghị chăm sóc cây đang chạy trên board.
              </p>
            </div>
            <div class="quick-metrics">
              <div class="metric-card">
                <span class="metric-label">Nhiệt độ hiện tại</span>
                <strong id="tempReadout">--.- °C</strong>
                <small id="tempStateText">Chưa có dữ liệu</small>
              </div>
              <div class="metric-card">
                <span class="metric-label">Độ ẩm hiện tại</span>
                <strong id="humiReadout">-- %</strong>
                <small id="humiStateText">Chưa có dữ liệu</small>
              </div>
              <div class="metric-card">
                <span class="metric-label">Cập nhật gần nhất</span>
                <strong id="lastTelemetryText">--:--:--</strong>
                <small>Dữ liệu qua WebSocket nội bộ</small>
              </div>
              <div class="metric-card">
                <span class="metric-label">Relay đã cấu hình</span>
                <strong id="relayCount">0</strong>
                <small>Fan, pump hoặc relay tùy biến</small>
              </div>
            </div>
          </div>

          <div class="hero-card hero-secondary">
            <span class="hero-chip accent">System Notes</span>
            <h3>Luồng mode hiện tại</h3>
            <div class="mode-flow">
              <div class="mode-step">NORMAL<div>WiFi client + CoreIoT + telemetry</div></div>
              <div class="mode-step">ACCESS POINT<div>Portal cấu hình + relay dashboard</div></div>
              <div class="mode-step">MANUAL<div>IR remote điều khiển fan, pump, LED</div></div>
            </div>
            <p class="mode-note">
              Trong MANUAL mode, board vẫn giữ kết nối giống NORMAL mode nhưng actuator/indicator được ưu tiên theo lệnh remote.
            </p>
          </div>
        </div>

        <div class="gauge-layout">
          <div class="gauge-card">
            <div class="card-head">
              <span class="section-kicker">Environment</span>
              <h3>Nhiệt độ</h3>
            </div>
            <div id="gauge_temp"></div>
          </div>
          <div class="gauge-card">
            <div class="card-head">
              <span class="section-kicker">Environment</span>
              <h3>Độ ẩm</h3>
            </div>
            <div id="gauge_humi"></div>
          </div>
        </div>

        <div class="insight-grid">
          <article class="care-card">
            <div class="card-head">
              <span class="section-kicker">TinyML Insight</span>
              <h3>Khuyến nghị chăm sóc</h3>
            </div>
            <div class="care-banner">
              <div class="care-chip">Plant Care Model</div>
              <div class="care-confidence" id="careConfidenceText">Độ tin cậy: --</div>
            </div>
            <h2 id="careActionLabel">Đang chờ dữ liệu cảm biến</h2>
            <p class="care-caption">
              Mô hình hiện phân loại 3 nhóm hành động: không cần thao tác, bật quạt hoặc cần tưới nước.
            </p>
          </article>

          <article class="guide-card">
            <div class="card-head">
              <span class="section-kicker">Feature Guide</span>
              <h3>Portal hiện hỗ trợ gì?</h3>
            </div>
            <ul class="feature-list">
              <li>Xem nhiệt độ và độ ẩm gần nhất khi board đang ở AP mode.</li>
              <li>Thêm, bật/tắt và xóa relay trực tiếp từ dashboard này.</li>
              <li>Lưu cấu hình Wi‑Fi và CoreIoT để board quay về NORMAL mode.</li>
              <li>Dùng IR remote để vào MANUAL mode và điều khiển fan, pump, Green LED, Neo LED.</li>
            </ul>
          </article>
        </div>
      </section>

      <section id="device" class="section" style="display:none;">
        <div class="section-header">
          <div>
            <span class="section-kicker">Relay Workspace</span>
            <h2>Thiết bị và relay cục bộ</h2>
            <p>Relay lưu trong NVS sẽ được khôi phục khi board reboot hoặc đổi mode.</p>
          </div>
          <div class="device-summary">
            <div class="summary-pill">
              <span class="summary-label">Configured</span>
              <strong id="relayConfiguredCount">0 relay</strong>
            </div>
            <div class="summary-pill">
              <span class="summary-label">Manual IR</span>
              <strong>Fan / Pump / Green / Neo</strong>
            </div>
          </div>
        </div>

        <div id="relayEmpty" class="empty-state">
          <i class="fa-solid fa-plug-circle-plus"></i>
          <h3>Chưa có relay nào</h3>
          <p>Thêm relay đầu tiên để bắt đầu điều khiển quạt, bơm hoặc thiết bị GPIO tùy chọn.</p>
        </div>

        <div class="device-list" id="relayContainer"></div>
        <button class="add-relay-btn" onclick="openAddRelayDialog()" aria-label="Thêm relay">
          <i class="fa fa-plus"></i>
        </button>
      </section>

      <section id="settings" class="section" style="display:none;">
        <div class="settings-layout">
          <article class="settings-card">
            <div class="card-head">
              <span class="section-kicker">Network Setup</span>
              <h2>Cấu hình Wi‑Fi và CoreIoT</h2>
            </div>
            <p class="settings-intro">
              Sau khi lưu, board sẽ dùng cấu hình này để quay về NORMAL mode, kết nối Wi‑Fi client và gửi telemetry lên CoreIoT.
            </p>

            <form id="settingsForm">
              <label class="field">
                <span>Wi‑Fi SSID</span>
                <div class="input-shell">
                  <i class="fa fa-wifi"></i>
                  <input type="text" id="ssid" placeholder="Ví dụ: P425">
                </div>
              </label>

              <label class="field">
                <span>Wi‑Fi Password</span>
                <div class="input-shell">
                  <i class="fa fa-lock"></i>
                  <input type="password" id="password" placeholder="Mật khẩu Wi‑Fi">
                </div>
              </label>

              <label class="field">
                <span>Device Token</span>
                <div class="input-shell">
                  <i class="fa fa-key"></i>
                  <input type="text" id="token" placeholder="Access token / mqtt_user">
                </div>
              </label>

              <div class="field-grid">
                <label class="field">
                  <span>Server</span>
                  <div class="input-shell">
                    <i class="fa fa-server"></i>
                    <input type="text" id="server" placeholder="app.coreiot.io">
                  </div>
                </label>

                <label class="field">
                  <span>Port</span>
                  <div class="input-shell">
                    <i class="fa fa-plug"></i>
                    <input type="number" id="port" placeholder="1883">
                  </div>
                </label>
              </div>

              <button type="submit" class="btn-save" id="saveSettingsButton">
                LƯU CẤU HÌNH VÀ TRỞ VỀ NORMAL
              </button>
            </form>

            <div id="settingsMessage" class="message-banner" style="display:none;"></div>
          </article>

          <aside class="guide-card settings-side">
            <div class="card-head">
              <span class="section-kicker">Deployment Hints</span>
              <h3>Checklist nhanh</h3>
            </div>
            <ul class="feature-list">
              <li>Lưu SSID và password đúng để tránh `AUTH_FAIL` khi thoát AP mode.</li>
              <li>Điền token đúng device nếu muốn CoreIoT nhận telemetry và OTA.</li>
              <li>Fan/Pump relay chỉ có hiệu lực nếu GPIO relay đã được cấu hình trong firmware.</li>
              <li>IR remote learn log sẽ hiện trên serial cho tới khi bạn map code thật vào `config.h`.</li>
            </ul>
          </aside>
        </div>
      </section>
    </main>
  </div>

  <div id="addRelayDialog" class="dialog-overlay" style="display:none;">
    <div class="dialog">
      <h3>Thêm relay mới</h3>
      <p>Đặt tên để dễ phân biệt và nhập đúng GPIO đầu ra.</p>
      <input type="text" id="relayName" placeholder="Ví dụ: Đèn bàn">
      <input type="number" id="relayGPIO" placeholder="GPIO">
      <div class="dialog-buttons">
        <button class="confirm" onclick="saveRelay()">Lưu relay</button>
        <button class="cancel" onclick="closeAddRelayDialog()">Hủy</button>
      </div>
    </div>
  </div>

  <div id="confirmDeleteDialog" class="dialog-overlay" style="display:none;">
    <div class="dialog">
      <h3>Xóa relay này?</h3>
      <p>Relay sẽ bị xóa khỏi danh sách NVS của portal.</p>
      <div class="dialog-buttons">
        <button class="danger" onclick="confirmDelete()">Xóa</button>
        <button class="cancel" onclick="closeDeleteDialog()">Hủy</button>
      </div>
    </div>
  </div>

  <script src="script.js"></script>
</body>
</html>
)rawliteral";

#endif
