#ifndef HASH_SHA
#define HASH_SHA


#if defined _MSC_VER || defined __BORLANDC__
typedef __int64 int64;
typedef unsigned __int64 uint64;
#else
typedef long long int64;
typedef unsigned long long uint64;
#endif

typedef struct md5_ctx_t {
    unsigned char wbuffer[64]; /* always correctly aligned for uint64_t */
    void (*process_block)(struct md5_ctx_t*);
    uint64 total64;    /* must be directly before hash[] */
    unsigned int hash[8];    /* 4 elements for md5, 5 for sha1, 8 for sha256 */
} md5_ctx_t;
typedef struct md5_ctx_t sha1_ctx_t;


void md5_hash(md5_ctx_t *ctx, const void *buffer, unsigned int len);
#define sha1_hash md5_hash

void sha1_begin(sha1_ctx_t *ctx);
void sha1_end(sha1_ctx_t *ctx, void *resbuf);



#endif