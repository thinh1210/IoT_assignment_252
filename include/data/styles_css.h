#ifndef STYLES_CSS_H
#define STYLES_CSS_H

#include <Arduino.h>

const char styles_css[] PROGMEM = R"rawliteral(
body {
  margin: 0;
  font-family: "Poppins", Arial, sans-serif;
  display: flex;
  height: 100vh;
  background-color: #eef2f7;
}
.sidebar {
  width: 260px;
  background-color: #2294F2;
  color: white;
  display: flex;
  flex-direction: column;
  align-items: center;
  box-shadow: 3px 0 10px rgba(0, 0, 0, 0.2);
}
.logo-container {
  width: 100%;
  background: white;
  display: flex;
  justify-content: center;
  align-items: center;
  padding: 20px 0;
  border-bottom: 2px solid rgba(0, 0, 0, 0.1);
}
.logo-img {
  width: 80%;
  height: auto;
}
.nav-item {
  width: 90%;
  padding: 16px 20px;
  margin: 8px 0;
  font-size: 18px;
  font-weight: 600;
  cursor: pointer;
  border-radius: 10px;
  color: #ecf6ff;
  transition: all 0.3s ease;
  display: flex;
  align-items: center;
  gap: 12px;
}
.nav-item.active {
  background: #fff;
  color: #2294F2;
  box-shadow: 0 4px 10px rgba(0, 0, 0, 0.15);
}
.main-content {
  flex: 1;
  padding: 40px 60px;
  overflow-y: auto;
}
.gauges-container {
  display: flex;
  flex-wrap: wrap;
  gap: 30px;
  justify-content: center;
}
.gauge-card {
  background: white;
  border-radius: 16px;
  box-shadow: 0 6px 16px rgba(0, 0, 0, 0.1);
  padding: 30px;
  text-align: center;
  width: 300px;
  transition: transform 0.3s ease;
}
#gauge_temp, #gauge_humi {
  width: 260px;
  height: 260px;
  margin: 0 auto;
}
.care-card {
  margin: 30px auto 0;
  max-width: 680px;
  background: linear-gradient(135deg, #103b5d, #1f7cc0);
  color: white;
  border-radius: 20px;
  padding: 28px 32px;
  box-shadow: 0 14px 28px rgba(16, 59, 93, 0.25);
}
.care-chip {
  display: inline-flex;
  align-items: center;
  padding: 6px 12px;
  border-radius: 999px;
  background: rgba(255, 255, 255, 0.16);
  font-size: 13px;
  font-weight: 700;
  letter-spacing: 0.08em;
  text-transform: uppercase;
}
.care-card h2 {
  margin: 14px 0 10px;
  font-size: 32px;
}
.care-card p {
  margin: 0;
}
.care-caption {
  margin-top: 12px !important;
  color: rgba(255, 255, 255, 0.84);
}
.device-list {
  display: flex;
  flex-wrap: wrap;
  gap: 25px;
  margin-top: 30px;
  justify-content: center;
}
.device-card {
  background: #fff;
  border-radius: 20px;
  box-shadow: 0 5px 20px rgba(0, 0, 0, 0.1);
  width: 240px;
  padding: 25px 20px;
  text-align: center;
  position: relative;
  transition: all 0.3s ease;
}
.device-card:hover {
  transform: translateY(-5px);
  box-shadow: 0 8px 25px rgba(34, 148, 242, 0.25);
}
.device-icon {
  font-size: 36px;
  color: #2294F2;
  margin-bottom: 10px;
}
.toggle-btn {
  width: 100%;
  border: none;
  border-radius: 12px;
  padding: 12px;
  font-weight: 600;
  font-size: 16px;
  cursor: pointer;
  background: #e3e3e3;
  transition: 0.3s;
}
.toggle-btn.on {
  background: linear-gradient(90deg, #2294F2, #1b7fe0);
  color: white;
  box-shadow: 0 4px 12px rgba(34, 148, 242, 0.4);
}
.delete-icon {
  position: absolute;
  top: 10px;
  right: 12px;
  color: #e74c3c;
  cursor: pointer;
}
.add-relay-btn {
  position: fixed;
  bottom: 35px;
  right: 35px;
  background: linear-gradient(135deg, #2294F2, #1a75e6);
  color: white;
  border: none;
  border-radius: 50%;
  width: 80px;
  height: 80px;
  font-size: 34px;
  cursor: pointer;
  box-shadow: 0 6px 20px rgba(0, 0, 0, 0.25);
  transition: 0.3s;
}
.add-relay-btn:hover {
  transform: scale(1.08);
}
.settings-section {
  display: flex;
  justify-content: center;
  align-items: flex-start;
  padding-top: 40px;
}
.settings-card {
  background: white;
  padding: 40px;
  border-radius: 20px;
  box-shadow: 0 10px 25px rgba(0, 0, 0, 0.15);
  width: 100%;
  max-width: 600px;
  text-align: center;
}
#settingsForm {
  display: flex;
  flex-direction: column;
  gap: 15px;
}
.input-group {
  position: relative;
}
.input-group i {
  position: absolute;
  left: 15px;
  top: 50%;
  transform: translateY(-50%);
  color: #2294F2;
}
.input-group input {
  width: 100%;
  padding: 12px 12px 12px 45px;
  border-radius: 10px;
  border: 1px solid #ddd;
  box-sizing: border-box;
}
.btn-save {
  background: #2294F2;
  color: white;
  padding: 15px;
  border: none;
  border-radius: 10px;
  font-weight: bold;
  cursor: pointer;
}
.dialog-overlay {
  position: fixed;
  top: 0; left: 0; right: 0; bottom: 0;
  background: rgba(0,0,0,0.5);
  display: flex;
  justify-content: center;
  align-items: center;
  z-index: 1000;
}
.dialog {
  background: white;
  padding: 30px;
  border-radius: 15px;
  width: 320px;
  text-align: center;
}
.dialog-buttons {
  margin-top: 20px;
  display: flex;
  gap: 10px;
  justify-content: center;
}
.dialog-buttons button {
  padding: 10px 20px;
  border-radius: 5px;
  border: none;
  cursor: pointer;
}
.confirm { background: #2294F2; color: white; }
.danger { background: #e74c3c; color: white; }
.cancel { background: #ccc; }
)rawliteral";

#endif
