
// demoDlg.h : 头文件
//
#include "afxwin.h"
#include "afxcmn.h"
#pragma once

#define BSP_RSP_BEGAIN_DOWN_MGR       0x20
#define BSP_RSP_BEGAIN_DOWN_EXE       0x21
#define BSP_RSP_BEGAIN_DOWN_SH        0x22
#define BSP_RSP_BEGAIN_DOWN_JHLIB        0x23
#define BSP_RSP_BEGAIN_DOWN_TEST        0x24
#define BSP_RSP_BEGAIN_DOWN_SWDELIB		0x25
#define BSP_RSP_BEGAIN_DOWN_CRT			0x26

#define BSP_RSP_CONNECT_OK			0x30
#define BSP_RSP_TIME_SNYC_OK			0x31
#define BSP_RSP_TIME_SNYC_ERR			0x32

// CdemoDlg 对话框
class CdemoDlg : public CDialogEx
{
// 构造
public:
	CdemoDlg(CWnd* pParent = NULL);	// 标准构造函数

	void ProcRecv(unsigned char* data, int data_len);
	void ShowDataString(unsigned char* data, int data_len, int tohex, int endline = 1);
	void ShowDataAppend(unsigned char* data, int data_len, int tohex, int endline);
	void ShowDataSent(unsigned char* data, int data_len);
// 对话框数据
	enum { IDD = IDD_DEMO_DIALOG };

	CComboBox	m_brateSelect;
	CComboBox	m_comSelect;
	CComboBox	m_csrOut;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	int m_connected;
    int m_upload;
    int m_upload_last_img;
	int m_stop_continue_upload;
    unsigned char m_bufSend[1024];
    unsigned char m_bufRecv[1024];

	int jhdev_check_connect();
    int m_bsp_rsp_id;
    char m_log_string[1024];
	enum {RSA, ECC} m_CrtType;

	HANDLE m_hProcThread;//线程句柄
    DWORD m_dwProcThreadID;//线程ID

	//DSP升级固件
	int m_dsp_updating;
	unsigned char* m_dsp_updatefile_buffer;
	unsigned char* m_dsp_updatefile_buffer_offset;

	int hex_to_binary2(const char *data, unsigned char *dest);
    void binary_to_hex_case(char *dest, const unsigned char *data, int size);
	bool enum_reg_key_value(HKEY hKey, LPCTSTR lpSubKey);

	int SendDSPUpdateLineData();

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonConnect();
	afx_msg void OnDestroy();
	afx_msg void OnButtonRefresh();
	afx_msg void OnBnClickedRadio(UINT idCtl);
	DECLARE_MESSAGE_MAP()
public:

	afx_msg void OnCbnSelchangeComboPort();
	afx_msg void OnCbnSelchangeComboBr();
	afx_msg void OnBnClickedButtonClear();
	afx_msg void OnEnChangeEditView();
	afx_msg void OnBnClickedButtonReloadAll();
	
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedButtonIn();
	afx_msg void OnBnClickedButtonOut();
	afx_msg void OnBnClickedRadioRsa();
};
