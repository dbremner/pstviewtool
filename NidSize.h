#pragma once

#include "NDBViewer.h"
#include "NDBViewDlg.h"
#include "NDBViewChildDlg.h"
#include "afxwin.h"

// CNidSize dialog
static const COLORREF c_FreeNID = RGB(255,255,255);
static const COLORREF c_FullNID = RGB(0,0,0);
static const COLORREF c_NIDPastEOF = RGB(210,210,210);
static const double c_minShadeNID = 0.15;

class CNidSize : public CNDBViewChildDlg
{
	DECLARE_DYNAMIC(CNidSize)

public:
	CNidSize(NID parentNid, NID nid, NDBViewer * pNDBViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent = NULL);   // standard constructor
	virtual ~CNidSize();

// Dialog Data
	enum { IDD = IDD_NID_SIZE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();

private:
	bool m_fLookupError;
	CBitmap * m_pBitmap;
	NDBViewer * m_pNDBViewer;
	NID m_nid;
	NID m_nidParent;
	BID m_dataBID;
	BID m_subnodeBID;

	void EachBID(BID bid, CB * pRunningTotal, int * pNumBlocks, CB * usedSpace, CB cbPerPixel, int numPixel);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};
#pragma once


// CGetNid dialog
enum {
	inspNodeSize,
	inspHN,
	inspES,
	inspBTH,
	inspTCVRI,
	inspPC,
	inspLTPAutoDetect,
	inspLTPMaxInspector
};

static const WCHAR*	ltplistvalues[] = {
	L"Node Size",
	L"Heap-on-Node (HN)",
	L"Element Stream (ES)",
	L"BTree on Heap (BTH)",
	L"TCVROOTINDEX",
	L"Property Context (PC)",
	L"Auto Detect",
};

class CGetNid : public CDialog
{
	DECLARE_DYNAMIC(CGetNid)

public:
	CGetNid(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGetNid();

// Dialog Data
	enum { IDD = IDD_NID_SIZE_NID };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	int m_type;
	CString m_nid;
	CString m_parentNid;
	CString m_hid;
public:
	CComboBox m_ccb;
public:
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnCbnSelchangeCombo1();
public:
	CEdit m_hidCtrl;
};
