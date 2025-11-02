#include "bleprph.h"
#include "bsp.h"

static const char* TAG="NimBLEModule";
static uint16_t ConnHandle = BLE_HS_CONN_HANDLE_NONE;
static bool BLEConnected = false;

// GAP事件处理
static int GAPEventCallback(struct ble_gap_event *Event, void *Arg);

// 特征访问回调
static int DataAccessCallback(uint16_t ConnHandle, uint16_t attr_handle,
							  struct ble_gatt_access_ctxt *ctxt, void *arg);

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

// 命令特征UUID
static const ble_uuid128_t CMDCharUUID = {
	.u = {.type = BLE_UUID_TYPE_128},
	.value = {0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x02, 0x03, 0x04,
			  0x05, 0x06, 0x07, 0x09, 0x01}};

// 特征句柄
static uint16_t DataCharValHandle;
static uint16_t CMDCharValHandle;
static uint16_t DataCharAttrHandle;
static uint16_t CMDCharAttrHandle;

// GATT服务定义
static const struct ble_gatt_svc_def GATTServerServices[] = {
	{
		.type = BLE_GATT_SVC_TYPE_PRIMARY,
		.uuid = &MuonServiceUUID.u,
		.characteristics =
		(struct ble_gatt_chr_def[]){
			{
				.uuid = &DataCharUUID.u,
				.access_cb = DataAccessCallback,
				.flags = BLE_GATT_CHR_F_READ,
				.val_handle = &DataCharValHandle,
			},
			{
				.uuid = &CMDCharUUID.u,
				.access_cb = DataAccessCallback,
				.flags = BLE_GATT_CHR_F_WRITE,
				.val_handle = &CMDCharValHandle
			},
			{0}
		}
	},
	{0}
};

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

// GATT 服务初始化
static int InitGATTServer(void) {
	int rc;
	rc = ble_gatts_count_cfg(GATTServerServices);
	if (rc) {
		return rc;
	}
	rc = ble_gatts_add_svcs(GATTServerServices);
	if (rc) {
		return rc;
	}
	ble_gatts_find_chr(&MuonServiceUUID.u, &DataCharUUID.u, NULL, &DataCharAttrHandle);
	ble_gatts_find_chr(&MuonServiceUUID.u, &CMDCharUUID.u, NULL, &CMDCharAttrHandle);
	return 0;
}

// BLE 同步回调
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
		ESP_LOGE(TAG, "Failed to initialize NimBLE: %d", ret);
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

// Data 特征 read 访问回调
static int DataAccessCallback(uint16_t ConnHandle, uint16_t attr_handle,
							  struct ble_gatt_access_ctxt *ctxt, void *arg) {
	if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
		if (attr_handle != DataCharAttrHandle) {
			ESP_LOGE(TAG, "Read from invalid handle");
			return BLE_ATT_ERR_READ_NOT_PERMITTED;
		}
		// TODO DATA->TxBuffer
		int rc = os_mbuf_append(ctxt->om, &TxBuffer, DATA_BUFFER_SIZE);
		return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
	}
	else if (ctxt->op) {
		if (attr_handle != CMDCharAttrHandle) {
			ESP_LOGE(TAG, "Write to invalid handle");
			return BLE_ATT_ERR_WRITE_NOT_PERMITTED;
		}
		int len = OS_MBUF_PKTLEN(ctxt->om);
		if (len > CMD_BUFFER_SIZE) len = CMD_BUFFER_SIZE;
		int rc = os_mbuf_copydata(ctxt->om, 0, len, &RxBuffer);
		if (rc) {
			return BLE_ATT_ERR_UNLIKELY;
		}
		// TODO queue pend
		return 0;
	}
	return 0;
}