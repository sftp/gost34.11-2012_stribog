/*
 * Copyright (c) 2013 by sftp
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 * There's ABSOLUTELY NO WARRANTY, express or implied.
 */

#ifndef _STRIBOG_H
#define _STRIBOG_H

#include <string.h>

#include "types.h"
#include "stribog_data.h"

#define BLOCK_SIZE      64
#define BLOCK_BIT_SIZE  (BLOCK_SIZE * 8)

#define OUTPUT_SIZE_512 64
#define OUTPUT_SIZE_256 32

#define HASH256         0
#define HASH512         1


struct stribog_ctx_t {
	u8 h[BLOCK_SIZE];
	u8 N[BLOCK_SIZE];
	u8 S[BLOCK_SIZE];

	u8 size;
};

void addmod512(u8 *dst, u8 *src, u8 *add)
{
	u8 i;
	u16 overrun = 0;

	for (i = BLOCK_SIZE; i-- > 0;) {
		overrun = src[i] + add[i] + (overrun >> 8);
		dst[i] = (u8) overrun;
	}
}

void addmod512_u32(u8 *dst, u8 *src, u32 add)
{
	u8 i;

	for (i = BLOCK_SIZE; i-- > 0;) {
		add = (u32) src[i] + add;
		dst[i] = add;
		add >>= 8;
	}
}

void xor512(u8 *dst, u8 *a, u8 *b)
{
	u8 i;

	for (i = 0; i < BLOCK_SIZE; i++) {
		dst[i] = a[i] ^ b[i];
	}
}

void S(u8 *vect)
{
	u8 i;

	for (i = 0; i < BLOCK_SIZE; i++) {
		vect[i] = sbox[vect[i]];
	}
}

void LP(u8 *vect)
{
	u8 i, j, k;

	u8 tmp[64];

	memcpy(tmp, vect, 64);

	u64 c;

	/*
	 * subvectors of 512-bit vector (64*8 bits)
	 * an subvector is start at [j*8], its componenst placed
	 * with step of 8 bytes (due to this function is composition
	 * of P and L) and have length of 64 bits (8*8 bits)
	 */
	for (i = 0; i < 8; i++) {
		c = 0;

		/*
		 * 8-bit components of 64-bit subvectors
		 * components is placed at [j*8+i]
		 */
		for (j = 0; j < 8; j++) {

			/* bit index of current 8-bit component */
			for (k = 0; k < 8; k++) {

				/* check if current bit is set */
				if (tmp[j*8+i] & 0x80 >> k)
					c ^= A[j*8+k];
			}
		}

		for (j = 0; j < 8; j++) {
			vect[i*8+j] = c >> (7 - j) * 8;
		}
	}
}

void X(u8 *dst, u8 *a, u8 *b)
{
	xor512(dst, a, b);
}

void E(u8 *dst, u8 *k, u8 *m)
{
	u8 i;

	u8 K[64];

	memcpy(K, k, BLOCK_SIZE);

	X(dst, K, m);

	for (i = 1; i < 13; i++) {
		S(dst);
		LP(dst);

		/* next K */
		X(K, K, C[i-1]);

		S(K);
		LP(K);

		X(dst, K, dst);
	}
}

void g_N(u8 *h, u8 *N, u8 *m)
{
	u8 hash[BLOCK_SIZE];
	memcpy(hash, h, BLOCK_SIZE);

	xor512(h, h, N);

	S(h);
	LP(h);

	E(h, h, m);

	xor512(h, h, hash);
	xor512(h, h, m);
}

void g_0(u8 *h, u8 *m)
{
	u8 hash[64];
	memcpy(hash, h, BLOCK_SIZE);

	S(h);
	LP(h);

	E(h, h, m);

	xor512(h, h, hash);
	xor512(h, h, m);
}

void stribog(struct stribog_ctx_t *ctx, u8 *message, u64 len)
{
	u8 m[BLOCK_SIZE];

	u8 padding;

	while (len >= BLOCK_SIZE) {
		memcpy(m, message + len - BLOCK_SIZE, BLOCK_SIZE);

		g_N(ctx->h, ctx->N, m);

		len -= BLOCK_SIZE;

		addmod512_u32(ctx->N, ctx->N, BLOCK_BIT_SIZE);
		addmod512(ctx->S, ctx->S, m);
	}

	padding = BLOCK_SIZE - len;

	if (padding) {
		memset(m, 0x00, padding - 1);
		memset(m + padding - 1, 0x01, 1);
		memcpy(m + padding, message, len);
	}

	g_N(ctx->h, ctx->N, m);

	addmod512_u32(ctx->N, ctx->N, len*8);
	addmod512(ctx->S, ctx->S, m);

	g_0(ctx->h, ctx->N);
	g_0(ctx->h, ctx->S);
}

void init(struct stribog_ctx_t *ctx, u8 size)
{
	memset(ctx->N, 0x00, BLOCK_SIZE);
	memset(ctx->S, 0x00, BLOCK_SIZE);

	ctx->size = !!size;

	if (ctx->size)
		memcpy(ctx->h, iv512, BLOCK_SIZE);
	else
		memcpy(ctx->h, iv256, BLOCK_SIZE);
}

#endif
