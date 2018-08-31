/*************************************************************************
// CClientSocket.h
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
#pragma once
#include <afxmt.h>
#include <vector>
#define RECV_DATA_MAX	10240

struct struct_DeviceInfo{
	char  cDeviceIpAdddress[40];	// IP address
	int   nDevicePort;				// Port number
	ULONG lDeviceIpAddress;

};
class CClientSocket
{
public:

	CClientSocket(int nTotalNumDevice);
	~CClientSocket();

private:
	int								m_nTotalNumDevice;
	BOOL							m_bUseShareSocketForSameIpPort;
	std::vector<struct_DeviceInfo>	m_arrDeviceInfo;
	std::vector<SOCKET>				m_arrSerialSocket;
	std::vector<BOOL>				m_arrSocketErrorWhenComm;
	std::vector<int>				m_arrnDeviceMapId;
	std::vector<int>				m_arrbConnectionDone;
	std::vector<int>				m_arrbConnectionResult;
	std::vector<int>				m_arrbReceiveingData;
	std::vector<CString>			m_arrstrTriggerCmd;
	std::vector<CCriticalSection*>	m_arrpCsOperate;

public:
	void Initialise();		//Call this after change IP address & Port
	BOOL Connect(int nDeviceId);
	void Disconnect();
	void SetSocketData(int nDeviceId, CString strIpAddr, int nPort);
	void SetUseShareSocketForSameIpPort(BOOL bUse)				{m_bUseShareSocketForSameIpPort = bUse;}
	BOOL SendTriggerCmd(int nDeviceId, BOOL bQuietMode = FALSE);
	BOOL ReceiveData(int nDeviceID, CString *strData, double nTimeoutMs);
	int  GetTotalNumDevice()									{return m_nTotalNumDevice;}
	void SetTriggerCmd(int nDeviceId, CString strCmd)			{m_arrstrTriggerCmd[nDeviceId] = strCmd;}
private:
	void GenerateActualDeviceMapping();			
	int GetRealDevId(int nDev)									{return m_arrnDeviceMapId[nDev];}
};