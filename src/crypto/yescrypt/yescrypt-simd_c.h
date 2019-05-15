/*-
 * Copyright 2009 Colin Percival
 * Copyright 2012-2015 Alexander Peslyak
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file was originally written by Colin Percival as part of the Tarsnap
 * online backup system.
 */

/*
 * On 64-bit, enabling SSE4.1 helps our pwxform code indirectly, via avoiding
 * gcc bug 54349 (fixed for gcc 4.9+).  On 32-bit, it's of direct help.  AVX
 * and XOP are of further help either way.
 */
#ifndef __SSE4_1__
#warning "Consider enabling SSE4.1, AVX, or XOP in the C compiler for significantly better performance"
#endif

#include <emmintrin.h>
#ifdef __XOP__
#include <x86intrin.h>
#endif

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sha256.h"
#include "sysendian.h"

#include "yescrypt.h"

#include "yescrypt-platform_c.h"

#if __STDC_VERSION__ >= 199901L
/* have restrict */
#elif defined(__GNUC__)
#define restrict __restrict
#else
#define restrict
#endif

#ifdef __GNUC__
#define unlikely(exp) __builtin_expect(exp, 0)
#else
#define unlikely(exp) (exp)
#endif

#define PREFETCH(x, hint) _mm_prefetch((const char *)(x), (hint));

#ifdef __XOP__
#define ARX(out, in1, in2, s) \
	out = _mm_xor_si128(out, _mm_roti_epi32(_mm_add_epi32(in1, in2), s));
#else
#define ARX(out, in1, in2, s) \
	{ \
		__m128i T = _mm_add_epi32(in1, in2); \
		out = _mm_xor_si128(out, _mm_slli_epi32(T, s)); \
		out = _mm_xor_si128(out, _mm_srli_epi32(T, 32-s)); \
	}
#endif

#define SALSA20_2ROUNDS \
	/* Operate on "columns" */ \
	ARX(X1, X0, X3, 7) \
	ARX(X2, X1, X0, 9) \
	ARX(X3, X2, X1, 13) \
	ARX(X0, X3, X2, 18) \
\
	/* Rearrange data */ \
	X1 = _mm_shuffle_epi32(X1, 0x93); \
	X2 = _mm_shuffle_epi32(X2, 0x4E); \
	X3 = _mm_shuffle_epi32(X3, 0x39); \
\
	/* Operate on "rows" */ \
	ARX(X3, X0, X1, 7) \
	ARX(X2, X3, X0, 9) \
	ARX(X1, X2, X3, 13) \
	ARX(X0, X1, X2, 18) \
\
	/* Rearrange data */ \
	X1 = _mm_shuffle_epi32(X1, 0x39); \
	X2 = _mm_shuffle_epi32(X2, 0x4E); \
	X3 = _mm_shuffle_epi32(X3, 0x93);

/**
 * Apply the Salsa20/2 core to the block provided in (X0 ... X3).
 */
#define SALSA20_2(out) \
	{ \
		__m128i Y0 = X0; \
		__m128i Y1 = X1; \
		__m128i Y2 = X2; \
		__m128i Y3 = X3; \
		SALSA20_2ROUNDS \
		(out)[0] = X0 = _mm_add_epi32(X0, Y0); \
		(out)[1] = X1 = _mm_add_epi32(X1, Y1); \
		(out)[2] = X2 = _mm_add_epi32(X2, Y2); \
		(out)[3] = X3 = _mm_add_epi32(X3, Y3); \
	}

/**
 * Apply the Salsa20/8 core to the block provided in (X0 ... X3) ^ (Z0 ... Z3).
 */
#define SALSA20_8_XOR_ANY(maybe_decl, Z0, Z1, Z2, Z3, out) \
	X0 = _mm_xor_si128(X0, Z0); \
	X1 = _mm_xor_si128(X1, Z1); \
	X2 = _mm_xor_si128(X2, Z2); \
	X3 = _mm_xor_si128(X3, Z3); \
	{ \
		maybe_decl Y0 = X0; \
		maybe_decl Y1 = X1; \
		maybe_decl Y2 = X2; \
		maybe_decl Y3 = X3; \
		SALSA20_2ROUNDS \
		SALSA20_2ROUNDS \
		SALSA20_2ROUNDS \
		SALSA20_2ROUNDS \
		(out)[0] = X0 = _mm_add_epi32(X0, Y0); \
		(out)[1] = X1 = _mm_add_epi32(X1, Y1); \
		(out)[2] = X2 = _mm_add_epi32(X2, Y2); \
		(out)[3] = X3 = _mm_add_epi32(X3, Y3); \
	}

#define SALSA20_8_XOR_MEM(in, out) \
	SALSA20_8_XOR_ANY(__m128i, (in)[0], (in)[1], (in)[2], (in)[3], out)

#define SALSA20_8_XOR_REG(out) \
	SALSA20_8_XOR_ANY(/* empty */, Y0, Y1, Y2, Y3, out)

typedef union {
	uint32_t w[16];
	__m128i q[4];
} salsa20_blk_t;

/**
 * blockmix_salsa8(Bin, Bout, r):
 * Compute Bout = BlockMix_{salsa20/8, r}(Bin).  The input Bin must be 128r
 * bytes in length; the output Bout must also be the same size.
 */
static void
blockmix_salsa8(const salsa20_blk_t *restrict Bin,
    salsa20_blk_t *restrict Bout, size_t r)
{
	size_t i;
	__m128i X0, X1, X2, X3;

	r--;
	PREFETCH(&Bin[r * 2 + 1], _MM_HINT_T0)
	for (i = 0; i < r; i++) {
		PREFETCH(&Bin[i * 2], _MM_HINT_T0)
		PREFETCH(&Bin[i * 2 + 1], _MM_HINT_T0)
	}
	PREFETCH(&Bin[r * 2], _MM_HINT_T0)

	/* 1: X <-- B_{2r - 1} */
	X0 = Bin[r * 2 + 1].q[0];
	X1 = Bin[r * 2 + 1].q[1];
	X2 = Bin[r * 2 + 1].q[2];
	X3 = Bin[r * 2 + 1].q[3];

	/* 2: for i = 0 to 2r - 1 do */
	for (i = 0; i <= r; i++) {
		/* 3: X <-- H(X \xor B_i) */
		/* 4: Y_i <-- X */
		/* 6: B' <-- (Y_0, Y_2 ... Y_{2r-2}, Y_1, Y_3 ... Y_{2r-1}) */
		SALSA20_8_XOR_MEM(Bin[i * 2].q, Bout[i].q)

		/* 3: X <-- H(X \xor B_i) */
		/* 4: Y_i <-- X */
		/* 6: B' <-- (Y_0, Y_2 ... Y_{2r-2}, Y_1, Y_3 ... Y_{2r-1}) */
		SALSA20_8_XOR_MEM(Bin[i * 2 + 1].q, Bout[r + 1 + i].q)
	}
}

/*
 * (V)PSRLDQ and (V)PSHUFD have higher throughput than (V)PSRLQ on some CPUs
 * starting with Sandy Bridge.  Additionally, PSHUFD uses separate source and
 * destination registers, whereas the shifts would require an extra move
 * instruction for our code when building without AVX.  Unfortunately, PSHUFD
 * is much slower on Conroe (4 cycles latency vs. 1 cycle latency for PSRLQ)
 * and somewhat slower on some non-Intel CPUs (luckily not including AMD
 * Bulldozer and Piledriver).
 */
#ifdef __AVX__
#define HI32(X) \
	_mm_srli_si128((X), 4)
#elif 1 /* As an option, check for __SSE4_1__ here not to hurt Conroe */
#define HI32(X) \
	_mm_shuffle_epi32((X), _MM_SHUFFLE(2,3,0,1))
#else
#define HI32(X) \
	_mm_srli_epi64((X), 32)
#endif

#if defined(__x86_64__) && (defined(__ICC) || defined(__llvm__))
/* Intel's name, also supported by recent gcc */
#define EXTRACT64(X) _mm_cvtsi128_si64(X)
#elif defined(__x86_64__) && !defined(_MSC_VER) && !defined(__OPEN64__)
/* gcc got the 'x' name earlier than non-'x', MSVC and Open64 had bugs */
#define EXTRACT64(X) _mm_cvtsi128_si64x(X)
#elif defined(__x86_64__) && defined(__SSE4_1__)
/* No known bugs for this intrinsic */
#include <smmintrin.h>
#define EXTRACT64(X) _mm_extract_epi64((X), 0)
#elif defined(__SSE4_1__)
/* 32-bit */
#include <smmintrin.h>
#if 0
/* This is currently unused by the code below, which instead uses these two
 * intrinsics explicitly when (!defined(__x86_64__) && defined(__SSE4_1__)) */
#define EXTRACT64(X) \
	((uint64_t)(uint32_t)_mm_cvtsi128_si32(X) | \
	((uint64_t)(uint32_t)_mm_extract_epi32((X), 1) << 32))
#endif
#else
/* 32-bit or compilers with known past bugs in _mm_cvtsi128_si64*() */
#define EXTRACT64(X) \
	((uint64_t)(uint32_t)_mm_cvtsi128_si32(X) | \
	((uint64_t)(uint32_t)_mm_cvtsi128_si32(HI32(X)) << 32))
#endif

/* This is tunable */
#define Swidth 8

/* Not tunable in this implementation, hard-coded in a few places */
#define PWXsimple 2
#define PWXgather 4

/* Derived values.  Not tunable except via Swidth above. */
#define PWXbytes (PWXgather * PWXsimple * 8)
#define Sbytes (3 * (1 << Swidth) * PWXsimple * 8)
#define Smask (((1 << Swidth) - 1) * PWXsimple * 8)
#define Smask2 (((uint64_t)Smask << 32) | Smask)

#if !defined(__x86_64__) && defined(__SSE4_1__)
/* 32-bit with SSE4.1 */
#define PWXFORM_X_T __m128i
#define PWXFORM_SIMD(X, x, s0, s1) \
	x = _mm_and_si128(X, _mm_set1_epi64x(Smask2)); \
	s0 = *(__m128i *)(S0 + (uint32_t)_mm_cvtsi128_si32(x)); \
	s1 = *(__m128i *)(S1 + (uint32_t)_mm_extract_epi32(x, 1)); \
	X = _mm_mul_epu32(HI32(X), X); \
	X = _mm_add_epi64(X, s0); \
	X = _mm_xor_si128(X, s1);
#else
/* 64-bit, or 32-bit without SSE4.1 */
#define PWXFORM_X_T uint64_t
#define PWXFORM_SIMD(X, x, s0, s1) \
	x = EXTRACT64(X) & Smask2; \
	s0 = *(__m128i *)(S0 + (uint32_t)x); \
	s1 = *(__m128i *)(S1 + (x >> 32)); \
	X = _mm_mul_epu32(HI32(X), X); \
	X = _mm_add_epi64(X, s0); \
	X = _mm_xor_si128(X, s1);
#endif

#define PWXFORM_WRITE \
	*(__m128i *)(S2 + w) = X0; \
	*(__m128i *)(S2 + w + 16) = X1; \
	*(__m128i *)(S2 + w + 32) = X2; \
	*(__m128i *)(S2 + w + 48) = X3; \
	w += 64;

#define PWXFORM_ROUND \
	PWXFORM_SIMD(X0, x0, s00, s01) \
	PWXFORM_SIMD(X1, x1, s10, s11) \
	PWXFORM_SIMD(X2, x2, s20, s21) \
	PWXFORM_SIMD(X3, x3, s30, s31)

#define PWXFORM \
	{ \
		PWXFORM_X_T x0, x1, x2, x3; \
		__m128i s00, s01, s10, s11, s20, s21, s30, s31; \
		PWXFORM_ROUND \
		PWXFORM_ROUND PWXFORM_WRITE \
		PWXFORM_ROUND PWXFORM_WRITE \
		PWXFORM_ROUND PWXFORM_WRITE \
		PWXFORM_ROUND PWXFORM_WRITE \
		PWXFORM_ROUND \
		w &= Smask; \
		{ \
			uint8_t * Stmp = S2; \
			S2 = S1; \
			S1 = S0; \
			S0 = Stmp; \
		} \
	}

#define XOR4(in) \
	X0 = _mm_xor_si128(X0, (in)[0]); \
	X1 = _mm_xor_si128(X1, (in)[1]); \
	X2 = _mm_xor_si128(X2, (in)[2]); \
	X3 = _mm_xor_si128(X3, (in)[3]);

#define OUT(out) \
	(out)[0] = X0; \
	(out)[1] = X1; \
	(out)[2] = X2; \
	(out)[3] = X3;

typedef struct {
	uint8_t *S0, *S1, *S2;
	size_t w;
} pwxform_ctx_t;

#define Salloc (Sbytes + ((sizeof(pwxform_ctx_t) + 63) & ~63U))

/**
 * blockmix_pwxform(Bin, Bout, r, S):
 * Compute Bout = BlockMix_pwxform{salsa20/8, r, S}(Bin).  The input Bin must
 * be 128r bytes in length; the output Bout must also be the same size.
 */
static void
blockmix(const salsa20_blk_t *restrict Bin, salsa20_blk_t *restrict Bout,
    size_t r, pwxform_ctx_t *restrict ctx)
{
	uint8_t *S0 = ctx->S0, *S1 = ctx->S1, *S2 = ctx->S2;
	size_t w = ctx->w;
	size_t i;
	__m128i X0, X1, X2, X3;

	/* Convert 128-byte blocks to 64-byte blocks */
	/* 1: r_1 <-- 128r / PWXbytes */
	r *= 2;

	r--;
	PREFETCH(&Bin[r], _MM_HINT_T0)
	for (i = 0; i < r; i++) {
		PREFETCH(&Bin[i], _MM_HINT_T0)
	}

	/* 2: X <-- B'_{r_1 - 1} */
	X0 = Bin[r].q[0];
	X1 = Bin[r].q[1];
	X2 = Bin[r].q[2];
	X3 = Bin[r].q[3];

	/* 3: for i = 0 to r_1 - 1 do */
	i = 0;
	do {
		/* 5: X <-- X \xor B'_i */
		XOR4(Bin[i].q)
		/* 7: X <-- pwxform(X) */
		PWXFORM

		if (unlikely(i >= r))
			break;

		/* 8: B'_i <-- X */
		OUT(Bout[i].q)

		i++;
	} while (1);

	ctx->S0 = S0; ctx->S1 = S1; ctx->S2 = S2;
	ctx->w = w;

	/* 11: B_i <-- H(B_i) */
	SALSA20_2(Bout[i].q)
}

#define XOR4_2(in1, in2) \
	X0 = _mm_xor_si128((in1)[0], (in2)[0]); \
	X1 = _mm_xor_si128((in1)[1], (in2)[1]); \
	X2 = _mm_xor_si128((in1)[2], (in2)[2]); \
	X3 = _mm_xor_si128((in1)[3], (in2)[3]);

static uint32_t
blockmix_salsa8_xor(const salsa20_blk_t *restrict Bin1,
    const salsa20_blk_t *restrict Bin2, salsa20_blk_t *restrict Bout,
    size_t r)
{
	size_t i;
	__m128i X0, X1, X2, X3;

	r--;
	PREFETCH(&Bin2[r * 2 + 1], _MM_HINT_T0)
	PREFETCH(&Bin1[r * 2 + 1], _MM_HINT_T0)
	for (i = 0; i < r; i++) {
		PREFETCH(&Bin2[i * 2], _MM_HINT_T0)
		PREFETCH(&Bin1[i * 2], _MM_HINT_T0)
		PREFETCH(&Bin2[i * 2 + 1], _MM_HINT_T0)
		PREFETCH(&Bin1[i * 2 + 1], _MM_HINT_T0)
	}
	PREFETCH(&Bin2[r * 2], _MM_HINT_T0)
	PREFETCH(&Bin1[r * 2], _MM_HINT_T0)

	/* 1: X <-- B_{2r - 1} */
	XOR4_2(Bin1[r * 2 + 1].q, Bin2[r * 2 + 1].q)

	/* 2: for i = 0 to 2r - 1 do */
	for (i = 0; i <= r; i++) {
		/* 3: X <-- H(X \xor B_i) */
		/* 4: Y_i <-- X */
		/* 6: B' <-- (Y_0, Y_2 ... Y_{2r-2}, Y_1, Y_3 ... Y_{2r-1}) */
		XOR4(Bin1[i * 2].q)
		SALSA20_8_XOR_MEM(Bin2[i * 2].q, Bout[i].q)

		/* 3: X <-- H(X \xor B_i) */
		/* 4: Y_i <-- X */
		/* 6: B' <-- (Y_0, Y_2 ... Y_{2r-2}, Y_1, Y_3 ... Y_{2r-1}) */
		XOR4(Bin1[i * 2 + 1].q)
		SALSA20_8_XOR_MEM(Bin2[i * 2 + 1].q, Bout[r + 1 + i].q)
	}

	return _mm_cvtsi128_si32(X0);
}

static uint32_t
blockmix_xor(const salsa20_blk_t *restrict Bin1,
    const salsa20_blk_t *restrict Bin2, salsa20_blk_t *restrict Bout,
    size_t r, int Bin2_in_ROM, pwxform_ctx_t *restrict ctx)
{
	uint8_t *S0 = ctx->S0, *S1 = ctx->S1, *S2 = ctx->S2;
	size_t w = ctx->w;
	size_t i;
	__m128i X0, X1, X2, X3;

	/* Convert 128-byte blocks to 64-byte blocks */
	/* 1: r_1 <-- 128r / PWXbytes */
	r *= 2;

	r--;
	if (Bin2_in_ROM) {
		PREFETCH(&Bin2[r], _MM_HINT_NTA)
		PREFETCH(&Bin1[r], _MM_HINT_T0)
		for (i = 0; i < r; i++) {
			PREFETCH(&Bin2[i], _MM_HINT_NTA)
			PREFETCH(&Bin1[i], _MM_HINT_T0)
		}
	} else {
		PREFETCH(&Bin2[r], _MM_HINT_T0)
		PREFETCH(&Bin1[r], _MM_HINT_T0)
		for (i = 0; i < r; i++) {
			PREFETCH(&Bin2[i], _MM_HINT_T0)
			PREFETCH(&Bin1[i], _MM_HINT_T0)
		}
	}

	/* 2: X <-- B'_{r_1 - 1} */
	XOR4_2(Bin1[r].q, Bin2[r].q)

	/* 3: for i = 0 to r_1 - 1 do */
	i = 0;
	r--;
	do {
		/* 5: X <-- X \xor B'_i */
		XOR4(Bin1[i].q)
		XOR4(Bin2[i].q)
		/* 7: X <-- pwxform(X) */
		PWXFORM
		/* 8: B'_i <-- X */
		OUT(Bout[i].q)

		/* 5: X <-- X \xor B'_i */
		XOR4(Bin1[i + 1].q)
		XOR4(Bin2[i + 1].q)
		/* 7: X <-- pwxform(X) */
		PWXFORM

		if (unlikely(i >= r))
			break;

		/* 8: B'_i <-- X */
		OUT(Bout[i + 1].q)

		i += 2;
	} while (1);
	i++;

	ctx->S0 = S0; ctx->S1 = S1; ctx->S2 = S2;
	ctx->w = w;

	/* 11: B_i <-- H(B_i) */
	SALSA20_2(Bout[i].q)

	return _mm_cvtsi128_si32(X0);
}

#undef XOR4
#define XOR4(in, out) \
	(out)[0] = Y0 = _mm_xor_si128((in)[0], (out)[0]); \
	(out)[1] = Y1 = _mm_xor_si128((in)[1], (out)[1]); \
	(out)[2] = Y2 = _mm_xor_si128((in)[2], (out)[2]); \
	(out)[3] = Y3 = _mm_xor_si128((in)[3], (out)[3]);

#define XOR4_Y \
	X0 = _mm_xor_si128(X0, Y0); \
	X1 = _mm_xor_si128(X1, Y1); \
	X2 = _mm_xor_si128(X2, Y2); \
	X3 = _mm_xor_si128(X3, Y3);

static uint32_t
blockmix_xor_save(const salsa20_blk_t *restrict Bin1,
    salsa20_blk_t *restrict Bin2, salsa20_blk_t *restrict Bout,
    size_t r, pwxform_ctx_t *restrict ctx)
{
	__m128i X0, X1, X2, X3, Y0, Y1, Y2, Y3;
	uint8_t *S0 = ctx->S0, *S1 = ctx->S1, *S2 = ctx->S2;
	size_t w = ctx->w;
	size_t i;

	/* Convert 128-byte blocks to 64-byte blocks */
	/* 1: r_1 <-- 128r / PWXbytes */
	r *= 2;

	r--;
	PREFETCH(&Bin2[r], _MM_HINT_T0)
	PREFETCH(&Bin1[r], _MM_HINT_T0)
	for (i = 0; i < r; i++) {
		PREFETCH(&Bin2[i], _MM_HINT_T0)
		PREFETCH(&Bin1[i], _MM_HINT_T0)
	}

	/* 2: X <-- B'_{r_1 - 1} */
	XOR4_2(Bin1[r].q, Bin2[r].q)

	/* 3: for i = 0 to r_1 - 1 do */
	i = 0;
	r--;
	do {
		XOR4(Bin1[i].q, Bin2[i].q)
		/* 5: X <-- X \xor B'_i */
		XOR4_Y
		/* 7: X <-- pwxform(X) */
		PWXFORM
		/* 8: B'_i <-- X */
		OUT(Bout[i].q)

		XOR4(Bin1[i + 1].q, Bin2[i + 1].q)
		/* 5: X <-- X \xor B'_i */
		XOR4_Y
		/* 7: X <-- pwxform(X) */
		PWXFORM

		if (unlikely(i >= r))
			break;

		/* 8: B'_i <-- X */
		OUT(Bout[i + 1].q)

		i += 2;
	} while (1);
	i++;

	ctx->S0 = S0; ctx->S1 = S1; ctx->S2 = S2;
	ctx->w = w;

	/* 11: B_i <-- H(B_i) */
	SALSA20_2(Bout[i].q)

	return _mm_cvtsi128_si32(X0);
}

#undef ARX
#undef SALSA20_2ROUNDS
#undef SALSA20_2
#undef SALSA20_8_XOR_ANY
#undef SALSA20_8_XOR_MEM
#undef SALSA20_8_XOR_REG
#undef PWXFORM_X_T
#undef PWXFORM_SIMD
#undef PWXFORM_ROUND
#undef PWXFORM
#undef OUT
#undef XOR4
#undef XOR4_2
#undef XOR4_Y

/**
 * integerify(B, r):
 * Return the result of parsing B_{2r-1} as a little-endian integer.
 */
static inline uint32_t
integerify(const salsa20_blk_t * B, size_t r)
{
	return B[2 * r - 1].w[0];
}

/**
 * smix1(B, r, N, flags, V, NROM, VROM, XY, ctx):
 * Compute first loop of B = SMix_r(B, N).  The input B must be 128r bytes in
 * length; the temporary storage V must be 128rN bytes in length; the temporary
 * storage XY must be 128r bytes in length.  The value N must be even and no
 * smaller than 2.  The array V must be aligned to a multiple of 64 bytes, and
 * arrays B and XY to a multiple of at least 16 bytes (aligning them to 64
 * bytes as well saves cache lines, but might result in cache bank conflicts).
 */
static void
smix1(uint8_t * B, size_t r, uint32_t N, yescrypt_flags_t flags,
    salsa20_blk_t * V, uint32_t NROM, const salsa20_blk_t * VROM,
    salsa20_blk_t * XY, pwxform_ctx_t * ctx)
{
	size_t s = 2 * r;
	salsa20_blk_t * X = V, * Y;
	uint32_t i, j;
	size_t k;

	/* 1: X <-- B */
	/* 3: V_i <-- X */
	for (k = 0; k < 2 * r; k++) {
		for (i = 0; i < 16; i++) {
			X[k].w[i] = le32dec(&B[(k * 16 + (i * 5 % 16)) * 4]);
		}
	}

	if (VROM) {
		uint32_t n;
		salsa20_blk_t * V_n;
		const salsa20_blk_t * V_j;

		/* 4: X <-- H(X) */
		/* 3: V_i <-- X */
		Y = &V[s];
		blockmix(X, Y, r, ctx);

		X = &V[2 * s];
		/* j <-- Integerify(X) mod NROM */
		j = integerify(Y, r) & (NROM - 1);
		V_j = &VROM[j * s];

		/* X <-- H(X \xor VROM_j) */
		j = blockmix_xor(Y, V_j, X, r, 1, ctx);

		for (n = 2; n < N; n <<= 1) {
			uint32_t m = (n < N / 2) ? n : (N - 1 - n);

			V_n = &V[n * s];

			/* 2: for i = 0 to N - 1 do */
			for (i = 1; i < m; i += 2) {
				/* j <-- Wrap(Integerify(X), i) */
				j &= n - 1;
				j += i - 1;
				V_j = &V[j * s];

				/* X <-- X \xor V_j */
				/* 4: X <-- H(X) */
				/* 3: V_i <-- X */
				Y = &V_n[i * s];

				/* j <-- Integerify(X) mod NROM */
				j = blockmix_xor(X, V_j, Y, r, 0, ctx) & (NROM - 1);
				V_j = &VROM[j * s];

				/* X <-- H(X \xor VROM_j) */
				X = &V_n[(i + 1) * s];
				j = blockmix_xor(Y, V_j, X, r, 1, ctx);
			}
		}

		n >>= 1;

		/* j <-- Wrap(Integerify(X), i) */
		j &= n - 1;
		j += N - 2 - n;
		V_j = &V[j * s];

		/* X <-- X \xor V_j */
		/* 4: X <-- H(X) */
		/* 3: V_i <-- X */
		Y = &V[(N - 1) * s];

		/* j <-- Integerify(X) mod NROM */
		j = blockmix_xor(X, V_j, Y, r, 0, ctx) & (NROM - 1);
		V_j = &VROM[j * s];

		/* X <-- X \xor V_j */
		/* 4: X <-- H(X) */
		X = XY;
		blockmix_xor(Y, V_j, X, r, 1, ctx);
	} else if (flags & YESCRYPT_RW) {
		uint32_t n;
		salsa20_blk_t * V_n, * V_j;

		/* 4: X <-- H(X) */
		/* 3: V_i <-- X */
		Y = &V[s];
		blockmix(X, Y, r, ctx);

		/* 4: X <-- H(X) */
		/* 3: V_i <-- X */
		X = &V[2 * s];
		blockmix(Y, X, r, ctx);
		j = integerify(X, r);

		for (n = 2; n < N; n <<= 1) {
			uint32_t m = (n < N / 2) ? n : (N - 1 - n);

			V_n = &V[n * s];

			/* 2: for i = 0 to N - 1 do */
			for (i = 1; i < m; i += 2) {
				Y = &V_n[i * s];

				/* j <-- Wrap(Integerify(X), i) */
				j &= n - 1;
				j += i - 1;
				V_j = &V[j * s];

				/* X <-- X \xor V_j */
				/* 4: X <-- H(X) */
				/* 3: V_i <-- X */
				j = blockmix_xor(X, V_j, Y, r, 0, ctx);

				/* j <-- Wrap(Integerify(X), i) */
				j &= n - 1;
				j += i;
				V_j = &V[j * s];

				/* X <-- X \xor V_j */
				/* 4: X <-- H(X) */
				/* 3: V_i <-- X */
				X = &V_n[(i + 1) * s];
				j = blockmix_xor(Y, V_j, X, r, 0, ctx);
			}
		}

		n >>= 1;

		/* j <-- Wrap(Integerify(X), i) */
		j &= n - 1;
		j += N - 2 - n;
		V_j = &V[j * s];

		/* X <-- X \xor V_j */
		/* 4: X <-- H(X) */
		/* 3: V_i <-- X */
		Y = &V[(N - 1) * s];
		j = blockmix_xor(X, V_j, Y, r, 0, ctx);

		/* j <-- Wrap(Integerify(X), i) */
		j &= n - 1;
		j += N - 1 - n;
		V_j = &V[j * s];

		/* X <-- X \xor V_j */
		/* 4: X <-- H(X) */
		X = XY;
		blockmix_xor(Y, V_j, X, r, 0, ctx);
	} else {
		/* 2: for i = 0 to N - 1 do */
		for (i = 1; i < N - 1; i += 2) {
			/* 4: X <-- H(X) */
			/* 3: V_i <-- X */
			Y = &V[i * s];
			blockmix_salsa8(X, Y, r);

			/* 4: X <-- H(X) */
			/* 3: V_i <-- X */
			X = &V[(i + 1) * s];
			blockmix_salsa8(Y, X, r);
		}

		/* 4: X <-- H(X) */
		/* 3: V_i <-- X */
		Y = &V[i * s];
		blockmix_salsa8(X, Y, r);

		/* 4: X <-- H(X) */
		X = XY;
		blockmix_salsa8(Y, X, r);
	}

	/* B' <-- X */
	for (k = 0; k < 2 * r; k++) {
		for (i = 0; i < 16; i++) {
			le32enc(&B[(k * 16 + (i * 5 % 16)) * 4], X[k].w[i]);
		}
	}
}

/**
 * smix2(B, r, N, Nloop, flags, V, NROM, VROM, XY, ctx):
 * Compute second loop of B = SMix_r(B, N).  The input B must be 128r bytes in
 * length; the temporary storage V must be 128rN bytes in length; the temporary
 * storage XY must be 256r bytes in length.  The value N must be a power of 2
 * greater than 1.  The value Nloop must be even.  The array V must be aligned
 * to a multiple of 64 bytes, and arrays B and XY to a multiple of at least 16
 * bytes (aligning them to 64 bytes as well saves cache lines, but might result
 * in cache bank conflicts).
 */
static void
smix2(uint8_t * B, size_t r, uint32_t N, uint64_t Nloop,
    yescrypt_flags_t flags, salsa20_blk_t * V, uint32_t NROM,
    const salsa20_blk_t * VROM, salsa20_blk_t * XY, pwxform_ctx_t * ctx)
{
	size_t s = 2 * r;
	salsa20_blk_t * X = XY, * Y = &XY[s];
	uint64_t i;
	uint32_t j;
	size_t k;

	if (Nloop == 0)
		return;

	/* X <-- B' */
	/* 3: V_i <-- X */
	for (k = 0; k < 2 * r; k++) {
		for (i = 0; i < 16; i++) {
			X[k].w[i] = le32dec(&B[(k * 16 + (i * 5 % 16)) * 4]);
		}
	}

	i = Nloop / 2;

	/* 7: j <-- Integerify(X) mod N */
	j = integerify(X, r) & (N - 1);

/*
 * Normally, VROM implies YESCRYPT_RW, but we check for these separately
 * because our SMix resets YESCRYPT_RW for the smix2() calls operating on the
 * entire V when p > 1.
 */
	if (VROM && (flags & YESCRYPT_RW)) {
		/* 6: for i = 0 to N - 1 do */
		for (i = 0; i < Nloop; i += 2) {
			salsa20_blk_t * V_j = &V[j * s];
			const salsa20_blk_t * VROM_j;

			/* 8: X <-- H(X \xor V_j) */
			/* V_j <-- Xprev \xor V_j */
			/* j <-- Integerify(X) mod NROM */
			j = blockmix_xor_save(X, V_j, Y, r, ctx) & (NROM - 1);
			VROM_j = &VROM[j * s];

			/* X <-- H(X \xor VROM_j) */
			/* 7: j <-- Integerify(X) mod N */
			j = blockmix_xor(Y, VROM_j, X, r, 1, ctx) & (N - 1);
			V_j = &V[j * s];
		}
	} else if (VROM) {
		/* 6: for i = 0 to N - 1 do */
		for (i = 0; i < Nloop; i += 2) {
			const salsa20_blk_t * V_j = &V[j * s];

			/* 8: X <-- H(X \xor V_j) */
			/* V_j <-- Xprev \xor V_j */
			/* j <-- Integerify(X) mod NROM */
			j = blockmix_xor(X, V_j, Y, r, 0, ctx) & (NROM - 1);
			V_j = &VROM[j * s];

			/* X <-- H(X \xor VROM_j) */
			/* 7: j <-- Integerify(X) mod N */
			j = blockmix_xor(Y, V_j, X, r, 1, ctx) & (N - 1);
			V_j = &V[j * s];
		}
	} else if (flags & YESCRYPT_RW) {
		/* 6: for i = 0 to N - 1 do */
		do {
			salsa20_blk_t * V_j = &V[j * s];

			/* 8: X <-- H(X \xor V_j) */
			/* V_j <-- Xprev \xor V_j */
			/* 7: j <-- Integerify(X) mod N */
			j = blockmix_xor_save(X, V_j, Y, r, ctx) & (N - 1);
			V_j = &V[j * s];

			/* 8: X <-- H(X \xor V_j) */
			/* V_j <-- Xprev \xor V_j */
			/* 7: j <-- Integerify(X) mod N */
			j = blockmix_xor_save(Y, V_j, X, r, ctx) & (N - 1);
		} while (--i);
	} else if (ctx) {
		/* 6: for i = 0 to N - 1 do */
		do {
			const salsa20_blk_t * V_j = &V[j * s];

			/* 8: X <-- H(X \xor V_j) */
			/* 7: j <-- Integerify(X) mod N */
			j = blockmix_xor(X, V_j, Y, r, 0, ctx) & (N - 1);
			V_j = &V[j * s];

			/* 8: X <-- H(X \xor V_j) */
			/* 7: j <-- Integerify(X) mod N */
			j = blockmix_xor(Y, V_j, X, r, 0, ctx) & (N - 1);
		} while (--i);
	} else {
		/* 6: for i = 0 to N - 1 do */
		do {
			const salsa20_blk_t * V_j = &V[j * s];

			/* 8: X <-- H(X \xor V_j) */
			/* 7: j <-- Integerify(X) mod N */
			j = blockmix_salsa8_xor(X, V_j, Y, r) & (N - 1);
			V_j = &V[j * s];

			/* 8: X <-- H(X \xor V_j) */
			/* 7: j <-- Integerify(X) mod N */
			j = blockmix_salsa8_xor(Y, V_j, X, r) & (N - 1);
		} while (--i);
	}

	/* 10: B' <-- X */
	for (k = 0; k < 2 * r; k++) {
		for (i = 0; i < 16; i++) {
			le32enc(&B[(k * 16 + (i * 5 % 16)) * 4], X[k].w[i]);
		}
	}
}

/**
 * p2floor(x):
 * Largest power of 2 not greater than argument.
 */
static uint64_t
p2floor(uint64_t x)
{
	uint64_t y;
	while ((y = x & (x - 1)))
		x = y;
	return x;
}

/**
 * smix(B, r, N, p, t, flags, V, NROM, VROM, XY, S, passwd):
 * Compute B = SMix_r(B, N).  The input B must be 128rp bytes in length; the
 * temporary storage V must be 128rN bytes in length; the temporary storage XY
 * must be 256r or 256rp bytes in length (the larger size is required with
 * OpenMP-enabled builds).  The value N must be a power of 2 greater than 1.
 * The array V must be aligned to a multiple of 64 bytes, and arrays B and
 * XY to a multiple of at least 16 bytes (aligning them to 64 bytes as well
 * saves cache lines and helps avoid false sharing in OpenMP-enabled builds
 * when p > 1, but it might also result in cache bank conflicts).
 */
static void
smix(uint8_t * B, size_t r, uint32_t N, uint32_t p, uint32_t t,
    yescrypt_flags_t flags,
    salsa20_blk_t * V, uint32_t NROM, const salsa20_blk_t * VROM,
    salsa20_blk_t * XY, uint8_t * S, uint8_t * passwd)
{
	size_t s = 2 * r;
	uint32_t Nchunk;
	uint64_t Nloop_all, Nloop_rw;
	uint32_t i;

	/* 1: n <-- N / p */
	Nchunk = N / p;

	/* 2: Nloop_all <-- fNloop(n, t, flags) */
	Nloop_all = Nchunk;
	if (flags & YESCRYPT_RW) {
		if (t <= 1) {
			if (t)
				Nloop_all *= 2; /* 2/3 */
			Nloop_all = (Nloop_all + 2) / 3; /* 1/3, round up */
		} else {
			Nloop_all *= t - 1;
		}
	} else if (t) {
		if (t == 1)
			Nloop_all += (Nloop_all + 1) / 2; /* 1.5, round up */
		Nloop_all *= t;
	}

	/* 6: Nloop_rw <-- 0 */
	Nloop_rw = 0;
	if (flags & __YESCRYPT_INIT_SHARED) {
		Nloop_rw = Nloop_all;
	} else {
		/* 3: if YESCRYPT_RW flag is set */
		if (flags & YESCRYPT_RW) {
			/* 4: Nloop_rw <-- Nloop_all / p */
			Nloop_rw = Nloop_all / p;
		}
	}

	/* 8: n <-- n - (n mod 2) */
	Nchunk &= ~(uint32_t)1; /* round down to even */
	/* 9: Nloop_all <-- Nloop_all + (Nloop_all mod 2) */
	Nloop_all++; Nloop_all &= ~(uint64_t)1; /* round up to even */
	/* 10: Nloop_rw <-- Nloop_rw + (Nloop_rw mod 2) */
	Nloop_rw++; Nloop_rw &= ~(uint64_t)1; /* round up to even */

	/* 11: for i = 0 to p - 1 do */
#ifdef _OPENMP
#pragma omp parallel if (p > 1) default(none) private(i) shared(B, r, N, p, flags, V, NROM, VROM, XY, S, passwd, s, Nchunk, Nloop_all, Nloop_rw)
	{
#pragma omp for
#endif
	for (i = 0; i < p; i++) {
		/* 12: u <-- in */
		uint32_t Vchunk = i * Nchunk;
		/* 13: if i = p - 1 */
		/* 14:   n <-- N - u */
		/* 15: end if */
		/* 16: v <-- u + n - 1 */
		uint32_t Np = (i < p - 1) ? Nchunk : (N - Vchunk);
		uint8_t * Bp = &B[128 * r * i];
		salsa20_blk_t * Vp = &V[Vchunk * s];
#ifdef _OPENMP
		salsa20_blk_t * XYp = &XY[i * (2 * s)];
#else
		salsa20_blk_t * XYp = XY;
#endif
		pwxform_ctx_t * ctx_i = NULL;
		/* 17: if YESCRYPT_RW flag is set */
		if (flags & YESCRYPT_RW) {
			uint8_t *Si = S + i * Salloc;
			/* 18: SMix1_1(B_i, Sbytes / 128, S_i, no flags) */
			smix1(Bp, 1, Sbytes / 128, 0 /* no flags */,
			    (salsa20_blk_t *)Si, 0, NULL, XYp, NULL);
			ctx_i = (pwxform_ctx_t *)(Si + Sbytes);
			/* 19: S2_i <-- S_{i,0...2^Swidth-1} */
			ctx_i->S2 = Si;
			/* 20: S1_i <-- S_{i,2^Swidth...2*2^Swidth-1} */
			ctx_i->S1 = Si + Sbytes / 3;
			/* 21: S0_i <-- S_{i,2*2^Swidth...3*2^Swidth-1} */
			ctx_i->S0 = Si + Sbytes / 3 * 2;
			/* 22: w_i <-- 0 */
			ctx_i->w = 0;
			/* 23: if i = 0 */
			if (i == 0) {
				/* 24: passwd <-- HMAC-SHA256(B_{0,2r-1}, passwd) */
				HMAC_SHA256_CTX ctx;
				HMAC_SHA256_Init(&ctx, Bp + (128 * r - 64), 64);
				HMAC_SHA256_Update(&ctx, passwd, 32);
				HMAC_SHA256_Final(passwd, &ctx);
			}
		}
		if (!(flags & __YESCRYPT_INIT_SHARED_2)) {
			/* 27: SMix1_r(B_i, n, V_{u..v}, flags) */
			smix1(Bp, r, Np, flags, Vp, NROM, VROM, XYp, ctx_i);
		}
		/* 28: SMix2_r(B_i, p2floor(n), Nloop_rw, V_{u..v}, flags) */
		smix2(Bp, r, p2floor(Np), Nloop_rw, flags, Vp,
		    NROM, VROM, XYp, ctx_i);
	}

	/* 30: for i = 0 to p - 1 do */
	if (Nloop_all > Nloop_rw) {
#ifdef _OPENMP
#pragma omp for
#endif
		for (i = 0; i < p; i++) {
			uint8_t * Bp = &B[128 * r * i];
#ifdef _OPENMP
			salsa20_blk_t * XYp = &XY[i * (2 * s)];
#else
			salsa20_blk_t * XYp = XY;
#endif
			pwxform_ctx_t * ctx_i = NULL;
			if (flags & YESCRYPT_RW) {
				uint8_t *Si = S + i * Salloc;
				ctx_i = (pwxform_ctx_t *)(Si + Sbytes);
			}
			/* 31: SMix2_r(B_i, N, Nloop_all - Nloop_rw, V, flags excluding YESCRYPT_RW) */
			smix2(Bp, r, N, Nloop_all - Nloop_rw,
			    flags & ~YESCRYPT_RW, V, NROM, VROM, XYp, ctx_i);
		}
	}
#ifdef _OPENMP
	}
#endif
}

/**
 * yescrypt_kdf_body(shared, local, passwd, passwdlen, salt, saltlen,
 *     N, r, p, t, flags, buf, buflen):
 * Compute scrypt(passwd[0 .. passwdlen - 1], salt[0 .. saltlen - 1], N, r,
 * p, buflen), or a revision of scrypt as requested by flags and shared, and
 * write the result into buf.  The parameters r, p, and buflen must satisfy
 * r * p < 2^30 and buflen <= (2^32 - 1) * 32.  The parameter N must be a power
 * of 2 greater than 1.  (This optimized implementation currently additionally
 * limits N to the range from 8 to 2^31, but other implementation might not.)
 *
 * t controls computation time while not affecting peak memory usage.  shared
 * and flags may request special modes as described in yescrypt.h.  local is
 * the thread-local data structure, allowing to preserve and reuse a memory
 * allocation across calls, thereby reducing its overhead.
 *
 * Return 0 on success; or -1 on error.
 */
static int
yescrypt_kdf_body(const yescrypt_shared_t * shared, yescrypt_local_t * local,
    const uint8_t * passwd, size_t passwdlen,
    const uint8_t * salt, size_t saltlen,
    uint64_t N, uint32_t r, uint32_t p, uint32_t t, yescrypt_flags_t flags,
    uint8_t * buf, size_t buflen)
{
	yescrypt_region_t tmp;
	uint64_t NROM;
	const salsa20_blk_t * VROM;
	size_t B_size, V_size, XY_size, need;
	uint8_t * B, * S;
	salsa20_blk_t * V, * XY;
	uint8_t sha256[32];
	uint8_t dk[sizeof(sha256)], * dkp = buf;

	/* Sanity-check parameters */
	if (flags & ~YESCRYPT_KNOWN_FLAGS) {
		errno = EINVAL;
		return -1;
	}
#if SIZE_MAX > UINT32_MAX
	if (buflen > (((uint64_t)(1) << 32) - 1) * 32) {
		errno = EFBIG;
		return -1;
	}
#endif
	if ((uint64_t)(r) * (uint64_t)(p) >= (1 << 30)) {
		errno = EFBIG;
		return -1;
	}
	if (N > UINT32_MAX) {
		errno = EFBIG;
		return -1;
	}
	if (((N & (N - 1)) != 0) || (N <= 3) || (r < 1) || (p < 1)) {
		errno = EINVAL;
		return -1;
	}
	if ((r > SIZE_MAX / 256 / p) ||
	    (N > SIZE_MAX / 128 / r)) {
		errno = ENOMEM;
		return -1;
	}
	if (flags & YESCRYPT_RW) {
		if (N / p <= 3) {
			errno = EINVAL;
			return -1;
		}
		if (p > SIZE_MAX / Salloc) {
			errno = ENOMEM;
			return -1;
		}
	}
#ifdef _OPENMP
	else if (N > SIZE_MAX / 128 / (r * p)) {
		errno = ENOMEM;
		return -1;
	}
#endif

	NROM = 0;
	VROM = NULL;
	if (shared->aligned) {
		NROM = shared->aligned_size / ((size_t)128 * r);
		if (NROM > UINT32_MAX) {
			errno = EFBIG;
			return -1;
		}
		if (((NROM & (NROM - 1)) != 0) || (NROM <= 1) ||
		    !(flags & YESCRYPT_RW)) {
			errno = EINVAL;
			return -1;
		}
		VROM = shared->aligned;
	}

	/* Allocate memory */
	V = NULL;
	V_size = (size_t)128 * r * N;
#ifdef _OPENMP
	if (!(flags & YESCRYPT_RW))
		V_size *= p;
#endif
	need = V_size;
	if (flags & __YESCRYPT_INIT_SHARED) {
		if (local->aligned_size < need) {
			if (local->base || local->aligned ||
			    local->base_size || local->aligned_size) {
				errno = EINVAL;
				return -1;
			}
			if (!alloc_region(local, need))
				return -1;
		}
		V = (salsa20_blk_t *)local->aligned;
		need = 0;
	}
	B_size = (size_t)128 * r * p;
	need += B_size;
	if (need < B_size) {
		errno = ENOMEM;
		return -1;
	}
	XY_size = (size_t)256 * r;
#ifdef _OPENMP
	XY_size *= p;
#endif
	need += XY_size;
	if (need < XY_size) {
		errno = ENOMEM;
		return -1;
	}
	if (flags & YESCRYPT_RW) {
		size_t S_size = (size_t)Salloc * p;
		need += S_size;
		if (need < S_size) {
			errno = ENOMEM;
			return -1;
		}
	}
	if (flags & __YESCRYPT_INIT_SHARED) {
		if (!alloc_region(&tmp, need))
			return -1;
		B = (uint8_t *)tmp.aligned;
		XY = (salsa20_blk_t *)((uint8_t *)B + B_size);
	} else {
		init_region(&tmp);
		if (local->aligned_size < need) {
			if (free_region(local))
				return -1;
			if (!alloc_region(local, need))
				return -1;
		}
		B = (uint8_t *)local->aligned;
		V = (salsa20_blk_t *)((uint8_t *)B + B_size);
		XY = (salsa20_blk_t *)((uint8_t *)V + V_size);
	}
	S = NULL;
	if (flags & YESCRYPT_RW)
		S = (uint8_t *)XY + XY_size;

	if (flags) {
		HMAC_SHA256_CTX ctx;
		HMAC_SHA256_Init(&ctx, "yescrypt-prehash",
		    (flags & __YESCRYPT_PREHASH) ? 16 : 8);
		HMAC_SHA256_Update(&ctx, passwd, passwdlen);
		HMAC_SHA256_Final(sha256, &ctx);
		passwd = sha256;
		passwdlen = sizeof(sha256);
	}

	/* 1: (B_0 ... B_{p-1}) <-- PBKDF2(P, S, 1, p * MFLen) */
	PBKDF2_SHA256(passwd, passwdlen, salt, saltlen, 1, B, B_size);

	if (t || flags)
		memcpy(sha256, B, sizeof(sha256));

	if (p == 1 || (flags & YESCRYPT_RW)) {
		smix(B, r, N, p, t, flags, V, NROM, VROM, XY, S, sha256);
	} else {
		uint32_t i;

		/* 2: for i = 0 to p - 1 do */
#ifdef _OPENMP
#pragma omp parallel for default(none) private(i) shared(B, r, N, p, t, flags, V, NROM, VROM, XY, S)
#endif
		for (i = 0; i < p; i++) {
			/* 3: B_i <-- MF(B_i, N) */
#ifdef _OPENMP
			smix(&B[(size_t)128 * r * i], r, N, 1, t, flags,
			    &V[(size_t)2 * r * i * N],
			    NROM, VROM,
			    &XY[(size_t)4 * r * i], NULL, NULL);
#else
			smix(&B[(size_t)128 * r * i], r, N, 1, t, flags, V,
			    NROM, VROM, XY, NULL, NULL);
#endif
		}
	}

	dkp = buf;
	if (flags && buflen < sizeof(dk)) {
		PBKDF2_SHA256(passwd, passwdlen, B, B_size, 1, dk, sizeof(dk));
		dkp = dk;
	}

	/* 5: DK <-- PBKDF2(P, B, 1, dkLen) */
	PBKDF2_SHA256(passwd, passwdlen, B, B_size, 1, buf, buflen);

	/*
	 * Except when computing classic scrypt, allow all computation so far
	 * to be performed on the client.  The final steps below match those of
	 * SCRAM (RFC 5802), so that an extension of SCRAM (with the steps so
	 * far in place of SCRAM's use of PBKDF2 and with SHA-256 in place of
	 * SCRAM's use of SHA-1) would be usable with yescrypt hashes.
	 */
	if (flags && !(flags & __YESCRYPT_PREHASH)) {
		/* Compute ClientKey */
		{
			HMAC_SHA256_CTX ctx;
			HMAC_SHA256_Init(&ctx, dkp, sizeof(dk));
			HMAC_SHA256_Update(&ctx, "Client Key", 10);
			HMAC_SHA256_Final(sha256, &ctx);
		}
		/* Compute StoredKey */
		{
			SHA256_CTX ctx;
			size_t clen = buflen;
			if (clen > sizeof(dk))
				clen = sizeof(dk);
			SHA256_Init(&ctx);
			SHA256_Update(&ctx, sha256, sizeof(sha256));
			SHA256_Final(dk, &ctx);
			memcpy(buf, dk, clen);
		}
	}

	if (free_region(&tmp))
		return -1;

	/* Success! */
	return 0;
}

/**
 * yescrypt_kdf(shared, local, passwd, passwdlen, salt, saltlen,
 *     N, r, p, t, g, flags, buf, buflen):
 * Compute scrypt or its revision as requested by the parameters.  The inputs
 * to this function are the same as those for yescrypt_kdf_body() above, with
 * the addition of g, which controls hash upgrades (0 for no upgrades so far).
 */
int
yescrypt_kdf(const yescrypt_shared_t * shared, yescrypt_local_t * local,
    const uint8_t * passwd, size_t passwdlen,
    const uint8_t * salt, size_t saltlen,
    uint64_t N, uint32_t r, uint32_t p, uint32_t t, uint32_t g,
    yescrypt_flags_t flags,
    uint8_t * buf, size_t buflen)
{
	uint8_t dk[32];

	if ((flags & (YESCRYPT_RW | __YESCRYPT_INIT_SHARED)) == YESCRYPT_RW &&
	    p >= 1 && N / p >= 0x100 && N / p * r >= 0x20000) {
		int retval = yescrypt_kdf_body(shared, local,
		    passwd, passwdlen, salt, saltlen,
		    N >> 6, r, p, 0, flags | __YESCRYPT_PREHASH,
		    dk, sizeof(dk));
		if (retval)
			return retval;
		passwd = dk;
		passwdlen = sizeof(dk);
	}

	do {
		uint8_t * dkp = g ? dk : buf;
		size_t dklen = g ? sizeof(dk) : buflen;
		int retval = yescrypt_kdf_body(shared, local,
		    passwd, passwdlen, salt, saltlen,
		    N, r, p, t, flags, dkp, dklen);
		if (retval)
			return retval;

		passwd = dkp;
		passwdlen = dklen;

		N <<= 2;
		if (!N)
			return -1;
		t >>= 1;
	} while (g--);

	return 0;
}
