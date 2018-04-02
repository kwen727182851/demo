#include "sha1_sum.h"
#include "hash_sha.h"
#include <stdlib.h>
#include <string.h>
/* This is a NOEXEC applet. Be very careful! */

enum {
	/* 4th letter of applet_name is... */
	HASH_MD5 = 's', /* "md5>s<um" */
	HASH_SHA1 = '1',
	HASH_SHA256 = '2',
	HASH_SHA3 = '3',
	HASH_SHA512 = '5',
};

#define FLAG_SILENT  1
#define FLAG_CHECK   2
#define FLAG_WARN    4

const char bb_hexdigits_upcase[] = "0123456789ABCDEF";
/* Emit a string of hex representation of bytes */
char* bin2hex(char *p, const char *cp, int count)
{
    while (count) {
        unsigned char c = *cp++;
        /* put lowercase hex digits */
        *p++ = 0x20 | bb_hexdigits_upcase[c >> 4];
        *p++ = 0x20 | bb_hexdigits_upcase[c & 0xf];
        count--;
    }
    return p;
}

/* This might be useful elsewhere */
static unsigned char *hash_bin_to_hex(unsigned char *hash_value,
				unsigned hash_length)
{
	/* xzalloc zero-terminates */
	char *hex_value = (char *)malloc((hash_length * 2) + 1);
	bin2hex(hex_value, (char*)hash_value, hash_length);
	return (unsigned char *)hex_value;
}

//return hash bin length
unsigned int hash_data(const void* data, unsigned int len, unsigned char* hash_bin)
{
	int hash_len;
	union _ctx_ {		
		sha1_ctx_t sha1;

	} context;
	unsigned char *hash_value;
	void  (*update)(void*, const void*, size_t);
	void  (*final)(void*, void*);

	sha1_begin(&context.sha1);
	update = (void*)sha1_hash;
	final = (void*)sha1_end;
	hash_len = 20;


	update(&context, data, len);
	
	hash_value = NULL;

	final(&context, hash_bin);
	//hash_value = hash_bin_to_hex(in_buf, hash_len);
	return hash_len;
}
