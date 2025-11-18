#include "bsp.h"

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
// TODO
static void GenTestData(uint8_t *TxBuffer) {

	// // 设置包头
	// packet->head[0] = 0xAA;
	// packet->head[1] = 0xBB;
	// packet->head[2] = 0xCC;

	// // 固定测试数据，计划替换为实际测得的数据
	// MuonData_t MuonDataCur; // 提前在栈中申请空间，避免反复申请空间
	// // 固定测试的GPS信息 16字节
	// for (int i = 0; i < 16; i++) {
	// 	MuonDataCur = packet->MuonData[i];
	// 	MuonDataCur.cpu_time = 174532421;
	// 	MuonDataCur.pps = 104;
	// 	MuonDataCur.utc = 1640995200;  // 固定UTC时间戳 (2022-01-01 00:00:00)
	// 	MuonDataCur.gps_lat = 1163268; // 需要除1e4，经度取到小数点后4位
	// 	MuonDataCur.gps_long = 400037;
	// 	// 清华经纬度
	// 	MuonDataCur.gps_alt = 0; // 海拔为0
	// 	MuonDataCur.acc_x = 0;
	// 	MuonDataCur.acc_y = 0;
	// 	MuonDataCur.acc_z = 98; // 需要除10，重力加速度
	// }

	// // 填充保留空间
	// for (int i = 0; i < 7; i++) {
	// 	packet->reserved[i] = 0xF5;
	// }

	// // 设置包尾
	// packet->tail[0] = 0xDD;
	// packet->tail[1] = 0xEE;
	// packet->tail[2] = 0xFF;

	// // 计算CRC 校验
	// packet->crc = CalcCRC((uint8_t *)packet, sizeof(MuonPackage_t) - 2);
}
