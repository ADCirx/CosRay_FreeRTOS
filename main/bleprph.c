#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "bleprph.h"

static const char *TAG = "NimBLEModule";
static uint16_t conn_handle = BLE_HS_CONN_HANDLE_NONE;
static bool ble_connected = false;

// 模拟谬子数据包
#pragma pack(push, 1)
typedef struct {
    uint8_t header[3];      // 包头 0xAA, 0xBB, 0xCC
    uint16_t energy;        // 谬子能量 (2字节)
    uint64_t cpu_time;      // CPU时间 (8字节)
    uint32_t pps;           // PPS时间 (4字节)
    uint32_t utc_timestamp; // UTC时间戳 (4字节)
    uint8_t gps_info[16];   // GPS信息 (16字节)
    uint8_t reserved[470];  // 预留空间 (470字节)
    uint8_t footer[3];      // 包尾 0xDD, 0xEE, 0xFF
    uint16_t crc;           // CRC校验 (2字节)
} MuonPackage_t;
#pragma pack(pop)

// 自定义服务UUID
static const ble_uuid128_t muon_service_uuid = {
    .u = {.type = BLE_UUID_TYPE_128},
    .value = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
              0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}
};

// 数据特征UUID
static const ble_uuid128_t data_char_uuid = {
    .u = {.type = BLE_UUID_TYPE_128},
    .value = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
              0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x09}
};

static uint16_t data_char_handle;

// GAP事件处理
static int gap_event_cb(struct ble_gap_event *event, void *arg)
{
    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI(TAG, "Device connected");
        conn_handle = event->connect.conn_handle;
        ble_connected = true;
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "Device disconnected");
        conn_handle = BLE_HS_CONN_HANDLE_NONE;
        ble_connected = false;
        break;
    default:
        break;
    }
    return 0;
}

// 开始广播
static void start_advertising(void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    int rc;
    
    // 设置广播数据
    memset(&fields, 0, sizeof(fields));
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
    
    // 设备名称
    fields.name = (uint8_t *)"MuonDetector";
    fields.name_len = strlen("MuonDetector");
    fields.name_is_complete = 1;
    
    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        ESP_LOGE(TAG, "Error setting adv data: %d", rc);
        return;
    }
    
    // 开始广播
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    
    rc = ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER,
                          &adv_params, gap_event_cb, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "Error enabling advertising: %d", rc);
    }
}

// CRC16计算
static uint16_t calculate_crc(const uint8_t *data, size_t length)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

// 生成测试数据
static void generate_test_data(MuonPackage_t *packet)
{
    // 设置包头
    packet->header[0] = 0xAA;
    packet->header[1] = 0xBB;
    packet->header[2] = 0xCC;
    
    // 固定测试数据，计划替换为实际测得的数据
    packet->energy = 12345;           // 固定能量值
    packet->cpu_time = 9876543210;    // 固定CPU时间
    packet->pps = 1234567890;         // 固定PPS时间
    packet->utc_timestamp = 1640995200; // 固定UTC时间戳 (2022-01-01 00:00:00)
    
    // 固定测试的GPS信息 16字节
    for (int i = 0; i < 16; i++) {
        packet->gps_info[i] = 0x10 + i; // 0x10, 0x11, 0x12, ...
    }
    
    // 填充保留空间，之后也可以替换为13个谬子信息数据组，留出28字节
    for (int i = 0; i < 470; i++) {
        packet->reserved[i] = i % 256;
    }
    
    // 设置包尾
    packet->footer[0] = 0xDD;
    packet->footer[1] = 0xEE;
    packet->footer[2] = 0xFF;
    
    // 计算CRC 校验
    packet->crc = calculate_crc((uint8_t*)packet, sizeof(MuonPackage_t) - 2);
}

// 特征访问回调
static int data_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                         struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
        // 生成并返回测试数据
        MuonPackage_t packet;
        generate_test_data(&packet);

        int rc = os_mbuf_append(ctxt->om, (uint8_t*)&packet, sizeof(packet));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    return 0;
}

// GATT服务定义
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &muon_service_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = &data_char_uuid.u,
                .access_cb = data_access_cb,
                .flags = BLE_GATT_CHR_F_READ,
                .val_handle = &data_char_handle,
            },
            {0}
        }
    },
    {0}
};

// GATT服务初始化
static int gatt_svr_init(void)
{
    int rc;
    
    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }
    
    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }
    
    return 0;
}

// BLE同步回调
static void on_sync(void)
{
    int rc;
    
    // 设置设备名称
    rc = ble_svc_gap_device_name_set("MuonDetector");
    if (rc != 0) {
        ESP_LOGE(TAG, "Error setting device name: %d", rc);
        return;
    }
    // 开始广播
    start_advertising();
    
    ESP_LOGI(TAG, "BLE initialized and advertising");
}

// BLE 模块任务
void AppBlueTooth(void *pvParameters)
{
    ESP_LOGI(TAG, "BLE Host Task Started");
    nimble_port_run();
    while (1) {
        ESP_LOGI(TAG, "Device is %s", ble_connected ? "connected" : "advertising");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
    nimble_port_freertos_deinit();
}

esp_err_t InitBlueTooth(void)
{
    // 初始化 NVS Flash
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 初始化 NimBLE
    ret = nimble_port_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init NimBLE: %d", ret);
        return ret;
    }

    // 初始化 GATT 服务器
    ble_hs_cfg.sync_cb = on_sync;
    int rc = gatt_svr_init();
    if (rc != 0) {
        ESP_LOGE(TAG, "Error initializing GATT server: %d", rc);
        return ESP_FAIL;
    }
    return ESP_OK;
}