#ifndef BLEPRPH_H
#define BLEPRPH_H
#ifdef __cplusplus
extern "C" {
#endif
#include "esp_log.h"
//#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"
#include "host/ble_gap.h"
#include "host/ble_hs.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "nvs_flash.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include <stdio.h>
#include <string.h>

#include "typedefs.h"



// 初始化 BLE 模块
esp_err_t InitBlueTooth(void);
// BLE 任务函数

#ifdef __cplusplus
}
#endif
#endif // BLEPRPH_H
