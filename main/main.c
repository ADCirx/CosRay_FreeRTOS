#include "config.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include <inttypes.h>
#include <stdio.h>

#include "bsp.h"

// TAG 变量指向存储在 flash 中的一个字符串字面量
// 见esp_log使用教程：https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32/api-reference/system/log.html
static const char *TAG = "MainModule";

// 任务句柄声明
TaskHandle_t dataProcessTaskHandle;
TaskHandle_t bluetoothTaskHandle;
TaskHandle_t dataStoreTaskHandle;
TaskHandle_t telTaskHandle;

//********************GLOBAL VARS*************************//
volatile uint8_t RxBuffer[CMD_BUFFER_SIZE];
volatile uint8_t TxBuffer[DATA_BUFFER_SIZE * 2];
volatile uint8_t gpsBuffer[256];

static void GpsRxIntTask(void);

static void GpsRxIntSetup(void);

/*!
 * \brief
 *
 */
static void PPSIntTask(void);

static void PPSIntSetup(void);

/*!
 * \brief
 *
 */
static void MuonIntTask(void);

static void MuonInttSetup(void);

/*!
 * \brief
 *
 */
static void AppDataProcess(void *pvParameters);

/*!
 * \brief
 *
 */
static void AppDataStore(void *pvParameters);

/*!
 * \brief
 *
 */
static void AppBlueTooth(void *pvParameters);

/*!
 * \brief
 *
 */
void InterruptSetup(void);

/*!
 * \brief
 *
 */
void AppSetup(void);

void app_main(void) {
	ESP_LOGI(TAG, "FreeRTOS Application Starting...");
	AppSetup();
	InterruptSetup();
	ESP_LOGI(TAG, "All tasks created, scheduler will start");
	// 注意：esp-idf 的 freertos不需要用户启动系统任务调度
	// vTaskStartScheduler();  //Enables task scheduling
	while (1) {
		ESP_LOGI(TAG, "Running...");
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

static void GpsRxIntTask(void) {}

static void GpsRxIntSetup(void) {}

/*!
 * \brief
 *
 */
static void PPSIntTask(void) {}

static void PPSIntSetup(void) {}

/*!
 * \brief
 *
 */
static void MuonIntTask(void) {}

static void MuonInttSetup(void) {}

/*!
 * \brief
 *
 */
static void AppDataProcess(void *pvParameters) {}

/*!
 * \brief
 *
 */
static void AppDataStore(void *pvParameters) {}

/*!
 * \brief
 *
 */
// static void AppDataTEL(void *pvParameters) {}

// BLE 模块任务
static void AppBlueTooth(void *pvParameters) {
	ESP_LOGI(TAG, "BLE Host Task Started");
	nimble_port_run();
	while (1) {
		// ESP_LOGI(TAG, "Device is %s",
		//			BLEConnected ? "connected" : "advertising");
		vTaskDelay(pdMS_TO_TICKS(5000));
	}
	nimble_port_freertos_deinit();
}

void InterruptSetup(void) {
	ESP_LOGI(TAG, "Setting up interrupts...");

	GpsRxIntSetup();
	ESP_LOGI(TAG, "GPS interrupt setup completed");

	PPSIntSetup();
	ESP_LOGI(TAG, "PPS interrupt setup completed");

	MuonInttSetup();
	ESP_LOGI(TAG, "Muon interrupt setup completed");

	ESP_LOGI(TAG, "All interrupts setup completed");
}

void AppSetup(void) {
	esp_err_t ESPRet;
	BaseType_t RTOSRet;

	ESPRet = BSPInit();
	if (ESPRet != ESP_OK) {
		ESP_LOGE(TAG, "Failed to initialize BSP");
		return;
	}

	// 创建GPS接收任务
	// RTOSRet = xTaskCreate(
	// 	AppDataProcess, "DataProcess_Task", DATA_PROCESS_TASK_STACK_SIZE, NULL,
	// 	DATA_PROCESS_TASK_PRIORITY, &dataProcessTaskHandle);
	// if (RTOSRet != pdPASS) {
	// 	ESP_LOGE(TAG, "Failed to create Task1");
	// 	return;
	// }
	// ESP_LOGI(TAG, "Task1 created successfully");

	// 创建蓝牙任务
	ESP_LOGI(TAG, "Creating BlueTooth Task");
	RTOSRet =
		xTaskCreate(AppBlueTooth, "BlueTooth_Task", BLUETOOTH_TASK_STACK_SIZE,
					NULL, BLUETOOTH_TASK_PRIORITY, &bluetoothTaskHandle);
	if (RTOSRet != pdPASS) {
		ESP_LOGE(TAG, "Failed to create BlueTooth Task");
		return;
	}
	ESP_LOGI(TAG, "BlueTooth Task created successfully");

	// 创建数据存储任务
	// RTOSRet =
	// 	xTaskCreate(AppDataStore, "DataStore_Task", DATA_STORE_TASK_STACK_SIZE,
	// 				NULL, DATA_STORE_TASK_PRIORITY, &dataStoreTaskHandle);
	// if (RTOSRet != pdPASS) {
	// 	ESP_LOGE(TAG, "Failed to create Task1");
	// 	return;
	// }
	// ESP_LOGI(TAG, "Task1 created successfully");
	// RTOSRet = xTaskCreate(AppDataTEL, "DataTEL_Task",
	// DATA_TEL_TASK_STACK_SIZE, 				  NULL, DATA_TEL_TASK_PRIORITY, &telTaskHandle);
	// if (RTOSRet != pdPASS) {
	// 	ESP_LOGE(TAG, "Failed to create Task1");
	// 	return;
	// }
	// ESP_LOGI(TAG, "Task1 created successfully");
}
