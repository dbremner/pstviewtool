#pragma once

#include "NDBViewer.h"
#include "NDBViewDlg.h"
#include "NDBViewChildDlg.h"

// CBTDialog dialog

static const COLORREF c_BBTPage = RGB(125,0,0);
static const COLORREF c_NBTPage = RGB(0,0,125);
static const COLORREF c_BothPage = RGB(0,0,0);
static const COLORREF c_Empty = RGB(255,255,255);
static const COLORREF c_BTPastEOF = RGB(210,210,210);

class CBTDialog : public CNDBViewChildDlg
{
	DECLARE_DYNAMIC(CBTDialog)

public:
	CBTDialog(NDBViewer * pNDBViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent = NULL);   // standard constructor
	virtual ~CBTDialog();

// Dialog Data
	enum { IDD = IDD_AMAP_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	NDBViewer * m_pNDBViewer;
	CBitmap * m_pBTMap;
	DECLARE_MESSAGE_MAP()
};
