#ifndef __DesEncrypt_H
#define __DesEncrypt_H


#define EN0	0	// MODE == encrypt
#define DE1	1	// MODE == decrypt
	
#define BYTE  unsigned char
#define WORD  unsigned short
#define DWORD unsigned long   
	
	typedef union {
		unsigned long blok[2];
		unsigned short word[4];
		unsigned char byte[8];
	} M68K;
	
	//void des8(unsigned char *InData,unsigned char *key,unsigned char *OutData,short Mode,int readlen);
	//void des16(unsigned char *InData,unsigned char *key,unsigned char *OutData,short Mode,int readlen);
	
	//void hex_asc (BYTE *ascii_buf, BYTE *hex_buf, BYTE lg);
	//int data_convert(unsigned char *in,int in_len,unsigned char * out,char flag,unsigned char *key);
	//int DESTrans(unsigned char *in,int in_len,unsigned char * out,char flag,unsigned char *key);

	
	void Ks(BYTE *Key, BYTE Kn[16][48]);
	void fonction(BYTE *Knn, BYTE *r, BYTE *s);
	void permutation(BYTE *org, BYTE *tab);
	void chiffrement(BYTE *xi, BYTE *xo, BYTE Kn[16][48]);
	void dechiffrement(BYTE *xi, BYTE *xo, BYTE Kn[16][48]);
	void eclater(BYTE *buf_bit, BYTE *byte);
	void compacter(BYTE *byte, BYTE *buf_bit);
	void des(BYTE *binput, BYTE *boutput, BYTE *bkey);
	void desm1(BYTE *binput, BYTE *boutput, BYTE *bkey);
	//void ThreeDes(BYTE *binput, BYTE *boutput, BYTE *bkey);
	//void DeThreeDes(BYTE *binput, BYTE *boutput, BYTE *bkey);
	
#endif
