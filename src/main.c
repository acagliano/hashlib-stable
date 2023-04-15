//--------------------------------------
// Program Name: VAPOR
// Author: Anthony Cagliano
// License:
// Description:
//--------------------------------------

/* Keep these headers */

#include <stdint.h>
#include <tice.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <hashlib.h>

/* Standard headers - it's recommended to leave them included */

/* Other available headers */
#include "add64.h"

// stdarg.h, setjmp.h, assert.h, ctype.h, float.h, iso646.h, limits.h, errno.h, debug.h
#define	SHA1_BLOCK_LENGTH		64
#define	SHA1_DIGEST_LENGTH		20
#define SHA256_BLOCK_SIZE 32            // SHA256 outputs a 32 byte digest
typedef uint8_t BYTE;             // 8-bit byte
typedef uint32_t WORD;             // 32-bit word, change to "long" for 16-bit machines

typedef struct {
	BYTE data[64];
	WORD datalen;
	uint8_t bitlen[8];
	WORD state[5];
	WORD k[4];
} SHA1_CTX;


typedef struct {
	BYTE data[64];
	WORD datalen;
	uint8_t bitlen[8];
	WORD state[8];
} SHA256_CTX;


typedef union
{
	uint8_t c[64];
	uint32_t l[16];
} CHAR64LONG16;

typedef struct _aes_cbc {
	uint8_t padding_mode;
} aes_cbc_t;

typedef struct _aes_ctr {
	uint8_t counter_pos_start;
	uint8_t counter_len;
	uint8_t last_block_stop;
	uint8_t last_block[16];
} aes_ctr_t;

typedef struct _aes_gcm {
	uint8_t last_block_stop;
	uint8_t last_block[16];
	uint8_t ghash_key[16];
	uint8_t aad_cache[16];
	uint8_t auth_tag[16];
	uint8_t auth_j0[16];
	uint8_t aad_cache_len;
	size_t aad_len;
	size_t ct_len;
	uint8_t gcm_op;
} aes_gcm_t;

typedef struct _aes_keyschedule_ctx {
	uint24_t keysize;
	uint32_t round_keys[60];
	uint8_t iv[16];
	uint8_t cipher_mode;
	uint8_t op_assoc;
	union {
		aes_gcm_t gcm;
		aes_ctr_t ctr;
		aes_cbc_t cbc;
	} mode;
} aes_ctx;

// Entropy Pool
#define CEMU_CONSOLE (char*)0xFB0000
#define EPOOL_SIZE 128
typedef struct _csprng_state {
	volatile uint8_t* eread;
	uint8_t epool[EPOOL_SIZE];
} csprng_state_t;
csprng_state_t csprng_state={NULL, {}};
bool prng_init = false;
uint8_t Base64Code[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
// MAIN FUNCTIONS


// LIB FUNCTIONS
bool hashlib_CSPRNGInit(void);
bool hashlib_CSPRNGAddEntropy(void);
uint32_t hashlib_CSPRNGRandom(void);
void hashlib_RandomBytes(uint8_t* buffer, size_t size);


uint32_t hashlib_CRC32(const uint8_t *buf, size_t len);

//void hashlib_Sha1Init(SHA1_CTX *ctx);
//void hashlib_Sha1Update(SHA1_CTX *ctx, const BYTE data[], uint32_t len);
void hashlib_Sha256Init(SHA256_CTX *ctx);
void hashlib_Sha256Update(SHA256_CTX *ctx, const BYTE data[], uint32_t len);
void hashlib_Sha256Final(SHA256_CTX *ctx, BYTE hash[]);
void hashlib_MGF1Hash(uint8_t* data, size_t datalen, uint8_t* outbuf, size_t outlen);
bool hashlib_CompareDigest(uint8_t *dig1, uint8_t *dig2, size_t len);

void aes_encrypt_block(const BYTE in[], BYTE out[], aes_ctx* ks);

#define RANDBYTES 16


typedef enum {
	AES_OK,
	AES_INVALID_ARG,
	AES_INVALID_MSG,
	AES_INVALID_CIPHERMODE,
	AES_INVALID_PADDINGMODE,
	AES_INVALID_CIPHERTEXT,
	AES_INVALID_OPERATION,
	AES_BLOCK_THRESHOLD_EXCEEDED
} aes_error_t;

typedef enum {
	RSA_OK,
	RSA_INVALID_ARG,
	RSA_INVALID_MSG,
	RSA_INVALID_MODULUS,
	RSA_ENCODING_ERROR
} rsa_error_t;



size_t hashlib_b64encode(char *b64buffer, const uint8_t *data, size_t len);
size_t hashlib_b64decode(uint8_t *buffer, size_t len, const char *b64data);

aes_error_t hashlib_AESEncrypt(aes_ctx* ctx, const BYTE in[], size_t in_len, BYTE out[]);
aes_error_t hashlib_AESDecrypt(aes_ctx* ctx, const BYTE in[], size_t in_len, BYTE out[]);
size_t hashlib_AESPadMessage(const uint8_t* in, size_t len, uint8_t* out, uint8_t schm);
size_t hashlib_AESStripPadding(const uint8_t* in, size_t len, uint8_t* out, uint8_t schm);
//bool hashlib_AESVerifyMAC(uint8_t *ciphertext, size_t len, aes_ctx *ks_mac);

rsa_error_t hashlib_RSAEncrypt(const uint8_t* msg, size_t msglen, uint8_t *ct, const uint8_t* pubkey, size_t keylen);
void rmemcpy(void *dest, void *src, size_t len);
/*
 #########################
 ### Math Dependencies ###
 #########################
 */

/*
 
 First the uint32_t value of the address is read
 Then we do 50 rounds of XOR'ing the value at the address with the existing value
 The RNG is seeded using the resulting value
 */


// sprng stuff done in asm, removed from C

/* #define ROTLEFT(a,b) (((a) << (b)) | ((a) >> (32-(b))))
 #define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))
 
 #define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
 #define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
 #define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
 #define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
 #define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
 #define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))
 */
/**************************** VARIABLES *****************************/
/* static const WORD k[64] = {
 0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
 0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
 0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
 0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
 0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
 0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
 0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
 0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
 }; */

/*********************** FUNCTION DEFINITIONS ***********************/
/* void sha256_transform(SHA256_CTX *ctx, const BYTE data[])
 {
 WORD a, b, c, d, e, f, g, h, i, j, tmp1, tmp2, m[64];
 
 for (i = 0, j = 0; i < 16; ++i, j += 4){
 WORD d1 = data[j];
 WORD d2 = data[j + 1];
 WORD d3 = data[j + 2];
 WORD d4 = data[j + 3];
 m[i] = (d1 << 24) + (d2 << 16) + (d3 << 8) + d4;
 }
 for ( ; i < 64; ++i)
 m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];
 
 a = ctx->state[0];
 b = ctx->state[1];
 c = ctx->state[2];
 d = ctx->state[3];
 e = ctx->state[4];
 f = ctx->state[5];
 g = ctx->state[6];
 h = ctx->state[7];
 
 for (i = 0; i < 64; ++i) {
 tmp1 = h + EP1(e) + CH(e,f,g) + k[i] + m[i];
 tmp2 = EP0(a) + MAJ(a,b,c);
 h = g;
 g = f;
 f = e;
 e = d + tmp1;
 d = c;
 c = b;
 b = a;
 a = tmp1 + tmp2;
 }
 
 ctx->state[0] += a;
 ctx->state[1] += b;
 ctx->state[2] += c;
 ctx->state[3] += d;
 ctx->state[4] += e;
 ctx->state[5] += f;
 ctx->state[6] += g;
 ctx->state[7] += h;
 } */

/* void hashlib_Sha256Init(SHA256_CTX *ctx)
 {
 ctx->datalen = 0;
 zero64(ctx->bitlen);
 ctx->state[0] = 0x6a09e667;
 ctx->state[1] = 0xbb67ae85;
 ctx->state[2] = 0x3c6ef372;
 ctx->state[3] = 0xa54ff53a;
 ctx->state[4] = 0x510e527f;
 ctx->state[5] = 0x9b05688c;
 ctx->state[6] = 0x1f83d9ab;
 ctx->state[7] = 0x5be0cd19;
 }
 
 void hashlib_Sha256Update(SHA256_CTX *ctx, const BYTE data[], uint32_t len)
 {
 WORD i;
 
 for (i = 0; i < len; ++i) {
 ctx->data[ctx->datalen] = data[i];
 ctx->datalen++;
 if (ctx->datalen == 64) {
 sha256_transform(ctx, ctx->data);
 add64iu(&ctx->bitlen, 512);
 ctx->datalen = 0;
 }
 }
 }
 
 void hashlib_Sha256Final(SHA256_CTX *ctx, BYTE hash[])
 {
 WORD i;
 int x;
 i = ctx->datalen;
 
 // Pad whatever data is left in the buffer.
 if (ctx->datalen < 56) {
 ctx->data[i++] = 0x80;
 while (i < 56)
 ctx->data[i++] = 0x00;
 }
 else {
 ctx->data[i++] = 0x80;
 while (i < 64)
 ctx->data[i++] = 0x00;
 sha256_transform(ctx, ctx->data);
 memset(ctx->data, 0, 56);
 }
 
 // Append to the padding the total message's length in bits and transform.
 add64iu(&ctx->bitlen,ctx->datalen * 8);
 for (x=0;x<8;x++){ //put this in a loop for efficiency
 ctx->data[63-x] = ctx->bitlen[x];
 }
 sha256_transform(ctx, ctx->data);
 
 // Since this implementation uses little endian byte ordering and SHA uses big endian,
 // reverse all the bytes when copying the final state to the output hash.
 for (i = 0; i < 4; ++i) {
 hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
 hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
 hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
 hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
 hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
 hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0x000000ff;
 hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0x000000ff;
 hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0x000000ff;
 }
 }
 */





/****************************** MACROS ******************************/
// The least significant byte of the word is rotated to the end.
#define KE_ROTWORD(x) (((x) << 8) | ((x) >> 24))

#define TRUE  1
#define FALSE 0

/**************************** DATA TYPES ****************************/
#define AES_128_ROUNDS 10
#define AES_192_ROUNDS 12
#define AES_256_ROUNDS 14

#define AES_BLOCK_SIZE 16               // AES operates on 16 bytes at a time

static const BYTE aes_sbox[16][16] = {
	{0x63,0x7C,0x77,0x7B,0xF2,0x6B,0x6F,0xC5,0x30,0x01,0x67,0x2B,0xFE,0xD7,0xAB,0x76},
	{0xCA,0x82,0xC9,0x7D,0xFA,0x59,0x47,0xF0,0xAD,0xD4,0xA2,0xAF,0x9C,0xA4,0x72,0xC0},
	{0xB7,0xFD,0x93,0x26,0x36,0x3F,0xF7,0xCC,0x34,0xA5,0xE5,0xF1,0x71,0xD8,0x31,0x15},
	{0x04,0xC7,0x23,0xC3,0x18,0x96,0x05,0x9A,0x07,0x12,0x80,0xE2,0xEB,0x27,0xB2,0x75},
	{0x09,0x83,0x2C,0x1A,0x1B,0x6E,0x5A,0xA0,0x52,0x3B,0xD6,0xB3,0x29,0xE3,0x2F,0x84},
	{0x53,0xD1,0x00,0xED,0x20,0xFC,0xB1,0x5B,0x6A,0xCB,0xBE,0x39,0x4A,0x4C,0x58,0xCF},
	{0xD0,0xEF,0xAA,0xFB,0x43,0x4D,0x33,0x85,0x45,0xF9,0x02,0x7F,0x50,0x3C,0x9F,0xA8},
	{0x51,0xA3,0x40,0x8F,0x92,0x9D,0x38,0xF5,0xBC,0xB6,0xDA,0x21,0x10,0xFF,0xF3,0xD2},
	{0xCD,0x0C,0x13,0xEC,0x5F,0x97,0x44,0x17,0xC4,0xA7,0x7E,0x3D,0x64,0x5D,0x19,0x73},
	{0x60,0x81,0x4F,0xDC,0x22,0x2A,0x90,0x88,0x46,0xEE,0xB8,0x14,0xDE,0x5E,0x0B,0xDB},
	{0xE0,0x32,0x3A,0x0A,0x49,0x06,0x24,0x5C,0xC2,0xD3,0xAC,0x62,0x91,0x95,0xE4,0x79},
	{0xE7,0xC8,0x37,0x6D,0x8D,0xD5,0x4E,0xA9,0x6C,0x56,0xF4,0xEA,0x65,0x7A,0xAE,0x08},
	{0xBA,0x78,0x25,0x2E,0x1C,0xA6,0xB4,0xC6,0xE8,0xDD,0x74,0x1F,0x4B,0xBD,0x8B,0x8A},
	{0x70,0x3E,0xB5,0x66,0x48,0x03,0xF6,0x0E,0x61,0x35,0x57,0xB9,0x86,0xC1,0x1D,0x9E},
	{0xE1,0xF8,0x98,0x11,0x69,0xD9,0x8E,0x94,0x9B,0x1E,0x87,0xE9,0xCE,0x55,0x28,0xDF},
	{0x8C,0xA1,0x89,0x0D,0xBF,0xE6,0x42,0x68,0x41,0x99,0x2D,0x0F,0xB0,0x54,0xBB,0x16}
};

static const BYTE aes_invsbox[16][16] = {
	{0x52,0x09,0x6A,0xD5,0x30,0x36,0xA5,0x38,0xBF,0x40,0xA3,0x9E,0x81,0xF3,0xD7,0xFB},
	{0x7C,0xE3,0x39,0x82,0x9B,0x2F,0xFF,0x87,0x34,0x8E,0x43,0x44,0xC4,0xDE,0xE9,0xCB},
	{0x54,0x7B,0x94,0x32,0xA6,0xC2,0x23,0x3D,0xEE,0x4C,0x95,0x0B,0x42,0xFA,0xC3,0x4E},
	{0x08,0x2E,0xA1,0x66,0x28,0xD9,0x24,0xB2,0x76,0x5B,0xA2,0x49,0x6D,0x8B,0xD1,0x25},
	{0x72,0xF8,0xF6,0x64,0x86,0x68,0x98,0x16,0xD4,0xA4,0x5C,0xCC,0x5D,0x65,0xB6,0x92},
	{0x6C,0x70,0x48,0x50,0xFD,0xED,0xB9,0xDA,0x5E,0x15,0x46,0x57,0xA7,0x8D,0x9D,0x84},
	{0x90,0xD8,0xAB,0x00,0x8C,0xBC,0xD3,0x0A,0xF7,0xE4,0x58,0x05,0xB8,0xB3,0x45,0x06},
	{0xD0,0x2C,0x1E,0x8F,0xCA,0x3F,0x0F,0x02,0xC1,0xAF,0xBD,0x03,0x01,0x13,0x8A,0x6B},
	{0x3A,0x91,0x11,0x41,0x4F,0x67,0xDC,0xEA,0x97,0xF2,0xCF,0xCE,0xF0,0xB4,0xE6,0x73},
	{0x96,0xAC,0x74,0x22,0xE7,0xAD,0x35,0x85,0xE2,0xF9,0x37,0xE8,0x1C,0x75,0xDF,0x6E},
	{0x47,0xF1,0x1A,0x71,0x1D,0x29,0xC5,0x89,0x6F,0xB7,0x62,0x0E,0xAA,0x18,0xBE,0x1B},
	{0xFC,0x56,0x3E,0x4B,0xC6,0xD2,0x79,0x20,0x9A,0xDB,0xC0,0xFE,0x78,0xCD,0x5A,0xF4},
	{0x1F,0xDD,0xA8,0x33,0x88,0x07,0xC7,0x31,0xB1,0x12,0x10,0x59,0x27,0x80,0xEC,0x5F},
	{0x60,0x51,0x7F,0xA9,0x19,0xB5,0x4A,0x0D,0x2D,0xE5,0x7A,0x9F,0x93,0xC9,0x9C,0xEF},
	{0xA0,0xE0,0x3B,0x4D,0xAE,0x2A,0xF5,0xB0,0xC8,0xEB,0xBB,0x3C,0x83,0x53,0x99,0x61},
	{0x17,0x2B,0x04,0x7E,0xBA,0x77,0xD6,0x26,0xE1,0x69,0x14,0x63,0x55,0x21,0x0C,0x7D}
};

// This table stores pre-calculated values for all possible GF(2^8) calculations.This
// table is only used by the (Inv)aes_MixColumns steps.
// USAGE: The second index (column) is the coefficient of multiplication. Only 7 different
// coefficients are used: 0x01, 0x02, 0x03, 0x09, 0x0b, 0x0d, 0x0e, but multiplication by
// 1 is negligible leaving only 6 coefficients. Each column of the table is devoted to one
// of these coefficients, in the ascending order of value, from values 0x00 to 0xFF.
static const BYTE gf_mul[256][6] = {
	{0x00,0x00,0x00,0x00,0x00,0x00},{0x02,0x03,0x09,0x0b,0x0d,0x0e},
	{0x04,0x06,0x12,0x16,0x1a,0x1c},{0x06,0x05,0x1b,0x1d,0x17,0x12},
	{0x08,0x0c,0x24,0x2c,0x34,0x38},{0x0a,0x0f,0x2d,0x27,0x39,0x36},
	{0x0c,0x0a,0x36,0x3a,0x2e,0x24},{0x0e,0x09,0x3f,0x31,0x23,0x2a},
	{0x10,0x18,0x48,0x58,0x68,0x70},{0x12,0x1b,0x41,0x53,0x65,0x7e},
	{0x14,0x1e,0x5a,0x4e,0x72,0x6c},{0x16,0x1d,0x53,0x45,0x7f,0x62},
	{0x18,0x14,0x6c,0x74,0x5c,0x48},{0x1a,0x17,0x65,0x7f,0x51,0x46},
	{0x1c,0x12,0x7e,0x62,0x46,0x54},{0x1e,0x11,0x77,0x69,0x4b,0x5a},
	{0x20,0x30,0x90,0xb0,0xd0,0xe0},{0x22,0x33,0x99,0xbb,0xdd,0xee},
	{0x24,0x36,0x82,0xa6,0xca,0xfc},{0x26,0x35,0x8b,0xad,0xc7,0xf2},
	{0x28,0x3c,0xb4,0x9c,0xe4,0xd8},{0x2a,0x3f,0xbd,0x97,0xe9,0xd6},
	{0x2c,0x3a,0xa6,0x8a,0xfe,0xc4},{0x2e,0x39,0xaf,0x81,0xf3,0xca},
	{0x30,0x28,0xd8,0xe8,0xb8,0x90},{0x32,0x2b,0xd1,0xe3,0xb5,0x9e},
	{0x34,0x2e,0xca,0xfe,0xa2,0x8c},{0x36,0x2d,0xc3,0xf5,0xaf,0x82},
	{0x38,0x24,0xfc,0xc4,0x8c,0xa8},{0x3a,0x27,0xf5,0xcf,0x81,0xa6},
	{0x3c,0x22,0xee,0xd2,0x96,0xb4},{0x3e,0x21,0xe7,0xd9,0x9b,0xba},
	{0x40,0x60,0x3b,0x7b,0xbb,0xdb},{0x42,0x63,0x32,0x70,0xb6,0xd5},
	{0x44,0x66,0x29,0x6d,0xa1,0xc7},{0x46,0x65,0x20,0x66,0xac,0xc9},
	{0x48,0x6c,0x1f,0x57,0x8f,0xe3},{0x4a,0x6f,0x16,0x5c,0x82,0xed},
	{0x4c,0x6a,0x0d,0x41,0x95,0xff},{0x4e,0x69,0x04,0x4a,0x98,0xf1},
	{0x50,0x78,0x73,0x23,0xd3,0xab},{0x52,0x7b,0x7a,0x28,0xde,0xa5},
	{0x54,0x7e,0x61,0x35,0xc9,0xb7},{0x56,0x7d,0x68,0x3e,0xc4,0xb9},
	{0x58,0x74,0x57,0x0f,0xe7,0x93},{0x5a,0x77,0x5e,0x04,0xea,0x9d},
	{0x5c,0x72,0x45,0x19,0xfd,0x8f},{0x5e,0x71,0x4c,0x12,0xf0,0x81},
	{0x60,0x50,0xab,0xcb,0x6b,0x3b},{0x62,0x53,0xa2,0xc0,0x66,0x35},
	{0x64,0x56,0xb9,0xdd,0x71,0x27},{0x66,0x55,0xb0,0xd6,0x7c,0x29},
	{0x68,0x5c,0x8f,0xe7,0x5f,0x03},{0x6a,0x5f,0x86,0xec,0x52,0x0d},
	{0x6c,0x5a,0x9d,0xf1,0x45,0x1f},{0x6e,0x59,0x94,0xfa,0x48,0x11},
	{0x70,0x48,0xe3,0x93,0x03,0x4b},{0x72,0x4b,0xea,0x98,0x0e,0x45},
	{0x74,0x4e,0xf1,0x85,0x19,0x57},{0x76,0x4d,0xf8,0x8e,0x14,0x59},
	{0x78,0x44,0xc7,0xbf,0x37,0x73},{0x7a,0x47,0xce,0xb4,0x3a,0x7d},
	{0x7c,0x42,0xd5,0xa9,0x2d,0x6f},{0x7e,0x41,0xdc,0xa2,0x20,0x61},
	{0x80,0xc0,0x76,0xf6,0x6d,0xad},{0x82,0xc3,0x7f,0xfd,0x60,0xa3},
	{0x84,0xc6,0x64,0xe0,0x77,0xb1},{0x86,0xc5,0x6d,0xeb,0x7a,0xbf},
	{0x88,0xcc,0x52,0xda,0x59,0x95},{0x8a,0xcf,0x5b,0xd1,0x54,0x9b},
	{0x8c,0xca,0x40,0xcc,0x43,0x89},{0x8e,0xc9,0x49,0xc7,0x4e,0x87},
	{0x90,0xd8,0x3e,0xae,0x05,0xdd},{0x92,0xdb,0x37,0xa5,0x08,0xd3},
	{0x94,0xde,0x2c,0xb8,0x1f,0xc1},{0x96,0xdd,0x25,0xb3,0x12,0xcf},
	{0x98,0xd4,0x1a,0x82,0x31,0xe5},{0x9a,0xd7,0x13,0x89,0x3c,0xeb},
	{0x9c,0xd2,0x08,0x94,0x2b,0xf9},{0x9e,0xd1,0x01,0x9f,0x26,0xf7},
	{0xa0,0xf0,0xe6,0x46,0xbd,0x4d},{0xa2,0xf3,0xef,0x4d,0xb0,0x43},
	{0xa4,0xf6,0xf4,0x50,0xa7,0x51},{0xa6,0xf5,0xfd,0x5b,0xaa,0x5f},
	{0xa8,0xfc,0xc2,0x6a,0x89,0x75},{0xaa,0xff,0xcb,0x61,0x84,0x7b},
	{0xac,0xfa,0xd0,0x7c,0x93,0x69},{0xae,0xf9,0xd9,0x77,0x9e,0x67},
	{0xb0,0xe8,0xae,0x1e,0xd5,0x3d},{0xb2,0xeb,0xa7,0x15,0xd8,0x33},
	{0xb4,0xee,0xbc,0x08,0xcf,0x21},{0xb6,0xed,0xb5,0x03,0xc2,0x2f},
	{0xb8,0xe4,0x8a,0x32,0xe1,0x05},{0xba,0xe7,0x83,0x39,0xec,0x0b},
	{0xbc,0xe2,0x98,0x24,0xfb,0x19},{0xbe,0xe1,0x91,0x2f,0xf6,0x17},
	{0xc0,0xa0,0x4d,0x8d,0xd6,0x76},{0xc2,0xa3,0x44,0x86,0xdb,0x78},
	{0xc4,0xa6,0x5f,0x9b,0xcc,0x6a},{0xc6,0xa5,0x56,0x90,0xc1,0x64},
	{0xc8,0xac,0x69,0xa1,0xe2,0x4e},{0xca,0xaf,0x60,0xaa,0xef,0x40},
	{0xcc,0xaa,0x7b,0xb7,0xf8,0x52},{0xce,0xa9,0x72,0xbc,0xf5,0x5c},
	{0xd0,0xb8,0x05,0xd5,0xbe,0x06},{0xd2,0xbb,0x0c,0xde,0xb3,0x08},
	{0xd4,0xbe,0x17,0xc3,0xa4,0x1a},{0xd6,0xbd,0x1e,0xc8,0xa9,0x14},
	{0xd8,0xb4,0x21,0xf9,0x8a,0x3e},{0xda,0xb7,0x28,0xf2,0x87,0x30},
	{0xdc,0xb2,0x33,0xef,0x90,0x22},{0xde,0xb1,0x3a,0xe4,0x9d,0x2c},
	{0xe0,0x90,0xdd,0x3d,0x06,0x96},{0xe2,0x93,0xd4,0x36,0x0b,0x98},
	{0xe4,0x96,0xcf,0x2b,0x1c,0x8a},{0xe6,0x95,0xc6,0x20,0x11,0x84},
	{0xe8,0x9c,0xf9,0x11,0x32,0xae},{0xea,0x9f,0xf0,0x1a,0x3f,0xa0},
	{0xec,0x9a,0xeb,0x07,0x28,0xb2},{0xee,0x99,0xe2,0x0c,0x25,0xbc},
	{0xf0,0x88,0x95,0x65,0x6e,0xe6},{0xf2,0x8b,0x9c,0x6e,0x63,0xe8},
	{0xf4,0x8e,0x87,0x73,0x74,0xfa},{0xf6,0x8d,0x8e,0x78,0x79,0xf4},
	{0xf8,0x84,0xb1,0x49,0x5a,0xde},{0xfa,0x87,0xb8,0x42,0x57,0xd0},
	{0xfc,0x82,0xa3,0x5f,0x40,0xc2},{0xfe,0x81,0xaa,0x54,0x4d,0xcc},
	{0x1b,0x9b,0xec,0xf7,0xda,0x41},{0x19,0x98,0xe5,0xfc,0xd7,0x4f},
	{0x1f,0x9d,0xfe,0xe1,0xc0,0x5d},{0x1d,0x9e,0xf7,0xea,0xcd,0x53},
	{0x13,0x97,0xc8,0xdb,0xee,0x79},{0x11,0x94,0xc1,0xd0,0xe3,0x77},
	{0x17,0x91,0xda,0xcd,0xf4,0x65},{0x15,0x92,0xd3,0xc6,0xf9,0x6b},
	{0x0b,0x83,0xa4,0xaf,0xb2,0x31},{0x09,0x80,0xad,0xa4,0xbf,0x3f},
	{0x0f,0x85,0xb6,0xb9,0xa8,0x2d},{0x0d,0x86,0xbf,0xb2,0xa5,0x23},
	{0x03,0x8f,0x80,0x83,0x86,0x09},{0x01,0x8c,0x89,0x88,0x8b,0x07},
	{0x07,0x89,0x92,0x95,0x9c,0x15},{0x05,0x8a,0x9b,0x9e,0x91,0x1b},
	{0x3b,0xab,0x7c,0x47,0x0a,0xa1},{0x39,0xa8,0x75,0x4c,0x07,0xaf},
	{0x3f,0xad,0x6e,0x51,0x10,0xbd},{0x3d,0xae,0x67,0x5a,0x1d,0xb3},
	{0x33,0xa7,0x58,0x6b,0x3e,0x99},{0x31,0xa4,0x51,0x60,0x33,0x97},
	{0x37,0xa1,0x4a,0x7d,0x24,0x85},{0x35,0xa2,0x43,0x76,0x29,0x8b},
	{0x2b,0xb3,0x34,0x1f,0x62,0xd1},{0x29,0xb0,0x3d,0x14,0x6f,0xdf},
	{0x2f,0xb5,0x26,0x09,0x78,0xcd},{0x2d,0xb6,0x2f,0x02,0x75,0xc3},
	{0x23,0xbf,0x10,0x33,0x56,0xe9},{0x21,0xbc,0x19,0x38,0x5b,0xe7},
	{0x27,0xb9,0x02,0x25,0x4c,0xf5},{0x25,0xba,0x0b,0x2e,0x41,0xfb},
	{0x5b,0xfb,0xd7,0x8c,0x61,0x9a},{0x59,0xf8,0xde,0x87,0x6c,0x94},
	{0x5f,0xfd,0xc5,0x9a,0x7b,0x86},{0x5d,0xfe,0xcc,0x91,0x76,0x88},
	{0x53,0xf7,0xf3,0xa0,0x55,0xa2},{0x51,0xf4,0xfa,0xab,0x58,0xac},
	{0x57,0xf1,0xe1,0xb6,0x4f,0xbe},{0x55,0xf2,0xe8,0xbd,0x42,0xb0},
	{0x4b,0xe3,0x9f,0xd4,0x09,0xea},{0x49,0xe0,0x96,0xdf,0x04,0xe4},
	{0x4f,0xe5,0x8d,0xc2,0x13,0xf6},{0x4d,0xe6,0x84,0xc9,0x1e,0xf8},
	{0x43,0xef,0xbb,0xf8,0x3d,0xd2},{0x41,0xec,0xb2,0xf3,0x30,0xdc},
	{0x47,0xe9,0xa9,0xee,0x27,0xce},{0x45,0xea,0xa0,0xe5,0x2a,0xc0},
	{0x7b,0xcb,0x47,0x3c,0xb1,0x7a},{0x79,0xc8,0x4e,0x37,0xbc,0x74},
	{0x7f,0xcd,0x55,0x2a,0xab,0x66},{0x7d,0xce,0x5c,0x21,0xa6,0x68},
	{0x73,0xc7,0x63,0x10,0x85,0x42},{0x71,0xc4,0x6a,0x1b,0x88,0x4c},
	{0x77,0xc1,0x71,0x06,0x9f,0x5e},{0x75,0xc2,0x78,0x0d,0x92,0x50},
	{0x6b,0xd3,0x0f,0x64,0xd9,0x0a},{0x69,0xd0,0x06,0x6f,0xd4,0x04},
	{0x6f,0xd5,0x1d,0x72,0xc3,0x16},{0x6d,0xd6,0x14,0x79,0xce,0x18},
	{0x63,0xdf,0x2b,0x48,0xed,0x32},{0x61,0xdc,0x22,0x43,0xe0,0x3c},
	{0x67,0xd9,0x39,0x5e,0xf7,0x2e},{0x65,0xda,0x30,0x55,0xfa,0x20},
	{0x9b,0x5b,0x9a,0x01,0xb7,0xec},{0x99,0x58,0x93,0x0a,0xba,0xe2},
	{0x9f,0x5d,0x88,0x17,0xad,0xf0},{0x9d,0x5e,0x81,0x1c,0xa0,0xfe},
	{0x93,0x57,0xbe,0x2d,0x83,0xd4},{0x91,0x54,0xb7,0x26,0x8e,0xda},
	{0x97,0x51,0xac,0x3b,0x99,0xc8},{0x95,0x52,0xa5,0x30,0x94,0xc6},
	{0x8b,0x43,0xd2,0x59,0xdf,0x9c},{0x89,0x40,0xdb,0x52,0xd2,0x92},
	{0x8f,0x45,0xc0,0x4f,0xc5,0x80},{0x8d,0x46,0xc9,0x44,0xc8,0x8e},
	{0x83,0x4f,0xf6,0x75,0xeb,0xa4},{0x81,0x4c,0xff,0x7e,0xe6,0xaa},
	{0x87,0x49,0xe4,0x63,0xf1,0xb8},{0x85,0x4a,0xed,0x68,0xfc,0xb6},
	{0xbb,0x6b,0x0a,0xb1,0x67,0x0c},{0xb9,0x68,0x03,0xba,0x6a,0x02},
	{0xbf,0x6d,0x18,0xa7,0x7d,0x10},{0xbd,0x6e,0x11,0xac,0x70,0x1e},
	{0xb3,0x67,0x2e,0x9d,0x53,0x34},{0xb1,0x64,0x27,0x96,0x5e,0x3a},
	{0xb7,0x61,0x3c,0x8b,0x49,0x28},{0xb5,0x62,0x35,0x80,0x44,0x26},
	{0xab,0x73,0x42,0xe9,0x0f,0x7c},{0xa9,0x70,0x4b,0xe2,0x02,0x72},
	{0xaf,0x75,0x50,0xff,0x15,0x60},{0xad,0x76,0x59,0xf4,0x18,0x6e},
	{0xa3,0x7f,0x66,0xc5,0x3b,0x44},{0xa1,0x7c,0x6f,0xce,0x36,0x4a},
	{0xa7,0x79,0x74,0xd3,0x21,0x58},{0xa5,0x7a,0x7d,0xd8,0x2c,0x56},
	{0xdb,0x3b,0xa1,0x7a,0x0c,0x37},{0xd9,0x38,0xa8,0x71,0x01,0x39},
	{0xdf,0x3d,0xb3,0x6c,0x16,0x2b},{0xdd,0x3e,0xba,0x67,0x1b,0x25},
	{0xd3,0x37,0x85,0x56,0x38,0x0f},{0xd1,0x34,0x8c,0x5d,0x35,0x01},
	{0xd7,0x31,0x97,0x40,0x22,0x13},{0xd5,0x32,0x9e,0x4b,0x2f,0x1d},
	{0xcb,0x23,0xe9,0x22,0x64,0x47},{0xc9,0x20,0xe0,0x29,0x69,0x49},
	{0xcf,0x25,0xfb,0x34,0x7e,0x5b},{0xcd,0x26,0xf2,0x3f,0x73,0x55},
	{0xc3,0x2f,0xcd,0x0e,0x50,0x7f},{0xc1,0x2c,0xc4,0x05,0x5d,0x71},
	{0xc7,0x29,0xdf,0x18,0x4a,0x63},{0xc5,0x2a,0xd6,0x13,0x47,0x6d},
	{0xfb,0x0b,0x31,0xca,0xdc,0xd7},{0xf9,0x08,0x38,0xc1,0xd1,0xd9},
	{0xff,0x0d,0x23,0xdc,0xc6,0xcb},{0xfd,0x0e,0x2a,0xd7,0xcb,0xc5},
	{0xf3,0x07,0x15,0xe6,0xe8,0xef},{0xf1,0x04,0x1c,0xed,0xe5,0xe1},
	{0xf7,0x01,0x07,0xf0,0xf2,0xf3},{0xf5,0x02,0x0e,0xfb,0xff,0xfd},
	{0xeb,0x13,0x79,0x92,0xb4,0xa7},{0xe9,0x10,0x70,0x99,0xb9,0xa9},
	{0xef,0x15,0x6b,0x84,0xae,0xbb},{0xed,0x16,0x62,0x8f,0xa3,0xb5},
	{0xe3,0x1f,0x5d,0xbe,0x80,0x9f},{0xe1,0x1c,0x54,0xb5,0x8d,0x91},
	{0xe7,0x19,0x4f,0xa8,0x9a,0x83},{0xe5,0x1a,0x46,0xa3,0x97,0x8d}
};



/*********************** FUNCTION DEFINITIONS ***********************/
// XORs the in and out buffers, storing the result in out. Length is in bytes.
void xor_buf(const BYTE in[], BYTE out[], size_t len)
{
	size_t idx;
	
	for (idx = 0; idx < len; idx++)
		out[idx] ^= in[idx];
}

/////////////////
// KEY EXPANSION
/////////////////

// Substitutes a word using the AES S-Box.
WORD aes_SubWord(WORD word)
{
	WORD result;
	
	result = (uint32_t)aes_sbox[(word >> 4) & 0x0000000F][word & 0x0000000F];
	result += (uint32_t)aes_sbox[(word >> 12) & 0x0000000F][(word >> 8) & 0x0000000F] << 8;
	result += (uint32_t)aes_sbox[(word >> 20) & 0x0000000F][(word >> 16) & 0x0000000F] << 16;
	result += (uint32_t)aes_sbox[(((uint32_t)word) >> 28) & 0x0000000F][(((uint32_t)word) >> 24) & 0x0000000F] << 24;
	return result;
}

enum _ciphermodes{
	AES_CBC,
	AES_CTR,
	AES_GCM
};

enum _aes_padding_schemes {
	SCHM_PKCS7,         // Pad with padding size
	SCHM_DEFAULT = SCHM_PKCS7,
	SCHM_ISO2        // Pad with 0x80,0x00...0x00
};


// AES_FLAG_CTR_IVNONCELEN(len) = ((len)<<4)
// AES_FLAG_CTR_IVCOUNTERLEN(len) = ((len)<<8)

/*
 typedef struct _aes_cbc {
 uint8_t padding_mode;
 } aes_cbc_t;
 
 typedef struct _aes_ctr {
 uint8_t ctr_len;
 uint8_t block_pos;
 uint8_t prev_block[16];
 } aes_ctr_t;
 
 typedef struct _aes_keyschedule_ctx {
 uint24_t keysize;
 uint32_t round_keys[60];
 uint8_t iv[16];
 uint8_t cipher_mode;
 union {
 aes_ctr_t ctr;
 aes_cbc_t cbc;
 } mode;
 } aes_ctx;
 */
#define FLAGS_GET_LSB2 3
#define FLAGS_GET_LSB4 15
#define MIN(x, y) ((x) < (y)) ? (x) : (y)


/////////////////
// ADD ROUND KEY
/////////////////

// Performs the aes_AddRoundKey step. Each round has its own pre-generated 16-byte key in the
// form of 4 integers (the "w" array). Each integer is XOR'd by one column of the state.
// Also performs the job of aes_InvAddRoundKey(); since the function is a simple XOR process,
// it is its own inverse.
void aes_AddRoundKey(BYTE state[][4], const WORD w[])
{
	BYTE subkey[4];
	
	// memcpy(subkey,&w[idx],4); // Not accurate for big endian machines
	// Subkey 1
	subkey[0] = w[0] >> 24;
	subkey[1] = w[0] >> 16;
	subkey[2] = w[0] >> 8;
	subkey[3] = w[0];
	state[0][0] ^= subkey[0];
	state[1][0] ^= subkey[1];
	state[2][0] ^= subkey[2];
	state[3][0] ^= subkey[3];
	// Subkey 2
	subkey[0] = w[1] >> 24;
	subkey[1] = w[1] >> 16;
	subkey[2] = w[1] >> 8;
	subkey[3] = w[1];
	state[0][1] ^= subkey[0];
	state[1][1] ^= subkey[1];
	state[2][1] ^= subkey[2];
	state[3][1] ^= subkey[3];
	// Subkey 3
	subkey[0] = w[2] >> 24;
	subkey[1] = w[2] >> 16;
	subkey[2] = w[2] >> 8;
	subkey[3] = w[2];
	state[0][2] ^= subkey[0];
	state[1][2] ^= subkey[1];
	state[2][2] ^= subkey[2];
	state[3][2] ^= subkey[3];
	// Subkey 4
	subkey[0] = w[3] >> 24;
	subkey[1] = w[3] >> 16;
	subkey[2] = w[3] >> 8;
	subkey[3] = w[3];
	state[0][3] ^= subkey[0];
	state[1][3] ^= subkey[1];
	state[2][3] ^= subkey[2];
	state[3][3] ^= subkey[3];
	memset(subkey, 0, sizeof subkey);
}

/////////////////
// (Inv)aes_SubBytes
/////////////////

// Performs the aes_SubBytes step. All bytes in the state are substituted with a
// pre-calculated value from a lookup table.
void aes_SubBytes(BYTE state[][4])
{
	state[0][0] = aes_sbox[state[0][0] >> 4][state[0][0] & 0x0F];
	state[0][1] = aes_sbox[state[0][1] >> 4][state[0][1] & 0x0F];
	state[0][2] = aes_sbox[state[0][2] >> 4][state[0][2] & 0x0F];
	state[0][3] = aes_sbox[state[0][3] >> 4][state[0][3] & 0x0F];
	state[1][0] = aes_sbox[state[1][0] >> 4][state[1][0] & 0x0F];
	state[1][1] = aes_sbox[state[1][1] >> 4][state[1][1] & 0x0F];
	state[1][2] = aes_sbox[state[1][2] >> 4][state[1][2] & 0x0F];
	state[1][3] = aes_sbox[state[1][3] >> 4][state[1][3] & 0x0F];
	state[2][0] = aes_sbox[state[2][0] >> 4][state[2][0] & 0x0F];
	state[2][1] = aes_sbox[state[2][1] >> 4][state[2][1] & 0x0F];
	state[2][2] = aes_sbox[state[2][2] >> 4][state[2][2] & 0x0F];
	state[2][3] = aes_sbox[state[2][3] >> 4][state[2][3] & 0x0F];
	state[3][0] = aes_sbox[state[3][0] >> 4][state[3][0] & 0x0F];
	state[3][1] = aes_sbox[state[3][1] >> 4][state[3][1] & 0x0F];
	state[3][2] = aes_sbox[state[3][2] >> 4][state[3][2] & 0x0F];
	state[3][3] = aes_sbox[state[3][3] >> 4][state[3][3] & 0x0F];
}

void aes_InvSubBytes(BYTE state[][4])
{
	state[0][0] = aes_invsbox[state[0][0] >> 4][state[0][0] & 0x0F];
	state[0][1] = aes_invsbox[state[0][1] >> 4][state[0][1] & 0x0F];
	state[0][2] = aes_invsbox[state[0][2] >> 4][state[0][2] & 0x0F];
	state[0][3] = aes_invsbox[state[0][3] >> 4][state[0][3] & 0x0F];
	state[1][0] = aes_invsbox[state[1][0] >> 4][state[1][0] & 0x0F];
	state[1][1] = aes_invsbox[state[1][1] >> 4][state[1][1] & 0x0F];
	state[1][2] = aes_invsbox[state[1][2] >> 4][state[1][2] & 0x0F];
	state[1][3] = aes_invsbox[state[1][3] >> 4][state[1][3] & 0x0F];
	state[2][0] = aes_invsbox[state[2][0] >> 4][state[2][0] & 0x0F];
	state[2][1] = aes_invsbox[state[2][1] >> 4][state[2][1] & 0x0F];
	state[2][2] = aes_invsbox[state[2][2] >> 4][state[2][2] & 0x0F];
	state[2][3] = aes_invsbox[state[2][3] >> 4][state[2][3] & 0x0F];
	state[3][0] = aes_invsbox[state[3][0] >> 4][state[3][0] & 0x0F];
	state[3][1] = aes_invsbox[state[3][1] >> 4][state[3][1] & 0x0F];
	state[3][2] = aes_invsbox[state[3][2] >> 4][state[3][2] & 0x0F];
	state[3][3] = aes_invsbox[state[3][3] >> 4][state[3][3] & 0x0F];
}

/////////////////
// (Inv)aes_ShiftRows
/////////////////

// Performs the aes_ShiftRows step. All rows are shifted cylindrically to the left.
void aes_ShiftRows(BYTE state[][4])
{
	int t;
	
	// Shift left by 1
	t = state[1][0];
	state[1][0] = state[1][1];
	state[1][1] = state[1][2];
	state[1][2] = state[1][3];
	state[1][3] = t;
	// Shift left by 2
	t = state[2][0];
	state[2][0] = state[2][2];
	state[2][2] = t;
	t = state[2][1];
	state[2][1] = state[2][3];
	state[2][3] = t;
	// Shift left by 3
	t = state[3][0];
	state[3][0] = state[3][3];
	state[3][3] = state[3][2];
	state[3][2] = state[3][1];
	state[3][1] = t;
	t = 0;
}

// All rows are shifted cylindrically to the right.
void aes_InvShiftRows(BYTE state[][4])
{
	int t;
	
	// Shift right by 1
	t = state[1][3];
	state[1][3] = state[1][2];
	state[1][2] = state[1][1];
	state[1][1] = state[1][0];
	state[1][0] = t;
	// Shift right by 2
	t = state[2][3];
	state[2][3] = state[2][1];
	state[2][1] = t;
	t = state[2][2];
	state[2][2] = state[2][0];
	state[2][0] = t;
	// Shift right by 3
	t = state[3][3];
	state[3][3] = state[3][0];
	state[3][0] = state[3][1];
	state[3][1] = state[3][2];
	state[3][2] = t;
	t = 0;
}

/////////////////
// (Inv)aes_MixColumns
/////////////////

// Performs the MixColums step. The state is multiplied by itself using matrix
// multiplication in a Galios Field 2^8. All multiplication is pre-computed in a table.
// Addition is equivilent to XOR. (Must always make a copy of the column as the original
// values will be destoyed.)
void aes_MixColumns(BYTE state[][4])
{
	BYTE col[4];
	
	// Column 1
	col[0] = state[0][0];
	col[1] = state[1][0];
	col[2] = state[2][0];
	col[3] = state[3][0];
	state[0][0] = gf_mul[col[0]][0];
	state[0][0] ^= gf_mul[col[1]][1];
	state[0][0] ^= col[2];
	state[0][0] ^= col[3];
	state[1][0] = col[0];
	state[1][0] ^= gf_mul[col[1]][0];
	state[1][0] ^= gf_mul[col[2]][1];
	state[1][0] ^= col[3];
	state[2][0] = col[0];
	state[2][0] ^= col[1];
	state[2][0] ^= gf_mul[col[2]][0];
	state[2][0] ^= gf_mul[col[3]][1];
	state[3][0] = gf_mul[col[0]][1];
	state[3][0] ^= col[1];
	state[3][0] ^= col[2];
	state[3][0] ^= gf_mul[col[3]][0];
	// Column 2
	col[0] = state[0][1];
	col[1] = state[1][1];
	col[2] = state[2][1];
	col[3] = state[3][1];
	state[0][1] = gf_mul[col[0]][0];
	state[0][1] ^= gf_mul[col[1]][1];
	state[0][1] ^= col[2];
	state[0][1] ^= col[3];
	state[1][1] = col[0];
	state[1][1] ^= gf_mul[col[1]][0];
	state[1][1] ^= gf_mul[col[2]][1];
	state[1][1] ^= col[3];
	state[2][1] = col[0];
	state[2][1] ^= col[1];
	state[2][1] ^= gf_mul[col[2]][0];
	state[2][1] ^= gf_mul[col[3]][1];
	state[3][1] = gf_mul[col[0]][1];
	state[3][1] ^= col[1];
	state[3][1] ^= col[2];
	state[3][1] ^= gf_mul[col[3]][0];
	// Column 3
	col[0] = state[0][2];
	col[1] = state[1][2];
	col[2] = state[2][2];
	col[3] = state[3][2];
	state[0][2] = gf_mul[col[0]][0];
	state[0][2] ^= gf_mul[col[1]][1];
	state[0][2] ^= col[2];
	state[0][2] ^= col[3];
	state[1][2] = col[0];
	state[1][2] ^= gf_mul[col[1]][0];
	state[1][2] ^= gf_mul[col[2]][1];
	state[1][2] ^= col[3];
	state[2][2] = col[0];
	state[2][2] ^= col[1];
	state[2][2] ^= gf_mul[col[2]][0];
	state[2][2] ^= gf_mul[col[3]][1];
	state[3][2] = gf_mul[col[0]][1];
	state[3][2] ^= col[1];
	state[3][2] ^= col[2];
	state[3][2] ^= gf_mul[col[3]][0];
	// Column 4
	col[0] = state[0][3];
	col[1] = state[1][3];
	col[2] = state[2][3];
	col[3] = state[3][3];
	state[0][3] = gf_mul[col[0]][0];
	state[0][3] ^= gf_mul[col[1]][1];
	state[0][3] ^= col[2];
	state[0][3] ^= col[3];
	state[1][3] = col[0];
	state[1][3] ^= gf_mul[col[1]][0];
	state[1][3] ^= gf_mul[col[2]][1];
	state[1][3] ^= col[3];
	state[2][3] = col[0];
	state[2][3] ^= col[1];
	state[2][3] ^= gf_mul[col[2]][0];
	state[2][3] ^= gf_mul[col[3]][1];
	state[3][3] = gf_mul[col[0]][1];
	state[3][3] ^= col[1];
	state[3][3] ^= col[2];
	state[3][3] ^= gf_mul[col[3]][0];
	memset(col, 0, sizeof col);
}

void aes_InvMixColumns(BYTE state[][4])
{
	BYTE col[4];
	
	// Column 1
	col[0] = state[0][0];
	col[1] = state[1][0];
	col[2] = state[2][0];
	col[3] = state[3][0];
	state[0][0] = gf_mul[col[0]][5];
	state[0][0] ^= gf_mul[col[1]][3];
	state[0][0] ^= gf_mul[col[2]][4];
	state[0][0] ^= gf_mul[col[3]][2];
	state[1][0] = gf_mul[col[0]][2];
	state[1][0] ^= gf_mul[col[1]][5];
	state[1][0] ^= gf_mul[col[2]][3];
	state[1][0] ^= gf_mul[col[3]][4];
	state[2][0] = gf_mul[col[0]][4];
	state[2][0] ^= gf_mul[col[1]][2];
	state[2][0] ^= gf_mul[col[2]][5];
	state[2][0] ^= gf_mul[col[3]][3];
	state[3][0] = gf_mul[col[0]][3];
	state[3][0] ^= gf_mul[col[1]][4];
	state[3][0] ^= gf_mul[col[2]][2];
	state[3][0] ^= gf_mul[col[3]][5];
	// Column 2
	col[0] = state[0][1];
	col[1] = state[1][1];
	col[2] = state[2][1];
	col[3] = state[3][1];
	state[0][1] = gf_mul[col[0]][5];
	state[0][1] ^= gf_mul[col[1]][3];
	state[0][1] ^= gf_mul[col[2]][4];
	state[0][1] ^= gf_mul[col[3]][2];
	state[1][1] = gf_mul[col[0]][2];
	state[1][1] ^= gf_mul[col[1]][5];
	state[1][1] ^= gf_mul[col[2]][3];
	state[1][1] ^= gf_mul[col[3]][4];
	state[2][1] = gf_mul[col[0]][4];
	state[2][1] ^= gf_mul[col[1]][2];
	state[2][1] ^= gf_mul[col[2]][5];
	state[2][1] ^= gf_mul[col[3]][3];
	state[3][1] = gf_mul[col[0]][3];
	state[3][1] ^= gf_mul[col[1]][4];
	state[3][1] ^= gf_mul[col[2]][2];
	state[3][1] ^= gf_mul[col[3]][5];
	// Column 3
	col[0] = state[0][2];
	col[1] = state[1][2];
	col[2] = state[2][2];
	col[3] = state[3][2];
	state[0][2] = gf_mul[col[0]][5];
	state[0][2] ^= gf_mul[col[1]][3];
	state[0][2] ^= gf_mul[col[2]][4];
	state[0][2] ^= gf_mul[col[3]][2];
	state[1][2] = gf_mul[col[0]][2];
	state[1][2] ^= gf_mul[col[1]][5];
	state[1][2] ^= gf_mul[col[2]][3];
	state[1][2] ^= gf_mul[col[3]][4];
	state[2][2] = gf_mul[col[0]][4];
	state[2][2] ^= gf_mul[col[1]][2];
	state[2][2] ^= gf_mul[col[2]][5];
	state[2][2] ^= gf_mul[col[3]][3];
	state[3][2] = gf_mul[col[0]][3];
	state[3][2] ^= gf_mul[col[1]][4];
	state[3][2] ^= gf_mul[col[2]][2];
	state[3][2] ^= gf_mul[col[3]][5];
	// Column 4
	col[0] = state[0][3];
	col[1] = state[1][3];
	col[2] = state[2][3];
	col[3] = state[3][3];
	state[0][3] = gf_mul[col[0]][5];
	state[0][3] ^= gf_mul[col[1]][3];
	state[0][3] ^= gf_mul[col[2]][4];
	state[0][3] ^= gf_mul[col[3]][2];
	state[1][3] = gf_mul[col[0]][2];
	state[1][3] ^= gf_mul[col[1]][5];
	state[1][3] ^= gf_mul[col[2]][3];
	state[1][3] ^= gf_mul[col[3]][4];
	state[2][3] = gf_mul[col[0]][4];
	state[2][3] ^= gf_mul[col[1]][2];
	state[2][3] ^= gf_mul[col[2]][5];
	state[2][3] ^= gf_mul[col[3]][3];
	state[3][3] = gf_mul[col[0]][3];
	state[3][3] ^= gf_mul[col[1]][4];
	state[3][3] ^= gf_mul[col[2]][2];
	state[3][3] ^= gf_mul[col[3]][5];
	memset(col, 0, sizeof col);
}

void aes_encrypt_block(const BYTE in[], BYTE out[], aes_ctx* ks){
	BYTE state[4][4];
	int keysize = ks->keysize;
	uint32_t *key = ks->round_keys;
	// Copy input array (should be 16 bytes long) to a matrix (sequential bytes are ordered
	// by row, not col) called "state" for processing.
	// *** Implementation note: The official AES documentation references the state by
	// column, then row. Accessing an element in C requires row then column. Thus, all state
	// references in AES must have the column and row indexes reversed for C implementation.
	state[0][0] = in[0];
	state[1][0] = in[1];
	state[2][0] = in[2];
	state[3][0] = in[3];
	state[0][1] = in[4];
	state[1][1] = in[5];
	state[2][1] = in[6];
	state[3][1] = in[7];
	state[0][2] = in[8];
	state[1][2] = in[9];
	state[2][2] = in[10];
	state[3][2] = in[11];
	state[0][3] = in[12];
	state[1][3] = in[13];
	state[2][3] = in[14];
	state[3][3] = in[15];
	
	// Perform the necessary number of rounds. The round key is added first.
	// The last round does not perform the aes_MixColumns step.
	aes_AddRoundKey(state,&key[0]);
	aes_SubBytes(state); aes_ShiftRows(state); aes_MixColumns(state); aes_AddRoundKey(state,&key[4]);
	aes_SubBytes(state); aes_ShiftRows(state); aes_MixColumns(state); aes_AddRoundKey(state,&key[8]);
	aes_SubBytes(state); aes_ShiftRows(state); aes_MixColumns(state); aes_AddRoundKey(state,&key[12]);
	aes_SubBytes(state); aes_ShiftRows(state); aes_MixColumns(state); aes_AddRoundKey(state,&key[16]);
	aes_SubBytes(state); aes_ShiftRows(state); aes_MixColumns(state); aes_AddRoundKey(state,&key[20]);
	aes_SubBytes(state); aes_ShiftRows(state); aes_MixColumns(state); aes_AddRoundKey(state,&key[24]);
	aes_SubBytes(state); aes_ShiftRows(state); aes_MixColumns(state); aes_AddRoundKey(state,&key[28]);
	aes_SubBytes(state); aes_ShiftRows(state); aes_MixColumns(state); aes_AddRoundKey(state,&key[32]);
	aes_SubBytes(state); aes_ShiftRows(state); aes_MixColumns(state); aes_AddRoundKey(state,&key[36]);
	if (keysize != 128) {
		aes_SubBytes(state); aes_ShiftRows(state); aes_MixColumns(state); aes_AddRoundKey(state,&key[40]);
		aes_SubBytes(state); aes_ShiftRows(state); aes_MixColumns(state); aes_AddRoundKey(state,&key[44]);
		if (keysize != 192) {
			aes_SubBytes(state); aes_ShiftRows(state); aes_MixColumns(state); aes_AddRoundKey(state,&key[48]);
			aes_SubBytes(state); aes_ShiftRows(state); aes_MixColumns(state); aes_AddRoundKey(state,&key[52]);
			aes_SubBytes(state); aes_ShiftRows(state); aes_AddRoundKey(state,&key[56]);
		}
		else {
			aes_SubBytes(state); aes_ShiftRows(state); aes_AddRoundKey(state,&key[48]);
		}
	}
	else {
		aes_SubBytes(state); aes_ShiftRows(state); aes_AddRoundKey(state,&key[40]);
	}
	
	// Copy the state to the output array.
	out[0] = state[0][0];
	out[1] = state[1][0];
	out[2] = state[2][0];
	out[3] = state[3][0];
	out[4] = state[0][1];
	out[5] = state[1][1];
	out[6] = state[2][1];
	out[7] = state[3][1];
	out[8] = state[0][2];
	out[9] = state[1][2];
	out[10] = state[2][2];
	out[11] = state[3][2];
	out[12] = state[0][3];
	out[13] = state[1][3];
	out[14] = state[2][3];
	out[15] = state[3][3];
	memset(state, 0, sizeof state);
}

void aes_decrypt_block(const BYTE in[], BYTE out[], aes_ctx* ks){
	BYTE state[4][4];
	int keysize = ks->keysize;
	uint32_t *key = ks->round_keys;
	// Copy the input to the state.
	state[0][0] = in[0];
	state[1][0] = in[1];
	state[2][0] = in[2];
	state[3][0] = in[3];
	state[0][1] = in[4];
	state[1][1] = in[5];
	state[2][1] = in[6];
	state[3][1] = in[7];
	state[0][2] = in[8];
	state[1][2] = in[9];
	state[2][2] = in[10];
	state[3][2] = in[11];
	state[0][3] = in[12];
	state[1][3] = in[13];
	state[2][3] = in[14];
	state[3][3] = in[15];
	
	// Perform the necessary number of rounds. The round key is added first.
	// The last round does not perform the aes_MixColumns step.
	if (keysize > 128) {
		if (keysize > 192) {
			aes_AddRoundKey(state,&key[56]);
			aes_InvShiftRows(state);aes_InvSubBytes(state);aes_AddRoundKey(state,&key[52]);aes_InvMixColumns(state);
			aes_InvShiftRows(state);aes_InvSubBytes(state);aes_AddRoundKey(state,&key[48]);aes_InvMixColumns(state);
		}
		else {
			aes_AddRoundKey(state,&key[48]);
		}
		aes_InvShiftRows(state);aes_InvSubBytes(state);aes_AddRoundKey(state,&key[44]);aes_InvMixColumns(state);
		aes_InvShiftRows(state);aes_InvSubBytes(state);aes_AddRoundKey(state,&key[40]);aes_InvMixColumns(state);
	}
	else {
		aes_AddRoundKey(state,&key[40]);
	}
	aes_InvShiftRows(state);aes_InvSubBytes(state);aes_AddRoundKey(state,&key[36]);aes_InvMixColumns(state);
	aes_InvShiftRows(state);aes_InvSubBytes(state);aes_AddRoundKey(state,&key[32]);aes_InvMixColumns(state);
	aes_InvShiftRows(state);aes_InvSubBytes(state);aes_AddRoundKey(state,&key[28]);aes_InvMixColumns(state);
	aes_InvShiftRows(state);aes_InvSubBytes(state);aes_AddRoundKey(state,&key[24]);aes_InvMixColumns(state);
	aes_InvShiftRows(state);aes_InvSubBytes(state);aes_AddRoundKey(state,&key[20]);aes_InvMixColumns(state);
	aes_InvShiftRows(state);aes_InvSubBytes(state);aes_AddRoundKey(state,&key[16]);aes_InvMixColumns(state);
	aes_InvShiftRows(state);aes_InvSubBytes(state);aes_AddRoundKey(state,&key[12]);aes_InvMixColumns(state);
	aes_InvShiftRows(state);aes_InvSubBytes(state);aes_AddRoundKey(state,&key[8]);aes_InvMixColumns(state);
	aes_InvShiftRows(state);aes_InvSubBytes(state);aes_AddRoundKey(state,&key[4]);aes_InvMixColumns(state);
	aes_InvShiftRows(state);aes_InvSubBytes(state);aes_AddRoundKey(state,&key[0]);
	
	// Copy the state to the output array.
	out[0] = state[0][0];
	out[1] = state[1][0];
	out[2] = state[2][0];
	out[3] = state[3][0];
	out[4] = state[0][1];
	out[5] = state[1][1];
	out[6] = state[2][1];
	out[7] = state[3][1];
	out[8] = state[0][2];
	out[9] = state[1][2];
	out[10] = state[2][2];
	out[11] = state[3][2];
	out[12] = state[0][3];
	out[13] = state[1][3];
	out[14] = state[2][3];
	out[15] = state[3][3];
	memset(state, 0, state);
}

void increment_iv(BYTE iv[], size_t counter_start, size_t counter_size)
{
	int idx;
	
	// Use counter_size bytes at the end of the IV as the big-endian integer to increment.
	for (idx = counter_start + counter_size - 1; idx >= counter_start; idx--) {
		iv[idx]++;
		if (iv[idx] != 0) break;
	}
}


/*
 typedef struct _aes_cbc {
 uint8_t padding_mode;
 } aes_cbc_t;
 
 typedef struct _aes_ctr {
 uint8_t ctr_len;
 uint8_t block_pos;
 uint8_t prev_block[16];
 } aes_ctr_t;
 
 typedef struct _aes_keyschedule_ctx {
 uint24_t keysize;
 uint32_t round_keys[60];
 uint8_t iv[16];
 uint8_t cipher_mode;
 union {
 aes_ctr_t ctr;
 aes_cbc_t cbc;
 } mode;
 } aes_ctx;
 */

enum _aes_op_assoc {
	AES_OP_NONE,
	AES_OP_ENCRYPT,
	AES_OP_DECRYPT
};
#define AES_GCM_NONCE_LEN 12
#define AES_GCM_CTR_LEN 4
#define AES_GCM_POLY 0xe1 // polynomial used in AES-GCM

void gf128_mul(uint8_t *a, const uint8_t *b, uint8_t *c)
{
	uint8_t v[16] = {0};
	uint8_t z[16] = {0};
	uint8_t hibit = 0;
	int i, j;
	
	for (i = 0; i < 16; i++) {
		for (j = 0; j < 8; j++) {
			if (b[i] & (1 << j)) {
				xor_buf(&a[15 - i], v, 16);
			}
		}
	}
	
	for (i = 0; i < 16; i++) {
		for (j = 0; j < 8; j++) {
			if (a[i] & (1 << j)) {
				xor_buf(&v[15 - i], z, 16);
			}
		}
	}
	
	for (i = 0; i < 16; i++) {
		c[i] = z[15 - i];
	}
	
	for (i = 0; i < 15; i++) {
		if (a[i] & 0x80) {
			hibit = 1;
		}
	}
	
	if (hibit) {
		uint8_t r[16] = {0x87, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		xor_buf(r, c, 16);
	}
}

#define ghash_start(buf)	memset((buf), 0, 16)

void ghash(aes_ctx *ctx, uint8_t *out_buf, const uint8_t *data, size_t len)
{
	uint8_t tbuf[AES_BLOCK_SIZE];
	size_t data_offset = 0;
	
	// the cache allows for incomplete blocks to be queued
	// the next call to update will concat the queue and new aad
	if(ctx->mode.gcm.aad_cache_len){
		size_t cache_len = ctx->mode.gcm.aad_cache_len;
		data_offset = MIN(len, AES_BLOCK_SIZE - cache_len);
		if(data_offset + cache_len < AES_BLOCK_SIZE){
			// if new aad is not enough to fill a block, update queue and stop w/o processing
			memcpy(&ctx->mode.gcm.aad_cache[cache_len], data, data_offset);
			ctx->mode.gcm.aad_cache_len += data_offset;
			return;
		}
		else {
			// if new aad is enough to fill a block, concat queue and rest of block from aad
			// then update hash
			memcpy(tbuf, ctx->mode.gcm.aad_cache, cache_len);
			memcpy(&tbuf[cache_len], data, data_offset);
			xor_buf(tbuf, out_buf, AES_BLOCK_SIZE);
			gf128_mul(out_buf, ctx->mode.gcm.ghash_key, out_buf);
			ctx->mode.gcm.aad_cache_len = 0;
		}
	}
	
	// now process any remaining aad data
	for(uint24_t idx = data_offset; idx < len; idx += AES_BLOCK_SIZE){
		size_t bytes_copy = MIN(AES_BLOCK_SIZE, len - idx);
		if(bytes_copy < AES_BLOCK_SIZE){
			// if aad_len < block size, write bytes to queue.
			// no return here because this condition should just exit out next loop
			memcpy(ctx->mode.gcm.aad_cache, &data[idx], bytes_copy);
			ctx->mode.gcm.aad_cache_len = bytes_copy;
		}
		else {
			// if aad_len >= block size, update hash for block
			memcpy(tbuf, &data[idx], AES_BLOCK_SIZE);
			xor_buf(tbuf, out_buf, AES_BLOCK_SIZE);
			gf128_mul(out_buf, ctx->mode.gcm.ghash_key, out_buf);
		}
	}
}

void aes_gcm_prepare_iv(aes_ctx *ctx, const uint8_t *iv, size_t iv_len)
{
	uint8_t tbuf[AES_BLOCK_SIZE];
	// memset(ctx->iv, 0, AES_BLOCK_SIZE);
	// ^^ this should already be zero'd from aes_init
	
	if (iv_len == 12) {
		/* Prepare block J_0 = IV || 0^31 || 1 [len(IV) = 96] */
		// memcpy(ctx->iv, iv, iv_len);
		// ^^ this should already be done by aes_init
		ctx->iv[AES_BLOCK_SIZE - 1] = 0x01;
	} else {
		/*
		 * s = 128 * ceil(len(IV)/128) - len(IV)
		 * J_0 = GHASH_H(IV || 0^(s+64) || [len(IV)]_64)
		 */
		// hash the IV. Pad to block size
		memset(tbuf, 0, AES_BLOCK_SIZE);
		memcpy(tbuf, iv, iv_len);
		memset(ctx->iv, 0, AES_BLOCK_SIZE);
		ghash(ctx, ctx->iv, tbuf, sizeof(tbuf));
		//xor_buf(tbuf, ctx->iv, AES_BLOCK_SIZE);
		//gf128_mul(out_buf, ctx->mode.gcm.ghash_key, out_buf);
		
		memset(tbuf, 0, AES_BLOCK_SIZE>>1);
		bytelen_to_bitlen(iv_len, &tbuf[8]);		// outputs in BE
		
		ghash(ctx, ctx->iv, tbuf, sizeof(tbuf));
	}
}

// flag definition  [unused][ 0000 counter len ][ 0000 counter start pos ][ 00 padding mode][ 00 cipher mode ]

// Performs the action of generating the keys that will be used in every round of
// encryption. "key" is the user-supplied input key, "w" is the output key schedule,
// "keysize" is the length in bits of "key", must be 128, 192, or 256.
aes_error_t hashlib_AESLoadKey(aes_ctx* ctx, const BYTE key[], size_t keysize, uint8_t* iv, size_t iv_len, uint24_t cipher_flags)
{
	int Nb=4,Nr,Nk,idx;
	WORD temp,Rcon[]={0x01000000,0x02000000,0x04000000,0x08000000,0x10000000,0x20000000,
		0x40000000,0x80000000,0x1b000000,0x36000000,0x6c000000,0xd8000000,
		0xab000000,0x4d000000,0x9a000000};
	uint8_t mode = (cipher_flags & 3);
	if(mode>AES_GCM) return AES_INVALID_CIPHERMODE;
	if(iv_len>AES_BLOCK_SIZE) return AES_INVALID_ARG;
	memset(ctx, 0, sizeof(aes_ctx));
	ctx->cipher_mode = mode;
	keysize<<=3;
	switch (keysize) {
		case 128: Nr = 10; Nk = 4; break;
		case 192: Nr = 12; Nk = 6; break;
		case 256: Nr = 14; Nk = 8; break;
		default: return AES_INVALID_ARG;
	}
	
	memcpy(ctx->iv, iv, iv_len);
	memset(&ctx->iv[iv_len], 0, 16-iv_len);
	ctx->keysize = keysize;
	for (idx=0; idx < Nk; ++idx) {
		ctx->round_keys[idx] = ((uint32_t)(key[4 * idx]) << 24) | ((uint32_t)(key[4 * idx + 1]) << 16) |
		((uint32_t)(key[4 * idx + 2]) << 8) | ((uint32_t)(key[4 * idx + 3]));
	}
	
	for (idx = Nk; idx < Nb * (Nr+1); ++idx) {
		temp = ctx->round_keys[idx - 1];
		if ((idx % Nk) == 0)
			temp = aes_SubWord(KE_ROTWORD(temp)) ^ Rcon[(idx-1)/Nk];
		else if (Nk > 6 && (idx % Nk) == 4)
			temp = aes_SubWord(temp);
		ctx->round_keys[idx] = ctx->round_keys[idx-Nk] ^ temp;
	}
	
	if(mode == AES_CBC) ctx->mode.cbc.padding_mode = ((uint8_t)(cipher_flags>>2) & FLAGS_GET_LSB2);
	if(mode == AES_CTR) {
		uint8_t ctr_pos = ((uint8_t)(cipher_flags>>4) & FLAGS_GET_LSB4);
		uint8_t ctr_len = ((uint8_t)(cipher_flags>>8) & FLAGS_GET_LSB4);
		if((!ctr_len) && (!ctr_pos)) {ctr_pos = 8; ctr_len = 8;}
		else if(ctr_len && (!ctr_pos)) ctr_pos = AES_BLOCK_SIZE - ctr_len;
		else if(ctr_pos && (!ctr_len)) ctr_len = AES_BLOCK_SIZE - ctr_pos;
		if(ctr_len + ctr_pos > 16) return AES_INVALID_ARG;
		ctx->mode.ctr.counter_pos_start = ctr_pos;
		ctx->mode.ctr.counter_len = ctr_len;
	}
	if(mode == AES_GCM){
		
		// generate ghash key
		uint8_t tmp[16] = {0};
		aes_encrypt_block(tmp, ctx->mode.gcm.ghash_key, ctx);
		
		// sort out IV wonkiness in GCM mode
		aes_gcm_prepare_iv(ctx, iv, iv_len);
		memset(ctx->mode.gcm.auth_tag, 0, AES_BLOCK_SIZE);
		memcpy(ctx->mode.gcm.auth_j0, ctx->iv, AES_BLOCK_SIZE);
		increment_iv(ctx->iv, AES_GCM_NONCE_LEN, AES_GCM_CTR_LEN);
	}
	
	return AES_OK;
}

enum GCM_OPS_ALLOWED {
	GCM_ALLOW_AAD,
	GCM_ALLOW_ENCRYPT,
	GCM_ALLOW_NONE
};


aes_error_t cryptx_aes_update_aad(aes_ctx* ctx, uint8_t *aad, size_t aad_len){
	if(ctx->cipher_mode != AES_GCM) return AES_INVALID_OPERATION;
	if(ctx->mode.gcm.gcm_op > GCM_ALLOW_AAD) return AES_INVALID_OPERATION;
	
	// update the tag for full blocks of aad in input, cache any partial blocks
	ghash(ctx, ctx->mode.gcm.auth_tag, aad, aad_len);
	ctx->mode.gcm.aad_len += aad_len;
	return AES_OK;
}

aes_error_t cryptx_aes_digest(aes_ctx* ctx, uint8_t* digest){
	if( (ctx == NULL) || (digest == NULL)) return AES_INVALID_ARG;
	if(ctx->cipher_mode != AES_GCM) return AES_INVALID_CIPHERMODE;
	ctx->mode.gcm.gcm_op = GCM_ALLOW_NONE;
	uint8_t tbuf[AES_BLOCK_SIZE];
	uint8_t *tag = ctx->mode.gcm.auth_tag;
	
	// pad rest of ciphertext cache with 0s
	memset(tbuf, 0, AES_BLOCK_SIZE);
	ghash(ctx, tag, tbuf, AES_BLOCK_SIZE - ctx->mode.gcm.aad_cache_len);
	// at this point, tag should be GHASH(0-padded aad || 0-padded ciphertext)
	
	// final tag computed as GHASH( 0-padded aad || 0-padded ciphertext || u64-be-aad-len || u64-be-ciphertext-len)
	bytelen_to_bitlen(ctx->mode.gcm.aad_len, tbuf);
	bytelen_to_bitlen(ctx->mode.gcm.ct_len, &tbuf[8]);
	ghash(ctx, tag, tbuf, AES_BLOCK_SIZE);
	
	// encrypt auth tag with CTR0
	aes_encrypt_block(ctx->mode.gcm.auth_j0, tbuf, ctx);
	xor_buf(tag, tbuf, AES_BLOCK_SIZE);
	
	memcpy(digest, tbuf, AES_BLOCK_SIZE);
	return AES_OK;
}

bool cryptx_aes_verify(aes_ctx *ctx, uint8_t *aad, size_t aad_len, uint8_t *ciphertext, size_t ciphertext_len, uint8_t *tag){
	aes_ctx tmp;
	uint8_t digest[AES_BLOCK_SIZE];
	
	// do this work on a copy of ctx
	memcpy(&tmp, ctx, sizeof(tmp));
	
	cryptx_aes_update_aad(&tmp, aad, aad_len);
	hashlib_AESDecrypt(&tmp, ciphertext, ciphertext_len, NULL);
	cryptx_aes_digest(&tmp, digest);
	
	//memset(&tmp, 0, sizeof(tmp));
	return cryptx_digest_compare(tag, digest, AES_BLOCK_SIZE);
}

#define AES_BLOCKSIZE 16
static const char iso_pad[] = {0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
aes_error_t hashlib_AESEncrypt(aes_ctx* ctx, const BYTE in[], size_t in_len, BYTE out[])
{
	BYTE buf[AES_BLOCK_SIZE];
	uint8_t* iv = ctx->iv;
	//int keysize = key->keysize;
	//uint32_t *round_keys = key->round_keys;
	int blocks, idx;
	aes_error_t rv = AES_OK;
	
	if(ctx->op_assoc == AES_OP_DECRYPT) return AES_INVALID_OPERATION;
	ctx->op_assoc = AES_OP_ENCRYPT;
	
	if(in==NULL || out==NULL || ctx==NULL) return AES_INVALID_ARG;
	if(in_len == 0) return AES_INVALID_MSG;
	blocks = (in_len / AES_BLOCK_SIZE);
	
	switch(ctx->cipher_mode){
		case AES_CBC:
		{
			size_t bytes_to_copy, bytes_to_pad;
			for(idx = 0; idx <= blocks; idx++){
				bytes_to_copy = MIN(AES_BLOCK_SIZE, in_len - (idx * AES_BLOCK_SIZE));
				bytes_to_pad = AES_BLOCK_SIZE-bytes_to_copy;
				memcpy(buf, &in[idx*AES_BLOCK_SIZE], bytes_to_copy);
				if(idx==blocks){
					if(ctx->mode.cbc.padding_mode == SCHM_PKCS7) memset(&buf[bytes_to_copy], bytes_to_pad, bytes_to_pad);   // applies PKCS7 padding scheme
					if(ctx->mode.cbc.padding_mode == SCHM_ISO2) memcpy(&buf[bytes_to_copy], iso_pad, bytes_to_pad);          // applies ISO-9797 M2 padding scheme
				}
				xor_buf(iv, buf, AES_BLOCK_SIZE);
				aes_encrypt_block(buf, &out[idx * AES_BLOCK_SIZE], ctx);
				memcpy(iv, &out[idx * AES_BLOCK_SIZE], AES_BLOCK_SIZE);         // iv needs to update for continued use
			}
			break;
		}
		case AES_CTR:
		{
			size_t bytes_to_copy = ctx->mode.ctr.last_block_stop;
			size_t bytes_offset = 0;
			
			// xor remaining bytes from last encrypt operation into new plaintext
			if(bytes_to_copy%AES_BLOCK_SIZE){
				bytes_offset = AES_BLOCK_SIZE - bytes_to_copy;
				memcpy(out, in, bytes_offset);
				xor_buf(&ctx->mode.ctr.last_block[bytes_to_copy], out, bytes_offset);
				blocks = ((in_len - bytes_offset) / AES_BLOCK_SIZE);
				
			}
			// encrypt message in CTR mode
			for(idx = 0; idx <= blocks; idx++){
				bytes_to_copy = MIN(AES_BLOCK_SIZE, in_len - bytes_offset - (idx * AES_BLOCK_SIZE));
				//bytes_to_pad = AES_BLOCK_SIZE-bytes_to_copy;
				memcpy(&out[idx*AES_BLOCK_SIZE+bytes_offset], &in[idx*AES_BLOCK_SIZE+bytes_offset], bytes_to_copy);
				// memset(&buf[bytes_to_copy], 0, bytes_to_pad);        // if bytes_to_copy is less than blocksize, do nothing, since msg is truncated anyway
				aes_encrypt_block(iv, buf, ctx);
				xor_buf(buf, &out[idx*AES_BLOCK_SIZE+bytes_offset], bytes_to_copy);
				increment_iv(iv, ctx->mode.ctr.counter_pos_start, ctx->mode.ctr.counter_len);        // increment iv for continued use
				if(idx==blocks){
					memcpy(ctx->mode.ctr.last_block, buf, AES_BLOCK_SIZE);
					ctx->mode.ctr.last_block_stop = bytes_to_copy;
				}
			}
			break;
		}
		case AES_GCM:
		{
			uint8_t *tag = ctx->mode.gcm.auth_tag;
			size_t bytes_to_copy = ctx->mode.gcm.last_block_stop;
			size_t bytes_offset = 0;
			if(ctx->mode.gcm.gcm_op > GCM_ALLOW_ENCRYPT) return AES_INVALID_OPERATION;
			if((ctx->mode.gcm.gcm_op == GCM_ALLOW_AAD) &&
			   (ctx->mode.gcm.aad_cache_len)){
				// pad rest of aad cache with 0's
				memset(buf, 0, AES_BLOCK_SIZE);
				ghash(ctx, tag, buf, AES_BLOCK_SIZE - ctx->mode.gcm.aad_cache_len);
			}
			ctx->mode.gcm.gcm_op = GCM_ALLOW_ENCRYPT;
			// xor last bytes of encryption buf w/ new plaintext for new ciphertext
			if(bytes_to_copy%AES_BLOCK_SIZE){
				bytes_offset = AES_BLOCK_SIZE - bytes_to_copy;
				memcpy(out, in, bytes_offset);
				xor_buf(&ctx->mode.gcm.last_block[bytes_to_copy], out, bytes_offset);
				blocks = ((in_len - bytes_offset) / AES_BLOCK_SIZE);
				
			}
			
			// encrypt remaining plaintext
			for(idx = 0; idx <= blocks; idx++){
				bytes_to_copy = MIN(AES_BLOCK_SIZE, in_len - bytes_offset - (idx * AES_BLOCK_SIZE));
				//bytes_to_pad = AES_BLOCK_SIZE-bytes_to_copy;
				memcpy(&out[idx*AES_BLOCK_SIZE+bytes_offset], &in[idx*AES_BLOCK_SIZE+bytes_offset], bytes_to_copy);
				// memset(&buf[bytes_to_copy], 0, bytes_to_pad);        // if bytes_to_copy is less than blocksize, do nothing, since msg is truncated anyway
				aes_encrypt_block(iv, buf, ctx);
				xor_buf(buf, &out[idx*AES_BLOCK_SIZE+bytes_offset], bytes_to_copy);
				increment_iv(iv, AES_GCM_NONCE_LEN, AES_GCM_CTR_LEN);        // increment iv for continued use
				if(idx==blocks){
					memcpy(ctx->mode.gcm.last_block, buf, AES_BLOCK_SIZE);
					ctx->mode.gcm.last_block_stop = bytes_to_copy;
				}
			}
			
			// authenticate the ciphertext
			ghash(ctx, tag, out, in_len);
			ctx->mode.gcm.ct_len += in_len;
			
			break;
			
		}
			
		default: return AES_INVALID_CIPHERMODE;
			
	}
	//memcpy(iv, iv_buf, sizeof iv_buf);
	return rv;
}


aes_error_t hashlib_AESDecrypt(aes_ctx* ctx, const BYTE in[], size_t in_len, BYTE out[])
{
	BYTE buf_in[AES_BLOCK_SIZE], buf_out[AES_BLOCK_SIZE];
	uint8_t* iv = ctx->iv;
	aes_error_t rv = AES_OK;
	//int keysize = key->keysize;
	//uint32_t *round_keys = key->round_keys;
	int blocks = in_len / AES_BLOCK_SIZE, idx;
	if((out == NULL) && (ctx->cipher_mode != AES_GCM)) return AES_INVALID_ARG;
	
	if(ctx->op_assoc == AES_OP_ENCRYPT) return AES_INVALID_OPERATION;
	ctx->op_assoc = AES_OP_DECRYPT;
	
	if(in==NULL || ctx==NULL) return AES_INVALID_ARG;
	if(in_len == 0) return AES_INVALID_CIPHERTEXT;
	
	switch(ctx->cipher_mode){
		case AES_CBC:
			if((in_len % AES_BLOCK_SIZE) != 0) return AES_INVALID_CIPHERTEXT;
			//memcpy(iv_buf, iv, AES_BLOCK_SIZE);
			
			for (idx = 0; idx < blocks; idx++) {
				memcpy(buf_in, &in[idx * AES_BLOCK_SIZE], AES_BLOCK_SIZE);
				aes_decrypt_block(buf_in, buf_out, ctx);
				xor_buf(iv, buf_out, AES_BLOCK_SIZE);
				memcpy(&out[idx * AES_BLOCK_SIZE], buf_out, AES_BLOCK_SIZE);
				memcpy(iv, buf_in, AES_BLOCK_SIZE);
			}
			//hashlib_AESStripPadding(out, in_len, out, ctx->padding_mode);
			//memcpy(iv, iv_buf, sizeof iv_buf);
			break;
		case AES_CTR:
			ctx->op_assoc = AES_OP_ENCRYPT;
			hashlib_AESEncrypt(ctx, in, in_len, out);
			ctx->op_assoc = AES_OP_DECRYPT;
			break;
			
		case AES_GCM:
		{
			uint8_t *tag = ctx->mode.gcm.auth_tag;
			size_t bytes_to_copy = ctx->mode.gcm.last_block_stop;
			size_t bytes_offset = 0;
			if(ctx->mode.gcm.gcm_op > GCM_ALLOW_ENCRYPT) return AES_INVALID_OPERATION;
			if((ctx->mode.gcm.gcm_op == GCM_ALLOW_AAD) &&
			   (ctx->mode.gcm.aad_cache_len)){
				// pad rest of aad cache with 0's
				memset(buf_in, 0, AES_BLOCK_SIZE);
				ghash(ctx, tag, buf_in, AES_BLOCK_SIZE - ctx->mode.gcm.aad_cache_len);
			}
			ghash(ctx, tag, in, in_len);
			ctx->mode.gcm.ct_len += in_len;
			
			if(out){
				ctx->mode.gcm.gcm_op = GCM_ALLOW_ENCRYPT;
				if(bytes_to_copy%AES_BLOCK_SIZE){
					bytes_offset = AES_BLOCK_SIZE - bytes_to_copy;
					memcpy(out, in, bytes_offset);
					xor_buf(&ctx->mode.gcm.last_block[bytes_to_copy], out, bytes_offset);
					blocks = ((in_len - bytes_offset) / AES_BLOCK_SIZE);
				}
				
				// encrypt remaining plaintext
				for(idx = 0; idx <= blocks; idx++){
					bytes_to_copy = MIN(AES_BLOCK_SIZE, in_len - bytes_offset - (idx * AES_BLOCK_SIZE));
					//bytes_to_pad = AES_BLOCK_SIZE-bytes_to_copy;
					memcpy(&out[idx*AES_BLOCK_SIZE+bytes_offset], &in[idx*AES_BLOCK_SIZE+bytes_offset], bytes_to_copy);
					// memset(&buf[bytes_to_copy], 0, bytes_to_pad);        // if bytes_to_copy is less than blocksize, do nothing, since msg is truncated anyway
					aes_encrypt_block(iv, buf_in, ctx);
					xor_buf(buf_in, &out[idx*AES_BLOCK_SIZE+bytes_offset], bytes_to_copy);
					increment_iv(iv, AES_GCM_NONCE_LEN, AES_GCM_CTR_LEN);        // increment iv for continued use
					if(idx==blocks){
						memcpy(ctx->mode.gcm.last_block, buf_in, AES_BLOCK_SIZE);
						ctx->mode.gcm.last_block_stop = bytes_to_copy;
					}
				}
			}
		}
			break;
		default: return AES_INVALID_CIPHERMODE;
	}
	memset(buf_in, 0, sizeof buf_in);
	memset(buf_out, 0, sizeof buf_out);
	return AES_OK;
}

/*
 int hashlib_AESOutputMAC(const BYTE in[], size_t in_len, BYTE out[], aes_ctx* key){
 BYTE buf_in[AES_BLOCK_SIZE], buf_out[AES_BLOCK_SIZE], iv_buf[AES_BLOCK_SIZE];
 //int keysize = key->keysize;
 //uint32_t *round_keys = key->round_keys;
 int blocks, idx;
 
 if(in==NULL || out==NULL || key==NULL) return AES_INVALID_ARG;
 
 if ((in_len % AES_BLOCK_SIZE) != 0)
 return false;
 
 blocks = in_len / AES_BLOCK_SIZE;
 
 memset(iv_buf, 0, AES_BLOCK_SIZE);
 
 for (idx = 0; idx < blocks; idx++) {
 memcpy(buf_in, &in[idx * AES_BLOCK_SIZE], AES_BLOCK_SIZE);
 xor_buf(iv_buf, buf_in, AES_BLOCK_SIZE);
 aes_encrypt_block(buf_in, buf_out, key);
 memcpy(iv_buf, buf_out, AES_BLOCK_SIZE);
 }
 memcpy(out, buf_out, AES_BLOCK_SIZE);
 
 return true;
 }
 */

size_t hashlib_AESStripPadding(const uint8_t* in, size_t len, uint8_t* out, uint8_t schm){
	if(in==NULL) return 0;
	if(out==NULL) return 0;
	if(len==0) return 0;
	size_t outlen;
	//memset(out, 0, len);
	switch(schm){
		case SCHM_PKCS7:
			outlen = len - in[len-1];
			break;
		case SCHM_ISO2:
			for(outlen = len-1; in[outlen] != 0x80; outlen--);
			outlen++;
			break;
		default:
			return 0;
	}
	memcpy(out, in, outlen);
	memset(&out[outlen], 0, len-outlen);
	return outlen;
}

size_t hashlib_AESPadMessage(const uint8_t* in, size_t len, uint8_t* out, uint8_t schm){
	size_t blocksize=16, padded_len;
	if(in==NULL) return 0;
	if(out==NULL) return 0;
	if(len==0) return 0;
	if((len % blocksize) == 0) padded_len = len + blocksize;
	else padded_len = (( len / blocksize ) + 1) * blocksize;
	switch(schm){
		case SCHM_PKCS7:
			memset(&out[len], padded_len-len, padded_len-len);
			break;
		case SCHM_ISO2:
			memset(&out[len], 0, padded_len-len);
			out[len] |= 0b10000000;
			break;
		default:
			return 0;
	}
	memcpy(out, in, len);
	return padded_len;
}

enum _authmodes{
	AES_AUTH_NONE,
	AES_AUTH_SHA256,
	AES_AUTH_CBCMAC
};





enum _sig_algorithms {
	RSA_SHA256,
	ECDSA		// placeholder for something I may implement in the very distant future
};

bool hashlib_ReverseEndianness(const uint8_t* in, uint8_t* out, size_t len){
	if(in==NULL || out==NULL) return false;
	if(len==0) return false;
	for(size_t i=0; i<len; i++)
		out[len-i-1] = in[i];
	return true;
}







#define PBKDF2_SALT_LEN 16
#define SHA256_OUTSIZE  32
#define REVERSE_LONG(n) ((unsigned long) (((n & 0xff) << 24) | \
((n & 0xff00) << 8) | \
((n & 0xff0000) >> 8) | \
((n & 0xff000000) >> 24)))


typedef struct _sha256hmac_ctx {
	uint8_t ipad[64];       /**< holds the key xored with a magic value to be hashed with the inner digest */
	uint8_t opad[64];       /**< holds the key xored with a magic value to be hashed with the outer digest */
	uint8_t data[64];		/**< holds sha-256 block for transformation */
	uint8_t datalen;		/**< holds the current length of data in data[64] */
	uint8_t bitlen[8];		/**< holds the current length of transformed data */
	uint32_t state[8];		/**< holds hash state for transformed data */
} sha256hmac_ctx;

/*******************************************************************************************************************
 * @typedef hmac_ctx
 * Defines hash-state data for an instance of SHA-256-HMAC.
 * @note If you are hashing multiple data streams concurrently, allocate a seperate context for each.
 ********************************************************************************************************************/
typedef struct _hmac_ctx {
	uint24_t fn_init;       /**< pointer to an initialization method for the given hash algorithm */
	uint24_t fn_update;     /**< pointer to the update method for the given hash algorithm */
	uint24_t fn_final;      /**< pointer to the digest output method for the given hash algorithm */
	union _hmac {           /**< a union of computational states for various hashes */
		sha256hmac_ctx sha256hmac;
	} Hmac;
} hmac_ctx;

#define CEMU_CONSOLE ((char*)0xFB0000)
/*
 void hashlib_HMACSha256Init(hmac_ctx *hmac, const uint8_t* key, size_t keylen){
 size_t i;
 uint8_t sum[64] = {0};
 if(keylen > 64){
 hashlib_Sha256Init(&hmac->sha256_ctx);
 hashlib_Sha256Update(&hmac->sha256_ctx, key, keylen);
 hashlib_Sha256Final(&hmac->sha256_ctx, sum);
 keylen = 32;
 }
 else memcpy(sum, key, keylen);
 memset( hmac->ipad, 0x36, 64 );
 memset( hmac->opad, 0x5C, 64 );
 for( i = 0; i < 64; i++ ){
 hmac->ipad[i] ^= sum[i];
 hmac->opad[i] ^= sum[i];
 }
 hashlib_Sha256Init(&hmac->sha256_ctx);
 hashlib_Sha256Update(&hmac->sha256_ctx, hmac->ipad, 64);
 }
 
 void hashlib_HMACSha256Update(hmac_ctx *hmac, const uint8_t* msg, size_t len){
 hashlib_Sha256Update(&hmac->sha256_ctx, msg, len);
 }
 
 void hashlib_HMACSha256Final(hmac_ctx* hmac, uint8_t *output){
 uint8_t tmpbuf[32];
 hmac_ctx tmp;
 memcpy(&tmp, hmac, sizeof(tmp));
 hashlib_Sha256Final( &tmp.sha256_ctx, tmpbuf );
 
 hashlib_Sha256Init( &tmp.sha256_ctx);
 hashlib_Sha256Update( &tmp.sha256_ctx, hmac->opad, 64 );
 hashlib_Sha256Update( &tmp.sha256_ctx, tmpbuf, 32);
 hashlib_Sha256Final( &tmp.sha256_ctx, output );
 }
 
 */

void hexdump(uint8_t *addr, size_t len, uint8_t *label){
	if(label) sprintf(CEMU_CONSOLE, "\n%s\n", label);
	else sprintf(CEMU_CONSOLE, "\n");
	for(size_t rem_len = len, ct=1; rem_len>0; rem_len--, addr++, ct++){
		sprintf(CEMU_CONSOLE, "%02X ", *addr);
		if(!(ct%AES_BLOCKSIZE)) sprintf(CEMU_CONSOLE, "\n");
	}
	sprintf(CEMU_CONSOLE, "\n");
}




char hexc[16] = "0123456789ABCDEF";
bool hashlib_DigestToHexStr(const uint8_t* digest, size_t len, uint8_t* hexstr){
	if(digest==NULL || hexstr==NULL || len==0) return false;
	size_t j = 0;
	for (size_t i=0; i<len; i++) {
		hexstr[j++] = hexc[digest[i]>>4];
		hexstr[j++] = hexc[digest[i]&15];
	}
	hexstr[j] = 0;
	return true;
}

bool hashlib_CompareDigest(uint8_t *dig1, uint8_t *dig2, size_t len){
	bool result = false;
	for(size_t i=0; i<len; i++)
		result |= (dig1[i] ^ dig2[i]);
	return !result;
}


int main(void){
	
}


typedef struct _asn1_obj_t {
	uint8_t tag;
	uint8_t f_class;
	bool f_constr;
	size_t len;
	uint8_t *data;
} asn1_obj_t;

enum ASN1_TYPES {
	ANS1_RESVD = 0,
	ANS1_BOOLEAN,
	ANS1_INTEGER,
	ANS1_BITSTRING,
	ANS1_OCTETSTRING,
	ANS1_NULL,
	ANS1_OBJECTID,
	ANS1_OBJECTDESC,
	ANS1_INSTANCE,
	ANS1_REAL,
	ANS1_ENUMERATED,
	ASN1_EMBEDDEDPDV,
	ASN1_UTF8STRING,
	ASN1_RELATIVEOID,
	ASN1_SEQUENCE = 16,
	ASN1_SET,
	ASN1_NUMERICSTRING,
	ASN1_PRINTABLESTRING,
	ASN1_TELETEXSTRING,
	ASN1_VIDEOTEXSTRING,
	ASN1_IA5STRING,
	ASN1_UTCTIME,
	ASN1_GENERALIZEDTIME,
	ASN1_GRAPHICSTRING,
	ASN1_VISIBLESTRING,
	ASN1_GENERALSTRING,
	ASN1_UNIVERSALSTRING,
	ASN1_CHARSTRING,
	ASN1_BMPSTRING
};


typedef enum {
	ASN1_OK,
	ASN1_ARCH_BAD_LEN,
	ASN1_SPEC_MISMATCH
} asn1_error_t;

asn1_error_t asn1_decode(uint8_t *asn1_data, size_t asn1_len, asn1_obj_t *objs, size_t *objs_ret, uint8_t *spec){
	uint8_t *asn1_current = asn1_data;			// set current to start of data to decode
	uint8_t *asn1_end = asn1_current + asn1_len;
	if(*asn1_current == 0) asn1_current++;
	size_t i;
	
	for(i = 0; asn1_current < asn1_end; i++){		// loop until iter count hit. Break manually if done.
		asn1_obj_t *node_o = &objs[i];
		uint8_t spec_tag_form = (spec == NULL) ? 0 : spec[i];
		uint8_t tag = *asn1_current++;
		//node_o->tag = (*asn1_current++) & 0b11111;		// get numeric tag id
		uint8_t byte_2nd = *asn1_current++;	// get byte 2. Can be size or can be size of size
		node_o->len = 0;
		if((byte_2nd>>7) & 1){			// if bit 7 of byte is set, this is a size word length
			uint8_t size_len = byte_2nd & 0x7f;
			if(size_len > 3) return ASN1_ARCH_BAD_LEN;			// due to device limits, seq len limited to u24.
			rmemcpy((uint8_t*)&node_o->len, asn1_current, size_len);
			asn1_current += size_len;
		}
		else									// else, this is a size byte
			node_o->len = byte_2nd;
		
		// pull out tag with associated flags
		node_o->tag = tag & 0b11111;
		node_o->f_constr = (tag>>5 & 1);
		node_o->f_class = (tag>>6 & 0b11);
		
		// retrieve data addr and length
		node_o->data = asn1_current;
		asn1_current += node_o->len;
		
		// if the constructed flag is set, this element may contain other encoded types
		if((node_o->f_constr) || spec[i]){
			size_t node_interior_len;
			if(asn1_decode(node_o->data, node_o->len, node_o, &node_interior_len, NULL) == ASN1_OK){
				i--;
				i += node_interior_len;
			}
		}
		
	}
	*objs_ret = i;
	return ASN1_OK;
}

//#define MIN(a,b) (((a)<(b))?(a):(b))
char b64_charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static int mod_table[] = {0, 2, 1};
size_t base64_encode(uint8_t *out, const uint8_t *in, size_t len){
	size_t olen = 4 * ((len + 2) / 3);
	int j = 0;
	for (int i = 0; i < len;) {
		
		uint32_t octet_a = i < len ? (unsigned char)in[i++] : 0;
		uint32_t octet_b = i < len ? (unsigned char)in[i++] : 0;
		uint32_t octet_c = i < len ? (unsigned char)in[i++] : 0;
		
		uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
		
		out[j++] = b64_charset[(triple >> 3 * 6) & 0x3F];
		out[j++] = b64_charset[(triple >> 2 * 6) & 0x3F];
		out[j++] = b64_charset[(triple >> 1 * 6) & 0x3F];
		out[j++] = b64_charset[(triple >> 0 * 6) & 0x3F];
	}
	
	for (int i = 0; i < mod_table[len % 3]; i++)
		out[olen - 1 - i] = '=';
	
	return j;
}

size_t base64_decode(uint8_t *out, const uint8_t *in, size_t len){
	
	if (len % 4) return 0;
	
	size_t olen = len / 4 * 3;
	size_t j = 0;
	if (in[len - 1] == '=') olen--;
	if (in[len - 2] == '=') olen--;
	
	for (int i = 0; i < len;) {
		
		uint32_t sextet_a = in[i] == '=' ? 0 & i++ : strchr(b64_charset, in[i++]) - b64_charset;
		uint32_t sextet_b = in[i] == '=' ? 0 & i++ : strchr(b64_charset, in[i++]) - b64_charset;
		uint32_t sextet_c = in[i] == '=' ? 0 & i++ : strchr(b64_charset, in[i++]) - b64_charset;
		uint32_t sextet_d = in[i] == '=' ? 0 & i++ : strchr(b64_charset, in[i++]) - b64_charset;
		
		uint32_t triple = (sextet_a << 3 * 6)
		+ (sextet_b << 2 * 6)
		+ (sextet_c << 1 * 6)
		+ (sextet_d << 0 * 6);
		
		if (j < olen) out[j++] = (triple >> 2 * 8) & 0xFF;
		if (j < olen) out[j++] = (triple >> 1 * 8) & 0xFF;
		if (j < olen) out[j++] = (triple >> 0 * 8) & 0xFF;
	}
	
	return j;
}

