/*
 * bsp.c
 *
 *  Created on: 2025年10月31日
 *      Author: hp
 */

#include "bsp.h"

static const char* TAG = "BSPModule";

esp_err_t BSPInit(void) {
	ESP_LOGI(TAG, "Initializing BlueTooth Peripheral");
	esp_err_t ESPRet = InitBlueTooth();
	if (ESPRet != ESP_OK) {
		ESP_LOGE(TAG, "Failed to initialize BlueTooth Peripheral");
		return ESPRet;
	}
    return ESP_OK;
}
