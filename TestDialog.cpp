// TestDialog.cpp : implementation file
//

#include "stdafx.h"
#include "NDBView.h"
#include "TestDialog.h"


// CTestDialog dialog

IMPLEMENT_DYNAMIC(CTestDialog, CDialog)

CTestDialog::CTestDialog(const BREF& bref, CB cb, NDBViewer * pNDBViewer, CWnd* pParent)
	: CDialog(CTestDialog::IDD, pParent), m_btc(bref, cb, pNDBViewer), m_cfrc(0, pNDBViewer->GetHeader().root.ibFileEof, pNDBViewer)
{
	m_data = new BYTE[(UINT)cb];
	m_cbData = (UINT)cb;
	pNDBViewer->ReadBlock(m_data, m_cbData, NULL, bref.ib, m_cbData);
	pNDBViewer->DecodeBlockInPlace(m_data, m_cbData, bref.bid);
	m_ibStart = bref.ib;

	m_cfrc.AddRange(bref.ib, bref.ib+cb, TYPE_BLOCK);
}

CTestDialog::~CTestDialog()
{
	delete [] m_data;
}

void CTestDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CUSTOM1, m_btc);
	DDX_Control(pDX, IDC_LIST3, m_hexListBox);
	DDX_Control(pDX, IDC_CUSTOM2, m_cfrc);
}


BEGIN_MESSAGE_MAP(CTestDialog, CDialog)
	ON_BN_CLICKED(IDC_RADIO2, &CTestDialog::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO1, &CTestDialog::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO3, &CTestDialog::OnBnClickedRadio3)
END_MESSAGE_MAP()


// CTestDialog message handlers

BOOL CTestDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	CheckRadioButton(IDC_RADIO1, IDC_RADIO3, IDC_RADIO1);
	m_hexListBox.SetData(m_data, m_cbData, m_ibStart);
	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CTestDialog::OnBnClickedRadio2()
{
	m_hexListBox.SetMode(MODE_BINARY);
}

void CTestDialog::OnBnClickedRadio1()
{
	m_hexListBox.SetMode(MODE_HEX);
}

void CTestDialog::OnBnClickedRadio3()
{
	m_hexListBox.SetMode(MODE_UNICODE);
}
