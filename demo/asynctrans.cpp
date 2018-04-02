
#include "asynctrans.h"
#include "stdio.h"
#include <time.h>
static CPSerialPort s_serial;
static HANDLE	    s_hRevEvent = NULL;
static HANDLE       s_hWriteEvent= NULL;
static HANDLE         s_hReadThread = NULL;

static int m_toExitReadThread = 0;
static int s_recv_type = 0;
// 
// unsigned long WINAPI ReadThread(void * lparam)
// {
//     CPSerialPort* m_pSerial;
//     m_pSerial=(CPSerialPort*)lparam;
//     
//     
//     unsigned char recvBuf[1024] = {0};
//     int maxBufsize = 1024;
//     int nReadLength = 0;
//     int nTotalRead = 0;
//     int tosend = 0;
//     int firstrecv = 1;
// 
//     double st_time = clock();
//     double cur_time = 0;
//     double duration = 0;
//     while(m_toExitReadThread == 0)
//     {
//         //DebugLog("listen");
//         DWORD dwResult = WaitForSingleObject(m_pSerial->m_hReadEvent, 1000);
//         if (dwResult != WAIT_OBJECT_0)
//         {
//             //无数据
//             //continue;
//         }
//         else
//         {
//             nReadLength = m_pSerial->ReadPort(recvBuf+nTotalRead,maxBufsize - nTotalRead);//CMDLENGTH
//             nTotalRead += nReadLength;
//             if( nReadLength == 0)
//             {
//                 ResetEvent(m_pSerial->m_hReadEvent);
//             }
//             else
//             {
//                 if (firstrecv == 1)
//                 {
//                     firstrecv = 2;//接受中
//                 }
//             }
// 
//             if (maxBufsize - nTotalRead == 0)
//             {
//                 //缓存已满
//                 tosend = 1;
//             }
//             tosend = 1;
//         }
//         
//         if(tosend && nTotalRead)
//         {
//             //回送数据
//             if(m_pSerial->m_lpDataArriveProc!=NULL)
//             {
//                 m_pSerial->m_lpDataArriveProc(recvBuf,nTotalRead,m_pSerial->m_dwUserData);
// 		    }
// 
//             nTotalRead = 0;
//             nReadLength = 0;
//             firstrecv = 1;
//             tosend = 0;
//         }
//     }
//     
//     return 0;
// }

unsigned long WINAPI ReadThread(void * lparam)
{
    CPSerialPort* m_pSerial;
    m_pSerial=(CPSerialPort*)lparam;
    
    const int maxBufsize = 4096;
    unsigned char recvBuf[maxBufsize] = {0};
    
    int nReadLength = 0;
    int nTotalRead = 0;
    int tosend = 0;
    int firstrecv = 0;


    
    while(m_toExitReadThread == 0)
    {
        //DebugLog("listen");
        DWORD dwResult = WaitForSingleObject(m_pSerial->m_hReadEvent, 50);//firstrecv > 0 ? 5: 50);
        if (dwResult != WAIT_OBJECT_0)
        {
            //无数据
            //continue;
            if (firstrecv > 0)
            {
                tosend = 1;
            }
        }
        else
        {
            nReadLength = m_pSerial->ReadPort(recvBuf+nTotalRead,maxBufsize - nTotalRead);
            nTotalRead += nReadLength;
            if( nReadLength == 0)
            {
                ResetEvent(m_pSerial->m_hReadEvent);
                if (firstrecv > 0)
                {
                    tosend = 1;
                }
            }
            else
            {                
                firstrecv = 1;//接受中  
				/*
				//升级中直接回传
				if (s_recv_type == 0x11 && nReadLength == 1)
				{
					ResetEvent(m_pSerial->m_hReadEvent);
					tosend = 1;
				}*/
            }
            
            if (maxBufsize - nTotalRead == 0)
            {
                //缓存已满
                tosend = 1;
            }
            //tosend = 1;
        }
        
        if(tosend && nTotalRead)
        {

            //回送数据
            if(m_pSerial->m_lpDataArriveProc!=NULL)
            {
                m_pSerial->m_lpDataArriveProc(recvBuf,nTotalRead,m_pSerial->m_dwUserData);
            }
            
            nTotalRead = 0;
            nReadLength = 0;
            firstrecv = 0;
            tosend = 0;
        }
    }
    
    return 0;
}


//成功 1
int OpenPortAsyn(unsigned int portNo, unsigned int baudrate,LPDataArriveProc proc,DWORD userdata)
{
	char szPort[16] = {0};	
	// prepare port strings
	sprintf(szPort, "\\\\.\\COM%d", portNo);

    if (s_hReadThread || s_hRevEvent || s_hWriteEvent)
    {
        return -1;
    }
    s_hRevEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    s_hWriteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	
    s_serial.SetRxEvent(s_hRevEvent, s_hWriteEvent);
    int res =  s_serial.OpenPort(szPort, baudrate, 8,0,0,FILE_FLAG_OVERLAPPED,proc, userdata);

    if(res == TRUE)
    {
        s_hReadThread = CreateThread(NULL,0,ReadThread,&s_serial,0,NULL);
    }
    else
    {
        CloseHandle(s_hRevEvent);
        CloseHandle(s_hWriteEvent);
        s_hRevEvent = NULL;
        s_hWriteEvent = NULL;
    }

    m_toExitReadThread = 0;
    
    return res;
}

//成功 1
int OpenPort(unsigned int portNo, unsigned int baudrate)
{
    char szPort[16] = {0};	
    // prepare port strings
    sprintf(szPort, "\\\\.\\COM%d", portNo);
    
    if (s_hReadThread || s_hRevEvent || s_hWriteEvent)
    {
        return -1;
    }
    s_hRevEvent = NULL;
    s_hWriteEvent = NULL;
    
    s_serial.SetRxEvent(s_hRevEvent, s_hWriteEvent);
    int res =  s_serial.OpenPort(szPort, baudrate, 8,0,0,0);
    
    m_toExitReadThread = 0;
    s_hReadThread = NULL;
    return res;
}

int ClosePort(void)
{
    s_serial.ClosePort();

    m_toExitReadThread = 1;
    WaitForSingleObject(s_hReadThread, 2000);
    CloseHandle(s_hReadThread);
    CloseHandle(s_hRevEvent);
	CloseHandle(s_hWriteEvent);

    s_hReadThread = NULL;
    s_hRevEvent = NULL;
    s_hWriteEvent = NULL;
	return 1;
}

int SendRSData(unsigned char *sendBuf, unsigned int sendLen)
{
    
    return s_serial.WritePort(sendBuf, sendLen);
}

int RecvRSData(unsigned char* recvBuf, int bufsize, int timeout)//ms
{
    return s_serial.ReadPort(recvBuf, bufsize);
}


void SetRecvType(int type)
{
	s_recv_type = type;
}