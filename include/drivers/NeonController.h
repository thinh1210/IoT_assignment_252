#ifndef NEON_CONTROLLER_H
#define NEON_CONTROLLER_H

#include <Adafruit_NeoPixel.h>
#include "config.h"

#define NEO_NUM_PIXELS 1

class NeonController {
public:
    static void init();
    static void setNextColor();
    static void showColor(uint8_t r, uint8_t g, uint8_t b);

private:
    static Adafruit_NeoPixel strip;
    static int colorIndex;
    
    struct RGB {
        uint8_t r, g, b;
    };
    
    static const RGB colors[];
    static const int numColors;
};

#endif // NEON_CONTROLLER_H
