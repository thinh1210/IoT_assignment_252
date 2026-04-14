#ifndef SCRIPT_JS_H
#define SCRIPT_JS_H

#include <Arduino.h>

const char script_js[] PROGMEM = R"rawliteral(
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
var gaugeTemp;
var gaugeHumi;
var relayList = [];
var deleteTarget = null;

window.addEventListener('load', onLoad);

function onLoad() {
  initGauges();
  initWebSocket();
  updateRelaySummary();
}

function initWebSocket() {
  setWsState('Đang kết nối...', 'Chờ portal đồng bộ dữ liệu', false);
  websocket = new WebSocket(gateway);

  websocket.onopen = function() {
    console.log('WS Open');
    setWsState('Đã kết nối', 'Portal AP đang stream dữ liệu nội bộ', true);
  };

  websocket.onclose = function() {
    console.log('WS Closed');
    setWsState('Mất kết nối', 'Thử reconnect sau 2 giây', false);
    setTimeout(initWebSocket, 2000);
  };

  websocket.onmessage = onMessage;
}

function setWsState(text, hint, connected) {
  var wsText = document.getElementById('wsStateText');
  var wsHint = document.getElementById('wsStateHint');

  if (wsText) {
    wsText.textContent = text;
    wsText.classList.toggle('is-live', !!connected);
  }
  if (wsHint) {
    wsHint.textContent = hint;
  }
}

function sendData(data) {
  if (websocket && websocket.readyState === WebSocket.OPEN) {
    websocket.send(data);
    return true;
  }
  showSettingsMessage('WebSocket chưa sẵn sàng. Hãy thử lại sau.', true);
  return false;
}

function onMessage(event) {
  try {
    var data = JSON.parse(event.data);

    if (data.temp !== undefined) {
      var temp = Number(data.temp);
      gaugeTemp.refresh(temp);
      document.getElementById('tempReadout').textContent = temp.toFixed(1) + ' °C';
      document.getElementById('tempStateText').textContent =
        temp >= 35 ? 'Ngưỡng critical' : (temp >= 30 ? 'Hơi nóng' : 'Ổn định');
      updateLastTelemetryTime();
    }

    if (data.humi !== undefined) {
      var humi = Number(data.humi);
      gaugeHumi.refresh(humi);
      document.getElementById('humiReadout').textContent = humi.toFixed(0) + ' %';
      document.getElementById('humiStateText').textContent =
        humi <= 40 ? 'Quá khô' : (humi >= 80 ? 'Quá ẩm' : 'Trong vùng theo dõi');
      updateLastTelemetryTime();
    }

    if (data.care_action !== undefined) {
      document.getElementById('careActionLabel').textContent = data.care_action;
    }

    if (data.care_confidence !== undefined) {
      var confidence = Number(data.care_confidence);
      document.getElementById('careConfidenceText').textContent =
        'Độ tin cậy: ' + (confidence * 100).toFixed(1) + '%';
    }

    if (data.relays !== undefined) {
      relayList = data.relays;
      renderRelays();
      updateRelaySummary();
    }
  } catch (e) {
    console.warn('WS JSON error', e);
  }
}

function updateLastTelemetryTime() {
  var now = new Date();
  document.getElementById('lastTelemetryText').textContent =
    now.toLocaleTimeString('vi-VN');
}

function showSection(id, event) {
  document.querySelectorAll('.section').forEach(function(sec) {
    sec.style.display = 'none';
  });
  document.getElementById(id).style.display = 'block';
  document.querySelectorAll('.nav-item').forEach(function(item) {
    item.classList.remove('active');
  });
  event.currentTarget.classList.add('active');
}

function initGauges() {
  gaugeTemp = new JustGage({
    id: 'gauge_temp',
    value: 0,
    min: 0,
    max: 50,
    title: 'Nhiệt độ',
    label: '°C',
    levelColors: ['#2f7c92', '#3db37d', '#efb43c', '#d8513c'],
    symbol: ''
  });

  gaugeHumi = new JustGage({
    id: 'gauge_humi',
    value: 0,
    min: 0,
    max: 100,
    title: 'Độ ẩm',
    label: '%',
    levelColors: ['#d88a43', '#48a3b8', '#2c7ac9'],
    symbol: ''
  });
}

function openAddRelayDialog() {
  document.getElementById('relayName').value = '';
  document.getElementById('relayGPIO').value = '';
  document.getElementById('addRelayDialog').style.display = 'flex';
}

function closeAddRelayDialog() {
  document.getElementById('addRelayDialog').style.display = 'none';
}

function saveRelay() {
  var name = document.getElementById('relayName').value.trim();
  var gpio = document.getElementById('relayGPIO').value.trim();

  if (!name || !gpio) {
    showSettingsMessage('Điền đầy đủ tên relay và GPIO trước khi lưu.', true);
    return;
  }

  sendData(JSON.stringify({
    action: 'add_relay',
    name: name,
    gpio: parseInt(gpio, 10)
  }));
  closeAddRelayDialog();
}

function renderRelays() {
  var container = document.getElementById('relayContainer');
  var empty = document.getElementById('relayEmpty');

  container.innerHTML = '';
  empty.style.display = relayList.length ? 'none' : 'grid';

  relayList.forEach(function(relay) {
    var card = document.createElement('article');
    card.className = 'device-card';
    card.innerHTML = `
      <div class="device-card-top">
        <div class="device-icon-wrap"><i class="fa-solid fa-bolt device-icon"></i></div>
        <button class="icon-btn danger" onclick="showDeleteDialog(${relay.gpio})" aria-label="Xóa relay">
          <i class="fa-solid fa-trash"></i>
        </button>
      </div>
      <h3>${relay.name}</h3>
      <p>GPIO ${relay.gpio}</p>
      <div class="relay-state ${relay.state ? 'is-on' : 'is-off'}">
        ${relay.state ? 'Đang bật' : 'Đang tắt'}
      </div>
      <button class="toggle-btn ${relay.state ? 'on' : ''}" onclick="toggleRelay(${relay.gpio})">
        ${relay.state ? 'TẮT THIẾT BỊ' : 'BẬT THIẾT BỊ'}
      </button>
    `;
    container.appendChild(card);
  });
}

function updateRelaySummary() {
  var count = relayList.length;
  document.getElementById('relayCount').textContent = count;
  document.getElementById('relayConfiguredCount').textContent =
    count + (count === 1 ? ' relay' : ' relay');
}

function toggleRelay(gpio) {
  sendData(JSON.stringify({ action: 'toggle_relay', gpio: gpio }));
}

function showDeleteDialog(gpio) {
  deleteTarget = gpio;
  document.getElementById('confirmDeleteDialog').style.display = 'flex';
}

function closeDeleteDialog() {
  document.getElementById('confirmDeleteDialog').style.display = 'none';
}

function confirmDelete() {
  if (deleteTarget === null) {
    return;
  }
  sendData(JSON.stringify({ action: 'delete_relay', gpio: deleteTarget }));
  closeDeleteDialog();
}

function showSettingsMessage(message, isError) {
  var box = document.getElementById('settingsMessage');
  box.textContent = message;
  box.style.display = 'block';
  box.classList.toggle('is-error', !!isError);
  box.classList.toggle('is-success', !isError);
}

document.getElementById('settingsForm').addEventListener('submit', function(e) {
  e.preventDefault();

  var button = document.getElementById('saveSettingsButton');
  var payload = {
    action: 'save_settings',
    ssid: document.getElementById('ssid').value.trim(),
    pass: document.getElementById('password').value,
    token: document.getElementById('token').value.trim(),
    server: document.getElementById('server').value.trim(),
    port: parseInt(document.getElementById('port').value, 10) || 1883
  };

  if (!payload.ssid || !payload.server || !payload.token) {
    showSettingsMessage('SSID, server và token không được để trống.', true);
    return;
  }

  button.disabled = true;
  button.textContent = 'ĐANG GỬI CẤU HÌNH...';

  if (sendData(JSON.stringify(payload))) {
    showSettingsMessage(
      'Cấu hình đã được gửi. Board sẽ chuyển về NORMAL mode sau khi xử lý.',
      false
    );
  }

  setTimeout(function() {
    button.disabled = false;
    button.textContent = 'LƯU CẤU HÌNH VÀ TRỞ VỀ NORMAL';
  }, 1800);
});
)rawliteral";

#endif
