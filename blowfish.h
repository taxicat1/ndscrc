#ifndef BLOWFISH_H
#define BLOWFISH_H

#include <stdint.h>

typedef struct {
	uint32_t  P[18];
	uint32_t  S[4][256];
} BlowfishCtx;

void Blowfish_Init(BlowfishCtx* ctx, uint32_t game_code, int level);
void Blowfish_Encrypt(const BlowfishCtx* ctx, uint32_t* datap);
void Blowfish_Decrypt(const BlowfishCtx* ctx, uint32_t* datap);

#endif /* BLOWFISH_H */
