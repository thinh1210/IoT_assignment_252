#ifndef SCRIPT_JS_H
#define SCRIPT_JS_H

#include <Arduino.h>

const char script_js[] PROGMEM = R"rawliteral(
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
var gaugeTemp, gaugeHumi;
var relayList = [];
var deleteTarget = null;

window.addEventListener('load', onLoad);

function onLoad(event) {
    initGauges();
    initWebSocket();
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = function(e) { console.log('WS Open'); };
    websocket.onclose = function(e) { console.log('WS Closed'); setTimeout(initWebSocket, 2000); };
    websocket.onmessage = onMessage;
}

function Send_Data(data) {
    if (websocket && websocket.readyState === WebSocket.OPEN) {
        websocket.send(data);
    }
}

function onMessage(event) {
    try {
        var data = JSON.parse(event.data);
        if (data.temp !== undefined) gaugeTemp.refresh(data.temp);
        if (data.humi !== undefined) gaugeHumi.refresh(data.humi);
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
        }
    } catch (e) {
        console.warn("WS JSON error", e);
    }
}

function showSection(id, event) {
    document.querySelectorAll('.section').forEach(sec => sec.style.display = 'none');
    document.getElementById(id).style.display = (id === 'settings' ? 'flex' : 'block');
    document.querySelectorAll('.nav-item').forEach(i => i.classList.remove('active'));
    event.currentTarget.classList.add('active');
}

function initGauges() {
    gaugeTemp = new JustGage({
        id: "gauge_temp", value: 0, min: 0, max: 50, title: "Nhiệt độ",
        levelColors: ["#00BCD4", "#4CAF50", "#FFC107", "#F44336"]
    });
    gaugeHumi = new JustGage({
        id: "gauge_humi", value: 0, min: 0, max: 100, title: "Độ ẩm",
        levelColors: ["#42A5F5", "#00BCD4", "#0288D1"]
    });
}

function openAddRelayDialog() { document.getElementById('addRelayDialog').style.display = 'flex'; }
function closeAddRelayDialog() { document.getElementById('addRelayDialog').style.display = 'none'; }

function saveRelay() {
    const name = document.getElementById('relayName').value.trim();
    const gpio = document.getElementById('relayGPIO').value.trim();
    if (!name || !gpio) return;
    Send_Data(JSON.stringify({ action: "add_relay", name: name, gpio: parseInt(gpio) }));
    closeAddRelayDialog();
}

function renderRelays() {
    const container = document.getElementById('relayContainer');
    container.innerHTML = "";
    relayList.forEach(r => {
        const card = document.createElement('div');
        card.className = 'device-card';
        card.innerHTML = `
          <i class="fa-solid fa-bolt device-icon"></i>
          <h3>${r.name}</h3>
          <p>GPIO: ${r.gpio}</p>
          <button class="toggle-btn ${r.state ? 'on' : ''}" onclick="toggleRelay(${r.gpio})">
            ${r.state ? 'ON' : 'OFF'}
          </button>
          <i class="fa-solid fa-trash delete-icon" onclick="showDeleteDialog(${r.gpio})"></i>
        `;
        container.appendChild(card);
    });
}

function toggleRelay(gpio) {
    Send_Data(JSON.stringify({ action: "toggle_relay", gpio: gpio }));
}

function showDeleteDialog(gpio) {
    deleteTarget = gpio;
    document.getElementById('confirmDeleteDialog').style.display = 'flex';
}

function confirmDelete() {
    Send_Data(JSON.stringify({ action: "delete_relay", gpio: deleteTarget }));
    document.getElementById('confirmDeleteDialog').style.display = 'none';
}

document.getElementById("settingsForm").addEventListener("submit", function (e) {
    e.preventDefault();
    const data = {
        action: "save_settings",
        ssid: document.getElementById("ssid").value,
        pass: document.getElementById("password").value,
        token: document.getElementById("token").value,
        server: document.getElementById("server").value,
        port: parseInt(document.getElementById("port").value)
    };
    Send_Data(JSON.stringify(data));
    alert("Cấu hình đã được gửi!");
});
)rawliteral";

#endif
