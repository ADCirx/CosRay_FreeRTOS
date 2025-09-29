#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "config.h"

// 任务句柄声明
TaskHandle_t dataProcessTaskHandle;
TaskHandle_t bluetoothTaskHandle;
TaskHandle_t dataStoreTaskHandle;
TaskHandle_t telTaskHandle;

uint8_t gpsBuffer[256];


/*!
 * \brief
 *
 */
static void GpsReceiveInterrupt(void *pvParameters);


static void GpsInterruptSetup(void);

/*!
 * \brief
 *
 */
static void PpsReceiveInterrupt(void *pvParameters);


static void PpsInterruptSetup(void);

/*!
 * \brief
 *
 */
static void MuonReceiveInterrupt(void *pvParameters);


static void MuonInterruptSetup(void);

/*!
 * \brief
 *
 */
static void AppDataProcess(void *pvParameters);

/*!
 * \brief
 *
 */
static void AppBlueTooth(void);

/*!
 * \brief
 *
 */
static void AppDataStore(void);

/*!
 * \brief
 *
 */
static void AppDataTEL(void);

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

void app_main(void)
{
	ESP_LOGI(TAG, "FreeRTOS Application Starting...");
	AppSetup();
	InterruptSetup();
	ESP_LOGI(TAG, "All tasks created, scheduler will start");

	while (1)
    {
		ESP_LOGI(TAG, "error");
	}
}

void InterruptSetup(void)
{
    ESP_LOGI(TAG, "Setting up interrupts...");
    
    GpsInterruptSetup();
    ESP_LOGI(TAG, "GPS interrupt setup completed");
    
    PpsInterruptSetup();
    ESP_LOGI(TAG, "PPS interrupt setup completed");
    
    MuonInterruptSetup();
    ESP_LOGI(TAG, "Muon interrupt setup completed");
    
    ESP_LOGI(TAG, "All interrupts setup completed");
}

void AppSetup(void)
{
    // 创建GPS接收任务
    BaseType_t ret = xTaskCreate(
        AppDataProcess,
        "DataProcess_Task",
        DATA_PROCESS_TASK_STACK_SIZE,
        NULL,
        DATA_PROCESS_TASK_PRIORITY,
        &dataProcessTaskHandle);
	if (ret != pdPASS) {
		ESP_LOGE(TAG, "Failed to create Task1");
		return;
	}
	ESP_LOGI(TAG, "Task1 created successfully");
	// 创建蓝牙任务
    ret = xTaskCreate(
        AppBlueTooth,
        "Bluetooth_Task",
        BLUETOOTH_TASK_STACK_SIZE,
        NULL,
        BLUETOOTH_TASK_PRIORITY,
        &bluetoothTaskHandle);
	if (ret != pdPASS) {
		ESP_LOGE(TAG, "Failed to create Task1");
		return;
	}
	ESP_LOGI(TAG, "Task1 created successfully");
	// 创建数据存储任务
    ret = xTaskCreate(
        AppDataStore,
        "DataStore_Task",
        DATA_STORE_TASK_STACK_SIZE,
        NULL,
        DATA_STORE_TASK_PRIORITY,
        &dataStoreTaskHandle);
	if (ret != pdPASS) {
		ESP_LOGE(TAG, "Failed to create Task1");
		return;
	}
	ESP_LOGI(TAG, "Task1 created successfully");
	ret = xTaskCreate(
        AppDataTEL,
        "DataTEL_Task",
        DATA_TEL_TASK_STACK_SIZE,
        NULL,
        DATA_TEL_TASK_PRIORITY,
        &telTaskHandle);
	if (ret != pdPASS) {
		ESP_LOGE(TAG, "Failed to create Task1");
		return;
	}
	ESP_LOGI(TAG, "Task1 created successfully");
	}


