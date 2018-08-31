
// SocketClientSample.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

#define        WM_SELECT_DEVICE_LISTITEM			(WM_USER + 1)
#define        WM_UPDATE_LIST_STATUS				(WM_USER + 2)
#define        WM_UPDATE_BUTTON_CONNECTED_STATUS	(WM_USER + 3)

// CSocketClientSampleApp:
// See SocketClientSample.cpp for the implementation of this class
//

class CSocketClientSampleApp : public CWinApp
{
public:
	CSocketClientSampleApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CSocketClientSampleApp theApp;