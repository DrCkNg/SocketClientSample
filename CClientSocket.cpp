/*************************************************************************
// CClientSocket.cpp
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
#include <stdio.h>
#include <winsock2.h>
#include "stdafx.h"
#include "CClientSocket.h"

CClientSocket::CClientSocket(int nTotalNumDevices)
{
	m_nTotalNumDevice = nTotalNumDevices;

	m_arrDeviceInfo				.resize(nTotalNumDevices);
	m_arrSerialSocket			.resize(nTotalNumDevices);
	m_arrSocketErrorWhenComm	.resize(nTotalNumDevices);
	m_arrnDeviceMapId			.resize(nTotalNumDevices);
	m_arrbConnectionDone		.resize(nTotalNumDevices);
	m_arrbConnectionResult		.resize(nTotalNumDevices);
	m_arrbReceiveingData		.resize(nTotalNumDevices);
	m_arrstrTriggerCmd			.resize(nTotalNumDevices);
	m_arrpCsOperate				.resize(nTotalNumDevices);

	for (int nDeviceId = 0; nDeviceId < nTotalNumDevices; nDeviceId++) 
	{
		m_arrDeviceInfo[nDeviceId].cDeviceIpAdddress[0]		= NULL;
		m_arrDeviceInfo[nDeviceId].nDevicePort				= 0;
		m_arrSerialSocket[nDeviceId]						= INVALID_SOCKET;
		m_arrSocketErrorWhenComm[nDeviceId]					= FALSE;
		m_arrnDeviceMapId[nDeviceId]						= 0;
		m_arrbConnectionDone[nDeviceId]						= FALSE;
		m_arrbReceiveingData[nDeviceId]						= FALSE;
		m_arrbConnectionResult[nDeviceId]					= FALSE;
		m_arrstrTriggerCmd[nDeviceId]						= _T("TriggerCmd");
		m_arrpCsOperate[nDeviceId]							= new CCriticalSection();
	}
	m_bUseShareSocketForSameIpPort = FALSE;
}

CClientSocket::~CClientSocket()
{
	Disconnect();
	if(m_arrpCsOperate.size()>0)
	{
		for(int nDeviceId=0; nDeviceId<m_nTotalNumDevice; nDeviceId++)
		{
			if(m_arrpCsOperate[nDeviceId])
			{
				delete m_arrpCsOperate[nDeviceId];
				m_arrpCsOperate[nDeviceId] = NULL;
			}
		}
	}
}

void CClientSocket::Initialise()
{
	GenerateActualDeviceMapping();
}	
	
BOOL CClientSocket::Connect(int nDeviceId)
{
	if(GetRealDevId(nDeviceId)!=nDeviceId)
	{
		while(TRUE)
		{
			if(m_arrbConnectionDone[GetRealDevId(nDeviceId)])
				return m_arrbConnectionResult[GetRealDevId(nDeviceId)];
			else
				Sleep(100);
		}
	}
	WSADATA data;
	WSAStartup(MAKEWORD(2,0), &data);

	struct sockaddr_in dst;
	memset(&dst, 0, sizeof(dst));
	dst.sin_port             = htons(m_arrDeviceInfo[nDeviceId].nDevicePort);
	dst.sin_family           = AF_INET;
	dst.sin_addr.S_un.S_addr = inet_addr(m_arrDeviceInfo[nDeviceId].cDeviceIpAdddress);

	if (m_arrSerialSocket[nDeviceId] != INVALID_SOCKET) 
	{
		closesocket(m_arrSerialSocket[nDeviceId]);
		m_arrSerialSocket[nDeviceId] =INVALID_SOCKET;
	}

	m_arrSerialSocket[nDeviceId] = socket(AF_INET, SOCK_STREAM, 0);
	m_arrSocketErrorWhenComm[nDeviceId] = FALSE;		//reset
	if(connect(m_arrSerialSocket[nDeviceId], (struct sockaddr *) &dst, sizeof(dst)) != 0)
	{
		closesocket(m_arrSerialSocket[nDeviceId]);
		m_arrSerialSocket[nDeviceId] = INVALID_SOCKET;

		m_arrbConnectionResult[nDeviceId] = FALSE;
		m_arrbConnectionDone[nDeviceId] = TRUE;

		return FALSE;
	}

	int timeout = 100;
	setsockopt(m_arrSerialSocket[nDeviceId],	SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
	m_arrbConnectionResult[nDeviceId] = TRUE;
	m_arrbConnectionDone[nDeviceId] = TRUE;
	return TRUE;
}

void CClientSocket::Disconnect()
{
	for (int i = 0; i < m_nTotalNumDevice; i++) 
	{
		if (m_arrSerialSocket[i] != INVALID_SOCKET) 
		{
			closesocket(m_arrSerialSocket[i]);
			m_arrSerialSocket[i] = INVALID_SOCKET;
		}
	}
	WSACleanup();
}

void CClientSocket::SetSocketData(int nDeviceId, CString strIpAddr, int nPort)
{
	m_arrDeviceInfo[nDeviceId].nDevicePort = nPort;

	char* cIpAddress;
	CStringA strIpAddressA;
	strIpAddressA = CT2A(strIpAddr);
	cIpAddress = strIpAddressA.GetBufferSetLength(strIpAddressA.GetLength());
	m_arrDeviceInfo[nDeviceId].lDeviceIpAddress = inet_addr(cIpAddress);

	strcpy_s(m_arrDeviceInfo[nDeviceId].cDeviceIpAdddress,20,cIpAddress);
	strIpAddressA.ReleaseBuffer();
}

BOOL CClientSocket::SendTriggerCmd(int nDeviceId, BOOL bQuietMode)
{
	USES_CONVERSION;
	CStringA strCmdA = T2A(m_arrstrTriggerCmd[nDeviceId]);
	char* pCommand = strCmdA.GetBufferSetLength(strCmdA.GetLength());
	int nCharSent = 0;

	int nRealDeviceId = GetRealDevId(nDeviceId);
	m_arrpCsOperate[nRealDeviceId]->Lock();
	if (m_arrSerialSocket[nRealDeviceId] != INVALID_SOCKET && !m_arrSocketErrorWhenComm[nRealDeviceId]) 
	{
		nCharSent = send(m_arrSerialSocket[nRealDeviceId], pCommand, strCmdA.GetLength(), 0);
	}
	else 
	{
		CString strMsg;
		strMsg.Format(_T("%S is disconnected."), m_arrDeviceInfo[nRealDeviceId].cDeviceIpAdddress);		
		if(!bQuietMode)
			AfxMessageBox(strMsg);
	}
	strCmdA.ReleaseBuffer();
	return (nCharSent>0);
}

BOOL CClientSocket::ReceiveData(int nDeviceId, CString* pstrData, double nTimeoutMs)
{
	CString strReceive;
	char recvBytes[RECV_DATA_MAX];
	int recvSize = 0;
	strReceive.Empty();
	clock_t TimeBegin = clock();
	clock_t TimeNow;
	double TimeElapsed;
	BOOL bResult = FALSE;

	int nRealDeviceId = GetRealDevId(nDeviceId);
	m_arrbReceiveingData[nRealDeviceId] = TRUE;
	while(TRUE)
	{
		if (m_arrSerialSocket[nRealDeviceId] != INVALID_SOCKET) 
		{
			recvSize = recv(m_arrSerialSocket[nRealDeviceId], recvBytes, sizeof(recvBytes), 0);
		}
		else 
		{
			CString strMsg;
			strMsg.Format(_T("%S is disconnected."), m_arrDeviceInfo[nRealDeviceId].cDeviceIpAdddress);	
			break;
		}

		if (recvSize > 0) 
		{
			recvBytes[recvSize] = 0;	// Terminating null to handle as string.

			CString strData;
			strData.Format(_T("%s"), (CString)recvBytes);
			if(pstrData)
				*pstrData = strData;
			bResult = TRUE;
			break;
		}

		if(recvSize==SOCKET_ERROR)
		{
			int nError = WSAGetLastError();
			if(nError!=WSAETIMEDOUT)
				m_arrSocketErrorWhenComm[nRealDeviceId] |= TRUE;
		}

		TimeNow = clock();
		TimeElapsed = TimeNow - TimeBegin;
		if(TimeElapsed > nTimeoutMs)
			break;
		Sleep(10);
	}
	m_arrbReceiveingData[nRealDeviceId] = FALSE;
	m_arrpCsOperate[nRealDeviceId]->Unlock();

	return bResult;
}

void CClientSocket::GenerateActualDeviceMapping()
{
	// Pre-check for Ip and port conflict
	for(int nDeviceId=0; nDeviceId<m_nTotalNumDevice; nDeviceId++)
	{
		m_arrbConnectionDone[nDeviceId]=FALSE;		//reset	
		m_arrbConnectionResult[nDeviceId]=FALSE;	//reset	
		m_arrnDeviceMapId[nDeviceId] = nDeviceId;

		if(m_bUseShareSocketForSameIpPort)
		{
			// if the IP address and port is the same with previous device, map the ID to same device
			for(int nCheckDeviceId=0; nCheckDeviceId<nDeviceId; nCheckDeviceId++)
			{
				if(m_arrDeviceInfo[nDeviceId].lDeviceIpAddress==m_arrDeviceInfo[nCheckDeviceId].lDeviceIpAddress &&
					m_arrDeviceInfo[nDeviceId].nDevicePort==m_arrDeviceInfo[nCheckDeviceId].nDevicePort)
				{
					m_arrnDeviceMapId[nDeviceId] = nCheckDeviceId;
					break;
				}
			}
		}
	}
}

