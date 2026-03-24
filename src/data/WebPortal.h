#ifndef WEB_PORTAL_H
#define WEB_PORTAL_H

#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP32 Device Configuration</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link href="https://fonts.googleapis.com/css2?family=Lilita+One&family=Roboto:wght@400;700&display=swap" rel="stylesheet">
  <style>
    html, body {
      margin: 0; padding: 0; height: 100%;
      font-family: 'Roboto', sans-serif;
      background: linear-gradient(145deg, #1e3c72, #2a5298, #1e3c72);
      background-size: 400% 400%;
      animation: gradientShift 15s ease infinite;
      display: flex; align-items: center; justify-content: center;
      color: white;
    }

    @keyframes gradientShift {
      0% { background-position: 0% 50%; }
      50% { background-position: 100% 50%; }
      100% { background-position: 0% 50%; }
    }

    .input_wrapper {
      width: 95%; max-width: 450px;
      padding: 30px 24px;
      border-radius: 20px;
      backdrop-filter: blur(15px);
      -webkit-backdrop-filter: blur(15px);
      background: rgba(255, 255, 255, 0.1);
      box-shadow: 0 15px 35px rgba(0, 0, 0, 0.4);
      border: 1px solid rgba(255, 255, 255, 0.2);
    }

    h1 { text-align: center; font-family: 'Lilita One', cursive; font-size: 28px; margin-bottom: 20px; letter-spacing: 1px; }
    h2 { font-size: 16px; margin: 15px 0 10px; border-bottom: 1px solid rgba(255,255,255,0.2); padding-bottom: 5px; color: #f1c40f; }

    label { display: block; margin-bottom: 5px; font-size: 13px; color: rgba(255, 255, 255, 0.8); }
    .input_box {
      width: 100%; padding: 12px; margin-bottom: 15px;
      border-radius: 10px; border: none; font-size: 14px; outline: none;
      background: rgba(255, 255, 255, 0.2); color: white; box-sizing: border-box;
      transition: background 0.3s;
    }
    .input_box:focus { background: rgba(255, 255, 255, 0.3); }
    .input_box::placeholder { color: rgba(255, 255, 255, 0.5); }

    .btn-group { display: flex; gap: 10px; margin-top: 10px; }
    button {
      flex: 1; padding: 14px; font-size: 15px; font-weight: bold; color: white;
      border: none; border-radius: 50px; cursor: pointer;
      transition: transform 0.2s, box-shadow 0.2s;
    }
    button:active { transform: scale(0.98); }

    #submit { background: linear-gradient(120deg, #27ae60, #2ecc71); box-shadow: 0 4px 15px rgba(39, 174, 96, 0.3); }
    #toggle { background: linear-gradient(120deg, #e67e22, #f39c12); box-shadow: 0 4px 15px rgba(230, 126, 34, 0.3); }
    
    .message { margin-top: 15px; padding: 10px; border-radius: 10px; text-align: center; display: none; font-size: 14px; background: rgba(0,0,0,0.3); }
  </style>
</head>
<body>
  <div class="input_wrapper">
    <h1>DEVICE SETUP</h1>
    
    <form id="configForm">
      <h2>WiFi Connection</h2>
      <label for="wifi_ssid">SSID</label>
      <input type="text" id="wifi_ssid" class="input_box" placeholder="Network Name" required>
      <label for="wifi_pass">Password</label>
      <input type="password" id="wifi_pass" class="input_box" placeholder="WiFi Password">

      <h2>MQTT Broker (ThingsBoard)</h2>
      <label for="mqtt_server">Server Host</label>
      <input type="text" id="mqtt_server" class="input_box" placeholder="IP Address or Domain" required>
      <label for="mqtt_port">PortNumber</label>
      <input type="number" id="mqtt_port" class="input_box" value="1883" required>
      <label for="mqtt_user">Access Token</label>
      <input type="text" id="mqtt_user" class="input_box" placeholder="Device Token">

      <h2>Security Configuration</h2>
      <label for="key_url">Key Exchange URL</label>
      <input type="text" id="key_url" class="input_box" placeholder="Endpoint for encryption keys">

      <div class="btn-group">
        <button type="submit" id="submit">SAVE SETTINGS</button>
      </div>
    </form>
    
    <button id="toggle" onclick="toggleSystemMode()" style="margin-top: 15px; width: 100%;">SWITCH TO CLIENT MODE</button>
    <div id="message" class="message"></div>
  </div>

  <script>
    function toggleSystemMode() {
        if(confirm("Change System Mode?")) {
            const btn = document.getElementById('toggle');
            btn.innerHTML = "SWITCHING..."; btn.disabled = true;
            fetch('/toggle', { method: 'POST' })
            .then(() => {
                alert("Restaring device in new mode... Please connect back when ready.");
                setTimeout(() => location.reload(), 3000);
            });
        }
    }

    document.getElementById('configForm').addEventListener('submit', function(e) {
        e.preventDefault();
        const btn = document.getElementById('submit');
        const originalText = btn.innerHTML;
        btn.innerHTML = "SAVING..."; btn.disabled = true;

        const data = {
            wifi_ssid: document.getElementById('wifi_ssid').value,
            wifi_pass: document.getElementById('wifi_pass').value,
            mqtt_server: document.getElementById('mqtt_server').value,
            mqtt_port: parseInt(document.getElementById('mqtt_port').value),
            mqtt_user: document.getElementById('mqtt_user').value,
            key_url: document.getElementById('key_url').value
        };

        fetch('/config', {
            method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify(data)
        })
        .then(r => r.json())
        .then(d => {
            const msg = document.getElementById('message');
            msg.innerHTML = "✅ Settings Saved!"; msg.style.display = 'block';
            setTimeout(() => { location.reload(); }, 2000);
        })
        .catch(err => {
            btn.innerHTML = originalText; btn.disabled = false;
            alert("Save failed!");
        });
    });
  </script>
</body>
</html>
)rawliteral";

#endif // WEB_PORTAL_H
