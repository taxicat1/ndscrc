#include <stdlib.h>

#include "nds_file.h"

#include "blowfish.h"
#include "crc16.h"

#define HEADER_OFFSET       (0)
#define HEADER_SIZE         (0x160)

#define SECURE_AREA_SIZE    (0x4000)

#define SECURE_AREA_ENCRYPTED_SIZE  (0x800)

#define SECURE_AREA_START_OVERWRITE  (0xE7FFDEFF)

#define SECURE_AREA_START_PT1  (0x72636E65) // "encr"
#define SECURE_AREA_START_PT2  (0x6A624F79) // "yObj"

static uint16_t getu16LE(const uint8_t* src);
static void putu16LE(uint8_t* dst, uint16_t src);
static uint32_t getu32LE(const uint8_t* src);
static void putu32LE(uint8_t* dst, uint32_t src);
static void encryptSecureArea(uint32_t game_code, uint8_t* secure_area);


int NDSFile_Open(NDSFile* nds, const char* path) {
	if (nds != NULL) {
		nds->file_handle = fopen(path, "rb+");
		if (nds->file_handle != NULL) {
			return 0;
		}
	}
	
	return 1;
}


void NDSFile_Close(NDSFile* nds) {
	if (nds != NULL && nds->file_handle != NULL) {
		fclose(nds->file_handle);
	}
}


static uint16_t getu16LE(const uint8_t* src) {
	return (uint16_t)src[0] | ((uint16_t)src[1] << 8);
}


static void putu16LE(uint8_t* dst, uint16_t src) {
	dst[0] = src;
	dst[1] = src >> 8;
}


static uint32_t getu32LE(const uint8_t* src) {
	return (uint32_t)src[0] | ((uint32_t)src[1] << 8) | ((uint32_t)src[2] << 16) | ((uint32_t)src[3] << 24);
}


static void putu32LE(uint8_t* dst, uint32_t src) {
	dst[0] = src;
	dst[1] = src >> 8;
	dst[2] = src >> 16;
	dst[3] = src >> 24;
}


static void encryptSecureArea(uint32_t game_code, uint8_t* secure_area) {
	// "encryObj" into header
	putu32LE(&secure_area[0], SECURE_AREA_START_PT1);
	putu32LE(&secure_area[4], SECURE_AREA_START_PT2);
	
	BlowfishCtx bf;
	uint32_t data_block[2];
	
	// Encrypt syscall region at level 3
	Blowfish_Init(&bf, game_code, 3);
	for (int i = 0; i < SECURE_AREA_ENCRYPTED_SIZE; i += 8) {
		data_block[0] = getu32LE(&secure_area[i]);
		data_block[1] = getu32LE(&secure_area[i + 4]);
		
		Blowfish_Encrypt(&bf, &data_block[0]);
		
		putu32LE(&secure_area[i],     data_block[0]);
		putu32LE(&secure_area[i + 4], data_block[1]);
	}
	
	// Encrypt header at level 2 (single block)
	Blowfish_Init(&bf, game_code, 2);
	
	data_block[0] = getu32LE(&secure_area[0]);
	data_block[1] = getu32LE(&secure_area[4]);
	
	Blowfish_Encrypt(&bf, &data_block[0]);
	
	putu32LE(&secure_area[0], data_block[0]);
	putu32LE(&secure_area[4], data_block[1]);
}


int NDSFile_FixCRCs(NDSFile* nds) {
	int ret = 0;
	uint8_t* header_buffer = NULL;
	uint8_t* secure_area_buffer = NULL;
	
	if (nds == NULL || nds->file_handle == NULL) {
		ret = 1;
		goto cleanup;
	}
	
	header_buffer = malloc(HEADER_SIZE);
	if (header_buffer == NULL) {
		ret = 1;
		goto cleanup;
	}
	
	secure_area_buffer = malloc(SECURE_AREA_SIZE);
	if (secure_area_buffer == NULL) {
		ret = 1;
		goto cleanup;
	}
	
	// Seek to and read header
	if (fseek(nds->file_handle, HEADER_OFFSET, SEEK_SET) != 0) {
		ret = 1;
		goto cleanup;
	}
	
	if (fread(header_buffer, HEADER_SIZE, 1, nds->file_handle) != 1) {
		ret = 1;
		goto cleanup;
	}
	
	// Quick sanity check on header
	if (getu16LE(&header_buffer[0x15C]) != 0xCF56) {
		ret = 1;
		goto cleanup;
	}
	
	// Seek to and read secure area (expected to be at offset 0x4000)
	unsigned int secure_area_offset = getu32LE(&header_buffer[0x20]);
	if (fseek(nds->file_handle, secure_area_offset, SEEK_SET) != 0) {
		ret = 1;
		goto cleanup;
	}
	
	if (fread(secure_area_buffer, SECURE_AREA_SIZE, 1, nds->file_handle) != 1) {
		ret = 1;
		goto cleanup;
	}
	
	// Validate top of secure area
	if (
		getu32LE(&secure_area_buffer[0]) != SECURE_AREA_START_OVERWRITE ||
		getu32LE(&secure_area_buffer[4]) != SECURE_AREA_START_OVERWRITE
	) {
		// Invalid or encrypted secure area, do nothing
		goto cleanup;
	}
	
	// Get game code from header
	uint32_t game_code = getu32LE(&header_buffer[0xC]);
	
	// Encrypt secure area
	encryptSecureArea(game_code, secure_area_buffer);
	
	// Calculate secure area CRC
	uint16_t secure_area_crc = CRC16_Calc(&secure_area_buffer[0], SECURE_AREA_SIZE);
	
	// Put this into the header
	putu16LE(&header_buffer[0x6C], secure_area_crc);
	
	// Calculate the new header CRC
	uint16_t header_crc = CRC16_Calc(&header_buffer[0], HEADER_SIZE - 2);
	
	// Put that into the header
	putu16LE(&header_buffer[HEADER_SIZE - 2], header_crc);
	
	// Write new header contents back to the file
	if (fseek(nds->file_handle, HEADER_OFFSET, SEEK_SET) != 0) {
		ret = 1;
		goto cleanup;
	}
	
	if (fwrite(header_buffer, HEADER_SIZE, 1, nds->file_handle) != 1) {
		ret = 1;
		goto cleanup;
	}
	
cleanup:
	free(secure_area_buffer);
	free(header_buffer);
	
	return ret;
}
