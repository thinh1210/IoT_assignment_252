#ifndef PLANT_CARE_STATE_H
#define PLANT_CARE_STATE_H

#include <Arduino.h>

extern volatile int globalPlantCareAction;
extern volatile float globalPlantCareConfidence;
extern volatile bool globalPlantCareReady;

#endif // PLANT_CARE_STATE_H
