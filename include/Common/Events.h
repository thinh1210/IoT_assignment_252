#ifndef EVENTS_H
#define EVENTS_H

#include <Arduino.h>

// Define system modes
enum class SystemMode {
    MODE_A,
    MODE_B
};

// Event types for communication between Input and Processing
enum class EventType {
    NONE,
    SENSOR_DATA_READY,
    BUTTON_PRESSED,
    SYSTEM_ALERT,
    MODE_CHANGE
};

// Struct to hold event
struct SystemEvent {
    EventType type;
    SystemMode newMode; // Useful for passing mode
};

#endif // EVENTS_H
