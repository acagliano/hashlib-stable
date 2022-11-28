/*
 
 a tiny-ecdh implementation for the ez80 CPU
 
## CURVE SPEC ##
using curve secp224k1
define curve T = (p, a, b, G, n, h), where
finite field Fp is defined by:
	p = FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFE FFFFE56D
	f(x) = 2^224 − 2^32 − 2^12 − 2^11 − 2^9 − 2^7 − 2^4 − 2 − 1
curve E: y^2 = x^3 + ax + b over Fp is defined by:
	a = 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	b = 00000000 00000000 00000000 00000000 00000000 00000000 00000005
	G(x) = A1455B33 4DF099DF 30FC28A1 69A467E9 E47075A9 0F7E650E B6B7A45C
	G(y) = 7E089FED 7FBA3442 82CAFBD6 F7E319F7 C0B0BD59 E2CA4BDB 556D61A5
	n = 00000000 00000000 00000000 0001DCE8 D2EC6184 CAF0A971 769FB1F7
	h = 01
 
## KEYGEN ## generate key pair (d, Q)
d is secret. Assert d in range [1, n-1] (random).
Q = d*G
output (d, Q)
 
## PUBKEY VALID ##
assert Q != infinity point
assert xQ, yQ are of degree <= m-1
assert nQ = infinity point
if h = 1, skip final assertion
 
## SECRET COMPUTE ##
inputs:
	private key d(alice) associated with T(alice)
	public key Q(bob) associated with T(bob)
P = (x, y) = h * d(alice) * Q(bob)
if P = infinite point, invalid
output x as shared secret field
(optional, but recommended) pass x to a KDF to generate symmetric key
 */

#define ECC_PRV_KEY_SIZE	28
#define ECC_PUB_KEY_SIZE	(ECC_PRV_KEY_SIZE<<1)
#define CURVE_DEGREE		224

#define PLATFM_WORD_SIZE	sizeof(uint32_t)
#define ECC_NUM_WORDS		(ECC_PRV_KEY_SIZE / PLATFM_WORD_SIZE)

// main type definitions for variables
typedef uint32_t vec_t[ECC_NUM_WORDS];	// should be 24 bytes
struct Point {
	vec_t x;
	vec_t y;
};
struct Curve {
	vec_t polynomial;
	vec_t coeff_a;
	vec_t coeff_b;
	Point base;
	vec_t b_order;
	uint8_t cofactor;
};

#define

// each u32 word is written little-endian
struct Curve secp224k1 = {
	{0xFFFFE56D, 0xFFFFFFFE, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},	// p
	{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},	// a
	{0x00000005, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},	// b
	{
		{0xB6B7A45C, 0x0F7E650E, 0xE47075A9, 0x69A467E9, 0x30FC28A1, 0x4DF099DF, 0xA1455B33},	// x
		{0x556D61A5, 0xE2CA4BDB, 0xC0B0BD59, 0xF7E319F7, 0x82CAFBD6, 0x7FBA3442, 0x7E089FED}	// y
	},		// G
	{0x769FB1F7, 0xCAF0A971, 0xD2EC6184, 0x0001DCE8, 0x00000000, 0x00000000, 0x00000000},	// n
	1		// h
};
struct Point ta_resist = {0};

typedef enum _ecdh_errors {
	ECDH_OK,
	ECDH_INVALID_ARG,
	ECDH_PRIVKEY_INVALID,
} ecdh_error_t;


// ec point arithmetic prototypes
void point_mul_vect(struct Point *pt, vec_t *exp, uint24_t exp_bitlen);
void point_double(struct Point *pt);
void point_add(struct Point *pt1, struct Point *pt2);

uint24_t vect_get_bitlen(uint8_t *v, size_t v_len);

ecdh_error_t ecdh_keygen(uint8_t *pubkey, uint8_t *privkey, size_t privkey_len, uint32_t (rand*)()){
	if((pubkey==NULL) || (privkey==NULL))
		return ECDH_INVALID;
	
	if(rand!=NULL){
		uint32_t r;
		for(int i=0; i<ECC_NUM_WORDS; i++){
			r = rand();
			memcpy(&privkey[PLATFM_WORD_SIZE*i], &r, PLATFM_WORD_SIZE);
		}
	}
	/*
	// this is a nifty trick
	// you're supposed to reject a privkey that is not at least half the degree of the polynomial
	// degree of polynomial is 224, or 28 bytes
	// so only seek a set bit for half of the private key, return zero if fails, then error out
	size_t prkey_bitlen = vect_get_bitlen(privkey, ECC_PRV_KEY_SIZE>>1);
	if(prkey_bitlen == 0)
		return ECDH_PRIVKEY_INVALID;
	// then add half the key size to the bitlen
	prkey_bitlen += ECC_PRV_KEY_SIZE<<2;
	*/
	// as nifty as the trick is it's not needed because we're hard-forcing keylen to 28
	if (privkey_len < ECC_PRV_KEY_SIZE)
		return ECDH_PRIVKEY_INVALID;
	
	// copy G from curve parameters to pkey
	struct Point pkey;
	memcpy(pkey.x, secp224k1.base.x, ECC_PRV_KEY_SIZE);
	memcpy(pkey.y, secp224k1.base.y, ECC_PRV_KEY_SIZE);
	point_mul_vect(pkey, privkey, privkey_len<<3);
	
	// convert Point to bytearray => pubkey
	
	return ECDH_OK;
}

/*
 ### EC Point Arithmetic Functions ###
 */

#define GET_BIT(byte, bitnum) ((byte) & (1<<(bitnum)))
void point_mul_vect(struct Point *pt, uint8_t *exp, uint24_t exp_bitlen){
// multiplies pt by exp, result in pt
	struct Point tmp;
	struct Point res = {0};		// point-at-infinity
	memcpy(&tmp, pt, sizeof tmp);
	
	for(i = exp_bitlen; i >= 0; i--){
		if (GET_BIT(exp[i>>3], i&0x7))
			point_add(&res, &tmp);
		else
			point_add(&res, &ta_resist);	// add 0; timing resistance
		point_double(&tmp);
	}
	memcpy(pt, &res, sizeof pt);
}

void point_add(struct Point *pt1, struct Point *pt2){
	// how in the 37 layers of hell do you do this??
	// is this just a straight addition of two points or some weird bs?
	// I'm at like 16% soul right about now
}

void point_double(struct Point *pt){
	// same question here
	// https://www.nayuki.io/page/elliptic-curve-point-addition-in-projective-coordinates
	// it seems obnoxiously complex and computationally intensive
}


/*
 ### Vector Arithmetic Functions ###
 */

static void vect_add(vec_t *v1, vec_t *v2){
	
}

static void vect_double(vec_t *v){
	vect_lshift(v, 1);
}

static void vect_lshift(vec_t *v, int nbits){
	int nwords = (nbits / 32);
	
	/* Shift whole words first if nwords > 0 */
	int i,j;
	for (i = 0; i < nwords; ++i)
	{
		/* Zero-initialize from least-significant word until offset reached */
		v[i] = 0;
	}
	j = 0;
	/* Copy to x output */
	while (i < ECC_NUM_WORDS)
	{
		v[i] = v[j];
		i += 1;
		j += 1;
	}
	
	/* Shift the rest if count was not multiple of bitsize of DTYPE */
	nbits &= 31;
	if (nbits != 0)
	{
		/* Left shift rest */
		int i;
		for (i = (ECC_NUM_WORDS - 1); i > 0; --i)
		{
			x[i]  = (x[i] << nbits) | (x[i - 1] >> (32 - nbits));
		}
		x[0] <<= nbits;
	}
		
}
