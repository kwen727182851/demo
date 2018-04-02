// PSerialPort.h: interface for the CPSerialPort class.
//
//////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef void (*LPDataArriveProc)(unsigned char *data, int length,DWORD userdata);

class CPSerialPort  
{
public:
	CPSerialPort();
	virtual ~CPSerialPort();

	BOOL OpenPort(LPCTSTR Port,int Baudrate,int DataBits,int StopBits,int Parity,int type,LPDataArriveProc proc=NULL,DWORD userdata=0); //�򿪴���
	BOOL ClosePort(); //�رմ���
	
	//������رճ�������
	BOOL Activate();
	BOOL Deactivate();
	BOOL IsActive();
	
	//���������̺߳���
	static DWORD WINAPI ReadPortThread(LPVOID lpParameter);
	
	DWORD ReadPort(unsigned char *data,unsigned int length,int timeout_ms = 0); //��ȡһ�����ȵ�����
	DWORD WritePort(unsigned char *data,int length); //��������
	DWORD WriteFileToPort(LPCTSTR FileName); //�����ļ�
	
public:
	HANDLE m_hComm; //�����豸handle
	HANDLE m_hReadThread; //�������߳�handle
	BOOL m_bReceiving; //�Ƿ��������
	int m_nBufferSize; //�����С

	char *Buffer; //������
	
	LPDataArriveProc m_lpDataArriveProc;
	DWORD m_dwUserData;

	//���������Լ���ʱ����
	DCB dcb;
	COMMTIMEOUTS CommTimeOuts;	

	OVERLAPPED m_ReadOverlap, m_WriteOverlap;

	HANDLE	m_hReadEvent, m_hWriteEvent;

	BOOL	m_bMonitorExit;
	HANDLE	m_hMonitor;

	HANDLE	hWriteMutex;

	int		m_type;
	
public:
	void SetRxEvent(HANDLE hReadEvent, HANDLE hWriteEvent);
	static DWORD WINAPI	ComMonitor(LPVOID lparam);
	void SetState(int BaudRate,int DataBits,int StopBits,int Parity);
	void ClearRevPort(void);
	void WaitWriteComplete();
	bool m_bHasClosed;
};

