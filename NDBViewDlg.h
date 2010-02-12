// NDBViewDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "NDBView.h"
#include "NDBViewer.h"
#include "LTPViewer.h"

// A little hack to get around focus issues.
#define WM_DELAY_PERFORM_ACTION (WM_APP+1)

BOOL DoConsistencyCheck(CString filename);

// CNDBViewDlg dialog
class CNDBViewDlg : public CDialog
{
// Construction
public:
	CNDBViewDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_NDBVIEW_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFileOpen32771();
	void PerformAction(NodeData * pNodeData);
	afx_msg LRESULT PerformActionHandler(WPARAM wParam, LPARAM lParam);
	void ChildDialogClosed(CDialog * pDialog) { if(!m_fClosingChildren) m_ActiveDialogs.RemoveAt(m_ActiveDialogs.Find(pDialog)); }
	void AddChildDialog(CDialog * pDialog) { m_ActiveDialogs.AddHead(pDialog); }

private:
	LTPViewer *m_ndbViewer;
	CTreeCtrl m_nbt;
	CTreeCtrl m_bbt;
	CTreeCtrl m_header;
	CImageList m_il;
	bool m_fClosingChildren;
	CList< CDialog * > m_ActiveDialogs;

private:
	void Clear(void);
	void CloseAllChildDialogs(); 

	afx_msg void OnViewAddress();
	afx_msg void OnFileClose32775();
	afx_msg void OnDestroy();
	afx_msg void OnTvnItemexpandingTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnItemexpandingTree2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnDeleteitemTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnDeleteitemTree2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkTree1(NMHDR *pNMHDR, LRESULT *pResult);
	virtual void OnOK(void);
	afx_msg void OnViewAllocationmap();
	afx_msg void OnViewBtreepagedistribution();
	afx_msg void OnNMDblclkTree2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnViewScavenge();
	afx_msg void OnViewConsistencyreport();
	afx_msg void OnViewRefreport();
	afx_msg void OnViewNodesize();
	afx_msg void OnViewStatistics();
	afx_msg void OnHelpAboutndbview();
	afx_msg void OnFileExit();
	afx_msg void OnViewAmrinfo();
	afx_msg void OnViewNode();
};

inline void CNDBViewDlg::CloseAllChildDialogs()
{
	POSITION pos = m_ActiveDialogs.GetHeadPosition();

	// to prevent modification of the list during iteration - otherwise
	// DestroyWindow will eventually call ChildDialogClosed
	m_fClosingChildren = true;
	while( pos != NULL )
	{
		((CDialog*)(m_ActiveDialogs.GetNext(pos)))->DestroyWindow();
	}
	m_fClosingChildren = false;
	m_ActiveDialogs.RemoveAll();
}