// AMRResumeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NDBView.h"
#include "AMRResumeDlg.h"


// CAMRResumeDlg dialog

IMPLEMENT_DYNAMIC(CAMRResumeDlg, CDialog)

CAMRResumeDlg::CAMRResumeDlg(NDBViewer * pNDBViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent)
	: CNDBViewChildDlg(pNDBViewDlg, CAMRResumeDlg::IDD, pParent),
	m_frc(0, pNDBViewer->GetHeader().root.ibFileEof, pNDBViewer)
{
	m_pNDBViewer = pNDBViewer;
}

CAMRResumeDlg::~CAMRResumeDlg()
{
}

void CAMRResumeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CUSTOM1, m_frc);
	DDX_Control(pDX, IDC_LIST1, m_clc);
}


BEGIN_MESSAGE_MAP(CAMRResumeDlg, CDialog)
END_MESSAGE_MAP()


// CAMRResumeDlg message handlers

BOOL CAMRResumeDlg::OnInitDialog()
{
	AMAPREBUILDRESUMEINFO amri;
	BOOL fAMapValid;
	BOOL fNBTValid;
	BOOL fBBTValid;
	BOOL fEofValid;
	CBitmap cb;
	RECT r;
	BOOL fResumeValid;
	WCHAR buffer[255];
	int count = 0;
	int nImage;

	CNDBViewChildDlg::OnInitDialog();

	cb.LoadBitmap(IDB_BITMAP2);
	m_il.Create(16, 16, ILC_COLOR24, 10, 1);
	m_il.Add(&cb, RGB(255, 0, 255));
	m_il.SetBkColor(CLR_NONE);
	m_clc.SetImageList(&m_il, LVSIL_SMALL);
	m_clc.GetClientRect(&r);

	m_clc.InsertColumn(0, L"Field", LVCFMT_LEFT, r.right / 3);
	m_clc.InsertColumn(1, L"Value", LVCFMT_LEFT, r.right / 3);
	m_clc.InsertColumn(2, L"Comment", LVCFMT_LEFT, r.right / 3);

	m_pNDBViewer->ReadData((BYTE*)&amri, sizeof(AMAPREBUILDRESUMEINFO), NULL, NULL, ibAMapBase, sizeof(AMAPREBUILDRESUMEINFO));
	fNBTValid = amri.bidNBTRoot == m_pNDBViewer->GetHeader().root.brefNBT.bid;
	fBBTValid = amri.bidBBTRoot == m_pNDBViewer->GetHeader().root.brefNBT.bid;
	fEofValid = amri.ibAMapRebuildWatermark < m_pNDBViewer->GetHeader().root.ibFileEof;
	fResumeValid = fNBTValid && fBBTValid && fEofValid;

	// Is the AMapValid?
	fAMapValid = m_pNDBViewer->GetHeader().root.fAMapValid;
	nImage = iconPass;
	m_clc.InsertItem(count++, L"root.fAMapValid", nImage);

	wsprintf(buffer, L"0x%X", m_pNDBViewer->GetHeader().root.fAMapValid);
	m_clc.SetItemText(count-1, 1, buffer);

	if(!fAMapValid)
	{
		if(fResumeValid)
		{
			m_frc.AddRange(0, amri.ibAMapRebuildWatermark, TYPE_BLOCK);
		}
		else
		{
			m_frc.AddRange(0, m_pNDBViewer->GetHeader().root.ibFileEof, TYPE_BLOCK);
		}

		// bidNBTRoot
		nImage = fNBTValid ? iconPass : iconCorrupt;
		m_clc.InsertItem(count++, L"amri.bidNBTRoot", nImage);

		wsprintf(buffer, L"0x%I64X", amri.bidNBTRoot);
		m_clc.SetItemText(count-1, 1, buffer);
		
		if(!fNBTValid)
		{
			wsprintf(buffer, L"Does not match header (0x%I64X)", m_pNDBViewer->GetHeader().root.brefNBT.bid);
			m_clc.SetItemText(count-1, 2, buffer);
		}

		// bidBBTRoot
		nImage = fBBTValid ? iconPass : iconCorrupt;
		m_clc.InsertItem(count++, L"amri.bidBBTRoot", nImage);

		wsprintf(buffer, L"0x%I64X", amri.bidBBTRoot);
		m_clc.SetItemText(count-1, 1, buffer);
		
		if(!fBBTValid)
		{
			wsprintf(buffer, L"Does not match header (0x%I64X)", m_pNDBViewer->GetHeader().root.brefBBT.bid);
			m_clc.SetItemText(count-1, 2, buffer);
		}

		// ibAMapRebuildWatermark
		nImage = fEofValid ? iconPass : iconCorrupt;
		m_clc.InsertItem(count++, L"amri.ibAMapRebuildWatermark", nImage);

		wsprintf(buffer, L"%I64u", amri.ibAMapRebuildWatermark);
		m_clc.SetItemText(count-1, 1, buffer);

		if(!fEofValid)
		{
			wsprintf(buffer, L"Greater than header value (%I64u)", m_pNDBViewer->GetHeader().root.ibFileEof);
			m_clc.SetItemText(count-1, 2, buffer);
		}
//		else
//		{
//			double d = 0.0;
//			d = (((double)(m_pNDBViewer->GetHeader().root.ibFileEof) - (double)(amri.ibAMapRebuildWatermark)) / (double)(m_pNDBViewer->GetHeader().root.ibFileEof)) * 100.0;
//			wsprintf(buffer, L"Percent Complete: %.2ld%%", d);
//			m_clc.SetItemText(count-1, 2, buffer);
//		}

		// btKey
		nImage = iconInternal;
		m_clc.InsertItem(count++, L"amri.btKeyMax", nImage);

		wsprintf(buffer, L"0x%I64X", amri.btKeyMax);
		m_clc.SetItemText(count-1, 1, buffer);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
