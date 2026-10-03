#include <string.h>

#include "blowfish.h"

static uint32_t byteSwap(uint32_t x);

#include "blowfish_init.dat"


static uint32_t byteSwap(uint32_t x) {
	uint32_t a = x & 0xFF000000;
	uint32_t b = x & 0x00FF0000;
	uint32_t c = x & 0x0000FF00;
	uint32_t d = x & 0x000000FF;
	return (a >> 24) | (b >> 8) | (c << 8) | (d << 24);
}


void Blowfish_Init(BlowfishCtx* ctx, uint32_t game_code, int level) {
	memcpy(ctx, &sBlowfishInit, sizeof(BlowfishCtx));
	
	uint32_t keycode[3];
	keycode[0] = game_code;
	keycode[1] = game_code >> 1;
	keycode[2] = game_code << 1;
	
	for (int l = 0; l < level; l++) {
		if (l == 2) {
			keycode[1] <<= 1;
			keycode[2] >>= 1;
		}
		
		Blowfish_Encrypt(ctx, &keycode[1]); // ( keycode[1], keycode[2] )
		Blowfish_Encrypt(ctx, &keycode[0]); // ( keycode[0], keycode[1] )
		
		int32_t k0 = byteSwap(keycode[0]);
		int32_t k1 = byteSwap(keycode[1]);
		for (int i = 0; i < 18; i += 2) {
			ctx->P[i]     ^= k0;
			ctx->P[i + 1] ^= k1;
		}
		
		uint32_t tmp[2] = { 0, 0 };
		
		// Generate P-array
		for (int i = 0; i < 18; i += 2) {
			Blowfish_Encrypt(ctx, &tmp[0]);
			ctx->P[i]     = tmp[1];
			ctx->P[i + 1] = tmp[0];
		}
		
		// Generate S-boxes
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 256; j += 2) {
				Blowfish_Encrypt(ctx, &tmp[0]);
				ctx->S[i][j]     = tmp[1];
				ctx->S[i][j + 1] = tmp[0];
			}
		}
	}
}


void Blowfish_Encrypt(const BlowfishCtx* ctx, uint32_t* datap) {
	uint32_t x = datap[1];
	uint32_t y = datap[0];
	
	for (int i = 0; i < 16; i++) {
		x ^= ctx->P[i];
		
		uint32_t a = ctx->S[0][(uint8_t)(x >> 24)];
		uint32_t b = ctx->S[1][(uint8_t)(x >> 16)];
		uint32_t c = ctx->S[2][(uint8_t)(x >> 8)];
		uint32_t d = ctx->S[3][(uint8_t)(x)];
		y ^= ((a + b) ^ c) + d;
		
		uint32_t tmp = x;
		x = y;
		y = tmp;
	}
	
	datap[0] = x ^ ctx->P[16];
	datap[1] = y ^ ctx->P[17];
}


void Blowfish_Decrypt(const BlowfishCtx* ctx, uint32_t* datap) {
	uint32_t x = datap[1];
	uint32_t y = datap[0];
	
	for (int i = 17; i > 1; i--) {
		x ^= ctx->P[i];
		
		uint32_t a = ctx->S[0][(uint8_t)(x >> 24)];
		uint32_t b = ctx->S[1][(uint8_t)(x >> 16)];
		uint32_t c = ctx->S[2][(uint8_t)(x >> 8)];
		uint32_t d = ctx->S[3][(uint8_t)(x)];
		y ^= ((a + b) ^ c) + d;
		
		uint32_t tmp = x;
		x = y;
		y = tmp;
	}
	
	datap[0] = x ^ ctx->P[1];
	datap[1] = y ^ ctx->P[0];
}
