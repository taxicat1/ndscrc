#include "crc16.h"

static uint16_t getu16LE(const uint8_t* src);

static uint16_t Crc16Table[16] = {
	0x0000, 0xCC01, 0xD801, 0x1400,
	0xF001, 0x3C00, 0x2800, 0xE401,
	0xA001, 0x6C00, 0x7800, 0xB401,
	0x5000, 0x9C01, 0x8801, 0x4400
};


static uint16_t getu16LE(const uint8_t* src) {
	return (uint16_t)src[0] | ((uint16_t)src[1] << 8);
}


uint16_t CRC16_Calc(uint8_t* data, size_t length) {
	uint16_t crc = 0xFFFF;
	
	for (int i = 0; i < length; i += 2) {
		uint16_t hword = getu16LE(&data[i]);
		
		crc = (crc >> 4) ^ Crc16Table[crc & 15] ^ Crc16Table[(hword)       & 15];
		crc = (crc >> 4) ^ Crc16Table[crc & 15] ^ Crc16Table[(hword >> 4)  & 15];
		crc = (crc >> 4) ^ Crc16Table[crc & 15] ^ Crc16Table[(hword >> 8)  & 15];
		crc = (crc >> 4) ^ Crc16Table[crc & 15] ^ Crc16Table[(hword >> 12) & 15];
	}
	
	return crc;
}
