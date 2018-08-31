// CheckListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "SocketClientSample.h"
#include "CheckListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCheckListCtrl

CCheckListCtrl::CCheckListCtrl() : m_blInited(FALSE)
{
	m_pParent = NULL;
}

CCheckListCtrl::~CCheckListCtrl()
{
}


BEGIN_MESSAGE_MAP(CCheckListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CCheckListCtrl)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemChanged)		
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCheckListCtrl message handlers
void CCheckListCtrl::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	int aa = 0;
	UpdateData(TRUE);
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if (
		(pNMListView->uChanged & LVIF_STATE) && 
		!(pNMListView->uOldState & LVNI_SELECTED) && !(pNMListView->uOldState & LVNI_FOCUSED) &&
		(pNMListView->uNewState & LVNI_SELECTED) && (pNMListView->uNewState & LVNI_FOCUSED))
	{
		m_pParent->SendMessageW(WM_SELECT_DEVICE_LISTITEM,1);
	}
	UpdateData(FALSE);

}

