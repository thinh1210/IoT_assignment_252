#ifndef APP_LOG_H
#define APP_LOG_H

#include <Arduino.h>

namespace AppLog {

enum class Level : uint8_t {
  Error,
  Warn,
  Info,
  Debug,
  Verbose,
};

void init();
void write(Level level, const char *tag, const char *file, unsigned line,
           const char *function, const char *format, ...)
    __attribute__((format(printf, 6, 7)));
void writeRaw(const char *message);

} // namespace AppLog

#ifdef ESP_LOGE
#undef ESP_LOGE
#endif

#ifdef ESP_LOGW
#undef ESP_LOGW
#endif

#ifdef ESP_LOGI
#undef ESP_LOGI
#endif

#ifdef ESP_LOGD
#undef ESP_LOGD
#endif

#ifdef ESP_LOGV
#undef ESP_LOGV
#endif

#define ESP_LOGE(tag, format, ...)                                              \
  AppLog::write(AppLog::Level::Error, tag, __FILE__, __LINE__, __FUNCTION__,   \
                format, ##__VA_ARGS__)

#define ESP_LOGW(tag, format, ...)                                              \
  AppLog::write(AppLog::Level::Warn, tag, __FILE__, __LINE__, __FUNCTION__,    \
                format, ##__VA_ARGS__)

#define ESP_LOGI(tag, format, ...)                                              \
  AppLog::write(AppLog::Level::Info, tag, __FILE__, __LINE__, __FUNCTION__,    \
                format, ##__VA_ARGS__)

#define ESP_LOGD(tag, format, ...)                                              \
  AppLog::write(AppLog::Level::Debug, tag, __FILE__, __LINE__, __FUNCTION__,   \
                format, ##__VA_ARGS__)

#define ESP_LOGV(tag, format, ...)                                              \
  AppLog::write(AppLog::Level::Verbose, tag, __FILE__, __LINE__, __FUNCTION__, \
                format, ##__VA_ARGS__)

#endif // APP_LOG_H
