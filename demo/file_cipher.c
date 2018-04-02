#include "file_cipher.h"
#include "sha1_sum.h"
#include "DesEncrypt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define HASH_SHA1_LEN       20


typedef struct
{
    int type_id;
    const char* type_name;
    unsigned char* key1;
    unsigned char* key2;
    unsigned char* key3;
    unsigned char strhead[3];
}DES_KEY_TYPE;

DES_KEY_TYPE hash_keys = {    
        -1, "HASH","45710025","58547415","56585475",{0x43,0x21,0x11}
};

static int des1d(unsigned char* ensrc, int ensrc_length, unsigned char* destr, int *destr_length, unsigned char* key)
{
    int count = 0;
	int restlen = 0;
	int reslen = 0;
	int loop = 0;
	int content_len = 0;
	unsigned char tembuf[9];
	unsigned char result[9];

	count = ensrc_length/8;
	restlen = ensrc_length%8;


	//memset(destr,0x0,sizeof(destr));
	for(loop=0;loop<count;loop++)
	{
		memset(tembuf,0x00,sizeof(tembuf));
		memcpy(tembuf,ensrc+(loop*8),8);
		
		memset(result,0x00,sizeof(result));
		desm1(tembuf,result,key);//解密
		memcpy(destr+(loop*8),result,8);	
		content_len += 8;
	}
	
	
	if(restlen!=0)
	{
		memset(tembuf,0x00,sizeof(tembuf));
		memcpy(tembuf,ensrc+(loop*8),restlen);
		
		memset(result,0x00,sizeof(result));
		desm1(tembuf,result,key);//解密
		memcpy(destr+(loop*8),result,restlen);	
		content_len += restlen;
	}
	*destr_length = content_len;
	return 1;
}

static  int desdd(unsigned char* ensrc, int ensrc_length, unsigned char* destr, int *destr_length, unsigned char* key)
{
    int count = 0;
    int restlen = 0;
    int reslen = 0;
    int loop = 0;
    int content_len = 0;
    unsigned char tembuf[9];
    unsigned char result[9];
    
    count = ensrc_length/8;
    restlen = ensrc_length%8;
    
    
    //memset(destr,0x0,sizeof(destr));
    for(loop=0;loop<count;loop++)
    {
        memset(tembuf,0x00,sizeof(tembuf));
        memcpy(tembuf,ensrc+(loop*8),8);
        
        memset(result,0x00,sizeof(result));
        des(tembuf,result,key);//加密
        memcpy(destr+(loop*8),result,8);	
        content_len += 8;
    }
    
    
    if(restlen!=0)
    {
        memset(tembuf,0x00,sizeof(tembuf));
        memcpy(tembuf,ensrc+(loop*8),restlen);
        
        memset(result,0x00,sizeof(result));
        des(tembuf,result,key);//加密
        memcpy(destr+(loop*8),result,restlen);	
        content_len += restlen;
    }
    *destr_length = content_len;
    return 1;
}

static  int decrypt_hash(unsigned char* srcdata, int srcdata_len, unsigned char* dstdata)
{
    unsigned char data_tmp[1024];
    int dstdata_len = 0;

    unsigned char strlength = 0;
    int i = 0,decode_length = 0;
    
	
    unsigned char* des_key1 = hash_keys.key1;
    unsigned char* des_key2 = hash_keys.key2;



	des1d(srcdata,srcdata_len,data_tmp,&decode_length,des_key2);
	des1d(data_tmp,decode_length,dstdata,&dstdata_len,des_key1);
	
	return dstdata_len;
}

static  int encrypt_hash(unsigned char* srcdata, int srcdata_len, unsigned char* dstdata)
{
    unsigned char data_tmp[1024];
    int dstdata_len = 0;
    
    unsigned char strlength = 0;
    int i = 0,decode_length = 0;
    
    
    unsigned char* des_key1 = hash_keys.key1;
    unsigned char* des_key2 = hash_keys.key2;
    
    
    
    desdd(srcdata,srcdata_len,data_tmp,&decode_length,des_key1);
    desdd(data_tmp,decode_length,dstdata,&dstdata_len,des_key2);
    
    return dstdata_len;
}




int gen_cipher_file(const char* filepath, const char* cipherpath)
{
    //生成sha1
    unsigned char sha1_bin[HASH_SHA1_LEN+4] = {0};
    unsigned char sha1_bin_encrypt[HASH_SHA1_LEN+4];
    int sha1_bin_encrypt_len = 0; 
    FILE* stream;
    int FileSize;
    int retVal=1;
    unsigned char* ptr = NULL,*pptr;
    stream=fopen(filepath, "rb");
    if (stream==NULL)
    {
        return 0;
    }
    fseek(stream,0,SEEK_END);
    FileSize=ftell(stream);
    fseek(stream,0,SEEK_SET);
    ptr=(unsigned char*)malloc((FileSize)+1);
    if (ptr==NULL)
        retVal=0;
    else
    {
        if (fread(ptr, 1, FileSize,stream) != (unsigned int)FileSize)
            retVal=0;
        
    }
    fclose(stream);

    if (retVal)
    {
        char cfilepath[1024];
        retVal = 0;
        hash_data(ptr,(unsigned int)FileSize,sha1_bin);//生成sha1
        sha1_bin_encrypt_len = encrypt_hash(sha1_bin,HASH_SHA1_LEN+4, sha1_bin_encrypt);//加密sha1
        if (cipherpath)
        {
            sprintf(cfilepath, "%s",cipherpath);
        }
        else
        {
            sprintf(cfilepath, "%s_cipher",filepath);
        }       

        stream=fopen(cfilepath, "wb");
        if (stream)
        {
            //写密文件
            fwrite(sha1_bin_encrypt, sha1_bin_encrypt_len,1,stream);
            pptr = ptr + FileSize - 1;
            while(pptr != ptr)//倒序写源文件
            {
                fwrite(pptr,1,1,stream);
                pptr--;
            }
            fwrite(pptr,1,1,stream);
            fclose(stream);
            retVal = 1;
        }
    }

    if (ptr)
    {
        free(ptr);
    }

    return retVal;
}


unsigned char* gen_cipher_file_to_buffer(const char* filepath, int* cipher_len)
{
	//生成sha1
	unsigned char sha1_bin[HASH_SHA1_LEN+4] = {0};
	unsigned char sha1_bin_encrypt[HASH_SHA1_LEN+4];
	int sha1_bin_encrypt_len = 0; 
	FILE* stream;
	int FileSize;
	int retVal=1;
	unsigned char* ptr = NULL,*pptr, *ci_buf=NULL, *pcibuf;
	int clen = 0;
	stream=fopen(filepath, "rb");

	if (stream==NULL)
	{
		return 0;
	}
	fseek(stream,0,SEEK_END);
	FileSize=ftell(stream);
	fseek(stream,0,SEEK_SET);
	ptr=(unsigned char*)malloc((FileSize)+1);
	if (ptr==NULL)
		retVal=0;
	else
	{
		if (fread(ptr, 1, FileSize,stream) != (unsigned int)FileSize)
			retVal=0;

	}
	fclose(stream);

	if (retVal)
	{
		char cfilepath[1024];
		retVal = 0;
		hash_data(ptr,(unsigned int)FileSize,sha1_bin);//生成sha1
		sha1_bin_encrypt_len = encrypt_hash(sha1_bin,HASH_SHA1_LEN+4, sha1_bin_encrypt);//加密sha1
		
		clen = sha1_bin_encrypt_len + FileSize;
		ci_buf = (unsigned char*)malloc(clen);
		if (ci_buf == NULL)
		{
			ci_buf = NULL;
			*cipher_len = 0;
		}
		else
		{
			
			*cipher_len = clen;
			//写密文件
			pcibuf = ci_buf;
			memcpy(pcibuf, sha1_bin_encrypt, sha1_bin_encrypt_len);
			pcibuf += sha1_bin_encrypt_len;
			pptr = ptr + FileSize - 1;
			while(pptr != ptr)//倒序写源文件
			{
				*pcibuf = *pptr;
				//fwrite(pptr,1,1,stream);
				pptr--;
				pcibuf++;
			}
			*pcibuf = *pptr;
			//fwrite(pptr,1,1,stream);
			//fclose(stream);
			retVal = 1;
		}
	}

	if (ptr)
	{
		free(ptr);
	}

	return ci_buf;
}





int restore_cipher_file(const char* cfilepath, const char* restorepath)
{

    FILE* stream;
    int FileSize;
    int retVal=1;
    unsigned char* ptr = NULL;
    stream=fopen(cfilepath, "rb");
    if (stream==NULL)
    {
        return 0;
    }
    fseek(stream,0,SEEK_END);
    FileSize=ftell(stream);
    fseek(stream,0,SEEK_SET);
    ptr=(unsigned char*)malloc((FileSize)+1);
    if (ptr==NULL)
        retVal=0;
    else
    {
        if (fread(ptr, 1, FileSize,stream) != (unsigned int)FileSize)
            retVal=0;
        
    }
    fclose(stream);

    if (retVal)
    {        
        //保存文件
        char nfilepath[1024];
        retVal = 0;

        if (restorepath)
        {
            sprintf(nfilepath, "%s_restore",restorepath);
        }
        else
        {
            sprintf(nfilepath, "%s_restore",cfilepath);
        }
        
        retVal = restore_cipher_buffer(ptr, FileSize, nfilepath);
    }
    
    if (ptr)
    {
        free(ptr);
    }
    
    return retVal;
   
}

//返回 0 验证错误，恢复失败 1 验证成功
int restore_cipher_buffer(unsigned char* cbuffer, int buf_len, const char* restorepath)
{
    unsigned char sha1_bin[HASH_SHA1_LEN+4] = {0};
    unsigned char sha1_bin2[HASH_SHA1_LEN+4] = {0};
    unsigned char sha1_bin_encrypt[HASH_SHA1_LEN+4];
    int sha1_bin_encrypt_len = HASH_SHA1_LEN+4; 
    FILE* stream;
    int FileSize;
    int retVal=0;
    unsigned char* ptr = cbuffer;
    unsigned char *pptrs,*pptre;
    int hfsize = 0;
    unsigned char tmpc;

    FileSize = buf_len;

    //获取解密sha1
    memcpy(sha1_bin_encrypt,ptr,sha1_bin_encrypt_len);
    decrypt_hash(sha1_bin_encrypt, sha1_bin_encrypt_len, sha1_bin2);
    //获取源文件数据
    pptrs = ptr + sha1_bin_encrypt_len;
    pptre = ptr + FileSize - 1;
    hfsize = (FileSize - sha1_bin_encrypt_len)/ 2;
    while(hfsize)
    {
        tmpc = *pptre;
        *pptre = *pptrs;
        *pptrs = tmpc;
        pptrs++;
        pptre--;
        hfsize--;
    }
    
    
    //验证源文件sha1
    pptrs = ptr + sha1_bin_encrypt_len;
    FileSize = FileSize - sha1_bin_encrypt_len;
    
    hash_data(pptrs,(unsigned int)FileSize,sha1_bin);        
    
    if (memcmp(sha1_bin,sha1_bin2,HASH_SHA1_LEN) == 0)
    {
        retVal = 1;
        //保存文件
        if (restorepath)
        {
            stream=fopen(restorepath, "wb");
            if (stream)
            {
                
                fwrite(pptrs,FileSize,1,stream);
                fclose(stream);
            }
        }
    }        
    
    return retVal;
    
}


