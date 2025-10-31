#ifndef TYPEDEFS_PRESENT
#define TYPEDEFS_PRESENT


#include <stdio.h>

// 模拟谬子数据包

typedef struct{
	uint8_t cpu_time[8];  //CPU时钟（8字节，lifetime counter）
	uint8_t energy[2]; //μ子能量，2字节（16位ADC测量值）
	uint8_t pps[4]; //本次上电以来的PPS脉冲计数，4字节
	uint8_t utc[4]; //最近一次收到的utc时间戳，4字节
	uint8_t gps_long[4];  //1m级精度，4字节存储gps经度
	uint8_t gps_lat[4]; //1m级精度，4字节存储gps纬度
	uint8_t gps_alt[2]; //1m级精度，2字节，存储gps海拔
	uint8_t  acc_x;  //加速度x值，1字节
	uint8_t  acc_y; //加速度y值，1字节
	uint8_t  acc_z; //加速度z值，1字节
} MuonData_t;  //每一个μ子事件总共31字节

typedef struct {
	uint8_t head[3];  //0xAA, 0xBB, 0xCC for MuonPackage
	uint8_t valid_count;  //本数据包内到哪一个μ子事件是有效的
	//（因为μ子计数率相对较低，需要周期性保存当前正在写入的数据包，保存时很可能并没有填充满有效值）
	//从0开始，15为本数据包已填充满有效值。如果本数据包内还没有填充有效数据值，则为0xFF。缺省的字节为0xF5
	MuonData_t MuonData[16]; //μ子事件，每个数据包最多能填充16个有效值
	uint8_t reserved[7];
	uint8_t tail[3];   //0xDD, 0xEE, 0xFF for MuonPackage
	uint8_t crc[2];
}MuonPackage_t;


/*
#pragma pack(push, 1)
typedef struct {
	uint8_t header[3];		// 包头 0xAA, 0xBB, 0xCC
	uint16_t energy;		// 谬子能量 (2字节)
	uint64_t cpu_time;		// CPU时间 (8字节)
	uint32_t pps;			// PPS时间 (4字节)
	uint32_t utc_timestamp; // UTC时间戳 (4字节)
	uint8_t gps_info[16];	// GPS信息 (16字节)
	uint8_t reserved[470];	// 预留空间 (470字节)
	uint8_t footer[3];		// 包尾 0xDD, 0xEE, 0xFF
	uint16_t crc;			// CRC校验 (2字节)
} MuonPackage_t;
#pragma pack(pop)
*/

#endif 