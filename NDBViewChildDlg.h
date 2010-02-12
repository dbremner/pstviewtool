#pragma once
#include "afxwin.h"
#include "stdafx.h"
#include "NDBViewDlg.h"

class CNDBViewChildDlg : public CDialog
{
public:
	CNDBViewChildDlg(CNDBViewDlg * pNDBViewDlg, UINT nIDTemplate, CWnd* pParent = NULL);
	virtual ~CNDBViewChildDlg(void);
	virtual INT_PTR DoModal();

protected:
	virtual void PostNcDestroy();
	virtual void OnCancel();
	virtual void OnOK();

	bool m_fDoModal;
	CNDBViewDlg * m_pNDBViewDlg;
};
