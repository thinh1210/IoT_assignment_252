#ifndef STYLES_CSS_H
#define STYLES_CSS_H

#include <Arduino.h>

const char styles_css[] PROGMEM = R"rawliteral(
:root {
  --bg: #f3efe7;
  --bg-panel: rgba(255, 252, 247, 0.82);
  --panel-solid: #fffaf1;
  --panel-border: rgba(41, 67, 72, 0.12);
  --text: #183338;
  --muted: #56747a;
  --accent: #1f8f9f;
  --accent-strong: #145f6b;
  --accent-soft: #d7eef1;
  --warm: #e28d3f;
  --warm-soft: #f7d9b8;
  --success: #2f8e68;
  --danger: #c8533f;
  --shadow: 0 20px 50px rgba(28, 53, 63, 0.12);
  --radius-xl: 28px;
  --radius-lg: 22px;
  --radius-md: 16px;
}

* {
  box-sizing: border-box;
}

html, body {
  margin: 0;
  min-height: 100%;
}

body {
  font-family: "Manrope", sans-serif;
  color: var(--text);
  background:
    radial-gradient(circle at top left, rgba(31, 143, 159, 0.18), transparent 28%),
    radial-gradient(circle at bottom right, rgba(226, 141, 63, 0.20), transparent 24%),
    linear-gradient(180deg, #f7f1e7 0%, #f2eee8 100%);
}

.app-shell {
  width: min(1440px, calc(100vw - 40px));
  margin: 20px auto;
  padding: 22px;
  border-radius: 34px;
  background: rgba(255, 255, 255, 0.42);
  backdrop-filter: blur(18px);
  border: 1px solid rgba(255, 255, 255, 0.56);
  box-shadow: var(--shadow);
}

.topbar {
  display: grid;
  grid-template-columns: 1.35fr 1fr;
  gap: 18px;
  margin-bottom: 18px;
}

.brand-block,
.header-panel,
.hero-card,
.gauge-card,
.care-card,
.guide-card,
.settings-card,
.summary-pill,
.metric-card,
.device-card,
.empty-state {
  background: var(--bg-panel);
  border: 1px solid var(--panel-border);
  backdrop-filter: blur(12px);
  box-shadow: 0 14px 28px rgba(25, 49, 58, 0.08);
}

.brand-block {
  padding: 28px 30px;
  border-radius: var(--radius-xl);
  background:
    radial-gradient(circle at top right, rgba(31, 143, 159, 0.14), transparent 30%),
    radial-gradient(circle at bottom left, rgba(226, 141, 63, 0.14), transparent 24%),
    var(--bg-panel);
}

.eyebrow,
.section-kicker,
.panel-label,
.summary-label,
.metric-label,
.hero-chip {
  display: inline-flex;
  align-items: center;
  gap: 8px;
  font-size: 12px;
  letter-spacing: 0.14em;
  text-transform: uppercase;
  color: var(--accent-strong);
  font-weight: 700;
}

.brand-block h1,
.hero-card h2,
.care-card h2,
.section-header h2,
.settings-card h2,
.guide-card h3 {
  font-family: "Space Grotesk", sans-serif;
}

.brand-block h1 {
  margin: 10px 0 8px;
  font-size: clamp(28px, 4vw, 46px);
  line-height: 1.02;
}

.subtitle,
.brand-block p,
.mode-note,
.section-header p,
.settings-intro,
.care-caption,
.empty-state p,
.dialog p,
.panel-meta,
.metric-card small,
.feature-list li {
  color: var(--muted);
  line-height: 1.6;
}

.header-panels {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 14px;
}

.header-panel {
  border-radius: var(--radius-lg);
  padding: 20px;
  min-height: 148px;
  display: flex;
  flex-direction: column;
  justify-content: space-between;
}

.header-panel strong {
  font-size: 24px;
  line-height: 1.15;
}

#wsStateText.is-live {
  color: var(--success);
}

.tabbar {
  display: inline-flex;
  gap: 10px;
  padding: 10px;
  background: rgba(255, 250, 241, 0.76);
  border: 1px solid rgba(34, 75, 88, 0.08);
  border-radius: 999px;
  margin-bottom: 22px;
}

.nav-item {
  border: none;
  cursor: pointer;
  padding: 12px 18px;
  border-radius: 999px;
  font: inherit;
  font-weight: 700;
  color: var(--muted);
  background: transparent;
  transition: 180ms ease;
}

.nav-item.active {
  background: linear-gradient(135deg, var(--accent), var(--accent-strong));
  color: white;
  box-shadow: 0 10px 22px rgba(20, 95, 107, 0.22);
}

.main-content {
  display: grid;
  gap: 24px;
}

.section {
  display: grid;
  gap: 24px;
}

.hero-grid,
.gauge-layout,
.insight-grid,
.settings-layout {
  display: grid;
  gap: 20px;
}

.hero-grid {
  grid-template-columns: 1.2fr 0.8fr;
}

.hero-card {
  border-radius: var(--radius-xl);
  padding: 26px;
}

.hero-primary {
  display: grid;
  gap: 22px;
}

.hero-copy h2 {
  margin: 10px 0 10px;
  font-size: clamp(28px, 3vw, 40px);
  line-height: 1.05;
}

.hero-copy p {
  margin: 0;
  max-width: 62ch;
  color: var(--muted);
}

.hero-chip {
  padding: 7px 12px;
  border-radius: 999px;
  background: rgba(31, 143, 159, 0.10);
}

.hero-chip.accent {
  background: rgba(226, 141, 63, 0.16);
  color: #8f4b12;
}

.quick-metrics {
  display: grid;
  grid-template-columns: repeat(2, minmax(0, 1fr));
  gap: 14px;
}

.metric-card {
  border-radius: var(--radius-md);
  padding: 18px 18px 16px;
  background: rgba(255, 252, 247, 0.94);
}

.metric-card strong {
  display: block;
  margin: 8px 0 6px;
  font-family: "Space Grotesk", sans-serif;
  font-size: clamp(24px, 3vw, 34px);
}

.mode-flow {
  display: grid;
  gap: 12px;
  margin-top: 16px;
}

.mode-step {
  padding: 14px 16px;
  border-radius: 18px;
  background: rgba(255, 250, 241, 0.9);
  border: 1px solid rgba(31, 143, 159, 0.12);
  font-weight: 700;
}

.mode-step div {
  margin-top: 4px;
  font-weight: 500;
  color: var(--muted);
  font-size: 14px;
}

.gauge-layout {
  grid-template-columns: repeat(2, minmax(0, 1fr));
}

.gauge-card {
  border-radius: var(--radius-xl);
  padding: 22px;
}

.card-head {
  margin-bottom: 12px;
}

.card-head h3,
.settings-card h2,
.section-header h2 {
  margin: 8px 0 0;
  font-size: 28px;
}

#gauge_temp,
#gauge_humi {
  width: min(100%, 290px);
  height: 260px;
  margin: 0 auto;
}

.insight-grid {
  grid-template-columns: 1fr 0.92fr;
}

.care-card,
.guide-card,
.settings-card,
.device-card,
.empty-state {
  border-radius: var(--radius-xl);
  padding: 24px;
}

.care-card {
  background:
    linear-gradient(135deg, rgba(31, 143, 159, 0.16), rgba(226, 141, 63, 0.12)),
    var(--bg-panel);
}

.care-banner {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 14px;
  margin-bottom: 14px;
}

.care-chip,
.care-confidence,
.relay-state,
.summary-pill {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  border-radius: 999px;
  padding: 8px 14px;
  font-size: 13px;
  font-weight: 700;
}

.care-chip {
  background: rgba(31, 143, 159, 0.12);
  color: var(--accent-strong);
}

.care-confidence {
  background: rgba(226, 141, 63, 0.14);
  color: #8f4b12;
}

.care-card h2 {
  margin: 0 0 12px;
  font-size: clamp(28px, 4vw, 42px);
  line-height: 1.04;
}

.feature-list {
  display: grid;
  gap: 10px;
  margin: 14px 0 0;
  padding-left: 18px;
}

.section-header {
  display: flex;
  align-items: flex-end;
  justify-content: space-between;
  gap: 18px;
}

.device-summary {
  display: flex;
  flex-wrap: wrap;
  gap: 12px;
}

.summary-pill {
  background: rgba(255, 250, 241, 0.92);
  border: 1px solid rgba(31, 143, 159, 0.12);
  flex-direction: column;
  align-items: flex-start;
  gap: 6px;
  min-width: 190px;
}

.summary-pill strong {
  color: var(--text);
}

.device-list {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(230px, 1fr));
  gap: 18px;
}

.device-card {
  position: relative;
  display: grid;
  gap: 14px;
  background: rgba(255, 252, 247, 0.92);
}

.device-card h3 {
  margin: 0;
  font-family: "Space Grotesk", sans-serif;
  font-size: 24px;
}

.device-card p {
  margin: 0;
  color: var(--muted);
}

.device-card-top {
  display: flex;
  justify-content: space-between;
  align-items: flex-start;
}

.device-icon-wrap {
  width: 54px;
  height: 54px;
  border-radius: 18px;
  display: inline-flex;
  align-items: center;
  justify-content: center;
  background: rgba(31, 143, 159, 0.12);
}

.device-icon {
  font-size: 22px;
  color: var(--accent-strong);
}

.relay-state {
  width: max-content;
  background: rgba(86, 116, 122, 0.12);
  color: var(--muted);
}

.relay-state.is-on {
  background: rgba(47, 142, 104, 0.14);
  color: var(--success);
}

.relay-state.is-off {
  background: rgba(200, 83, 63, 0.12);
  color: var(--danger);
}

.toggle-btn,
.btn-save,
.dialog-buttons button,
.add-relay-btn,
.icon-btn {
  border: none;
  cursor: pointer;
  font: inherit;
}

.toggle-btn {
  width: 100%;
  border-radius: 16px;
  padding: 14px 16px;
  font-weight: 800;
  background: rgba(31, 143, 159, 0.08);
  color: var(--accent-strong);
  transition: transform 180ms ease, box-shadow 180ms ease;
}

.toggle-btn.on {
  background: linear-gradient(135deg, var(--accent), var(--accent-strong));
  color: white;
  box-shadow: 0 12px 22px rgba(20, 95, 107, 0.18);
}

.toggle-btn:active,
.btn-save:active,
.add-relay-btn:active,
.icon-btn:active,
.dialog-buttons button:active {
  transform: translateY(1px) scale(0.99);
}

.icon-btn {
  width: 42px;
  height: 42px;
  border-radius: 14px;
  background: rgba(86, 116, 122, 0.1);
  color: var(--muted);
}

.icon-btn.danger {
  color: var(--danger);
}

.empty-state {
  display: grid;
  place-items: center;
  gap: 8px;
  text-align: center;
  min-height: 240px;
  background: rgba(255, 252, 247, 0.9);
}

.empty-state i {
  font-size: 44px;
  color: var(--accent);
}

.add-relay-btn {
  position: fixed;
  right: 34px;
  bottom: 34px;
  width: 76px;
  height: 76px;
  border-radius: 50%;
  background: linear-gradient(135deg, var(--warm), #d76637);
  color: white;
  font-size: 30px;
  box-shadow: 0 16px 26px rgba(210, 111, 45, 0.28);
}

.settings-layout {
  grid-template-columns: 1.05fr 0.75fr;
  align-items: start;
}

.settings-card {
  background:
    radial-gradient(circle at top right, rgba(31, 143, 159, 0.08), transparent 25%),
    var(--bg-panel);
}

#settingsForm {
  display: grid;
  gap: 16px;
  margin-top: 20px;
}

.field {
  display: grid;
  gap: 8px;
  font-weight: 700;
}

.field span {
  font-size: 14px;
  color: var(--accent-strong);
}

.field-grid {
  display: grid;
  grid-template-columns: 1fr 170px;
  gap: 16px;
}

.input-shell {
  position: relative;
}

.input-shell i {
  position: absolute;
  left: 16px;
  top: 50%;
  transform: translateY(-50%);
  color: var(--accent);
}

.input-shell input {
  width: 100%;
  padding: 15px 16px 15px 48px;
  border-radius: 16px;
  border: 1px solid rgba(31, 143, 159, 0.12);
  background: rgba(255, 255, 255, 0.84);
  color: var(--text);
  font: inherit;
}

.input-shell input::placeholder {
  color: rgba(86, 116, 122, 0.8);
}

.btn-save {
  border-radius: 18px;
  padding: 16px 20px;
  font-weight: 800;
  color: white;
  background: linear-gradient(135deg, var(--accent), var(--accent-strong));
  box-shadow: 0 14px 22px rgba(20, 95, 107, 0.16);
}

.btn-save:disabled {
  opacity: 0.6;
  cursor: default;
}

.message-banner {
  margin-top: 18px;
  padding: 14px 16px;
  border-radius: 16px;
  font-weight: 700;
}

.message-banner.is-success {
  background: rgba(47, 142, 104, 0.12);
  color: var(--success);
}

.message-banner.is-error {
  background: rgba(200, 83, 63, 0.12);
  color: var(--danger);
}

.dialog-overlay {
  position: fixed;
  inset: 0;
  background: rgba(20, 40, 47, 0.44);
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 20px;
  z-index: 1000;
}

.dialog {
  width: min(420px, 100%);
  padding: 28px;
  border-radius: 24px;
  background: #fffaf1;
  border: 1px solid rgba(31, 143, 159, 0.10);
  box-shadow: 0 20px 50px rgba(20, 40, 47, 0.18);
}

.dialog h3 {
  margin: 0 0 10px;
  font-family: "Space Grotesk", sans-serif;
  font-size: 28px;
}

.dialog input {
  width: 100%;
  margin-top: 10px;
  padding: 14px 16px;
  border-radius: 16px;
  border: 1px solid rgba(31, 143, 159, 0.12);
  font: inherit;
}

.dialog-buttons {
  display: flex;
  gap: 12px;
  margin-top: 18px;
}

.dialog-buttons button {
  flex: 1;
  padding: 14px 16px;
  border-radius: 16px;
  font-weight: 800;
}

.confirm {
  background: linear-gradient(135deg, var(--accent), var(--accent-strong));
  color: white;
}

.danger {
  background: linear-gradient(135deg, #df6b4e, #c8533f);
  color: white;
}

.cancel {
  background: rgba(86, 116, 122, 0.14);
  color: var(--text);
}

@media (max-width: 1080px) {
  .topbar,
  .hero-grid,
  .insight-grid,
  .settings-layout {
    grid-template-columns: 1fr;
  }

  .header-panels {
    grid-template-columns: 1fr;
  }

  .gauge-layout {
    grid-template-columns: 1fr;
  }
}

@media (max-width: 760px) {
  .app-shell {
    width: min(100vw - 20px, 100%);
    margin: 10px auto;
    padding: 16px;
    border-radius: 24px;
  }

  .quick-metrics,
  .field-grid {
    grid-template-columns: 1fr;
  }

  .tabbar {
    width: 100%;
    justify-content: space-between;
  }

  .nav-item {
    flex: 1;
    text-align: center;
  }

  .section-header {
    align-items: flex-start;
    flex-direction: column;
  }

  .add-relay-btn {
    right: 18px;
    bottom: 18px;
    width: 68px;
    height: 68px;
  }
}
)rawliteral";

#endif
