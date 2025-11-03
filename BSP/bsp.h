#ifndef BSP_H
#define BSP_H

#include "bleprph.h"
#include "config.h"
#include "typedefs.h"

esp_err_t BSPInit(void);

// 缓冲区定义
extern volatile uint8_t RxBuffer[CMD_BUFFER_SIZE];
extern volatile uint8_t TxBuffer[DATA_BUFFER_SIZE * 2];
extern volatile uint8_t gpsBuffer[256];

#endif // BSP_H