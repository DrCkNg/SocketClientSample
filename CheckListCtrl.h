#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CheckListCtrl.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CCheckListCtrl window

class CCheckListCtrl : public CListCtrl
{
// Construction
public:
	CCheckListCtrl();

// Attributes
public:
	void SetParent(CDialog* pParent){m_pParent=pParent;}
	
// Operations
private:
	BOOL	m_blInited;
	CImageList	m_checkImgList;
	CDialog* m_pParent;


public:

protected:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCheckListCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCheckListCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CCheckListCtrl)
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);		
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

