// PSerialPort.cpp: implementation of the CPSerialPort class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PSerialPort.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPSerialPort::CPSerialPort()
: m_bHasClosed(false)
{
	m_hComm=INVALID_HANDLE_VALUE;
	m_hReadThread=NULL;
	m_bReceiving=FALSE;
	m_nBufferSize=256; //缓冲大小

	m_ReadOverlap.Internal = 0;
	m_ReadOverlap.InternalHigh = 0;
	m_ReadOverlap.Offset = 0;
	m_ReadOverlap.OffsetHigh = 0;
	m_ReadOverlap.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);

	m_WriteOverlap.Internal = 0;
	m_WriteOverlap.InternalHigh = 0;
	m_WriteOverlap.Offset = 0;
	m_WriteOverlap.OffsetHigh = 0;
	m_WriteOverlap.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);

	m_hReadEvent = NULL;
	m_hWriteEvent = NULL;

	m_bMonitorExit = FALSE;

	m_hMonitor = NULL;

	hWriteMutex = CreateMutex(NULL, FALSE, NULL);

}

CPSerialPort::~CPSerialPort()
{
	m_bMonitorExit = TRUE;
	WaitForSingleObject(m_hMonitor, INFINITE);
	CloseHandle(m_ReadOverlap.hEvent);
	CloseHandle(m_WriteOverlap.hEvent);
	ClosePort();
	CloseHandle(hWriteMutex);
}

DWORD WINAPI CPSerialPort::ReadPortThread(LPVOID lpParameter)
{
	CPSerialPort* m_pSerial;

	m_pSerial=(CPSerialPort*)lpParameter;

	BOOL fReadState;
	DWORD dwLength;
	
	unsigned char* buf=new unsigned char[m_pSerial->m_nBufferSize];

	while((m_pSerial->m_hComm!=INVALID_HANDLE_VALUE)&&(m_pSerial->m_bReceiving))
	{		
		fReadState=ReadFile(m_pSerial->m_hComm,buf,m_pSerial->m_nBufferSize,&dwLength,NULL);
		if(!fReadState)
		{
			//AfxMessageBox(_T("无法从串口读取数据！"));
		}
		else
		{
			if(dwLength!=0)
			{
				//回送数据
				if(m_pSerial->m_lpDataArriveProc!=NULL)
				{
					m_pSerial->m_lpDataArriveProc(buf,dwLength,m_pSerial->m_dwUserData);
				}
			}
		}		
	}

	delete[] buf;

	return 0;
}

BOOL CPSerialPort::OpenPort(LPCTSTR Port,int BaudRate,int DataBits,int StopBits,int Parity,int type, LPDataArriveProc proc,DWORD userdata)
{
	m_lpDataArriveProc=proc;
	m_dwUserData=userdata;

	m_type = type;

	if(m_hComm==INVALID_HANDLE_VALUE)
	{
		m_hComm=CreateFile(Port,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,type,NULL);
		if(m_hComm==INVALID_HANDLE_VALUE )
		{
			::MessageBox(NULL,_T("Unable to open serial port specified!"),_T("Error"),MB_OK);
			return FALSE;
		}
		GetCommState(m_hComm,&dcb);
		dcb.BaudRate=BaudRate;
		dcb.ByteSize=DataBits;
		dcb.Parity=Parity;
		dcb.StopBits=StopBits;
		dcb.fParity=FALSE;
		dcb.fBinary=TRUE;

		
		dcb.fDtrControl=DTR_CONTROL_DISABLE;
		dcb.fRtsControl=DTR_CONTROL_DISABLE;
		dcb.fOutX=dcb.fInX=dcb.fTXContinueOnXoff=0;
		dcb.fOutxCtsFlow = FALSE;
		dcb.fOutxDsrFlow = FALSE;
		dcb.fDsrSensitivity = FALSE;

		//设置状态参数
		SetCommMask(m_hComm,EV_RXCHAR | EV_TXEMPTY);		
		SetupComm(m_hComm,16384,16384);		
		if(!SetCommState(m_hComm,&dcb))
		{
			::MessageBox(NULL,_T("Unable to SetupComm"),_T("Error"),MB_OK);
			PurgeComm(m_hComm,PURGE_TXCLEAR|PURGE_RXCLEAR);
			ClosePort();
			return FALSE;
		}
		
		//设置超时参数
		//usb gs 或 原DSP方案的虚拟串口 50 0 50（100） 
		//君正 x1000 串口 使用接口板的转换串口 要设置 MAXDWORD 0 0 否则会丢少量数据
		GetCommTimeouts(m_hComm,&CommTimeOuts);		
		CommTimeOuts.ReadIntervalTimeout=50;//100
		CommTimeOuts.ReadTotalTimeoutMultiplier=0;//1
		CommTimeOuts.ReadTotalTimeoutConstant=100;//100
		CommTimeOuts.WriteTotalTimeoutMultiplier=0;
		CommTimeOuts.WriteTotalTimeoutConstant=0;		
		if(!SetCommTimeouts(m_hComm,&CommTimeOuts))
		{
			::MessageBox(NULL,_T("Unable to SetCommTimeouts"),_T("Error!"),MB_OK);
			PurgeComm(m_hComm,PURGE_TXCLEAR|PURGE_RXCLEAR);
			ClosePort();
			return FALSE;
		}
		
		PurgeComm(m_hComm,PURGE_TXCLEAR|PURGE_RXCLEAR);		

		if (type == FILE_FLAG_OVERLAPPED)
		{
			m_hMonitor = CreateThread(NULL, 0, ComMonitor, this, 0, NULL);
		}

		return TRUE;		
	}
	
	return FALSE;
}

BOOL CPSerialPort::ClosePort()
{
	//Deactivate();
	m_bHasClosed = false;
	if(m_hComm!=INVALID_HANDLE_VALUE)
	{
		SetCommMask(m_hComm,0);		
		PurgeComm(m_hComm,PURGE_TXCLEAR|PURGE_RXCLEAR);
		if (!CloseHandle(m_hComm))
		{
			return FALSE;
		}
		
		m_hComm=INVALID_HANDLE_VALUE;
		return TRUE;
	}
	
	return TRUE;	
}

BOOL CPSerialPort::Activate()
{
	if(m_hComm==INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	if(!m_bReceiving)
	{
		//开始接收线程
		PurgeComm(m_hComm,PURGE_TXCLEAR|PURGE_RXCLEAR);
		m_bReceiving=TRUE;
		m_hReadThread=CreateThread(NULL,0,ReadPortThread,this,0,NULL);
	}
	if(m_hReadThread!=NULL)
	{		
		return TRUE;
	}
	else
	{
		m_bReceiving=FALSE;
		return FALSE;
	}

	return FALSE;
}

BOOL CPSerialPort::Deactivate()
{
	if(m_hComm==INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	//停止接收线程
	if(m_bReceiving)
	{
		m_bReceiving=FALSE;
		WaitForSingleObject(m_hReadThread,500);
		CloseHandle(m_hReadThread);
		m_hReadThread=NULL;
		return TRUE;
	}

	return FALSE;
}

BOOL CPSerialPort::IsActive()
{
	return m_bReceiving;
}

DWORD CPSerialPort::WritePort(unsigned char *data,int length)
{
	if(m_hComm==INVALID_HANDLE_VALUE)
	{
		return 0;
	}
	WaitForSingleObject(hWriteMutex, INFINITE);
	BOOL fWriteState;
	DWORD dwBytesWritten=0;

	if (m_type == FILE_FLAG_OVERLAPPED)
	{
		fWriteState=WriteFile(m_hComm,data,length*sizeof(unsigned char),&dwBytesWritten,&m_WriteOverlap);
		if(!fWriteState)
		{
			DWORD dwError = GetLastError();
			if (dwError == ERROR_IO_PENDING)
			{
				while(1)
				{
					DWORD dwRet = WaitForSingleObject(m_WriteOverlap.hEvent, INFINITE);
					if (dwRet == WAIT_OBJECT_0)
					{	
						GetOverlappedResult(m_hComm, &m_WriteOverlap, &dwBytesWritten,FALSE);
						break;
					}
				}
			}			
		}
	}
	else
	{
		fWriteState=WriteFile(m_hComm,data,length*sizeof(unsigned char),&dwBytesWritten,NULL);
	}
	
	

	ReleaseMutex(hWriteMutex);

	return dwBytesWritten;
}

DWORD CPSerialPort::ReadPort(unsigned char *data,unsigned int length, int timeout_ms)
{
	BOOL fReadState;
	DWORD dwLength,dwBytesRead=0;
	int TimeOutCount;
	int nToReadLength = length;
	dwBytesRead=0;
	TimeOutCount=0;

    double st_time = clock();
    double cur_time = 0;
    double duration = 0;
    int outtimes = 0;
    int maxtimes = 100;

	while(m_hComm!=INVALID_HANDLE_VALUE)
	{
		if (m_type == FILE_FLAG_OVERLAPPED)
		{
			fReadState=ReadFile(m_hComm,data,nToReadLength,&dwLength,&m_ReadOverlap);
			if (!fReadState)
			{
				DWORD dwError = GetLastError();
				if (dwError != ERROR_IO_PENDING)
				{
					break;         //发生大错误
				}
				DWORD dwRet = WaitForSingleObject(m_ReadOverlap.hEvent, INFINITE);
				if (dwRet == WAIT_OBJECT_0)
				{	
					GetOverlappedResult(m_hComm, &m_ReadOverlap, &dwLength,FALSE);
					dwBytesRead = dwLength;
					break;
				}
			}			
			else
			{
				if(dwLength==length)
				{
					dwBytesRead = dwLength;
					break;
				}
			}			
		}
		else
		{
            dwLength = 0;
			fReadState=ReadFile(m_hComm,data,nToReadLength,&dwLength,NULL);
			if(!fReadState || dwLength == 0)
			{
                if (dwBytesRead == 0 && timeout_ms > 0)
                {
                    cur_time = clock();
                    duration = ((double)(cur_time - st_time));
                    if (duration > timeout_ms)
                    {
                        break;
                    }
                    Sleep(2);
                }
                else
                {
                    if (outtimes < maxtimes )
                    {
                        if (dwBytesRead == 0)
                        {
                            Sleep(100);
                            maxtimes = 5;
                        }
                        else
                        {
                            Sleep(10);
                            maxtimes = 3;
                        }
                        outtimes++;                
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
			}
			dwBytesRead += dwLength;
    		data += dwLength;		
			nToReadLength-=dwLength;

            if (dwBytesRead == length)
            {
                break;
            }
		}
	}
	return dwBytesRead;
}

DWORD CPSerialPort::WriteFileToPort(LPCTSTR FileName)
{
	/*
	if(m_hComm==INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	CFile cf;

	BOOL fWriteState;
	DWORD dwBytesWritten;
	DWORD dwCharToWrite;

	dwCharToWrite=0;

	if(!cf.Open(FileName,CFile::modeRead))
	{
		//AfxMessageBox(_T("无法打开Hex文件！"));
		return 0;
	}
	dwCharToWrite=(DWORD)cf.GetLength();
	cf.Seek(0,CFile::begin);
	dwBytesWritten=0;
	
	if(m_hComm!=INVALID_HANDLE_VALUE&&dwCharToWrite!=0)
	{
		char* buf=new char[dwCharToWrite];
		cf.Read(buf,dwCharToWrite);

		fWriteState=WriteFile(m_hComm,buf,dwCharToWrite*sizeof(char),&dwBytesWritten,NULL);
		if(!fWriteState)
		{
			//AfxMessageBox(_T("无法向端口写入数据！"));
		}
		delete[] buf;		
	}
	cf.Close();
	return dwBytesWritten;
	*/
	return 0;
}
void CPSerialPort::SetRxEvent(HANDLE hReadEvent, HANDLE hWriteEvent)
{
	m_hReadEvent = hReadEvent;
	m_hWriteEvent = hWriteEvent;
}
DWORD CPSerialPort::ComMonitor(LPVOID lparam)
{
	CPSerialPort * pSP = (CPSerialPort *)lparam;
	
	while(!pSP->m_bMonitorExit)
	{
		DWORD dwError = 0;
		COMSTAT comstat;
		BOOL bRet = ClearCommError(pSP->m_hComm, &dwError, &comstat);
		if (!bRet)
		{
			DWORD err = GetLastError();
			if (err == ERROR_BAD_COMMAND)
			{
				pSP->m_bHasClosed = true;
				break;
			}
		}
		if (comstat.cbInQue != 0 )
		{
			SetEvent(pSP->m_hReadEvent);
		}
		Sleep(50);
	}
	return 0;
}

void CPSerialPort::SetState(int BaudRate,int DataBits,int StopBits,int Parity)
{
	DCB	dcb;
	GetCommState(m_hComm,&dcb);
	dcb.BaudRate=BaudRate;
	dcb.ByteSize=DataBits;
	dcb.Parity=Parity;
	dcb.StopBits=StopBits;
	dcb.fParity=FALSE;
	dcb.fBinary=TRUE;
	dcb.fDtrControl=0;
	dcb.fRtsControl=0;
	dcb.fOutX=dcb.fInX=dcb.fTXContinueOnXoff=0;

	if(!SetCommState(m_hComm,&dcb))
	{
		::MessageBox(NULL,_T("Unable to SetupComm"),_T("Error"),MB_OK);
		PurgeComm(m_hComm,PURGE_TXCLEAR|PURGE_RXCLEAR);
		ClosePort();
	}
}

void CPSerialPort::ClearRevPort(void)
{
	if (m_hComm != NULL)
	{
		PurgeComm(m_hComm,PURGE_RXCLEAR);
	}
}
void CPSerialPort::WaitWriteComplete()
{
	DWORD dwRet = WaitForSingleObject(m_WriteOverlap.hEvent, INFINITE);
}