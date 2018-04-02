#ifndef SHA1_SUM
#define SHA1_SUM

#ifdef __cplusplus
extern "C"
{
#endif


unsigned int hash_data(const void* data, unsigned int len, unsigned char* hash_bin);

#ifdef __cplusplus
}
#endif

#endif
