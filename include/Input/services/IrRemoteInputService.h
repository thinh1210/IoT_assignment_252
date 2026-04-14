#ifndef IR_REMOTE_INPUT_SERVICE_H
#define IR_REMOTE_INPUT_SERVICE_H

#include "Common/Events.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

class IrRemoteInputService {
public:
  static void task_ir_remote_poll(void *param);
};

#endif // IR_REMOTE_INPUT_SERVICE_H
