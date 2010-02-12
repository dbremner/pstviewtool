#pragma once
#include "customcontrols.h"
#include "afxwin.h"


// CTestDialog dialog

class CTestDialog : public CDialog
{
	DECLARE_DYNAMIC(CTestDialog)

public:
	CTestDialog(const BREF& bref, CB cb, NDBViewer * pNDBViewer, CWnd* pParent = NULL);   // standard constructor
	virtual ~CTestDialog();

// Dialog Data
	enum { IDD = IDD_TEST_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CBlockTrailerControl m_btc;
private:
	BYTE* m_data;
	UINT m_cbData;
	IB m_ibStart;
	CBinaryListBox m_hexListBox;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio3();
	CFileRangeControl m_cfrc;
};
