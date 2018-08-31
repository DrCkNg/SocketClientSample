/*************************************************************************
// SocketClientSampleDlg.cpp
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

// SocketClientSampleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SocketClientSample.h"
#include "SocketClientSampleDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
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

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSocketClientSampleDlg dialog




CSocketClientSampleDlg::CSocketClientSampleDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSocketClientSampleDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pClientSocket = NULL;

	m_strStatusLabel.resize(STATUS_MODE_ALL);
	m_strStatusLabel[STATUS_INIT			].Format(_T("Initial Mode"));
	m_strStatusLabel[STATUS_CONNECTING		].Format(_T("Connecting"));
	m_strStatusLabel[STATUS_CONNECTED		].Format(_T("Connected"));
	m_strStatusLabel[STATUS_CONNECT_ERROR	].Format(_T("Connection Error"));
	m_strStatusLabel[STATUS_SETTING_ERROR	].Format(_T("Setting Error"));
	m_strStatusLabel[STATUS_DISCONNECTING	].Format(_T("Disconnecting"));
	m_strStatusLabel[STATUS_DISCONNECTED	].Format(_T("Disconnected"));
	m_strStatusLabel[STATUS_DRYRUN_RUNNING	].Format(_T("Dry Run Mode"));
	m_strStatusLabel[STATUS_DRYRUN_STOPPING	].Format(_T("Dry Run Stopping"));
	m_strStatusLabel[STATUS_DRYRUN_STOPPED	].Format(_T("Dry Run Stopped"));
	m_strStatusLabel[STATUS_DISABLED		].Format(_T("Disabled"));
	m_strStatusLabel[STATUS_EXIT			].Format(_T("Closing Application"));

	ReadSettings();
	WriteSettings();

	for(int nDeviceId=0; nDeviceId<m_nTotalNumDevice; nDeviceId++)
	{
		m_arrstrReceiveDataDisplay[nDeviceId].Empty();
		m_arrnDeviceStatus[nDeviceId] = STATUS_INIT;
		m_arrnGoodCounter[nDeviceId] = 0;
		m_arrnBadCounter[nDeviceId] = 0;
		m_arrbNeedUpdate[nDeviceId] = FALSE;
		m_arrpEventThreadActionTrigger[nDeviceId] = NULL;
		m_arrbThreadExited[nDeviceId] = FALSE;
		m_arrhThreadWorker[nDeviceId] = NULL;
		m_arrbDryRunTestEnable[nDeviceId] = FALSE;
	}
}

CSocketClientSampleDlg::~CSocketClientSampleDlg()
{
	m_bUserInitKillThread = TRUE;
	for(int nDeviceId=0; nDeviceId<m_nTotalNumDevice; nDeviceId++)
		m_arrpEventThreadActionTrigger[nDeviceId]->SetEvent();

	QuitAllThreads();

	if(m_pClientSocket)
	{
		delete m_pClientSocket;
		m_pClientSocket = NULL;
	}

	if(m_arrpEventThreadActionTrigger.size()>0)
	{
		for(int i=0; i<m_arrpEventThreadActionTrigger.size();i++)
		{
			delete m_arrpEventThreadActionTrigger[i];
			m_arrpEventThreadActionTrigger[i] = NULL;
		}
	}
}

void CSocketClientSampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DEVICES, m_listCtrlDevices);
	DDX_Control(pDX, IDC_COMBO_TEST_IP, m_ComboTestIp);
}

BEGIN_MESSAGE_MAP(CSocketClientSampleDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CONNECT, &CSocketClientSampleDlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_DISCONNECT, &CSocketClientSampleDlg::OnBnClickedDisconnect)
	ON_BN_CLICKED(IDC_RESET, &CSocketClientSampleDlg::OnBnClickedResetDeviceListData)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_SETTING, &CSocketClientSampleDlg::OnBnClickedButtonEditSetting)
	ON_BN_CLICKED(IDC_BUTTON_APPLY_SETTING, &CSocketClientSampleDlg::OnBnClickedButtonApplySetting)
	ON_BN_CLICKED(IDC_BUTTON_LOAD, &CSocketClientSampleDlg::OnBnClickedButtonLoadSetting)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CSocketClientSampleDlg::OnBnClickedButtonSaveSetting)
	ON_BN_CLICKED(IDC_SINGLE_SCAN, &CSocketClientSampleDlg::OnBnClickedSingleTrigger)
	ON_BN_CLICKED(IDC_DRY_RUN_SINGLE, &CSocketClientSampleDlg::OnBnClickedDryRunSingleDevice)
	ON_BN_CLICKED(IDC_DRY_RUN_ALL, &CSocketClientSampleDlg::OnBnClickedDryRunAllDevices)
	ON_BN_CLICKED(IDC_STOP_RUN, &CSocketClientSampleDlg::OnBnClickedStopRunDryRun)
	ON_BN_CLICKED(IDOK, &CSocketClientSampleDlg::OnBnClickedExit)
	ON_MESSAGE(WM_SELECT_DEVICE_LISTITEM, &CSocketClientSampleDlg::SetDeviceListItemToEditGroup)
	ON_MESSAGE(WM_UPDATE_LIST_STATUS, &CSocketClientSampleDlg::DeviceListUpdateItemStatus)
	ON_MESSAGE(WM_UPDATE_BUTTON_CONNECTED_STATUS, &CSocketClientSampleDlg::ButtonUpdateConnectedStatus)
END_MESSAGE_MAP()


// CSocketClientSampleDlg message handlers

BOOL CSocketClientSampleDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CString strTemp;

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_hThreadConnectionStatus = NULL;
	m_bUserInitKillThread = FALSE;
	m_bUserInitConnection = FALSE;
	m_bUserInitDisconnection = FALSE;
	m_bUserInitRunDryRunLoop = FALSE;
	m_bUserInitQuitDryRunLoop = FALSE;
	m_nTimeInterval = 200;

	// Initialise Socket
	if(m_nTotalNumDevice)
		m_pClientSocket = new CClientSocket(m_nTotalNumDevice);
	else
	{
		AfxMessageBox(_T("Total Number of Device = 0. Please setup before running this App."));
		return FALSE;
	}

	// Initialise Threads
	m_nIdCurThreadCount = 0;
	for(int nDeviceId=0; nDeviceId<m_nTotalNumDevice; nDeviceId++)
	{
		strTemp.Format(_T("Connection%d"),nDeviceId);
		m_arrpEventThreadActionTrigger[nDeviceId] = new CEvent(FALSE, FALSE, strTemp);
	}

	for(int nDeviceId=0; nDeviceId<m_nTotalNumDevice; nDeviceId++)
		m_arrhThreadWorker[nDeviceId] = NULL;

	if(m_nIdCurThreadCount== 0)
	{
		for(int nDeviceId=0; nDeviceId<m_nTotalNumDevice; nDeviceId++)
		{
			CWinThread* hThread = AfxBeginThread(ThreadWorker,this,THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED);
			if ( NULL != hThread)
			{
				m_arrhThreadWorker[nDeviceId] = hThread;
				m_arrhThreadWorker[nDeviceId]->m_bAutoDelete = FALSE;
				m_arrhThreadWorker[nDeviceId]->ResumeThread();
			}
			else
				AfxMessageBox(_T("Unable to start ConnectSingleIpPort Thread."));
		}
	}

	CWinThread* hThread = AfxBeginThread(ThreadConnectionStatus,this,THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED);
	if ( NULL != hThread)
	{
		m_hThreadConnectionStatus = hThread;
		m_hThreadConnectionStatus->m_bAutoDelete = FALSE;
		m_hThreadConnectionStatus->ResumeThread();
	}
	else
		AfxMessageBox(_T("Unable to start ConnectionStatus Thread."));

	// Device List section
	InitDeviceListColumns();
	UpdateDeviceListAllRowsColsFromMember();
	m_listCtrlDevices.SetParent(this);
	m_listCtrlDevices.EnableWindow(FALSE);

	ButtonUpdateConnectedStatus(FALSE,FALSE);
	SetDeviceSettingUIEditMode(FALSE);

	UpdateDeviceComboBoxFromMember();
	m_ComboTestIp.SetCurSel(0);

	UpdateData(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSocketClientSampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSocketClientSampleDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSocketClientSampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSocketClientSampleDlg::OnBnClickedConnect()
{
	m_bUserInitConnection = TRUE;
	m_bUserInitDisconnection = FALSE;

	CString strCmd;
	for(int nDeviceId=0; nDeviceId<m_nTotalNumDevice; nDeviceId++)
	{
		m_pClientSocket->SetSocketData(nDeviceId, m_arrstrIpAddress[nDeviceId], m_arrnPort[nDeviceId]);
		strCmd.Format(_T("%s\r\n"),m_arrstrTriggerCmd[nDeviceId]);
		m_pClientSocket->SetTriggerCmd(nDeviceId, strCmd);
	}
	m_pClientSocket->Initialise();

	for(int nDeviceId=0; nDeviceId<m_nTotalNumDevice; nDeviceId++)
	{
		m_arrnDeviceStatus[nDeviceId] = STATUS_CONNECTING;
		m_arrstrStatus[nDeviceId] = m_strStatusLabel[m_arrnDeviceStatus[nDeviceId]];
	}

	for(int nDeviceId=0; nDeviceId<m_nTotalNumDevice; nDeviceId++)
	{
		m_arrpEventThreadActionTrigger[nDeviceId]->SetEvent();
		Sleep(100);
	}

	GetDlgItem(IDC_BUTTON_EDIT_SETTING)	->EnableWindow(FALSE);
	GetDlgItem(IDC_CONNECT)->EnableWindow(FALSE);		// let user interrupt
	GetDlgItem(IDC_DISCONNECT)->EnableWindow(TRUE);		// let user interrupt
}

void CSocketClientSampleDlg::OnBnClickedDisconnect()
{
	m_bUserInitConnection = FALSE;
	m_bUserInitDisconnection = TRUE;
	m_bUserInitQuitDryRunLoop = TRUE;

	m_pClientSocket->Disconnect();
	GetDlgItem(IDC_DISCONNECT)->EnableWindow(FALSE);		// do not let user interrupt
}

void CSocketClientSampleDlg::OnBnClickedResetDeviceListData()
{
	// TODO: Add your control notification handler code here
	for(int nDeviceId=0; nDeviceId<m_nTotalNumDevice; nDeviceId++)
	{
		m_arrnGoodCounter[nDeviceId] = 0;
		m_arrnBadCounter[nDeviceId] = 0;
		m_arrstrReceiveDataDisplay[nDeviceId].Empty();
		m_arrstrStatus[nDeviceId].Empty();
		UpdateDeviceListRowDataFromMember(nDeviceId);
	}
	UpdateData(FALSE);
}

void CSocketClientSampleDlg::OnBnClickedButtonEditSetting()
{
	CString strTemp;

	m_listCtrlDevices.EnableWindow(m_bModifyingSetting?TRUE:FALSE);
	//Modify Settings section
	SetDeviceSettingUIEditMode(m_bModifyingSetting);
	GetDlgItem(IDC_CONNECT)->EnableWindow(m_bModifyingSetting?FALSE:TRUE);

	if(m_nTotalNumDevice>0)
	{
		m_listCtrlDevices.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
		m_listCtrlDevices.SetSelectionMark(0);
		GetDlgItem(IDC_IP_ADDRESS_EDIT)->SetWindowTextW(m_arrstrIpAddress[0]);
		strTemp.Format(_T("%d"),m_arrnPort[0]);
		GetDlgItem(IDC_PORT_EDIT)->SetWindowTextW(strTemp);
		GetDlgItem(IDC_EDIT_SCANNERID)->SetWindowTextW(_T("1"));
	}
	if(!m_bModifyingSetting)
	{
		UpdateDeviceComboBoxFromMember();
	}
	m_bModifyingSetting = !m_bModifyingSetting;

	UpdateData(FALSE);
}

void CSocketClientSampleDlg::OnBnClickedButtonApplySetting()
{
	UpdateData(TRUE);

	int nPreviousSelectedRow = m_listCtrlDevices.GetSelectionMark(); 
	if(nPreviousSelectedRow>=0)
	{
		CString strTemp;
		((CEdit*)(GetDlgItem(IDC_IP_ADDRESS_EDIT)))->GetWindowTextW(strTemp);
		m_arrstrIpAddress[nPreviousSelectedRow] = strTemp;
		((CEdit*)(GetDlgItem(IDC_PORT_EDIT)))->GetWindowTextW(strTemp);
		m_arrnPort[nPreviousSelectedRow] = _ttoi(strTemp);

		strTemp.Format(_T("%d %s [%s:%d]"),nPreviousSelectedRow+1,m_arrstrDeviceNameLabel[nPreviousSelectedRow],m_arrstrIpAddress[nPreviousSelectedRow],m_arrnPort[nPreviousSelectedRow]);
		m_listCtrlDevices.SetItemText(nPreviousSelectedRow,0,strTemp);

		for(int nDeviceId=0; nDeviceId<m_nTotalNumDevice; nDeviceId++)
			m_arrbEnableDevice[nDeviceId] = m_listCtrlDevices.GetCheck(nDeviceId);
		strTemp.Empty();
	}
}

void CSocketClientSampleDlg::OnBnClickedButtonLoadSetting()
{
	int nSelect = 0;
	POSITION PosCurrent = m_listCtrlDevices.GetFirstSelectedItemPosition(); 
	ReadSettings();
	UpdateDeviceListAllRowsColsFromMember();

	if(PosCurrent>NULL)
		nSelect = int(PosCurrent) - 1;
	SetDeviceListItemToEditGroup(nSelect);
	m_listCtrlDevices.SetItemState(nSelect, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
	m_listCtrlDevices.SetSelectionMark(nSelect);

}

void CSocketClientSampleDlg::OnBnClickedButtonSaveSetting()
{
	WriteSettings();
}

void CSocketClientSampleDlg::OnBnClickedSingleTrigger()
{
	int nSelect = m_ComboTestIp.GetCurSel();
	if(nSelect>=0 && nSelect<m_nTotalNumDevice)
	{
		for(int nDeviceId=0; nDeviceId<m_nTotalNumDevice; nDeviceId++)
			m_arrbDryRunTestEnable[nDeviceId] = FALSE;	//reset the rest

		if(m_arrnDeviceStatus[nSelect]==STATUS_INIT || m_arrnDeviceStatus[nSelect]==STATUS_DISCONNECTED) 
			OnBnClickedConnect(); 

		SendTriggerCmd(nSelect,TRUE);
	}
}

void CSocketClientSampleDlg::OnBnClickedDryRunSingleDevice()
{
	int nSelect = m_ComboTestIp.GetCurSel();
	for(int nDeviceId=0; nDeviceId<m_nTotalNumDevice; nDeviceId++)
		m_arrbDryRunTestEnable[nDeviceId] = FALSE;	//reset the rest
	m_arrbDryRunTestEnable[nSelect] = TRUE;	

	m_bUserInitQuitDryRunLoop = FALSE;
	m_bUserInitRunDryRunLoop = TRUE;
	GetDlgItem(IDC_DRY_RUN_SINGLE)->EnableWindow(FALSE);
	GetDlgItem(IDC_DRY_RUN_ALL)->EnableWindow(FALSE);
	GetDlgItem(IDC_STOP_RUN)->EnableWindow(TRUE);
	for(int nDeviceId=0; nDeviceId<m_nTotalNumDevice; nDeviceId++)
		m_arrpEventThreadActionTrigger[nDeviceId]->SetEvent();
}

void CSocketClientSampleDlg::OnBnClickedDryRunAllDevices()
{
	int nSelect = m_ComboTestIp.GetCurSel();
	for(int nDeviceId=0; nDeviceId<m_nTotalNumDevice; nDeviceId++)
		m_arrbDryRunTestEnable[nDeviceId] = TRUE;	//reset the rest

	m_bUserInitQuitDryRunLoop = FALSE;
	m_bUserInitRunDryRunLoop = TRUE;
	GetDlgItem(IDC_DRY_RUN_SINGLE)->EnableWindow(FALSE);
	GetDlgItem(IDC_DRY_RUN_ALL)->EnableWindow(FALSE);
	GetDlgItem(IDC_STOP_RUN)->EnableWindow(TRUE);

	for(int nDeviceId=0; nDeviceId<m_nTotalNumDevice; nDeviceId++)
		m_arrpEventThreadActionTrigger[nDeviceId]->SetEvent();
}

void CSocketClientSampleDlg::OnBnClickedStopRunDryRun()
{
	m_bUserInitQuitDryRunLoop = TRUE;
	GetDlgItem(IDC_DRY_RUN_SINGLE)->EnableWindow(TRUE);
	GetDlgItem(IDC_DRY_RUN_ALL)->EnableWindow(TRUE);
}

void CSocketClientSampleDlg::OnBnClickedExit()
{	
	if(m_pClientSocket)
		m_pClientSocket->Disconnect();

	m_bUserInitQuitDryRunLoop = TRUE;
	m_bUserInitKillThread = TRUE;

	for(int nDeviceId=0; nDeviceId<m_nTotalNumDevice; nDeviceId++)
		m_arrpEventThreadActionTrigger[nDeviceId]->SetEvent();

	BOOL bAllThreadExited = TRUE;
	for(int nTry = 0; nTry < 5; nTry++)
	{
		for(int nDeviceId=0; nDeviceId<m_nTotalNumDevice; nDeviceId++)
		{
			if(!m_arrbThreadExited[nDeviceId])
				bAllThreadExited &= FALSE;
		}
		if(bAllThreadExited)
			break;
		else
			Sleep(200);
	}

	QuitAllThreads();
	CDialog::OnOK();
}

LRESULT CSocketClientSampleDlg::DeviceListUpdateItemStatus(WPARAM wParam,LPARAM lParam)
{
	int nDeviceId = int(wParam);
	CString strTemp;
	if(nDeviceId==-1)
	{
		for(int nRow=0; nRow<m_nTotalNumDevice; nRow++)
		{
			strTemp.Format(_T("%s"),m_arrstrReceiveDataDisplay[nRow]);
			m_listCtrlDevices.SetItemText(nRow,1,strTemp);
			strTemp.Format(_T("%d"),m_arrnGoodCounter[nRow]);
			m_listCtrlDevices.SetItemText(nRow,2,strTemp);
			strTemp.Format(_T("%d"),m_arrnBadCounter[nRow]);
			m_listCtrlDevices.SetItemText(nRow,3,strTemp);
			strTemp.Format(_T("%s"),m_arrstrStatus[nRow]);
			m_listCtrlDevices.SetItemText(nRow,4,strTemp);
			m_listCtrlDevices.SetCheck(nRow,m_arrbEnableDevice[nRow]);
		}
	}
	else
	{
		strTemp.Format(_T("%s"),m_arrstrReceiveDataDisplay[nDeviceId]);
		m_listCtrlDevices.SetItemText(nDeviceId,1,strTemp);
		strTemp.Format(_T("%d"),m_arrnGoodCounter[nDeviceId]);
		m_listCtrlDevices.SetItemText(nDeviceId,2,strTemp);
		strTemp.Format(_T("%d"),m_arrnBadCounter[nDeviceId]);
		m_listCtrlDevices.SetItemText(nDeviceId,3,strTemp);
		strTemp.Format(_T("%s"),m_arrstrStatus[nDeviceId]);
		m_listCtrlDevices.SetItemText(nDeviceId,4,strTemp);
		m_listCtrlDevices.SetCheck(nDeviceId,m_arrbEnableDevice[nDeviceId]);
	}
	return 0;
}

LRESULT CSocketClientSampleDlg::SetDeviceListItemToEditGroup(WPARAM wParam,LPARAM lParam)
{
	int nSelect = 0;
	POSITION pos = m_listCtrlDevices.GetFirstSelectedItemPosition(); 

	if (pos > NULL) 
		nSelect = int(pos-1);
	SetDeviceListItemToEditGroup(nSelect);
	return 0;
}

LRESULT CSocketClientSampleDlg::ButtonUpdateConnectedStatus(WPARAM wParam,LPARAM lParam)
{
	BOOL bConnectedMode = BOOL(wParam);
	//Testing section
	GetDlgItem(IDC_CONNECT)				->EnableWindow(bConnectedMode?FALSE:TRUE);
	GetDlgItem(IDC_DISCONNECT)			->EnableWindow(bConnectedMode?TRUE:FALSE);
	GetDlgItem(IDC_BUTTON_EDIT_SETTING)	->EnableWindow(bConnectedMode?FALSE:TRUE);
	GetDlgItem(IDC_COMBO_TEST_IP)		->EnableWindow(bConnectedMode?TRUE:FALSE);
	GetDlgItem(IDC_SINGLE_SCAN)			->EnableWindow(bConnectedMode?TRUE:FALSE);
	GetDlgItem(IDC_DRY_RUN_SINGLE)		->EnableWindow(bConnectedMode?TRUE:FALSE);
	GetDlgItem(IDC_DRY_RUN_ALL)			->EnableWindow(bConnectedMode?TRUE:FALSE);
	GetDlgItem(IDC_STOP_RUN)			->EnableWindow(bConnectedMode?TRUE:FALSE);
	return 0;
}

void CSocketClientSampleDlg::InitDeviceListColumns()
{
	int nColId = 0;
	int nColWidth = 0, nColWidthTotal = 0;
	CRect rect;
	m_listCtrlDevices.GetClientRect(rect);
	m_listCtrlDevices.InsertColumn(nColId++, _T("Device"),		LVCFMT_LEFT, nColWidth=220);	nColWidthTotal+=nColWidth;
	m_listCtrlDevices.InsertColumn(nColId++, _T("Device Data"),	LVCFMT_LEFT, nColWidth=120);	nColWidthTotal+=nColWidth;
	m_listCtrlDevices.InsertColumn(nColId++, _T("Good Cnt"),		LVCFMT_LEFT, nColWidth=40);		nColWidthTotal+=nColWidth;
	m_listCtrlDevices.InsertColumn(nColId++, _T("Bad Cnt"),		LVCFMT_LEFT, nColWidth=40);		nColWidthTotal+=nColWidth;
	m_listCtrlDevices.InsertColumn(nColId++, _T("Status"),			LVCFMT_LEFT, rect.Width() - nColWidthTotal);
	m_listCtrlDevices.SetExtendedStyle( m_listCtrlDevices.GetExtendedStyle() | LVS_EX_CHECKBOXES);
}

void CSocketClientSampleDlg::UpdateDeviceListAllRowsColsFromMember()
{
	m_listCtrlDevices.DeleteAllItems();
	CString strDeviceLine;
	for(int nDeviceId=0; nDeviceId<m_nTotalNumDevice; nDeviceId++)
	{
		strDeviceLine.Format(_T("%d. %s [%s:%d]"),nDeviceId+1,m_arrstrDeviceNameLabel[nDeviceId],m_arrstrIpAddress[nDeviceId],m_arrnPort[nDeviceId]);
		m_listCtrlDevices.InsertItem(nDeviceId, strDeviceLine);

		UpdateDeviceListRowDataFromMember(nDeviceId);
	}
}

void CSocketClientSampleDlg::UpdateDeviceListStatus(int nDeviceId)
{
	if(nDeviceId<0)
	{
		for(int nRow=0; nRow<m_nTotalNumDevice; nRow++)
		{
			m_arrstrStatus[nRow] = m_strStatusLabel[m_arrnDeviceStatus[nRow]];
			m_arrbNeedUpdate[nRow] = TRUE;
		}
	}
	else
	{
		m_arrstrStatus[nDeviceId] = m_strStatusLabel[m_arrnDeviceStatus[nDeviceId]];
		m_arrbNeedUpdate[nDeviceId] = TRUE;
	}
}

void CSocketClientSampleDlg::UpdateDeviceListRowDataFromMember(int nDeviceId)
{
	CString strTemp;
	strTemp.Format(_T("%s"),m_arrstrReceiveDataDisplay[nDeviceId]);
	m_listCtrlDevices.SetItemText(nDeviceId,1,strTemp);
	strTemp.Format(_T("%d"),m_arrnGoodCounter[nDeviceId]);
	m_listCtrlDevices.SetItemText(nDeviceId,2,strTemp);
	strTemp.Format(_T("%d"),m_arrnBadCounter[nDeviceId]);
	m_listCtrlDevices.SetItemText(nDeviceId,3,strTemp);
	strTemp.Format(_T("%s"),m_arrstrStatus[nDeviceId]);
	m_listCtrlDevices.SetItemText(nDeviceId,4,strTemp);
	m_listCtrlDevices.SetCheck(nDeviceId,m_arrbEnableDevice[nDeviceId]);
}

void CSocketClientSampleDlg::UpdateDeviceComboBoxFromMember()
{
	m_ComboTestIp.ResetContent();
	CString strDeviceLine;
	for(int nDeviceId=0; nDeviceId<m_nTotalNumDevice; nDeviceId++)
	{
		strDeviceLine.Format(_T("%d %s [%s:%d]"),nDeviceId+1,m_arrstrDeviceNameLabel[nDeviceId],m_arrstrIpAddress[nDeviceId],m_arrnPort[nDeviceId]);
		m_ComboTestIp.AddString(strDeviceLine);
	}
	m_ComboTestIp.SetCurSel(0);
}

void CSocketClientSampleDlg::SetDeviceSettingUIEditMode(BOOL bCanEdit)
{
	GetDlgItem(IDC_BUTTON_EDIT_SETTING)	->SetWindowTextW(bCanEdit?_T("Back"):_T("Modify"));
	GetDlgItem(IDC_BUTTON_APPLY_SETTING)->EnableWindow	(bCanEdit?TRUE:FALSE);
	GetDlgItem(IDC_IP_ADDRESS_EDIT)		->EnableWindow	(bCanEdit?TRUE:FALSE);
	GetDlgItem(IDC_PORT_EDIT)			->EnableWindow	(bCanEdit?TRUE:FALSE);
	GetDlgItem(IDC_BUTTON_LOAD)			->EnableWindow	(bCanEdit?TRUE:FALSE);
	GetDlgItem(IDC_BUTTON_SAVE)			->EnableWindow	(bCanEdit?TRUE:FALSE);
}

void CSocketClientSampleDlg::ReadSettings()
{
	WCHAR wbuf[1024];
	DWORD dwResult;

	CString strDirectory,strTempSection,strTempDefault,strTempReturn;
	GetCurrentDirectory(1024,wbuf);
	strDirectory = CString(wbuf);

	CString strFileCfg,strTemp;
	strFileCfg = strDirectory + _T("\\config.ini");

	m_nTotalNumDevice = GetPrivateProfileInt(_T("System"),_T("Number of Device Control"),2,strFileCfg); 
	strTemp.Format(_T("%d"),m_nTotalNumDevice); 
	WritePrivateProfileString(_T("System"),_T("Number of Device Control"),strTemp,strFileCfg);

	m_arrbEnableDevice				.resize(m_nTotalNumDevice);
	m_arrstrIpAddress				.resize(m_nTotalNumDevice);
	m_arrnPort						.resize(m_nTotalNumDevice);
	m_arrstrTriggerCmd				.resize(m_nTotalNumDevice);
	m_arrstrDeviceNameLabel			.resize(m_nTotalNumDevice);
	m_arrstrReceiveDataDisplay		.resize(m_nTotalNumDevice);
	m_arrstrStatus					.resize(m_nTotalNumDevice);
	m_arrnGoodCounter				.resize(m_nTotalNumDevice);
	m_arrnBadCounter				.resize(m_nTotalNumDevice);
	m_arrbNeedUpdate				.resize(m_nTotalNumDevice);
	m_arrnDeviceStatus				.resize(m_nTotalNumDevice);
	m_arrbDryRunTestEnable			.resize(m_nTotalNumDevice);
	m_arrhThreadWorker				.resize(m_nTotalNumDevice);
	m_arrpEventThreadActionTrigger	.resize(m_nTotalNumDevice);
	m_arrbThreadExited				.resize(m_nTotalNumDevice);

	for(int nDeviceId=0; nDeviceId<m_nTotalNumDevice; nDeviceId++)
	{
		strTempSection.Format(_T("Device_%d"),nDeviceId+1);

		strTempDefault.Format(_T("Device%d"),nDeviceId+1);
		m_arrstrDeviceNameLabel[nDeviceId] = strTempDefault;
		dwResult = GetPrivateProfileString(strTempSection,_T("Device Label"),strTempDefault,wbuf,1024,strFileCfg); 
		if(dwResult>0) 
			m_arrstrDeviceNameLabel[nDeviceId] = CString(wbuf);

		strTempDefault.Format(_T("127.0.0.1"));
		dwResult = GetPrivateProfileString(strTempSection,_T("IP Address"),strTempDefault,wbuf,1024,strFileCfg); 
		if(dwResult>0) 
			m_arrstrIpAddress[nDeviceId] = CString(wbuf);

		m_arrnPort[nDeviceId] = GetPrivateProfileInt(strTempSection,_T("Port"),10038,strFileCfg);  

		GetPrivateProfileString(strTempSection,_T("Enable"),_T("Yes"),wbuf,1024,strFileCfg);  
		if(dwResult>0) 
			strTempReturn = CString(wbuf); 
		m_arrbEnableDevice[nDeviceId] = strTempReturn.CompareNoCase(_T("YES"))==0;

		strTempDefault.Format(_T("S%dTRIG\r\n"),nDeviceId+1);
		m_arrstrTriggerCmd[nDeviceId] = strTempDefault;
		dwResult = GetPrivateProfileString(strTempSection,_T("Trigger Command"),strTempDefault,wbuf,1024,strFileCfg); 
		if(dwResult>0) 
			m_arrstrTriggerCmd[nDeviceId] = CString(wbuf);

		strTempDefault.Format(_T("Device%d"),nDeviceId+1);
		m_arrstrDeviceNameLabel[nDeviceId] = strTempDefault;
		dwResult = GetPrivateProfileString(strTempSection,_T("Device Label"),strTempDefault,wbuf,1024,strFileCfg); 
		if(dwResult>0) 
			m_arrstrDeviceNameLabel[nDeviceId] = CString(wbuf);

	}
}

void CSocketClientSampleDlg::WriteSettings()
{
	CString strTemp,strTempSection;
	WCHAR wbuf[1024];

	CString strDirectory;
	GetCurrentDirectory(1024,wbuf);
	strDirectory = CString(wbuf);

	CString strFileCfg;
	strFileCfg = strDirectory + _T("\\config.ini");

	strTemp.Format(_T("%d"),m_nTotalNumDevice); 
	WritePrivateProfileString(_T("System"),_T("Number of Device Control"),strTemp,strFileCfg);

	for(int nDevice=0; nDevice<m_nTotalNumDevice; nDevice++)
	{
		strTempSection.Format(_T("Device_%d"),nDevice+1);

		WritePrivateProfileString(strTempSection,_T("Device Label"),m_arrstrDeviceNameLabel[nDevice],strFileCfg);

		strTemp = m_arrbEnableDevice[nDevice]?_T("Yes"):_T("No"); 
		WritePrivateProfileString(strTempSection,_T("Enable"),strTemp,strFileCfg);

		WritePrivateProfileString(strTempSection,_T("IP Address"),m_arrstrIpAddress[nDevice],strFileCfg);

		strTemp.Format(_T("%d"),m_arrnPort[nDevice]); 
		WritePrivateProfileString(strTempSection,_T("Port"),strTemp,strFileCfg);

		WritePrivateProfileString(strTempSection,_T("Trigger Command"),m_arrstrTriggerCmd[nDevice],strFileCfg);
		WritePrivateProfileString(strTempSection,_T("Device Label"),m_arrstrDeviceNameLabel[nDevice],strFileCfg);
	}
}

void CSocketClientSampleDlg::SendTriggerCmd(int nDeviceId,BOOL bQuietMode)
{
	m_arrstrReceiveDataDisplay[nDeviceId].Empty();

	if(!m_arrbEnableDevice[nDeviceId])
	{
		if(m_arrnDeviceStatus[nDeviceId] != CSocketClientSampleDlg::STATUS_DISABLED)
		{
			m_arrnDeviceStatus[nDeviceId] = CSocketClientSampleDlg::STATUS_DISABLED;
			UpdateDeviceListStatus(nDeviceId);
		}
		return;
	}
	while(TRUE)
	{
		// use this to clear left over bytes
		if(m_pClientSocket->ReceiveData(nDeviceId, NULL,5)==FALSE)
			break;
	}

	if(m_pClientSocket->SendTriggerCmd(nDeviceId,TRUE))
		m_pClientSocket->ReceiveData(nDeviceId, &m_arrstrReceiveDataDisplay[nDeviceId],1000);
	else
		m_arrstrReceiveDataDisplay[nDeviceId] = _T("Disconnected");

	if(m_arrstrReceiveDataDisplay[nDeviceId].IsEmpty())
		m_arrstrReceiveDataDisplay[nDeviceId] = _T("[EMPTY]");

	if(m_arrstrReceiveDataDisplay[nDeviceId].Compare(_T("[EMPTY]"))==0 || 
		m_arrstrReceiveDataDisplay[nDeviceId].Compare(_T("ERROR\r"))==0 || 
		m_arrstrReceiveDataDisplay[nDeviceId].Compare(_T("Disconnected"))==0 )
	{
		m_arrnBadCounter[nDeviceId]++;
	}
	else
		m_arrnGoodCounter[nDeviceId]++;

	PostMessageW(WM_UPDATE_LIST_STATUS,nDeviceId);
}

void CSocketClientSampleDlg::SetDeviceListItemToEditGroup(int nSelect)
{
	if(nSelect>=m_nTotalNumDevice || nSelect<0)
		return;

	CString strText;
	strText.Format(_T("%d"),nSelect+1);
	GetDlgItem(IDC_EDIT_SCANNERID)->SetWindowTextW(strText);
	strText.Format(_T("%s"),m_arrstrIpAddress[nSelect]);
	GetDlgItem(IDC_IP_ADDRESS_EDIT)->SetWindowTextW(strText);
	strText.Format(_T("%d"),m_arrnPort[nSelect]);
	GetDlgItem(IDC_PORT_EDIT)->SetWindowTextW(strText);
}

int CSocketClientSampleDlg::GetUnitqueDeviceIdForThread()
{
	int nReturn;
	m_csGetThreadId.Lock();
	nReturn = m_nIdCurThreadCount;
	m_nIdCurThreadCount++;
	m_csGetThreadId.Unlock();
	return nReturn;
};

void CSocketClientSampleDlg::QuitAllThreads()
{
	m_bUserInitQuitDryRunLoop = TRUE;
	m_bUserInitKillThread = TRUE;
	for(int nDeviceId=0; nDeviceId<m_nTotalNumDevice; nDeviceId++)
	{
		if(m_arrhThreadWorker[nDeviceId])
		{
			for(int nCount=0; nCount<10; nCount++)
			{
				if(WaitForSingleObject(m_arrhThreadWorker[nDeviceId]->m_hThread,200)==WAIT_OBJECT_0)
					break;
			}

			if(WaitForSingleObject(m_arrhThreadWorker[nDeviceId]->m_hThread,1000)==WAIT_TIMEOUT)
				TerminateThread(m_arrhThreadWorker[nDeviceId]->m_hThread, 1L);

			CloseHandle(m_arrhThreadWorker[nDeviceId]->m_hThread);
			m_arrhThreadWorker[nDeviceId]->m_hThread = NULL;
			delete m_arrhThreadWorker[nDeviceId];
			m_arrhThreadWorker[nDeviceId] = NULL;
		}
	}
	if(m_hThreadConnectionStatus)
	{
		for(int nCount=0; nCount<10; nCount++)
		{
			if(WaitForSingleObject(m_hThreadConnectionStatus->m_hThread,200)==WAIT_OBJECT_0)
				break;
		}
		if(WaitForSingleObject(m_hThreadConnectionStatus->m_hThread,1000)==WAIT_TIMEOUT)
		{
			TerminateThread(m_hThreadConnectionStatus->m_hThread, 1L);
			CloseHandle(m_hThreadConnectionStatus->m_hThread);
			m_hThreadConnectionStatus->m_hThread = NULL;
		}
		else
		{
			CloseHandle(m_hThreadConnectionStatus->m_hThread);
			m_hThreadConnectionStatus->m_hThread = NULL;
		}
		delete m_hThreadConnectionStatus;
		m_hThreadConnectionStatus = NULL;
	}
}

BOOL CSocketClientSampleDlg::PreTranslateMessage(MSG* pMsg)
{
	BOOL bRet = CDialog::PreTranslateMessage(pMsg);
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
		OnBnClickedExit();
	return bRet;
}


UINT ThreadWorker(LPVOID lpvParam)
{
	CSocketClientSampleDlg *pSocketClientSampleDlg = (CSocketClientSampleDlg*)(lpvParam);
	int nDeviceId = pSocketClientSampleDlg->GetUnitqueDeviceIdForThread();
	pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId] = CSocketClientSampleDlg::STATUS_INIT;
	int nStatusLabelCnt = 0;
	CString strDot;
	CString strTemp;

	while(TRUE)
	{
		nStatusLabelCnt++;
		nStatusLabelCnt = nStatusLabelCnt%3;

		strDot.Empty();
		for(int i=0; i<(nStatusLabelCnt+1); i++)
			strDot += _T(".");

		if(WaitForSingleObject(pSocketClientSampleDlg->m_arrpEventThreadActionTrigger[nDeviceId]->m_hObject,INFINITE)==WAIT_OBJECT_0)
		{
			if(pSocketClientSampleDlg->m_bUserInitKillThread)	// exit sw
			{
				pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId] = CSocketClientSampleDlg::STATUS_DISCONNECTING;
				pSocketClientSampleDlg->UpdateDeviceListStatus(nDeviceId);
				break;
			}

			if(pSocketClientSampleDlg->m_bUserInitDisconnection)		// disconnect
			{
				pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId] = CSocketClientSampleDlg::STATUS_DISCONNECTING;
				pSocketClientSampleDlg->UpdateDeviceListStatus(nDeviceId);
				break;
			}

			if(pSocketClientSampleDlg->m_bUserInitConnection)
			{
				if(pSocketClientSampleDlg->m_arrbEnableDevice[nDeviceId] && !pSocketClientSampleDlg->m_arrstrIpAddress[nDeviceId].IsEmpty() && pSocketClientSampleDlg->m_arrnPort[nDeviceId]!=0)
				{
					pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId] = CSocketClientSampleDlg::STATUS_CONNECTING;
					pSocketClientSampleDlg->UpdateDeviceListStatus(nDeviceId);

					if(pSocketClientSampleDlg->m_pClientSocket->Connect(nDeviceId))
					{
						pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId] = CSocketClientSampleDlg::STATUS_CONNECTED;
						pSocketClientSampleDlg->UpdateDeviceListStatus(nDeviceId);
					}
					else
					{
						pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId] = CSocketClientSampleDlg::STATUS_CONNECT_ERROR;
						pSocketClientSampleDlg->UpdateDeviceListStatus(nDeviceId);
					}
				}
				else
				{
					if(!pSocketClientSampleDlg->m_arrbEnableDevice[nDeviceId])
					{
						pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId] = CSocketClientSampleDlg::STATUS_DISABLED;
						pSocketClientSampleDlg->UpdateDeviceListStatus(nDeviceId);
					}
					else
					{
						pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId] = CSocketClientSampleDlg::STATUS_SETTING_ERROR;
						pSocketClientSampleDlg->UpdateDeviceListStatus(nDeviceId);
					}
				}
			}

			if(pSocketClientSampleDlg->m_bUserInitRunDryRunLoop)
			{
				if(pSocketClientSampleDlg->m_arrbDryRunTestEnable[nDeviceId])
				{
					while(TRUE)
					{
						if(pSocketClientSampleDlg->m_bUserInitQuitDryRunLoop)
						{
							pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId] = CSocketClientSampleDlg::STATUS_DRYRUN_STOPPING;
							break;
						}

						if(pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId]==CSocketClientSampleDlg::STATUS_CONNECTED || 
							pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId]==CSocketClientSampleDlg::STATUS_CONNECT_ERROR ||
							pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId]==CSocketClientSampleDlg::STATUS_DRYRUN_STOPPED)
						{
							if(pSocketClientSampleDlg->m_bUserInitRunDryRunLoop)
							{
								pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId] = CSocketClientSampleDlg::STATUS_DRYRUN_RUNNING;
								pSocketClientSampleDlg->UpdateDeviceListStatus(nDeviceId);
							}
						}

						if(pSocketClientSampleDlg->m_bUserInitDisconnection)
						{
							pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId] = CSocketClientSampleDlg::STATUS_DISCONNECTING;
							pSocketClientSampleDlg->UpdateDeviceListStatus(nDeviceId);
							break;
						}


						// Dry run mode operation
						pSocketClientSampleDlg->m_arrstrReceiveDataDisplay[nDeviceId].Empty();
						if(!pSocketClientSampleDlg->m_arrbEnableDevice[nDeviceId])
						{
							if(pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId] != CSocketClientSampleDlg::STATUS_DISABLED)
							{
								pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId] = CSocketClientSampleDlg::STATUS_DISABLED;
								pSocketClientSampleDlg->UpdateDeviceListStatus(nDeviceId);
							}
							Sleep(pSocketClientSampleDlg->m_nTimeInterval);
							continue;
						}
						while(TRUE)
						{
							// use this to clear left over bytes
							if(pSocketClientSampleDlg->m_pClientSocket->ReceiveData(nDeviceId, NULL,5)==FALSE)
								break;
						}

						if(pSocketClientSampleDlg->m_pClientSocket->SendTriggerCmd(nDeviceId,TRUE))
							pSocketClientSampleDlg->m_pClientSocket->ReceiveData(nDeviceId, &pSocketClientSampleDlg->m_arrstrReceiveDataDisplay[nDeviceId],5000);
						else
							pSocketClientSampleDlg->m_arrstrReceiveDataDisplay[nDeviceId] = _T("Disconnected");

						if(pSocketClientSampleDlg->m_arrstrReceiveDataDisplay[nDeviceId].IsEmpty())
							pSocketClientSampleDlg->m_arrstrReceiveDataDisplay[nDeviceId] = _T("[EMPTY]");

						if(pSocketClientSampleDlg->m_arrstrReceiveDataDisplay[nDeviceId].Compare(_T("[EMPTY]"))==0 || 
							pSocketClientSampleDlg->m_arrstrReceiveDataDisplay[nDeviceId].Compare(_T("ERROR\r"))==0 || 
							pSocketClientSampleDlg->m_arrstrReceiveDataDisplay[nDeviceId].Compare(_T("Disconnected"))==0 )
						{
							pSocketClientSampleDlg->m_arrnBadCounter[nDeviceId]++;
						}
						else
							pSocketClientSampleDlg->m_arrnGoodCounter[nDeviceId]++;

						pSocketClientSampleDlg->PostMessageW(WM_UPDATE_LIST_STATUS,nDeviceId);
						Sleep(pSocketClientSampleDlg->m_nTimeInterval);

					}

				}
			}
		}
	}

	pSocketClientSampleDlg->m_arrbThreadExited[nDeviceId] = TRUE;
	TRACE(_T("==================================> Connection Thread[%d] Exited\r\n"), nDeviceId);
	return 0;
}

UINT ThreadConnectionStatus(LPVOID lpvParam)
{
	CSocketClientSampleDlg *pSocketClientSampleDlg = (CSocketClientSampleDlg*)(lpvParam);
	CString strDot;

	int nStatusLabelCnt = 0;
	while(TRUE)
	{
		nStatusLabelCnt++;
		nStatusLabelCnt = nStatusLabelCnt%3;

		strDot.Empty();
		for(int i=0; i<(nStatusLabelCnt+1); i++)
			strDot += _T(".");


		if(pSocketClientSampleDlg->m_bUserInitKillThread)	// exit sw
		{
			for(int nDeviceId=0; nDeviceId<pSocketClientSampleDlg->m_nTotalNumDevice; nDeviceId++)
			{
				pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId]= CSocketClientSampleDlg::STATUS_EXIT;
				pSocketClientSampleDlg->UpdateDeviceListStatus(nDeviceId);
			}
			break;
		}

		if(pSocketClientSampleDlg->m_bUserInitDisconnection)
		{
			BOOL bStillConnecting = FALSE;

			for(int nDeviceId=0; nDeviceId<pSocketClientSampleDlg->m_nTotalNumDevice; nDeviceId++)
				bStillConnecting |= (pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId]==CSocketClientSampleDlg::STATUS_CONNECTING);

			for(int nDeviceId=0; nDeviceId<pSocketClientSampleDlg->m_nTotalNumDevice; nDeviceId++)
			{
				if(pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId]==CSocketClientSampleDlg::STATUS_CONNECTED ||
					pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId]==CSocketClientSampleDlg::STATUS_CONNECT_ERROR ||
					pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId]==CSocketClientSampleDlg::STATUS_SETTING_ERROR ||
					pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId]==CSocketClientSampleDlg::STATUS_DISCONNECTING||
					pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId]==CSocketClientSampleDlg::STATUS_DRYRUN_STOPPED||
					pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId]==CSocketClientSampleDlg::STATUS_DISABLED)
				{
					pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId] = CSocketClientSampleDlg::STATUS_DISCONNECTED;
					pSocketClientSampleDlg->UpdateDeviceListStatus(nDeviceId);
				}
			}
			if(bStillConnecting==FALSE)
			{
				pSocketClientSampleDlg->m_bUserInitDisconnection = FALSE;		//reset
				pSocketClientSampleDlg->m_bUserInitRunDryRunLoop = FALSE;		//reset
				pSocketClientSampleDlg->PostMessageW(WM_UPDATE_BUTTON_CONNECTED_STATUS,FALSE);
			}
		}

		if(pSocketClientSampleDlg->m_bUserInitConnection)
		{

			BOOL bStillConnecting = FALSE;
			for(int nDeviceId=0; nDeviceId<pSocketClientSampleDlg->m_nTotalNumDevice; nDeviceId++)
				bStillConnecting |= pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId]==CSocketClientSampleDlg::STATUS_CONNECTING;

			if(!bStillConnecting)
			{
				pSocketClientSampleDlg->PostMessageW(WM_UPDATE_BUTTON_CONNECTED_STATUS,TRUE);
				pSocketClientSampleDlg->m_bUserInitConnection = FALSE; //reset
			}
		}

		if(pSocketClientSampleDlg->m_bUserInitRunDryRunLoop)
		{
			if(pSocketClientSampleDlg->m_bUserInitDisconnection)
			{
				BOOL bStillDryRun = FALSE;
				for(int nDeviceId=0; nDeviceId<pSocketClientSampleDlg->m_nTotalNumDevice; nDeviceId++)
					bStillDryRun |= pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId]==CSocketClientSampleDlg::STATUS_DRYRUN_RUNNING;

				if(!bStillDryRun)
				{
					pSocketClientSampleDlg->PostMessageW(WM_UPDATE_BUTTON_CONNECTED_STATUS,FALSE);
					pSocketClientSampleDlg->m_bUserInitRunDryRunLoop = FALSE; //reset
					pSocketClientSampleDlg->m_bUserInitDisconnection = FALSE; //reset

					for(int nDeviceId=0; nDeviceId<pSocketClientSampleDlg->m_nTotalNumDevice; nDeviceId++)
					{
						bStillDryRun = pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId]==CSocketClientSampleDlg::STATUS_DISCONNECTED;
						pSocketClientSampleDlg->UpdateDeviceListStatus(nDeviceId);
					}
				}
			}

			if(pSocketClientSampleDlg->m_bUserInitQuitDryRunLoop)
			{
				BOOL bStillDryRun = FALSE;
				for(int nDeviceId=0; nDeviceId<pSocketClientSampleDlg->m_nTotalNumDevice; nDeviceId++)
					bStillDryRun |= pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId]==CSocketClientSampleDlg::STATUS_DRYRUN_RUNNING;

				if(!bStillDryRun)
				{
					pSocketClientSampleDlg->m_bUserInitRunDryRunLoop = FALSE; //reset
					pSocketClientSampleDlg->m_bUserInitQuitDryRunLoop = FALSE; //reset

					for(int nDeviceId=0; nDeviceId<pSocketClientSampleDlg->m_nTotalNumDevice; nDeviceId++)
					{
						if(pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId]==CSocketClientSampleDlg::STATUS_DRYRUN_STOPPING)
							pSocketClientSampleDlg->m_arrnDeviceStatus[nDeviceId] = CSocketClientSampleDlg::STATUS_DRYRUN_STOPPED;
					}
					pSocketClientSampleDlg->UpdateDeviceListStatus(-1);
				}
			}
		}
		BOOL bUpdateNeeded = FALSE;
		for(int nDeviceId=0; nDeviceId<pSocketClientSampleDlg->m_nTotalNumDevice; nDeviceId++)
		{
			if(pSocketClientSampleDlg->m_arrbNeedUpdate[nDeviceId])
			{
				bUpdateNeeded |= TRUE;
				break;
			}
		}
		if(bUpdateNeeded)
			pSocketClientSampleDlg->PostMessageW(WM_UPDATE_LIST_STATUS,-1);

		for(int nDeviceId=0; nDeviceId<pSocketClientSampleDlg->m_nTotalNumDevice; nDeviceId++)
			pSocketClientSampleDlg->m_arrbNeedUpdate[nDeviceId] = FALSE; //reset

		Sleep(200);
	}
	TRACE(_T("==================================> Status Thread Exited\r\n"));
	return 0;
}
