#include "services/ApService.h"
#include "Common/PlantCareState.h"
#include "Input/InputLayer.h"
#include "Main_FSM/Main_FSM.h"
#include "config.h"
#include "data/index_html.h"
#include "data/script_js.h"
#include "data/styles_css.h"
#include "Common/AppLog.h"
#include "services/PlantCareInferenceService.h"
#include <ArduinoJson.h>

static const char *AP_TAG = "ApService";

namespace {

constexpr const char *kFanRelayName = "Fan PWM";
constexpr const char *kPumpRelayName = "Pump Relay";
bool gFanPwmConfigured = false;

void configureFanPwmIfNeeded() {
  if (gFanPwmConfigured || PLANT_CARE_FAN_RELAY_GPIO <= 0) {
    return;
  }

  ledcSetup(PLANT_CARE_FAN_PWM_CHANNEL, PLANT_CARE_FAN_PWM_FREQ,
            PLANT_CARE_FAN_PWM_RESOLUTION);
  ledcAttachPin(PLANT_CARE_FAN_RELAY_GPIO, PLANT_CARE_FAN_PWM_CHANNEL);
  ledcWrite(PLANT_CARE_FAN_PWM_CHANNEL, 0);
  gFanPwmConfigured = true;

  ESP_LOGI(AP_TAG,
           "Fan PWM configured on GPIO %d -> channel=%u freq=%uHz res=%ubit duty_on=%u",
           PLANT_CARE_FAN_RELAY_GPIO, PLANT_CARE_FAN_PWM_CHANNEL,
           PLANT_CARE_FAN_PWM_FREQ, PLANT_CARE_FAN_PWM_RESOLUTION,
           PLANT_CARE_FAN_PWM_DUTY_ON);
}

void driveRelayGPIO(int gpio, bool state) {
  if (gpio <= 0) {
    return;
  }

  if (gpio == PLANT_CARE_FAN_RELAY_GPIO) {
    if (!gFanPwmConfigured && !state) {
      pinMode(gpio, OUTPUT);
      digitalWrite(gpio, LOW);
      return;
    }

    configureFanPwmIfNeeded();
    ledcWrite(PLANT_CARE_FAN_PWM_CHANNEL, state ? PLANT_CARE_FAN_PWM_DUTY_ON : 0);
    ESP_LOGI(AP_TAG, "Fan PWM output -> %s (duty=%u)", state ? "ON" : "OFF",
             state ? PLANT_CARE_FAN_PWM_DUTY_ON : 0U);
    return;
  }

  pinMode(gpio, OUTPUT);
  digitalWrite(gpio, state ? HIGH : LOW);
}

} // namespace

// === Static member definitions ===
AsyncWebServer *ApService::server = nullptr;
AsyncWebSocket *ApService::ws = nullptr;
std::vector<RelayConfig> ApService::relayList;
SemaphoreHandle_t ApService::relayMutex = nullptr;

// ============================================================
//  INIT
// ============================================================

void ApService::init(AsyncWebServer *sharedServer, AsyncWebSocket *sharedWs) {
  server = sharedServer;
  ws = sharedWs;

  if (relayMutex == nullptr) {
    relayMutex = xSemaphoreCreateMutex();
  }

  loadRelays();

  // Register WebSocket
  ws->onEvent(onWsEvent);
  server->addHandler(ws);

  // HTTP Routes
  server->on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(200, "text/html", index_html);
  });
  server->on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(200, "text/css", styles_css);
  });
  server->on("/script.js", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(200, "application/javascript", script_js);
  });

  ESP_LOGI(AP_TAG, "ApService initialized.");
}

// ============================================================
//  ACCESS POINT CONTROL
// ============================================================

void ApService::startAP() {
  ESP_LOGI(AP_TAG, "Starting Access Point...");

  WiFi.mode(WIFI_OFF);
  vTaskDelay(pdMS_TO_TICKS(100));
  WiFi.mode(WIFI_AP);

  IPAddress local_IP(ACCESSPOINT_IP);
  IPAddress gateway(ACCESSPOINT_GATEWAY);
  IPAddress subnet(ACCESSPOINT_SUBNET);
  WiFi.softAPConfig(local_IP, gateway, subnet);

  if (WiFi.softAP(ACCESSPOINT_SSID, ACCESSPOINT_PASS)) {
    ESP_LOGI(AP_TAG, "AP Started → SSID: %s | IP: %s", ACCESSPOINT_SSID,
             WiFi.softAPIP().toString().c_str());
    broadcastRelayList();
  } else {
    ESP_LOGE(AP_TAG, "AP Start FAILED!");
  }
}

void ApService::stopAP() {
  WiFi.softAPdisconnect(true);
  ESP_LOGI(AP_TAG, "AP stopped.");
}

// ============================================================
//  RELAY PERSISTENCE
// ============================================================

void ApService::saveRelays() {
  Preferences prefs;
  prefs.begin("relays", false);
  DynamicJsonDocument doc(2048);
  JsonArray arr = doc.to<JsonArray>();
  for (auto &r : relayList) {
    JsonObject obj = arr.createNestedObject();
    obj["name"] = r.name;
    obj["gpio"] = r.gpio;
    obj["state"] = r.state;
  }
  String json;
  serializeJson(doc, json);
  prefs.putString("list", json);
  prefs.end();
}

void ApService::loadRelays() {
  Preferences prefs;
  prefs.begin("relays", true);
  String json = prefs.getString("list", "[]");
  prefs.end();

  DynamicJsonDocument doc(2048);
  if (deserializeJson(doc, json) || !doc.is<JsonArray>())
    return;

  relayList.clear();
  for (JsonObject obj : doc.as<JsonArray>()) {
    RelayConfig r;
    r.name = obj["name"].as<String>();
    r.gpio = obj["gpio"];
    r.state = obj["state"];
    relayList.push_back(r);
    applyRelayGPIO(r); // Khôi phục GPIO state
  }
  syncAutomationRelays();
  ESP_LOGI(AP_TAG, "Loaded %d relay(s) from NVS", (int)relayList.size());
}

void ApService::applyRelayGPIO(const RelayConfig &r) {
  driveRelayGPIO(r.gpio, r.state);
}

bool ApService::setRelayState(int gpio, bool state, bool persistState) {
  if (gpio <= 0) {
    return false;
  }

  bool foundRelay = false;
  bool stateChanged = false;

  if (relayMutex != nullptr &&
      xSemaphoreTake(relayMutex, pdMS_TO_TICKS(200)) == pdPASS) {
    for (auto &relay : relayList) {
      if (relay.gpio == gpio) {
        foundRelay = true;
        stateChanged = (relay.state != state);
        relay.state = state;
        break;
      }
    }

    driveRelayGPIO(gpio, state);

    if (foundRelay && stateChanged && persistState) {
      saveRelays();
    }
    if (foundRelay && stateChanged) {
      broadcastRelayList();
    }

    xSemaphoreGive(relayMutex);
    return true;
  }

  driveRelayGPIO(gpio, state);
  return true;
}

bool ApService::getRelayState(int gpio, bool fallback) {
  if (gpio <= 0) {
    return fallback;
  }

  if (relayMutex != nullptr &&
      xSemaphoreTake(relayMutex, pdMS_TO_TICKS(200)) == pdPASS) {
    for (const auto &relay : relayList) {
      if (relay.gpio == gpio) {
        const bool state = relay.state;
        xSemaphoreGive(relayMutex);
        return state;
      }
    }
    xSemaphoreGive(relayMutex);
  }

  return fallback;
}

void ApService::syncAutomationRelays() {
  if (relayMutex == nullptr ||
      xSemaphoreTake(relayMutex, pdMS_TO_TICKS(200)) != pdPASS) {
    return;
  }

  bool changed = false;
  const RelayConfig defaults[] = {
      {kFanRelayName, PLANT_CARE_FAN_RELAY_GPIO, false},
      {kPumpRelayName, PLANT_CARE_PUMP_RELAY_GPIO, false},
  };

  for (const auto &relay : defaults) {
    if (relay.gpio <= 0) {
      continue;
    }

    bool exists = false;
    for (const auto &current : relayList) {
      if (current.gpio == relay.gpio) {
        exists = true;
        break;
      }
    }

    if (!exists) {
      relayList.push_back(relay);
      applyRelayGPIO(relay);
      changed = true;
    }
  }

  if (changed) {
    saveRelays();
    broadcastRelayList();
  }

  xSemaphoreGive(relayMutex);
}

// ============================================================
//  BROADCAST TO BROWSER
// ============================================================

void ApService::broadcastTelemetry(float temp, float humi) {
  if (!ws)
    return;
  StaticJsonDocument<256> doc;
  doc["temp"] = serialized(String(temp, 1));
  doc["humi"] = serialized(String(humi, 0));

  if (globalPlantCareReady) {
    const int action = globalPlantCareAction;
    doc["care_action_id"] = action;
    doc["care_action"] = PlantCareInferenceService::labelToString(action);
    doc["care_confidence"] = serialized(String(globalPlantCareConfidence, 2));
  }

  String json;
  serializeJson(doc, json);
  ws->textAll(json);
}

void ApService::broadcastRelayList() {
  if (!ws)
    return;
  DynamicJsonDocument doc(2048);
  JsonArray arr = doc.createNestedArray("relays");
  for (auto &r : relayList) {
    JsonObject obj = arr.createNestedObject();
    obj["name"] = r.name;
    obj["gpio"] = r.gpio;
    obj["state"] = r.state;
  }
  String json;
  serializeJson(doc, json);
  ws->textAll(json);
}

// ============================================================
//  WEBSOCKET HANDLER (đẩy event vào InputLayer queue)
// ============================================================

void ApService::onWsEvent(AsyncWebSocket *s, AsyncWebSocketClient *c,
                          AwsEventType type, void *arg, uint8_t *data,
                          size_t len) {
  switch (type) {
  case WS_EVT_CONNECT:
    ESP_LOGI(AP_TAG, "WS Client #%u connected", c->id());
    broadcastRelayList();
    break;
  case WS_EVT_DISCONNECT:
    ESP_LOGI(AP_TAG, "WS Client #%u disconnected", c->id());
    break;
  case WS_EVT_DATA:
    handleWsMessage(arg, data, len);
    break;
  default:
    break;
  }
}

void ApService::handleWsMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (!(info->final && info->index == 0 && info->len == len &&
        info->opcode == WS_TEXT))
    return;

  DynamicJsonDocument doc(1024);
  if (deserializeJson(doc, data, len))
    return;

  String action = doc["action"].as<String>();
  ESP_LOGI(AP_TAG, "WS Action: %s", action.c_str());

  // --- ADD RELAY (xử lý cục bộ + mutex) ---
  if (action == "add_relay") {
    int gpio = doc["gpio"];
    String name = doc["name"].as<String>();
    if (xSemaphoreTake(relayMutex, pdMS_TO_TICKS(200)) == pdPASS) {
      bool exists = false;
      for (auto &r : relayList)
        if (r.gpio == gpio) {
          exists = true;
          break;
        }
      if (!exists && gpio > 0) {
        RelayConfig nr = {name, gpio, false};
        relayList.push_back(nr);
        applyRelayGPIO(nr);
        saveRelays();
        broadcastRelayList();
        // Thông báo cho Processing qua InputLayer
        SystemEvent ev{};
        ev.type = EventType::WS_RELAY_CMD;
        ev.relay_action = RelayAction::ADD;
        ev.relay_gpio = gpio;
        ev.relay_name = name;
        InputLayer::pushEvent(ev);
      }
      xSemaphoreGive(relayMutex);
    }

    // --- TOGGLE RELAY ---
  } else if (action == "toggle_relay") {
    int gpio = doc["gpio"];
    if (xSemaphoreTake(relayMutex, pdMS_TO_TICKS(200)) == pdPASS) {
      for (auto &r : relayList) {
        if (r.gpio == gpio) {
          r.state = !r.state;
          applyRelayGPIO(r);
          saveRelays();
          broadcastRelayList();
          SystemEvent ev{};
          ev.type = EventType::WS_RELAY_CMD;
          ev.relay_action = RelayAction::TOGGLE;
          ev.relay_gpio = gpio;
          InputLayer::pushEvent(ev);
          break;
        }
      }
      xSemaphoreGive(relayMutex);
    }

    // --- DELETE RELAY ---
  } else if (action == "delete_relay") {
    int gpio = doc["gpio"];
    if (xSemaphoreTake(relayMutex, pdMS_TO_TICKS(200)) == pdPASS) {
      for (auto it = relayList.begin(); it != relayList.end(); ++it) {
        if (it->gpio == gpio) {
          driveRelayGPIO(gpio, false);
          relayList.erase(it);
          saveRelays();
          broadcastRelayList();
          SystemEvent ev{};
          ev.type = EventType::WS_RELAY_CMD;
          ev.relay_action = RelayAction::DELETE;
          ev.relay_gpio = gpio;
          InputLayer::pushEvent(ev);
          break;
        }
      }
      xSemaphoreGive(relayMutex);
    }

    // --- SAVE SETTINGS → Trigger switch to WiFi via InputLayer → Processing
    // ---
  } else if (action == "save_settings") {
    SystemEvent ev{};
    ev.type = EventType::WS_SAVE_SETTINGS;
    ev.ws_ssid = doc["ssid"].as<String>();
    ev.ws_pass = doc["pass"].as<String>();
    ev.ws_token = doc["token"].as<String>();
    ev.ws_server = doc["server"].as<String>();
    ev.ws_port = doc["port"] | 1883;
    // Đẩy vào InputLayer → Processing sẽ xử lý: updateConfig + switchMode
    InputLayer::pushEvent(ev);
    ESP_LOGI(AP_TAG, "Save settings event forwarded to InputLayer. SSID: %s",
             ev.ws_ssid.c_str());
  }
}

// ============================================================
//  ACCESSORS
// ============================================================

std::vector<RelayConfig> &ApService::getRelayList() { return relayList; }
SemaphoreHandle_t ApService::getRelayMutex() { return relayMutex; }
