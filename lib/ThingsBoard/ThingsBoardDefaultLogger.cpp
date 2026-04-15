// Header include.
#include "ThingsBoardDefaultLogger.h"
#include "../../include/Common/AppLog.h"

// Local includes.
#include "Configuration.h"

// Library include.
#if THINGSBOARD_ENABLE_PROGMEM
#include <WString.h>
#endif // THINGSBOARD_ENABLE_PROGMEM
#include <stdio.h>
#include <string.h>


// Log messages.
#if THINGSBOARD_ENABLE_PROGMEM
constexpr char LOG_MESSAGE_FORMAT[] PROGMEM = "[TB] %s\n";
#else
constexpr char LOG_MESSAGE_FORMAT[] = "[TB] %s\n";
#endif // THINGSBOARD_ENABLE_PROGMEM


void ThingsBoardDefaultLogger::log(const char *msg) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), LOG_MESSAGE_FORMAT, msg != nullptr ? msg : "");
    AppLog::writeRaw(buffer);
}
