#ifndef NORMAL_MODE_H
#define NORMAL_MODE_H

#include <Arduino.h>

class NormalMode {
public:
  static void enter();
  static void exit();
  static void run(void *param);
};

#endif
