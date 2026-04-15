#include "drivers/DHTSensor.h"
#include <DHT.h>
#include "Common/AppLog.h"

static const char *TAG = "DHTSensor";

static DHT dht(DHT11_GPIO, DHT_TYPE);

void DHTSensor::init() {
    dht.begin();
    ESP_LOGI(TAG, "DHT Sensor Initialized on GPIO %d", DHT11_GPIO);
}

bool DHTSensor::readData(float &temp, float &humi) {
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (isnan(t) || isnan(h)) {
        ESP_LOGE(TAG, "Failed to read from DHT sensor!");
        return false;
    }

    temp = t;
    humi = h;
    return true;
}
