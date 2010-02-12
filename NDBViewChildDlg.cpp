#include "StdAfx.h"
#include "NDBViewChildDlg.h"

CNDBViewChildDlg::CNDBViewChildDlg(CNDBViewDlg * pNDBViewDlg, UINT nIDTemplate, CWnd* pParent)
	: CDialog(nIDTemplate, pParent),
	m_pNDBViewDlg(pNDBViewDlg),
	m_fDoModal(false)
{
	m_pNDBViewDlg->AddChildDialog(this);
}

CNDBViewChildDlg::~CNDBViewChildDlg(void)
{
}

void CNDBViewChildDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	if(!m_fDoModal)
	{
		m_pNDBViewDlg->ChildDialogClosed(this);
		delete this;
	}
}

INT_PTR CNDBViewChildDlg::DoModal()
{
	m_fDoModal = true;
	return CDialog::DoModal();
}

void CNDBViewChildDlg::OnCancel()
{
	CDialog::OnCancel();
	if(!m_fDoModal)
		DestroyWindow();
}

void CNDBViewChildDlg::OnOK()
{
	// eat OnOK, so it doesn't dismiss child dialogs
}
