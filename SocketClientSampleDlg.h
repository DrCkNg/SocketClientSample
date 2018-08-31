/*************************************************************************
// SocketClientSampleDlg.h
//
// License
// -------
// This code is provided "as is" with no expressed or implied warranty.
// 
// You may use this code in a commercial product with or without acknowledgment.
// However you may not sell this code or any modification of this code, this includes 
// commercial libraries and anything else for profit.
//
// I would appreciate a notification of any bugs or bug fixes to help the control grow.
//
// History:
// --------
//	See License.txt for full history information.
//
//
// Copyright (c) 2018
// DrckNg@mail.com
**************************************************************************/

// SocketClientSampleDlg.h : header file
//

#pragma once
#include <afxmt.h>
#include "CClientSocket.h"
#include "CheckListCtrl.h"

UINT ThreadWorker(LPVOID lpvParam);
UINT ThreadConnectionStatus(LPVOID lpvParam);


// CSocketClientSampleDlg dialog
class CSocketClientSampleDlg : public CDialogEx
{
// Construction
public:
	CSocketClientSampleDlg(CWnd* pParent = NULL);	// standard constructor
	~CSocketClientSampleDlg();

// Dialog Data
	enum { IDD = IDD_SOCKETCLIENTSAMPLE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
public:
	enum{	STATUS_INIT=0		, 
		STATUS_CONNECTING	,
		STATUS_CONNECTED	,
		STATUS_CONNECT_ERROR,
		STATUS_SETTING_ERROR,
		STATUS_DISCONNECTING,
		STATUS_DISCONNECTED	,
		STATUS_DRYRUN_RUNNING,
		STATUS_DRYRUN_STOPPING	,
		STATUS_DRYRUN_STOPPED	,
		STATUS_DISABLED		,
		STATUS_EXIT			,
		STATUS_MODE_ALL};

private:
	// Socket
	CClientSocket*		m_pClientSocket;

	// DeviceList 
	int						m_nTotalNumDevice			;
	std::vector<BOOL	>	m_arrbEnableDevice			;
	std::vector<CString	>	m_arrstrIpAddress			;
	std::vector<int		>	m_arrnPort					;
	std::vector<CString	>	m_arrstrTriggerCmd			;	//Trigger command per device, in C format
	std::vector<CString	>	m_arrstrDeviceNameLabel		;	//Device name for display

	std::vector<CString	>	m_arrstrReceiveDataDisplay	;	//Display shadow
	std::vector<int		>	m_arrnDeviceStatus			;	//Display shadow
	std::vector<CString	>	m_arrstrStatus				;	//Display shadow
	std::vector<int		>	m_arrnGoodCounter			;	//Display shadow
	std::vector<int		>	m_arrnBadCounter			;	//Display shadow
	std::vector<BOOL	>	m_arrbNeedUpdate			;	//Display shadow

	// Status label
	std::vector<CString	>	m_strStatusLabel			;		//0:init 1:Connecting 2:Connected 3:ConnectionError 4:SettingError 5:Disconnecting 6:Disconnected


	// Worker Thread
	int						m_nIdCurThreadCount;							//Common
	CCriticalSection		m_csGetThreadId;								//Common
	std::vector<CEvent*	>	m_arrpEventThreadActionTrigger;					//Common
	BOOL					m_bUserInitKillThread;							//Common
	std::vector<BOOL>		m_arrbThreadExited;								//Common

	std::vector<CWinThread*> m_arrhThreadWorker;							// For Connection 
	BOOL					m_bUserInitConnection;							// For Connection 
	BOOL					m_bUserInitDisconnection;						// For Connection 
	std::vector<BOOL	>	m_arrbDryRunTestEnable;							// For DryRun
	BOOL					m_bUserInitRunDryRunLoop;						// For DryRun
	BOOL					m_bUserInitQuitDryRunLoop;						// For DryRun
	int						m_nTimeInterval;								// For WorkerThread DryRun Loop
	// Status Thread
	CWinThread*				m_hThreadConnectionStatus;

	// UI Buttons
	BOOL					m_bModifyingSetting;
	CCheckListCtrl			m_listCtrlDevices;
	CComboBox				m_ComboTestIp;

protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedConnect();
	afx_msg void OnBnClickedDisconnect();
	afx_msg void OnBnClickedResetDeviceListData();
	afx_msg void OnBnClickedButtonEditSetting();
	afx_msg void OnBnClickedButtonApplySetting();
	afx_msg void OnBnClickedButtonLoadSetting();
	afx_msg void OnBnClickedButtonSaveSetting();
	afx_msg void OnBnClickedSingleTrigger();
	afx_msg void OnBnClickedDryRunSingleDevice();
	afx_msg void OnBnClickedDryRunAllDevices();
	afx_msg void OnBnClickedStopRunDryRun();
	afx_msg void OnBnClickedExit();
	afx_msg LRESULT DeviceListUpdateItemStatus(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT SetDeviceListItemToEditGroup(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT ButtonUpdateConnectedStatus(WPARAM wParam,LPARAM lParam);

	void InitDeviceListColumns();
	void UpdateDeviceListAllRowsColsFromMember();
	void UpdateDeviceListStatus(int nDeviceId);
	void UpdateDeviceListRowDataFromMember(int nRow);
	void UpdateDeviceComboBoxFromMember();
	void SetDeviceSettingUIEditMode(BOOL bCanEdit);
	void ReadSettings();
	void WriteSettings();
	void SendTriggerCmd(int nId,BOOL bQuietMode);
	void SetDeviceListItemToEditGroup(int nItemId);

	int  GetUnitqueDeviceIdForThread();
	void QuitAllThreads();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	friend UINT ThreadWorker(LPVOID lpvParam);
	friend UINT ThreadConnectionStatus(LPVOID lpvParam);

};
