/*
 * bsp.c
 *
 *  Created on: 2025年10月31日
 *      Author: hp
 */

#include "bsp.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "BSPModule";

esp_err_t BSPInit(void) {
	for (size_t i = 0; i < CMD_BUFFER_SIZE; i++) {
		RxBuffer[i] = 0;
	}
	for (size_t i = 0; i < DATA_BUFFER_SIZE * 2; i++) {
		TxBuffer[i] = 0;
	}
	TxBufferReadPtr = TxBuffer;
	TxBufferWritePtr = TxBuffer + DATA_BUFFER_SIZE;
	ESP_LOGI(TAG, "Initializing BlueTooth Peripheral");
	esp_err_t ESPRet = InitBlueTooth();
	if (ESPRet != ESP_OK) {
		ESP_LOGE(TAG, "Failed to initialize BlueTooth Peripheral");
		return ESPRet;
	}
	return ESP_OK;
}
