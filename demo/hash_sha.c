
#include "hash_sha.h"
#include <string.h>

#define BB_LITTLE_ENDIAN   1
static uint64 bswap_64(uint64 _x)
{
    
    return ((_x >> 56) | ((_x >> 40) & 0xff00) | ((_x >> 24) & 0xff0000) |
        ((_x >> 8) & 0xff000000) | ((_x << 8) & ((uint64)0xff << 32)) |
        ((_x << 24) & ((uint64)0xff << 40)) | 
        ((_x << 40) & ((uint64)0xff << 48)) | ((_x << 56)));
}

unsigned int bswap_32(unsigned int x)
{
    return     ( x & 0xff000000U ) >>24 | 
        ( x & 0x00ff0000U ) >>8 | 
        ( x & 0x0000ff00U ) <<8 | 
        ( x & 0x000000ffU ) << 24 
        ; 
}

#define SWAP_BE32(x) bswap_32(x)



/* gcc 4.2.1 optimizes rotr64 better with inline than with macro
 * (for rotX32, there is no difference). Why? My guess is that
 * macro requires clever common subexpression elimination heuristics
 * in gcc, while inline basically forces it to happen.
 */
//#define rotl32(x,n) (((x) << (n)) | ((x) >> (32 - (n))))
static unsigned int rotl32(unsigned int x, unsigned n)
{
	return (x << n) | (x >> (32 - n));
}


/* Feed data through a temporary buffer.
 * The internal buffer remembers previous data until it has 64
 * bytes worth to pass on.
 */
static void common64_hash(md5_ctx_t *ctx, const void *buffer, unsigned int len)
{
	unsigned bufpos = ctx->total64 & 63;

	ctx->total64 += len;

	while (1) {
		unsigned remaining = 64 - bufpos;
		if (remaining > len)
			remaining = len;
		/* Copy data into aligned buffer */
		memcpy(ctx->wbuffer + bufpos, buffer, remaining);
		len -= remaining;
		buffer = (const char *)buffer + remaining;
		bufpos += remaining;
		/* Clever way to do "if (bufpos != N) break; ... ; bufpos = 0;" */
		bufpos -= 64;
		if (bufpos != 0)
			break;
		/* Buffer is filled up, process it */
		ctx->process_block(ctx);
		/*bufpos = 0; - already is */
	}
}


/* Process the remaining bytes in the buffer */
static void common64_end(md5_ctx_t *ctx, int swap_needed)
{
    unsigned bufpos = ctx->total64 & 63;
    /* Pad the buffer to the next 64-byte boundary with 0x80,0,0,0... */
    ctx->wbuffer[bufpos++] = 0x80;
    
    /* This loop iterates either once or twice, no more, no less */
    while (1) {
        unsigned remaining = 64 - bufpos;
        memset(ctx->wbuffer + bufpos, 0, remaining);
        /* Do we have enough space for the length count? */
        if (remaining >= 8) {
            /* Store the 64-bit counter of bits in the buffer */
            uint64 t = ctx->total64 << 3;
            if (swap_needed)
                t = bswap_64(t);
            /* wbuffer is suitably aligned for this */
            *(uint64 *) (&ctx->wbuffer[64 - 8]) = t;
        }
        ctx->process_block(ctx);
        if (remaining >= 8)
            break;
        bufpos = 0;
    }
}


/* Used also for sha1 and sha256 */
void md5_hash(md5_ctx_t *ctx, const void *buffer, unsigned int len)
{
    common64_hash(ctx, buffer, len);
}


static void sha1_process_block64(sha1_ctx_t *ctx)
{
	static const unsigned int rconsts[] = {
		0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6
	};
	int i, j;
	int cnt;
	unsigned int W[16+16];
	unsigned int a, b, c, d, e;

	/* On-stack work buffer frees up one register in the main loop
	 * which otherwise will be needed to hold ctx pointer */
	for (i = 0; i < 16; i++)
		W[i] = W[i+16] = SWAP_BE32(((unsigned int*)ctx->wbuffer)[i]);

	a = ctx->hash[0];
	b = ctx->hash[1];
	c = ctx->hash[2];
	d = ctx->hash[3];
	e = ctx->hash[4];

	/* 4 rounds of 20 operations each */
	cnt = 0;
	for (i = 0; i < 4; i++) {
		j = 19;
		do {
			unsigned int work;

			work = c ^ d;
			if (i == 0) {
				work = (work & b) ^ d;
				if (j <= 3)
					goto ge16;
				/* Used to do SWAP_BE32 here, but this
				 * requires ctx (see comment above) */
				work += W[cnt];
			} else {
				if (i == 2)
					work = ((b | c) & d) | (b & c);
				else /* i = 1 or 3 */
					work ^= b;
 ge16:
				W[cnt] = W[cnt+16] = rotl32(W[cnt+13] ^ W[cnt+8] ^ W[cnt+2] ^ W[cnt], 1);
				work += W[cnt];
			}
			work += e + rotl32(a, 5) + rconsts[i];

			/* Rotate by one for next time */
			e = d;
			d = c;
			c = /* b = */ rotl32(b, 30);
			b = a;
			a = work;
			cnt = (cnt + 1) & 15;
		} while (--j >= 0);
	}

	ctx->hash[0] += a;
	ctx->hash[1] += b;
	ctx->hash[2] += c;
	ctx->hash[3] += d;
	ctx->hash[4] += e;
}


void sha1_begin(sha1_ctx_t *ctx)
{
    ctx->hash[0] = 0x67452301;
    ctx->hash[1] = 0xefcdab89;
    ctx->hash[2] = 0x98badcfe;
    ctx->hash[3] = 0x10325476;
    ctx->hash[4] = 0xc3d2e1f0;
    ctx->total64 = 0;
    ctx->process_block = sha1_process_block64;
}


/* Used also for sha256 */
void sha1_end(sha1_ctx_t *ctx, void *resbuf)
{
    unsigned hash_size;
    
    /* SHA stores total in BE, need to swap on LE arches: */
    common64_end(ctx, /*swap_needed:*/ BB_LITTLE_ENDIAN);
    
    hash_size = (ctx->process_block == sha1_process_block64) ? 5 : 8;
    /* This way we do not impose alignment constraints on resbuf: */
    if (BB_LITTLE_ENDIAN) {
        unsigned i;
        for (i = 0; i < hash_size; ++i)
            ctx->hash[i] = SWAP_BE32(ctx->hash[i]);
    }
    memcpy(resbuf, ctx->hash, sizeof(ctx->hash[0]) * hash_size);
}




