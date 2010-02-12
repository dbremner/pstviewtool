#pragma once
#include "customcontrols.h"
#include "NDBViewChildDlg.h"
#include "afxcmn.h"

// CAMRResumeDlg dialog

class CAMRResumeDlg : public CNDBViewChildDlg
{
	DECLARE_DYNAMIC(CAMRResumeDlg)

public:
	CAMRResumeDlg(NDBViewer * pNDBViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent = NULL);
	virtual ~CAMRResumeDlg();

// Dialog Data
	enum { IDD = IDD_AMRI };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	NDBViewer * m_pNDBViewer;

public:
	CFileRangeControl m_frc;
public:
	CListCtrl m_clc;
	CImageList m_il;
public:
	virtual BOOL OnInitDialog();
};
