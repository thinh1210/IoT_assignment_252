# ESP32-S3 IoT Plant Care System

A course project built on the ESP32-S3 using PlatformIO, FreeRTOS, TinyML, and CoreIOT. The system monitors temperature and humidity, provides local visual feedback through LED, NeoPixel, and OLED, exposes a responsive Access Point dashboard, runs on-device TinyML inference, publishes telemetry to CoreIOT, and supports OTA firmware updates.

## Overview

This repository implements a mode-based IoT firmware architecture for the Yolo UNO ESP32-S3 board. The project was developed to satisfy the assignment requirements for:

- temperature-aware LED behavior with task synchronization
- humidity-aware NeoPixel indication
- OLED-based environmental monitoring
- Access Point web control for actuators
- TinyML deployment and evaluation on-device
- telemetry publishing to CoreIOT in Station mode

Beyond the required scope, the firmware also includes:

- OTA firmware update flow through CoreIOT / ThingsBoard
- manual mode via IR remote
- unified debug logging for multi-task tracing
- automatic fan control driven by TinyML predictions

## Key Features

### 1. Environmental sensing and visual feedback

- DHT11 sensor polling every 3 seconds
- single LED blink profiles for temperature state handling
- NeoPixel color indication for humidity levels
- OLED status screens for normal, warning, and critical conditions

### 2. Multi-mode runtime

- `NORMAL_MODE`: local monitoring, TinyML-assisted automation, CoreIOT publishing
- `ACCESSPOINT_MODE`: local Wi-Fi AP with dashboard, WebSocket updates, relay control
- `MANUAL_MODE`: direct actuator control through IR remote

### 3. TinyML on the microcontroller

- input features: `temperature`, `humidity`
- labels:
  - `0`: No action
  - `1`: Turn on fan
  - `2`: Need watering
- quantized TensorFlow Lite Micro model embedded as `include/tinyml_model_data.h`
- inference result propagated to OLED, AP dashboard, and CoreIOT telemetry

### 4. Cloud integration

- Wi-Fi Station mode for Internet connectivity
- MQTT telemetry to `app.coreiot.io`
- OTA metadata request, firmware download, and update reporting

## Architecture

The firmware follows a layered, task-oriented design:

- `Input Layer`: button, IR remote, and sensor acquisition
- `Main FSM`: mode switching, event orchestration, and high-level control flow
- `Services`: Wi-Fi, AP dashboard, display, CoreIOT publishing, TinyML runtime, manual control
- `Drivers`: DHT11, LED, NeoPixel, OLED, button handling

FreeRTOS queues, semaphores, mutexes, and critical sections are used to coordinate access to shared resources such as GPIO, relay state, logging, and sensor-triggered workflows.

## Hardware Mapping

| Component | GPIO / Interface | Notes |
| --- | --- | --- |
| Green LED | `GPIO 48` | Local status / blink profiles |
| NeoPixel | `GPIO 45` | Humidity indication |
| DHT11 | `GPIO 1` | Temperature and humidity input |
| IR receiver | `GPIO 6` | Manual mode control |
| OLED SH1106 | `I2C SDA 11`, `I2C SCL 12` | Status and TinyML output |
| Fan PWM | `GPIO 21` | Main automatic actuator |
| Pump relay | disabled by default | `-1` in current configuration |

Default network settings in firmware:

- AP SSID: `ESP32`
- AP password: `12345678`
- AP IP: `192.168.4.1`

## Software Stack

- PlatformIO
- Arduino framework for ESP32
- FreeRTOS
- ESPAsyncWebServer + AsyncTCP
- ArduinoJson
- ThingsBoard client stack for CoreIOT
- U8g2 for OLED
- Adafruit NeoPixel
- IRremote
- TensorFlow Lite Micro

## Repository Structure

```text
.
├── data/                     # TinyML dataset and related inputs
├── include/
│   ├── Common/               # Shared state, events, logging, config structs
│   ├── Input/                # Input layer interfaces
│   ├── Main_FSM/             # Mode manager and FSM interfaces
│   ├── data/                 # Embedded AP dashboard assets
│   ├── drivers/              # Hardware drivers
│   ├── services/             # Wi-Fi, AP, display, TinyML, cloud services
│   ├── config.h              # Runtime configuration and GPIO mapping
│   └── tinyml_model_data.h   # Exported TFLM model header
├── ml_artifacts/             # Generated TinyML training outputs
├── report/                   # Assignment report (LaTeX + PDF)
├── scripts/                  # TinyML collection and training pipeline
├── src/                      # Firmware implementation
└── platformio.ini            # PlatformIO environment configuration
```

## Getting Started

### Prerequisites

- VS Code with PlatformIO extension, or PlatformIO Core installed locally
- Python 3.10+ for the TinyML training pipeline
- ESP32-S3 board supported by the `yolo_uno` PlatformIO environment

### 1. Clone the repository

```bash
git clone <your-repository-url>
cd IoT_assignment_252
```

### 2. Configure device settings

Update [`include/config.h`](include/config.h) with your own:

- Wi-Fi SSID and password
- CoreIOT / ThingsBoard server
- device access token
- OTA firmware metadata if needed
- GPIO assignments if your wiring differs

Note: credentials are currently defined at compile time. For a public repository, avoid committing real Wi-Fi passwords or production tokens.

### 3. Install TinyML training dependencies

```bash
pip install -r requirements.txt
```

### 4. Build the firmware

```bash
pio run
```

### 5. Flash the board

```bash
pio run -t upload
```

### 6. Open serial monitor

```bash
pio device monitor
```

The default monitor baud rate is `115200`, as configured in [`platformio.ini`](platformio.ini).

## Runtime Behavior

### Normal mode

- connects to Wi-Fi in Station mode
- publishes telemetry to CoreIOT
- reads DHT11 data periodically
- updates LED, NeoPixel, and OLED
- runs TinyML inference after each sensor read
- turns the fan on automatically when the predicted action is non-zero

### Access Point mode

- starts a local AP at `192.168.4.1`
- serves a dashboard with HTML, CSS, and JavaScript embedded in firmware
- supports relay management and real-time updates over WebSocket
- shows environmental telemetry and TinyML recommendation locally

### Manual mode

- entered through IR remote mode cycling
- allows direct control of fan, pump, green LED, and NeoPixel
- bypasses automatic actuator updates until the mode is exited

## TinyML Workflow

The project includes a small end-to-end TinyML pipeline for dataset collection, model training, quantization, and firmware embedding.

Training script:

- [`scripts/tinyml_pipeline.py`](scripts/tinyml_pipeline.py)

Dataset:

- `data/tinyml_samples.csv`

Generated artifacts:

- `ml_artifacts/tinyml_action_model.h5`
- `ml_artifacts/tinyml_action_model_int8.tflite`
- `include/tinyml_model_data.h`
- `ml_artifacts/tinyml_training_summary.json`

Example commands:

```bash
python scripts/tinyml_pipeline.py collect --csv data/tinyml_samples.csv
python scripts/tinyml_pipeline.py train --csv data/tinyml_samples.csv
```

Current recorded training summary:

- sample count: `90`
- train samples: `72`
- validation samples: `18`
- train accuracy: `1.0`
- validation accuracy: `1.0`

These metrics come from [`ml_artifacts/tinyml_training_summary.json`](ml_artifacts/tinyml_training_summary.json).

## CoreIOT and OTA

Cloud publishing is implemented through the ThingsBoard-compatible CoreIOT MQTT workflow:

- server: `app.coreiot.io`
- port: `1883`
- telemetry keys include temperature, humidity, alert status, fan state, pump state, TinyML action, and TinyML confidence

OTA support includes:

- shared-attribute subscription
- firmware metadata request
- firmware download and flashing
- update status reporting after success

## Assignment Coverage

| Task | Implementation Status | Notes |
| --- | --- | --- |
| Task 1 | Implemented | LED blink profiles with synchronized access |
| Task 2 | Implemented | NeoPixel states mapped to humidity ranges |
| Task 3 | Implemented | OLED state handling with sensor-driven synchronization |
| Task 4 | Implemented | AP dashboard with control actions for multiple devices |
| Task 5 | Implemented | TinyML model trained, exported, embedded, and executed on-device |
| Task 6 | Implemented | CoreIOT telemetry publishing in Station mode |

## Documentation

- Project report source: [`report/main.tex`](report/main.tex)
- Architecture notes: [`boards/design.md`](boards/design.md)
- Manual history notes: [`manual_history_index.md`](manual_history_index.md)

## Team

- Tăng Phồn Thịnh
- Nguyễn Trần Đăng Khoa

## License

This repository is maintained as an academic course project. Add a formal license here if the project is intended for public reuse.
