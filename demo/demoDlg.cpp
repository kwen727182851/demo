
// demoDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "demo.h"
#include "demoDlg.h"
#include "afxdialogex.h"
#include "asynctrans.h"
#include "file_cipher.h"
#include <math.h>
#include <direct.h>
#include <string>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ASYNC_MODE


typedef struct _bsp_cmd
{
	char* cmd_name;
	unsigned char cmd[20];
	short cmd_len;
}bsp_cmd;




//����ͷ 32 77 43 xx

//�ļ�����������
#define CMD_DOWN_MGR             0x01
#define CMD_DOWN_MGR_END         0x02
#define CMD_DOWN_EXE             0x04
#define CMD_DOWN_EXE_END         0x05  //�ش�������Ͻ��
#define CMD_DOWN_EXE1             0x21
#define CMD_DOWN_EXE_END1         0x22 
#define CMD_DOWN_EXE2             0x23
#define CMD_DOWN_EXE_END2         0x24 

#define CMD_DOWN_JHSH            0x08 //���ؽű�
#define CMD_DOWN_JHSH_END        0x09  //

#define CMD_DOWN_JHLIB           0x10
#define CMD_DOWN_JHLIB_END       0x11
#define CMD_DOWN_TEST            0x12
#define CMD_DOWN_TEST_END        0x13
#define CMD_DOWN_SWDELIB           0x15
#define CMD_DOWN_SWDELIB_END       0x16

#define CMD_DOWN_CRT           0x35
#define CMD_DOWN_CRT_END       0x36
#define CMD_DOWN_ECC_CRT           0x37
#define CMD_DOWN_ECC_CRT_END       0x38
void  ReadProc(unsigned char *data,int length,DWORD userdata)
{
    CdemoDlg* ptestDlg = (CdemoDlg*)userdata;
    ptestDlg->ProcRecv(data, length);
}
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// CdemoDlg �Ի���

CdemoDlg::CdemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CdemoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_connected = 0;
    m_upload = 0;
    m_upload_last_img = 0;
    m_hProcThread = NULL;
	m_bsp_rsp_id = -1;
	m_dsp_updatefile_buffer = NULL;
	m_dsp_updating = 0;
}

void CdemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_BR, m_brateSelect);
	DDX_Control(pDX, IDC_COMBO_PORT, m_comSelect);
	DDX_Control(pDX, IDC_COMBO_OUT, m_csrOut);
}

BEGIN_MESSAGE_MAP(CdemoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_SELCHANGE(IDC_COMBO_PORT, &CdemoDlg::OnCbnSelchangeComboPort)
	ON_CBN_SELCHANGE(IDC_COMBO_BR, &CdemoDlg::OnCbnSelchangeComboBr)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CdemoDlg::OnBnClickedButtonClear)
	ON_EN_CHANGE(IDC_EDIT_VIEW, &CdemoDlg::OnEnChangeEditView)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CdemoDlg::OnButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, &CdemoDlg::OnButtonRefresh)
	ON_BN_CLICKED(IDC_BUTTON_RELOAD_ALL, &CdemoDlg::OnBnClickedButtonReloadAll)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_IN, &CdemoDlg::OnBnClickedButtonIn)
	ON_BN_CLICKED(IDC_BUTTON_OUT, &CdemoDlg::OnBnClickedButtonOut)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_RSA, IDC_RADIO_ECC, OnBnClickedRadio)
END_MESSAGE_MAP()


// CdemoDlg ��Ϣ�������

BOOL CdemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	enum_reg_key_value(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM");
	m_brateSelect.SetCurSel(0);

	((CButton *)GetDlgItem(IDC_RADIO_RSA))->SetCheck(TRUE);//ѡ��
	((CButton *)GetDlgItem(IDC_RADIO_ECC))->SetCheck(FALSE);//��ѡ
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CdemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CdemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CdemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



unsigned char bsp_img_head[61] = {0};
int img_head_len = 61;
int bsp_img_head_len = 0;
int img_recving =  0;
int img_w = 0,img_h = 0,img_total_size = 0;
int recv_bytes = 0, recv_block_size = 0, block_size = 0;
char img_sn[50];

char img_type = 0, s_reserv = 0;

unsigned char bsp_upload[10];

char log_data[1024];
unsigned char img_buffer[1280*720];

static int loopno = 0;


//���ļ������ϴ�
int datafile_recving = 0;
int datafile_size = 0;
unsigned char datafile_type = 0;
unsigned char* datafile_buf = NULL;

int comp(const void* a,const void* b)
{
    return *(int*)a-*(int*)b;
}

bool CdemoDlg::enum_reg_key_value(HKEY hKey, LPCTSTR lpSubKey)
{	
#define MAX_VALUE_NAME 100
    
    HKEY    hOpenKey;	
    long    lResult = 0;	
    CHAR    achClass[MAX_PATH];	
    DWORD   cchClassNameLen;	
    DWORD   cMaxClassLen;	
    DWORD   cSubKeysNum;	
    DWORD   cbMaxSubKeyLen;	
    DWORD   cbValuesNum;	
    DWORD   cbMaxValueNameLen;	
    DWORD   cbMaxValueLen;	
    DWORD   cbSecurityDescriptor;	
    FILETIME ftLastWriteTime;		
    
    CHAR  achValue[MAX_VALUE_NAME];	
    DWORD cbValue = MAX_VALUE_NAME;	
    
    DWORD DataType;	
    BYTE  Data[255];	
    DWORD DataLen;	
    CString StrData = "";
    int comNOs[100];
    int comNOLen = 0;  
    
    lResult = RegOpenKeyEx(hKey, lpSubKey, 0, KEY_READ, &hOpenKey);
    
    if(ERROR_SUCCESS != lResult)		
    {
        return FALSE;		
    }
    
    RegQueryInfoKey(hOpenKey, //�Ѵ򿪵ļ��ľ��		
        achClass, //��������		
        &cchClassNameLen, //�������Ƶĳ���		
        NULL, //����		
        &cSubKeysNum, // �����Ӽ�����Ŀ		
        &cbMaxSubKeyLen, // ��������Ӽ�����		
        &cMaxClassLen, // ��������೤��		
        &cbValuesNum, // ����ֵ����Ŀ		
        &cbMaxValueNameLen, // �������ֵ��,���Ƶĳ���		
        &cbMaxValueLen, // �������ֵ�ĳ���		
        &cbSecurityDescriptor, //���ذ�ȫ����		
        &ftLastWriteTime); // ���ؼ����д���ʱ��
    
    for (DWORD j = 0, retValue = ERROR_SUCCESS; j < cbValuesNum; j++)		
    {
        cbValue = MAX_VALUE_NAME;
        DataLen = 10;
        memset(achValue, 0, sizeof(achValue));		
        memset(Data, 0, sizeof(Data));		
        StrData = L"";			
        retValue = RegEnumValue(hOpenKey, j, achValue, &cbValue, NULL, &DataType, Data, &DataLen);	
        
        if (retValue == (DWORD) ERROR_SUCCESS )			
        { 			
            sscanf((char*)Data,"COM%d",&comNOs[comNOLen++]); 
            //StrData.Format("%s", Data);			
            //m_comSelect.AddString(StrData);  			
        }		
    }	
    RegCloseKey(hOpenKey);	
    
    m_comSelect.ResetContent();
    qsort(comNOs,comNOLen,sizeof(int),comp);
    for (int i = 0; i < comNOLen; i++)
    {
        StrData = L"";
        StrData.Format("COM%d", comNOs[i]);
        m_comSelect.AddString(StrData);
    }
    
    m_comSelect.SetCurSel(0);
	m_csrOut.SetCurSel(0);
    return true;
    
#undef MAX_VALUE_NAME	
}

void CdemoDlg::OnCbnSelchangeComboPort()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
}


void CdemoDlg::OnCbnSelchangeComboBr()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
}


void CdemoDlg::OnBnClickedButtonClear()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CEdit* pEdView = (CEdit*)GetDlgItem(IDC_EDIT_VIEW);
    pEdView->SetWindowText("");
}


void CdemoDlg::OnEnChangeEditView()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}


void CdemoDlg::OnButtonConnect()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	recv_bytes = 0;
	recv_block_size = 0;
	img_total_size = 0;
	img_recving = 0;
	bsp_img_head_len = 0;
	datafile_recving = 0;
	datafile_size  = 0;
	m_upload = 0;
	m_upload_last_img = 0;
    if (m_connected == 0)
    {
        CString wintext;
        //���COM�˿ں�
        m_comSelect.GetWindowText(wintext);
        //char chnum[3] = {0};    
        wintext.MakeUpper();
        wintext.Replace("COM","");
        //comname.Remove("COM");
        int portno = atoi((LPTSTR)(LPCTSTR)wintext);
        
        
        m_brateSelect.GetWindowText(wintext);
        int bandrate = atoi((LPTSTR)(LPCTSTR)wintext);
#ifdef ASYNC_MODE
        int res = OpenPortAsyn(portno,bandrate,ReadProc, (DWORD)this);
#else
        int res = OpenPort(portno,bandrate);
#endif        
        
        
        if (res > 0)
        {
            m_connected = 1;
            SetDlgItemText(IDC_BUTTON_CONNECT, "�Ͽ�");
            GetDlgItem(IDC_BUTTON_REFRESH)->EnableWindow(FALSE);
        }
    }
    else
    {
        ClosePort();
        SetDlgItemText(IDC_BUTTON_CONNECT, "����");
        GetDlgItem(IDC_BUTTON_REFRESH)->EnableWindow(TRUE);
        m_connected = 0;

    }	
}

void CdemoDlg::OnDestroy() 
{

	if (m_dsp_updatefile_buffer)
	{
		free(m_dsp_updatefile_buffer);
		m_dsp_updatefile_buffer = NULL;
		m_dsp_updatefile_buffer_offset = NULL;

	}
	CDialog::OnDestroy();
	
    ClosePort();
}

void CdemoDlg::OnButtonRefresh()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	enum_reg_key_value(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM");

}

void CdemoDlg::binary_to_hex_case(char *dest, const unsigned char *data, int size)
{
    char *p;
    char base_char;
    int i;
    int value;
    
    /** @UNSAFE */
    base_char = 'A';//ucase ? 'A' : 'a';
    
    p = dest;
    for (i = 0; i < size; i++) 
    {
        //         if (data[i] > 31 && data[i] < 127)
        //         {
        //             *p++ = data[i];//ASCII�ɼ��ַ���ת��
        //             continue;
        //         }
        //         *p++ = '<';
        //         *p++ = '0';
        //         *p++ = 'x';
        value = data[i] >> 4;        
        *p++ = value < 10 ? value + '0' : value - 10 + base_char;        
        value = data[i] & 0x0f;
        *p++ = value < 10 ? value + '0' : value - 10 + base_char;
        *p++ = ' ';//֮��ո�
    }
    *p = '\0';
}

void CdemoDlg::ShowDataString(unsigned char* data, int data_len, int tohex, int endline)
{
    if (data_len > 0)
    {

        char timestamp[30];

        SYSTEMTIME currentTime;
        GetSystemTime(&currentTime);                
        sprintf(timestamp, "%02d:%02d:%02d.%03d ", currentTime.wHour,currentTime.wMinute,currentTime.wSecond,
                                currentTime.wMilliseconds);

        char* tmpstring = NULL;
		int tmpstring_len = 0;
		CEdit* pEdView = (CEdit*)GetDlgItem(IDC_EDIT_VIEW);
		int len = 0;
        if (tohex)
        {
            tmpstring = new char[data_len*4];
            binary_to_hex_case(tmpstring, data, data_len);

			len = pEdView->GetWindowTextLength();
			pEdView->SetSel(len, len);
			pEdView->ReplaceSel(timestamp);


			len = pEdView->GetWindowTextLength();
			pEdView->SetSel(len, len);
			pEdView->ReplaceSel("]] ");

			len = pEdView->GetWindowTextLength();
			pEdView->SetSel(len, len);
			pEdView->ReplaceSel(tmpstring);
        }
        else
        {
         
			string str_append;
			for (int i = 0; i < data_len; i++)
			{
				if (data[i] != 0)
				{
					str_append.append(1, data[i]);
				}
				else
				{
					str_append.append("<00>");
				}
			}

			len = pEdView->GetWindowTextLength();
			pEdView->SetSel(len, len);
			pEdView->ReplaceSel(timestamp);


			len = pEdView->GetWindowTextLength();
			pEdView->SetSel(len, len);
			pEdView->ReplaceSel("]] ");

			len = pEdView->GetWindowTextLength();
			pEdView->SetSel(len, len);
			pEdView->ReplaceSel(str_append.c_str());
        } 
        
		if (endline)
		{
			//����
			len = pEdView->GetWindowTextLength();
			pEdView->SetSel(len, len);
			pEdView->ReplaceSel("\r\n");
		}
        
        
        delete[] tmpstring;
    }
}


void CdemoDlg::ShowDataAppend(unsigned char* data, int data_len, int tohex, int endline)
{
	if (data_len > 0)
	{

		char timestamp[30];

		SYSTEMTIME currentTime;
		GetSystemTime(&currentTime);                
		sprintf(timestamp, "%02d:%02d:%02d.%03d ", currentTime.wHour,currentTime.wMinute,currentTime.wSecond,
			currentTime.wMilliseconds);

		char* tmpstring = NULL;
		if (tohex)
		{
			tmpstring = new char[data_len*4];
			binary_to_hex_case(tmpstring, data, data_len);
		}
		else
		{
			tmpstring = new char[data_len + 1];
			memcpy(tmpstring, data, data_len);
			tmpstring[data_len] = 0;
		}       


		CEdit* pEdView = (CEdit*)GetDlgItem(IDC_EDIT_VIEW);
		int len = 0;


		len = pEdView->GetWindowTextLength();
		pEdView->SetSel(len, len);
		pEdView->ReplaceSel(tmpstring);

		if (endline)
		{
			//����
			len = pEdView->GetWindowTextLength();
			pEdView->SetSel(len, len);
			pEdView->ReplaceSel("\r\n");
		}
		delete[] tmpstring;
	}
}

void CdemoDlg::ShowDataSent(unsigned char* data, int data_len)
{
    if (data_len > 0)
    {

        char timestamp[30];
        
        SYSTEMTIME currentTime;
        GetSystemTime(&currentTime);                
        sprintf(timestamp, "%02d:%02d:%02d.%03d ", currentTime.wHour,currentTime.wMinute,currentTime.wSecond,
                                currentTime.wMilliseconds);

        char* tmpstring = NULL;
        //if (tohex)
        {
            tmpstring = new char[data_len*4];
            binary_to_hex_case(tmpstring, data, data_len);
        }
     
        
        
        CEdit* pEdView = (CEdit*)GetDlgItem(IDC_EDIT_VIEW);
        int len = 0;

        len = pEdView->GetWindowTextLength();
        pEdView->SetSel(len, len);
        pEdView->ReplaceSel(timestamp);

        len = pEdView->GetWindowTextLength();
        pEdView->SetSel(len, len);
        pEdView->ReplaceSel("<< ");
        
        len = pEdView->GetWindowTextLength();
        pEdView->SetSel(len, len);
        pEdView->ReplaceSel(tmpstring);
        
        //����
        len = pEdView->GetWindowTextLength();
        pEdView->SetSel(len, len);
        pEdView->ReplaceSel("\r\n");
        
        delete[] tmpstring;
    }
}

//���ͨ���Ƿ�����
int CdemoDlg::jhdev_check_connect()
{
	unsigned char check_cnt_bsp[] = {0x32,0x77,0x43,0x00};
	int retry_count = 0,log_len;
	m_bsp_rsp_id = 0;
	ShowDataSent(check_cnt_bsp,4);
	SendRSData(check_cnt_bsp, 4);

	retry_count = 0;
	while (m_bsp_rsp_id != BSP_RSP_CONNECT_OK && retry_count < 60)
	{
		Sleep(100);
		retry_count++;
	}

	if (m_bsp_rsp_id != BSP_RSP_CONNECT_OK)
	{
		//�ȴ���ʱ����Ϊ���Ӳ�����
		log_len = sprintf(m_log_string, "�����쳣����ȷ�����ڽ�������");
		ShowDataString((unsigned char*)m_log_string, log_len,0);
		return 0;
	}

	log_len = sprintf(m_log_string, "����ͨѶ����");

	ShowDataString((unsigned char*)m_log_string, log_len,0);
	m_bsp_rsp_id = 0;
	return 1;

}
void CdemoDlg::OnBnClickedButtonReloadAll()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (jhdev_check_connect() == 0)
	{
		
		MessageBox("�����쳣����ȷ�����ڽ�������!", "��ʾ");
	}
}


int CdemoDlg::hex_to_binary2(const char *data, unsigned char *dest)
{    
    int hex_v = -1;
    int destLen = 0;
    int value = 0;    
    while (1) 
    {        
        if (*data == ' ' || *data == '\0' || *data == ':')
        {
            if (hex_v >= 0)
            {
                dest[destLen++] = hex_v;
                hex_v = -1;
            }
        }
        else
        {
            if (*data >= '0' && *data <= '9')
                value = (*data - '0');
            else if (*data >= 'a' && *data <= 'f')
                value = (*data - 'a' + 10);
            else if (*data >= 'A' && *data <= 'F')
                value = (*data - 'A' + 10);
            else
                return -1; 
            
            if (hex_v < 0)
            {
                hex_v = value;
            }
            else
            {
                hex_v = ((hex_v << 4 ) | value);
                dest[destLen++] = hex_v;
                hex_v = -1;
            }
        }
        if (*data == '\0')
        {
            break;
        }
        data++;
    }
    
    return destLen;
}


//���� 0 ֹͣ���ͣ���������
int CdemoDlg::SendDSPUpdateLineData()
{
	char linedata[100];
	unsigned char linebin[100];
	int lineLen,linebinlen;
	if (m_dsp_updating == 0 || m_dsp_updatefile_buffer_offset == 0)
	{
		return 0;
	}

	if(sscanf((char*)m_dsp_updatefile_buffer_offset, "%s\n",linedata) > 0)
	{
		if (linedata[0] < 0)
		{
			return 0;
		}
		lineLen = strlen(linedata);
		if (lineLen > 0)
		{
			lineLen += 2;//0d0a
			m_dsp_updatefile_buffer_offset += lineLen;


			linebinlen = hex_to_binary2(linedata, linebin);

			SendRSData(linebin,linebinlen);
			return 1;
		}
		else
		{
			return 0;
		}

	}

	return 0;
}
void CdemoDlg::ProcRecv(unsigned char* data, int data_len)
{
    //�����ϴ�ͼƬЭ��
    if (0 == img_recving && 0 == datafile_recving && 0 == m_dsp_updating)
    {
        int offs = 0;
		if (data_len >= 7 && data[0] == 0x63)//�ϴ������ļ�
		{
			/*
				63 type 00 size[4] 
			*/
			//ShowDataString(data, data_len,1);
#if 1
			recv_bytes = 0;
			block_size = 0;
			recv_block_size = -1;
			datafile_size = 0;

			datafile_type = data[1];
			datafile_size = (data[3] << 24) | (data[4] << 16) | (data[5] << 8) | (data[6]);
						
			block_size = datafile_size/10-2;

			if (datafile_buf)
			{
				delete[] datafile_buf;
				datafile_buf = NULL;
			}

			if (datafile_size == 0)
			{
				int len = sprintf(log_data, "upload datafile type %d size %d", datafile_type,datafile_size);
				ShowDataString((unsigned char*)log_data, len +1,0);
				return;
			}

			datafile_recving = 1;
			datafile_buf = new unsigned char[datafile_size];

			recv_bytes = data_len - 7;
			memcpy(datafile_buf, data + 7, recv_bytes);
			int len = sprintf(log_data, "upload datafile type %d size %d recv_bytes0 %d", datafile_type,datafile_size,recv_bytes);
			ShowDataString((unsigned char*)log_data, len +1,0);
#endif
		}
		else if (data_len == 2 && data[0] == 0x63 && data[1] == 0x00)
		{
			 MessageBox("�������ļ��ϴ�!", "��ʾ");
		}
		else
		{
			if (data_len == 5)
            {
                if (data[0] == 0x33 && data[1] == 0x77 && data[2] == 0x43)
                {
					if (data[3] == CMD_DOWN_MGR && data[4] == 0)
					{
						m_bsp_rsp_id = BSP_RSP_BEGAIN_DOWN_MGR;
					}
					else if (data[3] == CMD_DOWN_EXE && data[4] == 0)
					{
						m_bsp_rsp_id = BSP_RSP_BEGAIN_DOWN_EXE;
					}
					else if (data[3] == CMD_DOWN_EXE1 && data[4] == 0)
					{
						m_bsp_rsp_id = BSP_RSP_BEGAIN_DOWN_EXE;
					}
					else if (data[3] == CMD_DOWN_EXE2 && data[4] == 0)
					{
						m_bsp_rsp_id = BSP_RSP_BEGAIN_DOWN_EXE;
					}
					else if (data[3] == CMD_DOWN_JHSH && data[4] == 0)
					{
						m_bsp_rsp_id = BSP_RSP_BEGAIN_DOWN_SH;
					}
					else if (data[3] == CMD_DOWN_JHLIB && data[4] == 0)
					{
						m_bsp_rsp_id = BSP_RSP_BEGAIN_DOWN_JHLIB;
					}
					else if (data[3] == CMD_DOWN_TEST && data[4] == 0)
					{
						m_bsp_rsp_id = BSP_RSP_BEGAIN_DOWN_TEST;
					}
					else if (data[3] == CMD_DOWN_SWDELIB && data[4] == 0)
					{
						m_bsp_rsp_id = BSP_RSP_BEGAIN_DOWN_SWDELIB;
					}
					else if ((data[3] == CMD_DOWN_CRT && data[4] == 0)||(data[3] == CMD_DOWN_ECC_CRT && data[4] == 0))
					{
						m_bsp_rsp_id = BSP_RSP_BEGAIN_DOWN_CRT;
					}
					else if (data[3] == 0 && data[4] == 0)
					{
						
						m_bsp_rsp_id = BSP_RSP_CONNECT_OK;
						
					}
					if ((data[3] ==CMD_DOWN_CRT_END )||(data[3] ==CMD_DOWN_ECC_CRT_END))
					{
						if (data[4] == 0)
						{
							//���������ɹ�
							int log_len = sprintf(m_log_string, "����֤��ɹ�");
							ShowDataString((unsigned char*)m_log_string, log_len,0);
						}
						else
						{
							//��������ʧ��
							int log_len = sprintf(m_log_string, "����֤��ʧ��");
							ShowDataString((unsigned char*)m_log_string, log_len,0);
						}
					}
                    
                }
            }
		}
    }
	else if (datafile_recving == 1 && datafile_buf)
	{
		memcpy(datafile_buf + recv_bytes, data, data_len);
		recv_bytes += data_len;

		if (recv_block_size < 0)
		{
			recv_block_size =0;
			int len = sprintf(log_data, "");
			ShowDataString((unsigned char*)log_data, len +1,0,0);

		}
		recv_block_size += data_len;


		if (recv_block_size > block_size)
		{
			int len = sprintf(log_data, "*");
			ShowDataAppend((unsigned char*)log_data, len +1,0,0);
			recv_block_size = 0;
		}
	}
	if (datafile_recving == 1 && datafile_buf)
	{
		if (recv_bytes == datafile_size)
		{

			int len = sprintf(log_data, "*");
			ShowDataAppend((unsigned char*)log_data, len +1,0,1);
			len = sprintf(log_data, "recv datafile end");
			ShowDataString((unsigned char*)log_data, len +1,0);

			//datafile_type 1 �ӿڿ�log 3 ϵͳlog
			if (datafile_type == 5 || datafile_type == 6 )//log �ϴ�����
			{
				FILE *file; 
			
				time_t ti;
				struct tm *ptm;
				time(&ti);
				ptm=localtime(&ti);

				if (datafile_type == 5)
				{
					sprintf(log_data,"rsa-client.csr");
				}
				else if(datafile_type == 6)
				{
					sprintf(log_data,"ecc-client.csr");
				}

				file = fopen(log_data, "w");
				if (!file)
				{
					len = sprintf(log_data, "�����ļ�ʧ��");
					ShowDataString((unsigned char*)log_data, len +1,0);
				}
				fwrite(datafile_buf, sizeof(unsigned char), datafile_size, file);
				fclose(file);


				len = sprintf(log_data, "file recved");
				ShowDataString((unsigned char*)log_data, len +1,0);
			}

			datafile_type = 0;
			datafile_recving = 0;
			datafile_size = 0;
			recv_bytes = 0;
			recv_block_size = -1;
			block_size = 0;
			delete[] datafile_buf;
			datafile_buf = NULL;
		}
	}
}


HBRUSH CdemoDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ�����

	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	//���ض��Ŀؼ����޸�
	if (pWnd->GetDlgCtrlID() == IDC_STATIC)

	{
		 pDC->SetTextColor(RGB(0,0,205));

	}
	return hbr;
}

#define MAX_FILE_NAME  30
void CdemoDlg::OnBnClickedButtonIn()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	unsigned char bsp[] = {0x32,0x77,0x43,CMD_DOWN_CRT};

	CString szFilterFDlg = "�ļ� (*.crt)|*.crt||"; 
	CFileDialog cdlg( TRUE,"*.crt",NULL,OFN_HIDEREADONLY,szFilterFDlg,0);
	char filename[MAX_FILE_NAME] = {0};
	if(cdlg.DoModal() !=IDOK)
	{
		return;
	}    

	int FileSize = 0;
	int retVal=0;
	unsigned char* ptr = NULL,*pptr;
	ptr = gen_cipher_file_to_buffer(cdlg.GetPathName().GetBuffer(0), &FileSize);

	if (ptr==NULL)
		retVal=0;
	else
	{
		retVal = 1;
	}

	if(m_CrtType == ECC)
	{
		bsp[3] =CMD_DOWN_ECC_CRT;
	}
	if (retVal)
	{
		int retry_count;
		pptr = ptr;
		m_bsp_rsp_id = 0;
		CString fname = cdlg.GetFileName();
		int flen = fname.GetLength();
		if (flen >= MAX_FILE_NAME)
		{
			sprintf(filename, "%s",fname.Left(MAX_FILE_NAME-1).GetString());
		}
		else
		{
			sprintf(filename, "%s",fname.GetBuffer(0));
		}
		//׷�� �ļ����� filename \0 ����
		memcpy(bsp+4,filename, MAX_FILE_NAME);

		ShowDataSent(bsp,4);
		SendRSData(bsp, 4+MAX_FILE_NAME);


		retry_count = 0;
		while (m_bsp_rsp_id != BSP_RSP_BEGAIN_DOWN_CRT && retry_count < 60)
		{
			Sleep(100);
			retry_count++;
		}

		if (m_bsp_rsp_id == BSP_RSP_BEGAIN_DOWN_CRT)
		{
			m_bsp_rsp_id = 0;
			int tosendsize  = 4*1024;
			int totalsize = 0;
			int log_len = sprintf(m_log_string, "begin download");
			ShowDataString((unsigned char*)m_log_string, log_len,0);

			while(FileSize)
			{
				if(FileSize < tosendsize)
				{
					tosendsize = FileSize;
				}

				int ret = SendRSData(pptr, tosendsize);
				Sleep(10);
				if (ret < 0)
				{
					retVal = 0;
					break;

				}
				totalsize += ret;
				pptr += ret;
				FileSize -= tosendsize;
			} 

			log_len = sprintf(m_log_string, "download %d bytes end ", totalsize);
			ShowDataString((unsigned char*)m_log_string, log_len,0);

			//������϶Ͽ�����
			//OnButtonConnect();
		}

		//printf("total upload %d bytes\n", totalsize);
	}
	if (ptr)
	{
		free(ptr);
	}
}


void CdemoDlg::OnBnClickedButtonOut()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	unsigned char bsp[] = {0x62,0x05,0x00};//�ϴ���־
	int csridx = m_csrOut.GetCurSel();

	if (0 == csridx)
	{
		//�ӿ���־
		bsp[1] = 0x05;
	}
	else if (1==csridx)
	{
		//rsa-client.csr
		bsp[1] = 0x06;
	}
	else
	{
		return;
	}
	ShowDataSent(bsp,3);
	SendRSData(bsp, 3);
}

void CdemoDlg::OnBnClickedRadio(UINT idCtl)
{
    if(idCtl == IDC_RADIO_RSA)
        m_CrtType = RSA;
    if(idCtl == IDC_RADIO_ECC)
        m_CrtType = ECC;
}

