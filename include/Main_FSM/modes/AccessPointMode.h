#ifndef ACCESSPOINT_MODE_H
#define ACCESSPOINT_MODE_H

#include <Arduino.h>

class AccessPointMode {
public:
  static void enter();
  static void exit();
  static void run(void *param);
};

#endif
