#include "Common/AppLog.h"

#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

namespace {

constexpr TickType_t kLogMutexWait = pdMS_TO_TICKS(20);
constexpr uint32_t kUsbConnectWaitMs = 3000U;
constexpr uint32_t kUsbWriteTimeoutMs = 20U;
constexpr size_t kMessageBufferSize = 256U;
constexpr size_t kLineBufferSize = 448U;

SemaphoreHandle_t s_logMutex = nullptr;
bool s_initialized = false;

const char *basenameOf(const char *path) {
  if (path == nullptr) {
    return "?";
  }

  const char *lastSlash = strrchr(path, '/');
  if (lastSlash != nullptr) {
    return lastSlash + 1;
  }

  const char *lastBackslash = strrchr(path, '\\');
  return lastBackslash != nullptr ? lastBackslash + 1 : path;
}

char levelChar(AppLog::Level level) {
  switch (level) {
  case AppLog::Level::Error:
    return 'E';
  case AppLog::Level::Warn:
    return 'W';
  case AppLog::Level::Info:
    return 'I';
  case AppLog::Level::Debug:
    return 'D';
  case AppLog::Level::Verbose:
    return 'V';
  default:
    return '?';
  }
}

void writeToSerialLocked(const char *data, size_t len) {
  if (data == nullptr || len == 0U) {
    return;
  }

  const uint32_t startedAt = millis();
  size_t offset = 0U;
  while (offset < len) {
    const int writable = Serial.availableForWrite();
    if (writable <= 0) {
      if ((millis() - startedAt) >= kUsbWriteTimeoutMs) {
        break;
      }
      delay(1);
      continue;
    }

    const size_t chunk = min(static_cast<size_t>(writable), len - offset);
    const size_t written = Serial.write(
        reinterpret_cast<const uint8_t *>(data + offset), chunk);
    if (written == 0U) {
      break;
    }
    offset += written;
  }
}

void writeLine(const char *data) {
  if (data == nullptr) {
    return;
  }

  if (s_logMutex == nullptr) {
    writeToSerialLocked(data, strlen(data));
    return;
  }

  if (xSemaphoreTake(s_logMutex, kLogMutexWait) != pdPASS) {
    return;
  }

  writeToSerialLocked(data, strlen(data));
  xSemaphoreGive(s_logMutex);
}

} // namespace

void AppLog::init() {
  if (s_initialized) {
    return;
  }

  if (s_logMutex == nullptr) {
    s_logMutex = xSemaphoreCreateMutex();
  }

  Serial.setDebugOutput(false);
  Serial.setTxTimeoutMs(kUsbWriteTimeoutMs);

#if ARDUINO_USB_CDC_ON_BOOT
  const uint32_t startedAt = millis();
  while (!Serial && (millis() - startedAt) < kUsbConnectWaitMs) {
    delay(10);
  }
#endif

  s_initialized = true;
}

void AppLog::write(Level level, const char *tag, const char *file, unsigned line,
                   const char *function, const char *format, ...) {
  init();

  char messageBuffer[kMessageBufferSize];
  va_list args;
  va_start(args, format);
  vsnprintf(messageBuffer, sizeof(messageBuffer), format, args);
  va_end(args);

  char lineBuffer[kLineBufferSize];
  snprintf(lineBuffer, sizeof(lineBuffer),
           "[%6lu][%c][%s:%u] %s(): [%s] %s\r\n",
           static_cast<unsigned long>(esp_timer_get_time() / 1000ULL),
           levelChar(level), basenameOf(file), line,
           function != nullptr ? function : "?", tag != nullptr ? tag : "?",
           messageBuffer);

  writeLine(lineBuffer);
}

void AppLog::writeRaw(const char *message) {
  init();
  writeLine(message);
}
