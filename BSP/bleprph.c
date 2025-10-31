
#include "bsp.h"
#include "bleprph.h"
#include "typedefs.h"

static const char *TAG = "NimBLEModule";
static uint16_t ConnHandle = BLE_HS_CONN_HANDLE_NONE;
static bool BLEConnected = false;


// 自定义服务UUID
static const ble_uuid128_t MuonServiceUUID = {
	.u = {.type = BLE_UUID_TYPE_128},
	.value = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x02, 0x03,
			  0x04, 0x05, 0x06, 0x07, 0x08}};

// 数据特征UUID
static const ble_uuid128_t DataCharUUID = {
	.u = {.type = BLE_UUID_TYPE_128},
	.value = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x02, 0x03,
			  0x04, 0x05, 0x06, 0x07, 0x09}};

static uint16_t DataCharHandle;

// GAP事件处理
static int GAPEventCallback(struct ble_gap_event *Event, void *Arg) {
	switch (Event->type) {
	case BLE_GAP_EVENT_CONNECT:
		ESP_LOGI(TAG, "Device connected");
		ConnHandle = Event->connect.conn_handle;
		BLEConnected = true;
		break;
	case BLE_GAP_EVENT_DISCONNECT:
		ESP_LOGI(TAG, "Device disconnected");
		ConnHandle = BLE_HS_CONN_HANDLE_NONE;
		BLEConnected = false;
		break;
	default:
		break;
	}
	return 0;
}

// 开始广播
static void StartAdvertising(void) {
	struct ble_gap_adv_params AdvParams;
	struct ble_hs_adv_fields Fields;
	int rc;

	// 设置广播数据
	memset(&Fields, 0, sizeof(Fields));
	Fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
	Fields.tx_pwr_lvl_is_present = 1;
	Fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

	// 设备名称
	Fields.name = (uint8_t *)"MuonDetector";
	Fields.name_len = strlen("MuonDetector");
	Fields.name_is_complete = 1;

	rc = ble_gap_adv_set_fields(&Fields);
	if (rc != 0) {
		ESP_LOGE(TAG, "Error setting adv data: %d", rc);
		return;
	}

	// 开始广播
	memset(&AdvParams, 0, sizeof(AdvParams));
	AdvParams.conn_mode = BLE_GAP_CONN_MODE_UND;
	AdvParams.disc_mode = BLE_GAP_DISC_MODE_GEN;

	rc = ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER,
						&AdvParams, GAPEventCallback, NULL);
	if (rc != 0) {
		ESP_LOGE(TAG, "Error enabling advertising: %d", rc);
	}
}

// CRC16计算
static uint16_t CalcCRC(const uint8_t *data, size_t length) {
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
static void GenTestData(MuonPackage_t *packet) {
	// 设置包头
	packet->head[0] = 0xAA;
	packet->head[1] = 0xBB;
	packet->head[2] = 0xCC;

	// 固定测试数据，计划替换为实际测得的数据
	MuonData_t MuonDataCur;  //提前在栈中申请空间，避免反复申请空间
	// 固定测试的GPS信息 16字节
	for (int i = 0; i < 16; i++) {
		MuonDataCur = packet->MuonData[i];
		MuonDataCur.cpu_time = 174532421;
		MuonDataCur.pps = 104;
		MuonDataCur.utc = 1640995200; // 固定UTC时间戳 (2022-01-01 00:00:00)
		MuonDataCur.gps_lat = 1163268; //需要除1e4，经度取到小数点后4位
		MuonDataCur.gps_long = 400037; 
		//清华经纬度
		MuonDataCur.gps_alt = 0; //海拔为0
		MuonDataCur.acc_x = 0;
		MuonDataCur.acc_y = 0;
		MuonDataCur.acc_z = 98; //需要除10，重力加速度
	}

	// 填充保留空间
	for (int i = 0; i < 7; i++) {
		packet->reserved[i] = 0xF5;
	}

	// 设置包尾
	packet->tail[0] = 0xDD;
	packet->tail[1] = 0xEE;
	packet->tail[2] = 0xFF;

	// 计算CRC 校验
	packet->crc = CalcCRC((uint8_t *)packet, sizeof(MuonPackage_t) - 2);
}

// 特征访问回调
static int DataAccessCallback(uint16_t ConnHandle, uint16_t attr_handle,
							struct ble_gatt_access_ctxt *ctxt, void *arg) {
	if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
		// 生成并返回测试数据
		MuonPackage_t packet;
		GenTestData(&packet);

		int rc = os_mbuf_append(ctxt->om, (uint8_t *)&packet, sizeof(packet));
		return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
	}

	return 0;
}

// GATT服务定义
static const struct ble_gatt_svc_def GATTServerServices[] = {
	{
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &MuonServiceUUID.u,
        .characteristics = (struct ble_gatt_chr_def[]){{
            .uuid = &DataCharUUID.u,
            .access_cb = DataAccessCallback,
            .flags = BLE_GATT_CHR_F_READ,
            .val_handle = &DataCharHandle,
        },
        {0}}
    },
	{0}
};

// GATT服务初始化
static int InitGATTServer(void) {
	int rc;
	rc = ble_gatts_count_cfg(GATTServerServices);
	if (rc != 0) {
		return rc;
	}
	rc = ble_gatts_add_svcs(GATTServerServices);
	if (rc != 0) {
		return rc;
	}
	return 0;
}

// BLE同步回调
static void OnSyncCallback(void) {
	int rc;
	// 设置设备名称
	rc = ble_svc_gap_device_name_set("MuonDetector");
	if (rc != 0) {
		ESP_LOGE(TAG, "Error setting device name: %d", rc);
		return;
	}
	// 开始广播
	StartAdvertising();

	ESP_LOGI(TAG, "BLE initialized and advertising");
}



esp_err_t InitBlueTooth(void) {
	// 初始化 NVS Flash
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
		ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
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
	ble_hs_cfg.sync_cb = OnSyncCallback;
	int rc = InitGATTServer();
	if (rc != 0) {
		ESP_LOGE(TAG, "Error initializing GATT server: %d", rc);
		return ESP_FAIL;
	}
	return ESP_OK;
}