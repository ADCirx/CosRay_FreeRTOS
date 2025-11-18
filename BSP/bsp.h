#ifndef BSP_H
#define BSP_H

#include "config.h"
#include "typedefs.h"

// 缓冲区定义
extern volatile uint8_t RxBuffer[CMD_BUFFER_SIZE];
extern volatile uint8_t TxBuffer[DATA_BUFFER_SIZE * 2];
extern volatile uint8_t* TxBufferReadPtr;
extern volatile uint8_t* TxBufferWritePtr;
extern volatile uint8_t gpsBuffer[256];

// BSP Interface functions
esp_err_t BSPInit(void);

esp_err_t InitBlueTooth(void);
void RunBlueToothHost(void);

esp_err_t InitDataPeripheral(void);
void RunDataPeripheral(void);

#endif // BSP_H