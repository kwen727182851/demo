#ifndef GEN_CIPHER_FILE
#define GEN_CIPHER_FILE


#ifdef __cplusplus
extern "C" {
#endif
    
int gen_cipher_file(const char* filepath, const char* cipherpath);
unsigned char* gen_cipher_file_to_buffer(const char* filepath, int* cipher_len);
int restore_cipher_file(const char* filepath, const char* restorepath);
int restore_cipher_buffer(unsigned char* cbuffer, int buf_len, const char* restore_path);
    
#ifdef __cplusplus
}
#endif
#endif