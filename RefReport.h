#pragma once
#include "afxcmn.h"
#include "NDBViewDlg.h"
#include "NDBViewChildDlg.h"


// CRefReport dialog

class CRefReport : public CNDBViewChildDlg
{
	DECLARE_DYNAMIC(CRefReport)

public:
	CRefReport::CRefReport(BID bid, NDBViewer * pNDBViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent = NULL);
	virtual ~CRefReport();

// Dialog Data
	enum { IDD = IDD_REF_REPORT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CListCtrl m_lc;
	CImageList m_il;
	NDBViewer * m_pNDBViewer;
	BID m_bid;
	UINT m_cDiskRefCount;
	UINT m_cCalculatedCount;

	virtual BOOL OnInitDialog();
public:
	void NBTPage(BTPAGE * pBTPage, PAGETRAILER * pPT, IB ib);
	void BBTPage(BTPAGE * pBTPage, PAGETRAILER * pPT, IB ib);
public:
	afx_msg void OnLvnDeleteitemList3(NMHDR *pNMHDR, LRESULT *pResult);
public:
	afx_msg void OnNMDblclkList3(NMHDR *pNMHDR, LRESULT *pResult);
};
#pragma once


// CRefReportBID dialog

class CRefReportBID : public CDialog
{
	DECLARE_DYNAMIC(CRefReportBID)

public:
	CRefReportBID(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRefReportBID();

// Dialog Data
	enum { IDD = IDD_REF_REPORT_BID };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_cs;
};
