#pragma once

#include "NDBViewer.h"
#include "NDBViewDlg.h"
#include "NDBViewChildDlg.h"

// CAMapDialog dialog

static const COLORREF c_Free = RGB(255,255,255);
static const COLORREF c_Full = RGB(0,0,0);
static const double c_minShade = 0.15;
static const COLORREF c_FreeInvalid = RGB(230,230,230);
static const COLORREF c_FullInvalid = RGB(125,125,125);
static const COLORREF c_AMapPastEOF = RGB(210,210,210);

class CAMapDialog : public CNDBViewChildDlg
{
	DECLARE_DYNAMIC(CAMapDialog)

public:
	CAMapDialog(NDBViewer * pNDBViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent = NULL);   // standard constructor
	virtual ~CAMapDialog();

// Dialog Data
	enum { IDD = IDD_AMAP_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	NDBViewer * m_pNDBViewer;
	CBitmap * m_pAMap;

	DECLARE_MESSAGE_MAP()
};
