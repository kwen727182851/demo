#ifndef ASYNC_TRANS_H
#define ASYNC_TRANS_H
#include "windows.h"
#include "PSerialPort.h"

#ifdef __cplusplus
extern "C" {
#endif

int OpenPortAsyn(unsigned int portNo, unsigned int baudrate, LPDataArriveProc proc,DWORD userdata);
int OpenPort(unsigned int portNo, unsigned int baudrate);
int ClosePort(void);

int RecvRSData(unsigned char* recvBuf, int bufsize, int timeout);//ms
int SendRSData(unsigned char *sendBuf, unsigned int sendLen);

void SetRecvType(int type);

#ifdef __cplusplus
}
#endif

#endif