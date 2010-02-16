// Inspectors.cpp : implementation file
//

#include "stdafx.h"
#include "NDBView.h"
#include "Inspectors.h"

// InspectorPicker dialog

IMPLEMENT_DYNAMIC(InspectorPicker, CDialog)

InspectorPicker::InspectorPicker(CWnd* pParent /*=NULL*/)
	: CDialog(InspectorPicker::IDD, pParent)
	, m_start(0)
	, m_length(0)
	, m_type(0)
{
}

InspectorPicker::~InspectorPicker()
{
}

void InspectorPicker::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT2, m_start);
	DDX_Text(pDX, IDC_EDIT3, m_length);
	DDX_CBIndex(pDX, IDC_COMBO1, m_type);
	DDX_Control(pDX, IDC_COMBO1, m_typepick);
	DDX_Control(pDX, IDC_EDIT3, m_sizecontrol);
}

BOOL InspectorPicker::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add list of page inspectors to the drop down box
	for(int i = 0; i < inspMaxInspector; i++)
	{
		m_typepick.InsertString(i, listvalues[i]);
	}
	m_typepick.SetCurSel(0);

	return true;
}

BEGIN_MESSAGE_MAP(InspectorPicker, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO1, &InspectorPicker::OnCbnSelchangeCombo1)
END_MESSAGE_MAP()


// InspectorPicker message handlers

void InspectorPicker::OnCbnSelchangeCombo1()
{
	if(m_typepick.GetCurSel() > inspBinaryData)
	{
		m_sizecontrol.SetWindowText(L"512");
		m_sizecontrol.EnableWindow(FALSE);
	} 
	else
	{
		m_sizecontrol.EnableWindow();
	}
}
// Inspectors.cpp : implementation file
//

#include "stdafx.h"
#include "NDBView.h"
#include "Inspectors.h"


// CBinaryInspectorBase dialog

IMPLEMENT_DYNAMIC(CBinaryInspectorBase, CDialog)

CBinaryInspectorBase::CBinaryInspectorBase(NDBViewer * pNDBViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent)
	: CNDBViewChildDlg(pNDBViewDlg, CBinaryInspector::IDD, pParent),
	m_frc(0, pNDBViewer->GetHeader().root.ibFileEof, pNDBViewer),
	m_blb()
{
	m_pNDBViewer = pNDBViewer;
	m_fDoModal = false;
}

CBinaryInspectorBase::~CBinaryInspectorBase()
{
}

void CBinaryInspectorBase::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CUSTOM1, m_frc);
	DDX_Control(pDX, IDC_LIST1, m_blb);
}


BEGIN_MESSAGE_MAP(CBinaryInspectorBase, CDialog)
	ON_BN_CLICKED(IDC_RADIO1, &CBinaryInspectorBase::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CBinaryInspectorBase::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO3, &CBinaryInspectorBase::OnBnClickedRadio3)
END_MESSAGE_MAP()


// CBinaryInspector message handlers

BOOL CBinaryInspectorBase::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	SetBinaryData();
	SetupFRC();
	SetWindowTitle();

	CheckRadioButton(IDC_RADIO1, IDC_RADIO3, IDC_RADIO1);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CBinaryInspectorBase::OnBnClickedRadio1()
{
	m_blb.SetMode(MODE_HEX);
}

void CBinaryInspectorBase::OnBnClickedRadio2()
{
	m_blb.SetMode(MODE_BINARY);
}

void CBinaryInspectorBase::OnBnClickedRadio3()
{
	m_blb.SetMode(MODE_UNICODE);
}

CBinaryInspector::CBinaryInspector(IB ib, UINT cb, NDBViewer * pNDBViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent)
	: CBinaryInspectorBase(pNDBViewer, pNDBViewDlg, pParent)
{
	m_cb = cb;
	m_ib = ib;
}


CBinaryInspector::~CBinaryInspector()
{
}

void CBinaryInspector::SetWindowTitle()
{
	WCHAR windowTitle[255];
	wsprintf(windowTitle, L"Binary Data Inspector (%I64u-%I64u)", m_ib, m_ib+m_cb);
	SetWindowText(windowTitle);
}

void CBinaryInspector::SetBinaryData()
{
	BYTE * pData = new BYTE[m_cb];
	m_pNDBViewer->ReadData(pData, m_cb, NULL, 0, m_ib, (UINT)m_cb);
	m_blb.SetData(pData, m_cb, m_ib);
	delete [] pData;
}

void CBinaryInspector::SetupFRC()
{
	m_frc.AddRange(m_ib, m_ib+m_cb, TYPE_BLOCK);
}

CLogicalBlockInspector::CLogicalBlockInspector(NID nidParent, NID nid, NDBViewer * pNDBViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent)
	: CBinaryInspectorBase(pNDBViewer, pNDBViewDlg, pParent)
{
	m_cb = (UINT)pNDBViewer->NODE_GetSize(nidParent, nid);
	m_nid = nid;
	m_nidParent = nidParent;
}

CLogicalBlockInspector::~CLogicalBlockInspector()
{
}

void CLogicalBlockInspector::SetBinaryData()
{
	BYTE * pData = new BYTE[m_cb];
	m_pNDBViewer->NODE_ReadData(m_nidParent, m_nid, pData, m_cb, 0, m_cb);
	m_blb.SetData(pData, m_cb, 0);
	delete [] pData;
}


void CLogicalBlockInspector::SetupFRC()
{
}

void CLogicalBlockInspector::SetWindowTitle()
{
	WCHAR windowTitle[255];
	wsprintf(windowTitle, L"Element Stream Inspector (NID: 0x%X, Parent NID: 0x%X)", m_nid, m_nidParent);
	SetWindowText(windowTitle);
}

CHIDInspector::CHIDInspector(NID nidParent, NID nid, HID hid, LTPViewer * pLTPViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent)
	: CBinaryInspectorBase(pLTPViewer, pNDBViewDlg, pParent)
{
	m_cb = (UINT)pLTPViewer->NODE_GetSize(nidParent, nid);
	m_nid = nid;
	m_nidParent = nidParent;
	m_hid = hid;
}

CHIDInspector::~CHIDInspector()
{
}

void CHIDInspector::SetBinaryData()
{
	BYTE * pData = NULL;
	UINT cb = 0;
	cb = ((LTPViewer*)m_pNDBViewer)->HN_ReadHID(m_nidParent, m_nid, m_hid, &pData);
	m_blb.SetData(pData, cb, 0);
	delete [] pData;
}


void CHIDInspector::SetupFRC()
{
}

void CHIDInspector::SetWindowTitle()
{
	WCHAR windowTitle[255];
	wsprintf(windowTitle, L"HID Inspector (HID: 0x%X, NID: 0x%X, Parent NID: 0x%X)", m_hid, m_nid, m_nidParent);
	SetWindowText(windowTitle);
}

// Inspectors.cpp : implementation file
//

#include "stdafx.h"
#include "NDBView.h"
#include "Inspectors.h"


// CBlockInspector dialog

IMPLEMENT_DYNAMIC(CBlockInspector, CDialog)

CBlockInspector::CBlockInspector(const BREF& bref, UINT cb, NDBViewer * pNDBViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent /*=NULL*/)
	: CNDBViewChildDlg(pNDBViewDlg, CBlockInspector::IDD, pParent),
	m_btc(bref, cb, pNDBViewer),
	m_frc(0, pNDBViewer->GetHeader().root.ibFileEof, pNDBViewer)
{
	m_bref = bref;
	m_cb = BBufferSize(cb);
	m_pNDBViewer = pNDBViewer;
}

CBlockInspector::~CBlockInspector()
{
}

void CBlockInspector::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CUSTOM3, m_frc);
	DDX_Control(pDX, IDC_CUSTOM4, m_btc);
	DDX_Control(pDX, IDC_LIST1, m_blb);
}


BEGIN_MESSAGE_MAP(CBlockInspector, CDialog)
	ON_BN_CLICKED(IDC_RADIO1, &CBlockInspector::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CBlockInspector::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO3, &CBlockInspector::OnBnClickedRadio3)
END_MESSAGE_MAP()


// CBlockInspector message handlers

void CBlockInspector::OnBnClickedRadio1()
{
	m_blb.SetMode(MODE_HEX);
}

void CBlockInspector::OnBnClickedRadio2()
{
	m_blb.SetMode(MODE_BINARY);
}

void CBlockInspector::OnBnClickedRadio3()
{
	m_blb.SetMode(MODE_UNICODE);
}

BOOL CBlockInspector::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	BYTE * pData = new BYTE[m_cb];
	BLOCKTRAILER bt;
	WCHAR windowTitle[255];

	m_pNDBViewer->ReadBlock(pData, m_cb, &bt, m_bref.ib, CbAlignDisk(m_cb));

	if(m_bref.bid == 0)
		m_bref.bid = bt.bid;

	ASSERT(m_bref.bid == bt.bid);

	if(BIDIsExternal(m_bref.bid))
	{
		m_pNDBViewer->DecodeBlockInPlace(pData, m_cb, m_bref.bid);
		if(m_pNDBViewer->GetHeader().bCryptMethod != NDB_CRYPT_NONE)
			GetDlgItem(IDC_STATIC)->SetWindowText(L"(Decoded)");
		else
			GetDlgItem(IDC_STATIC)->SetWindowText(L"(Not Encoded)");
	}
	else
	{
		GetDlgItem(IDC_STATIC)->SetWindowText(L"(Internal)");
	}

	m_blb.SetData(pData, m_cb, m_bref.ib);
	m_frc.AddRange(m_bref.ib, m_bref.ib+m_cb, TYPE_BLOCK);

	CheckRadioButton(IDC_RADIO1, IDC_RADIO3, IDC_RADIO1);
	
	wsprintf(windowTitle, L"Block Inspector (BID: 0x%I64X)", m_bref.bid);
	SetWindowText(windowTitle);

	delete [] pData;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
// Inspectors.cpp : implementation file
//

#include "stdafx.h"
#include "NDBView.h"
#include "Inspectors.h"


// CPageInspector dialog

IMPLEMENT_DYNAMIC(CPageInspector, CDialog)

CPageInspector::CPageInspector(IB ib, NDBViewer * pNDBViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent /*=NULL*/)
	: CNDBViewChildDlg(pNDBViewDlg, CPageInspector::IDD, pParent),
	m_frc(0, pNDBViewer->GetHeader().root.ibFileEof, pNDBViewer),
	m_ptc(ib, pNDBViewer),
	m_blb()
{
	m_ib = ib;
	m_pNDBViewer = pNDBViewer;
}

CPageInspector::~CPageInspector()
{
}

void CPageInspector::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CUSTOM1, m_frc);
	DDX_Control(pDX, IDC_CUSTOM2, m_ptc);
	DDX_Control(pDX, IDC_LIST1, m_blb);
}


BEGIN_MESSAGE_MAP(CPageInspector, CDialog)
	ON_BN_CLICKED(IDC_RADIO1, &CPageInspector::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CPageInspector::OnBnClickedRadio2)
END_MESSAGE_MAP()


// CPageInspector message handlers

void CPageInspector::OnBnClickedRadio1()
{
	m_blb.SetMode(MODE_HEX);
}

void CPageInspector::OnBnClickedRadio2()
{
	m_blb.SetMode(MODE_BINARY);
}

BOOL CPageInspector::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	BYTE rgData[cbPage];
	PAGETRAILER pt;
	WCHAR windowTitle[255];

	m_pNDBViewer->ReadPage(rgData, cbPageData, &pt, m_ib);

	m_blb.SetData(rgData, cbPageData, m_ib);
	m_frc.AddRange(m_ib, m_ib+cbPage, TYPE_PAGE);

	CheckRadioButton(IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
	
	wsprintf(windowTitle, L"Page Inspector (IB: %I64u)", m_ib);
	SetWindowText(windowTitle);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
// Inspectors.cpp : implementation file
//

#include "stdafx.h"
#include "NDBView.h"
#include "Inspectors.h"


// CXBlockInspector dialog

IMPLEMENT_DYNAMIC(CXBlockInspector, CDialog)

CXBlockInspector::CXBlockInspector(const BREF& bref, UINT cb, NDBViewer * pNDBViewer, CNDBViewDlg * pNDBDlg, CWnd* pParent)
	: CNDBViewChildDlg(pNDBDlg, CXBlockInspector::IDD, pParent),
	m_btc(bref, cb, pNDBViewer),
	m_frc(0, pNDBViewer->GetHeader().root.ibFileEof, pNDBViewer)
{
	m_bref = bref;
	m_cbUnaligned = BBufferSize(cb);
	m_cbAligned = CbAlignDisk(cb);
	m_pNDBViewer = pNDBViewer;
	m_pData = NULL;
}

CXBlockInspector::~CXBlockInspector()
{
	if(m_pData)
		delete [] m_pData;
}

void CXBlockInspector::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CUSTOM4, m_btc);
	DDX_Control(pDX, IDC_CUSTOM3, m_frc);
	DDX_Control(pDX, IDC_LIST1, m_lb);
}


BEGIN_MESSAGE_MAP(CXBlockInspector, CDialog)
	ON_LBN_DBLCLK(IDC_LIST1, &CXBlockInspector::OnLbnDblclkList1)
END_MESSAGE_MAP()


// CXBlockInspector message handlers

BOOL CXBlockInspector::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_pData = new BYTE[m_cbUnaligned];
	BLOCKTRAILER bt;
	XBLOCK * pxb = (XBLOCK*)m_pData;
	WCHAR buffer[255];

	m_pNDBViewer->ReadBlock(m_pData, m_cbUnaligned, &bt, m_bref.ib, (UINT)m_cbAligned);
	
	m_frc.AddRange(m_bref.ib, m_bref.ib+m_cbAligned, TYPE_BLOCK);

	// if we weren't told a bid, trust the block trailer
	if(m_bref.bid == 0)
		m_bref.bid = bt.bid;

	GetDlgItem(IDC_LABELS)->SetWindowText(L"btype:\n\ncLevel:\n\ncEnt:\n\nlcbTotal:");
	wsprintf(buffer, L"%u (%s)\n\n%u\n\n%u\n\n%u", pxb->btype, BTYPESTRING(pxb->btype), pxb->cLevel, pxb->cEnt, pxb->lcbTotal);
	GetDlgItem(IDC_VALUES)->SetWindowText(buffer);

	for(UINT i = 0; i < XBEnt(*pxb, m_cbUnaligned); i++)
	{
		wsprintf(buffer, L"BID[%u]: 0x%I64X", i, pxb->rgbid[i]);
		m_lb.AddString(buffer);
	}

	wsprintf(buffer, L"XBlock Inspector (BID: 0x%I64X)", m_bref.bid);
	SetWindowText(buffer);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CXBlockInspector::OnLbnDblclkList1()
{
	BID bid = ((XBLOCK*)m_pData)->rgbid[ m_lb.GetCurSel() ];
	NodeData n;
	UINT cRefs;

	n.btkey = 0;
	n.bref.bid = bid;
	n.ulFlags = aBrowseBBT;
	if(m_pNDBViewer->LookupBID(bid, n.bref, n.cb, cRefs))
	{
		n.ulFlags |= aOpenBlock;
	}

	m_pNDBViewDlg->PerformAction(&n);
}
// Inspectors.cpp : implementation file
//

#include "stdafx.h"
#include "NDBView.h"
#include "Inspectors.h"


// CSBlockInspector dialog

IMPLEMENT_DYNAMIC(CSBlockInspector, CDialog)

CSBlockInspector::CSBlockInspector(const BREF& bref, UINT cb, NDBViewer * pNDBViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent)
	: CNDBViewChildDlg(pNDBViewDlg, CSBlockInspector::IDD, pParent),
	m_btc(bref, cb, pNDBViewer),
	m_frc(0, pNDBViewer->GetHeader().root.ibFileEof, pNDBViewer)
{
	m_bref = bref;
	m_cbAligned = CbAlignDisk(cb);
	m_cbUnaligned = BBufferSize(cb);
	m_pNDBViewer = pNDBViewer;
	m_pData = NULL;
}

CSBlockInspector::~CSBlockInspector()
{
	if(m_pData)
		delete [] m_pData;
}

void CSBlockInspector::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CUSTOM3, m_frc);
	DDX_Control(pDX, IDC_CUSTOM4, m_btc);
	DDX_Control(pDX, IDC_TREE1, m_tc);
}


BEGIN_MESSAGE_MAP(CSBlockInspector, CDialog)
	ON_NOTIFY(TVN_DELETEITEM, IDC_TREE1, &CSBlockInspector::OnTvnDeleteitemTree1)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE1, &CSBlockInspector::OnTvnItemexpandingTree1)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE1, &CSBlockInspector::OnNMDblclkTree1)
END_MESSAGE_MAP()


// CSBlockInspector message handlers

BOOL CSBlockInspector::OnInitDialog()
{
	CBitmap cb;
	CDialog::OnInitDialog();

	cb.LoadBitmap(IDB_BITMAP2);
	m_il.Create(16, 16, ILC_COLOR24, 10, 1);
	m_il.Add(&cb, RGB(255, 0, 255));
	m_il.SetBkColor(CLR_NONE);
	m_tc.SetImageList(&m_il, TVSIL_NORMAL);

	// TODO:  Add extra initialization here
	m_pData = new BYTE[m_cbUnaligned];
	BLOCKTRAILER bt;
	SBLOCK * psb = (SBLOCK*)m_pData;
	WCHAR buffer[255];

	m_pNDBViewer->ReadBlock(m_pData, m_cbUnaligned, &bt, m_bref.ib, (UINT)m_cbAligned);
	
	// If we're not told a BID, trust the block trailer
	if(m_bref.bid == 0)
		m_bref.bid = bt.bid;

	m_frc.AddRange(m_bref.ib, m_bref.ib+m_cbAligned, TYPE_BLOCK);

	GetDlgItem(IDC_LABELS)->SetWindowText(L"btype:\n\ncLevel:\n\ncEnt:");
	wsprintf(buffer, L"%u (%s)\n\n%u\n\n%u", psb->btype, BTYPESTRING(psb->btype), psb->cLevel, psb->cEnt);
	GetDlgItem(IDC_VALUES)->SetWindowText(buffer);

	m_pNDBViewer->PopulateSBlock(&m_tc, m_bref, m_cbUnaligned);

	wsprintf(buffer, L"SBlock Inspector (BID: 0x%I64X)", m_bref.bid);
	SetWindowText(buffer);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSBlockInspector::OnTvnDeleteitemTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	
	if(pNMTreeView->hdr.code == TVN_DELETEITEM)
	{
		m_pNDBViewer->DeleteNode(&m_tc, pNMTreeView->itemOld.hItem);
	}

	*pResult = 0;
}

void CSBlockInspector::OnTvnItemexpandingTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	// TODO: Add your control notification handler code here
	if(pNMTreeView->hdr.code == TVN_ITEMEXPANDING)
	{
		m_pNDBViewer->ExpandSBlockNode(&m_tc, pNMTreeView->itemNew.hItem);
	}

	*pResult = 0;
}

void CSBlockInspector::OnNMDblclkTree1(NMHDR *, LRESULT *pResult)
{
	HTREEITEM hItem = m_tc.GetSelectedItem();
	NodeData * pNodeData = NULL;
	
	// Do we have an item?
	if(hItem)
	{
		// Does it have data?
		pNodeData = (NodeData *)m_tc.GetItemData(hItem);
	}

	// don't bother opening another inspector that is a duplicate of us
	if(pNodeData && !((pNodeData->ulFlags & aOpenBlock) && (pNodeData->bref.ib == m_bref.ib)))
		m_pNDBViewDlg->PerformAction(pNodeData);

	*pResult = 0;
}

// Inspectors.cpp : implementation file
//

#include "stdafx.h"
#include "NDBView.h"
#include "Inspectors.h"


// CBTPageInspector dialog

IMPLEMENT_DYNAMIC(CBTPageInspector, CDialog)

CBTPageInspector::CBTPageInspector(IB ib, NDBViewer * pNDBViewer, CNDBViewDlg * pNDBDlg, CWnd* pParent)
	: CNDBViewChildDlg(pNDBDlg, CBTPageInspector::IDD, pParent),
	m_frc(0,pNDBViewer->GetHeader().root.ibFileEof, pNDBViewer),
	m_ptc(ib, pNDBViewer)
{
	m_pNDBViewer = pNDBViewer;
	m_ib = ib;
}

CBTPageInspector::~CBTPageInspector()
{
}

void CBTPageInspector::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CUSTOM3, m_frc);
	DDX_Control(pDX, IDC_CUSTOM4, m_ptc);
	DDX_Control(pDX, IDC_TREE1, m_tc);
}


BEGIN_MESSAGE_MAP(CBTPageInspector, CDialog)
	ON_NOTIFY(TVN_DELETEITEM, IDC_TREE1, &CBTPageInspector::OnTvnDeleteitemTree1)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE1, &CBTPageInspector::OnNMDblclkTree1)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE1, &CBTPageInspector::OnTvnItemexpandingTree1)
END_MESSAGE_MAP()


// CBTPageInspector message handlers

BOOL CBTPageInspector::OnInitDialog()
{
	CBitmap cb;
	CDialog::OnInitDialog();

	cb.LoadBitmap(IDB_BITMAP2);
	m_il.Create(16, 16, ILC_COLOR24, 10, 1);
	m_il.Add(&cb, RGB(255, 0, 255));
	m_il.SetBkColor(CLR_NONE);
	m_tc.SetImageList(&m_il, TVSIL_NORMAL);
	BTPAGE btPage;
	WCHAR buffer[255];

	m_pNDBViewer->ReadPage((BYTE*)&btPage, sizeof(btPage), &m_pt, m_ib);
	m_frc.AddRange(m_ib, m_ib+cbPage, TYPE_PAGE);

	GetDlgItem(IDC_LABELS)->SetWindowText(L"cEnt:\n\ncEntMax:\n\ncbEnt:\n\ncLevel:");
	wsprintf(buffer, L"%u\n\n%u\n\n%u\n\n%u", btPage.cEnt, btPage.cEntMax, btPage.cbEnt, btPage.cLevel);
	GetDlgItem(IDC_VALUES)->SetWindowText(buffer);

	// Setup the root node
	HTREEITEM hItem = NULL;
	NodeData * pNodeData = NULL;

	// Add root node to tree
	wsprintf(buffer, L"Page BID: 0x%I64X, IB: %I64u", m_pt.bid, m_ib);
	hItem = m_tc.InsertItem(buffer, TVI_ROOT);
	pNodeData = new NodeData;
	pNodeData->bref.ib = m_ib;
	pNodeData->bref.bid = m_pt.bid;
	pNodeData->btkey = 0;
	pNodeData->cb = 512;
	pNodeData->ulFlags = aOpenBTPage;
	m_tc.SetItemData(hItem, (DWORD_PTR)pNodeData);
	m_tc.SetItemImage(hItem, iconPage, iconPage);
	AddFalseChild(&m_tc, hItem);

	// Add children of root node
	m_pNDBViewer->AddBTNodeChildren(&m_tc, hItem);
	
	wsprintf(buffer, L"BTPage Inspector (IB: %I64u)", m_ib);
	SetWindowText(buffer);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CBTPageInspector::OnTvnDeleteitemTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	
	if(pNMTreeView->hdr.code == TVN_DELETEITEM)
	{
		m_pNDBViewer->DeleteNode(&m_tc, pNMTreeView->itemOld.hItem);
	}

	*pResult = 0;
}

void CBTPageInspector::OnNMDblclkTree1(NMHDR*, LRESULT *pResult)
{
	HTREEITEM hItem = m_tc.GetSelectedItem();
	NodeData * pNodeData = NULL;

	// Do we have an item?
	if(hItem)
	{
		// Does it have data?
		pNodeData = (NodeData *)m_tc.GetItemData(hItem);
	}

	// don't bother opening another inspector that is a duplicate of us
	if(pNodeData && !((pNodeData->ulFlags & aOpenBTPage) && (pNodeData->bref.ib == m_ib)))
		m_pNDBViewDlg->PerformAction(pNodeData);

	*pResult = 0;
}

void CBTPageInspector::OnTvnItemexpandingTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	// TODO: Add your control notification handler code here
	if(pNMTreeView->hdr.code == TVN_ITEMEXPANDING)
	{
		m_pNDBViewer->AddBTNodeChildren(&m_tc, pNMTreeView->itemNew.hItem);
	}

	*pResult = 0;
}


// CScavengeRange dialog

IMPLEMENT_DYNAMIC(CScavengeRange, CDialog)

CScavengeRange::CScavengeRange(CWnd* pParent /*=NULL*/)
	: CDialog(CScavengeRange::IDD, pParent)
	, m_length(0)
	, m_start(0)
	, m_fFreeSpaceOnly(0)
{

}

CScavengeRange::~CScavengeRange()
{
}

void CScavengeRange::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT2, m_start);
	DDX_Text(pDX, IDC_EDIT3, m_length);
	DDX_Check(pDX, IDC_CHECK1, m_fFreeSpaceOnly);
}


BEGIN_MESSAGE_MAP(CScavengeRange, CDialog)
END_MESSAGE_MAP()


// CScavengeRange message handlers
// Inspectors.cpp : implementation file
//

#include "stdafx.h"
#include "NDBView.h"
#include "Inspectors.h"


// CScavengeResults dialog

IMPLEMENT_DYNAMIC(CScavengeResults, CDialog)

CScavengeResults::CScavengeResults(IB start, UINT cb, bool fFreeOnly, NDBViewer * pNDBViewer, CNDBViewDlg * pNDBDlg, CWnd* pParent)
	: CNDBViewChildDlg(pNDBDlg, CScavengeResults::IDD, pParent),
	m_frc(start, start+cb, pNDBViewer)
{
	m_start = start;
	m_length = cb;
	m_fFreeOnly = fFreeOnly;
	m_pNDBViewer = pNDBViewer;
}

CScavengeResults::~CScavengeResults()
{
}

void CScavengeResults::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CUSTOM3, m_frc);
	DDX_Control(pDX, IDC_LIST1, m_lc);
}


BEGIN_MESSAGE_MAP(CScavengeResults, CDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &CScavengeResults::OnLvnItemchangedList1)
	ON_NOTIFY(LVN_DELETEITEM, IDC_LIST1, &CScavengeResults::OnLvnDeleteitemList1)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CScavengeResults::OnNMDblclkList1)
END_MESSAGE_MAP()


// CScavengeResults message handlers

BOOL CScavengeResults::OnInitDialog()
{
	CBitmap cb;
	bool fAllNull = true;
	CDialog::OnInitDialog();

	cb.LoadBitmap(IDB_BITMAP2);
	m_il.Create(16, 16, ILC_COLOR24, 10, 1);
	m_il.Add(&cb, RGB(255, 0, 255));
	m_il.SetBkColor(CLR_NONE);
	m_lc.SetImageList(&m_il, LVSIL_SMALL);

	// Add, size columns
	RECT r;
	m_lc.GetClientRect(&r);
	m_lc.InsertColumn(0, L"Type", LVCFMT_LEFT, r.right / 4);
	m_lc.InsertColumn(1, L"BID", LVCFMT_LEFT, r.right / 4);
	m_lc.InsertColumn(2, L"Range", LVCFMT_LEFT, r.right / 2);

	// Do the actual scavenge
	PAGETRAILER pt;
	BLOCKTRAILER bt;
	WCHAR buffer[255];
	int count = 0;
	int nImage = 0;
	IB start;
	BOOL fCRCValid = false;

	for(IB end = CbAlignSlotIB(m_start) + cbPerSlot; end < (m_start + m_length + 8192L); end += cbPerSlot)
	{
		if(m_fFreeOnly && !m_pNDBViewer->FFree(end - cbPerSlot))
			continue;

		if(m_fFreeOnly && fAllNull)
		{
			BYTE rgbSlot[cbPerSlot];
			m_pNDBViewer->ReadData(rgbSlot, cbPerSlot, NULL, 0, end - cbPerSlot, cbPerSlot);

			for(int i = 0; i < cbPerSlot; i++)
			{
				if(rgbSlot[i] != 0x00)
					fAllNull = false;
			}
		}
		// Is this a valid page trailer?
		m_pNDBViewer->ReadData((BYTE*)&pt, sizeof(pt), NULL, 0, end-sizeof(pt), sizeof(pt));
		if(	
			(pt.ptype == pt.ptypeRepeat) && 
			(pt.ptype >= ptypeBBT) &&
			(pt.bid > 0)
		)
		{
			bool validSig = (ComputeSig(end-cbPage, pt.bid) == pt.wSig);
			// close enough
			start = end - cbPage;
			
			if((start < (m_start + m_length)) && end > m_start)
			{
				BYTE * pPage = new BYTE[cbPageData];

				m_pNDBViewer->ReadData(pPage, cbPageData, NULL, 0, end-cbPage, cbPage);
				fCRCValid = (ComputeCRC(pPage, cbPageData) == pt.dwCRC);

				if(fCRCValid || validSig)
				{
					NodeData * pNodeData = new NodeData;
					nImage = fCRCValid ? iconPage : iconCorrupt;

					pNodeData->bref.ib = start;
					pNodeData->bref.bid = pt.bid;
					pNodeData->cb = 512;
					pNodeData->ulFlags = aOpenBTPage;

					m_frc.AddRange(start, end, TYPE_PAGE);
					m_lc.InsertItem(count++, L"Page", nImage);
					m_lc.SetItemData(count-1, (DWORD_PTR)pNodeData);

					wsprintf(buffer, L"0x%I64X", pt.bid);
					m_lc.SetItemText(count-1, 1, buffer);

					wsprintf(buffer, L"%I64u - %I64u (%u bytes)", start, end, (ULONG)(end - start));
					m_lc.SetItemText(count-1, 2, buffer);
				}
				delete [] pPage;
			}

			continue;
		}

		// Is this a valid block trailer?
		m_pNDBViewer->ReadData((BYTE*)&bt, sizeof(bt), NULL, 0, end-sizeof(bt), sizeof(bt));
		if(
			(bt.cb <= 8192L) && 
			(bt.cb > 0) &&
			(bt.bid >= bidIncrement)
		)
		{
			bool validSig = (ComputeSig(end-CbAlignDisk(bt.cb), bt.bid) == bt.wSig);
			// close enough..
			start = end - CbAlignDisk(bt.cb);

			if((start < (m_start + m_length)) && end > m_start)
			{
				BYTE * pBlock = new BYTE[BBufferSize(bt.cb)];

				m_pNDBViewer->ReadData(pBlock, BBufferSize(bt.cb), NULL, 0, end-CbAlignDisk(bt.cb), CbAlignDisk(bt.cb));
				fCRCValid = (ComputeCRC(pBlock, BBufferSize(bt.cb)) == bt.dwCRC);

				if(validSig || fCRCValid)
				{
					NodeData * pNodeData = new NodeData;

					nImage = fCRCValid ? (BIDIsInternal(bt.bid) ? iconInternalBlock : iconBlock) : iconCorrupt;

					pNodeData->bref.ib = start;
					pNodeData->bref.bid = bt.bid;
					pNodeData->cb = bt.cb;
					pNodeData->ulFlags = aOpenBlock | aBrowseBBT;

					m_frc.AddRange(start, end, TYPE_BLOCK);
					m_lc.InsertItem(count++, L"Block", nImage);
					m_lc.SetItemData(count-1, (DWORD_PTR)pNodeData);

					wsprintf(buffer, L"0x%I64X", bt.bid);
					m_lc.SetItemText(count-1, 1, buffer);

					wsprintf(buffer, L"%I64u - %I64u (%u bytes)", start, end, (ULONG)(end - start));
					m_lc.SetItemText(count-1, 2, buffer);
				}
				
				delete [] pBlock;
			}
		}
	}

	if(m_fFreeOnly && fAllNull)
	{
		GetDlgItem(IDC_STATIC)->SetWindowTextW(L"The specified range (only free space) contained only null data");
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CScavengeResults::OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	NodeData * pNodeData = (NodeData*)m_lc.GetItemData(pNMLV->iItem);
	
	ASSERT(pNodeData);

	if(pNodeData && pNodeData->ulFlags & aOpenBlock) 
	{
		// it's a block...
		m_frc.SelectRange(pNodeData->bref.ib, pNodeData->bref.ib+CbAlignDisk(pNodeData->cb), TYPE_BLOCK);
	}
	else if(pNodeData)
	{
		// it's a page
		m_frc.SelectRange(pNodeData->bref.ib, pNodeData->bref.ib + cbPage, TYPE_PAGE);
	}

	*pResult = 0;
}

void CScavengeResults::OnLvnDeleteitemList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	delete (NodeData*)m_lc.GetItemData(pNMLV->iItem);

	*pResult = 0;
}

void CScavengeResults::OnNMDblclkList1(NMHDR*, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	int i = m_lc.GetSelectionMark();

	if(i >= 0)
	{
		NodeData * pNodeData = (NodeData*)m_lc.GetItemData(i);

		m_pNDBViewDlg->PerformAction(pNodeData);
	}
	*pResult = 0;
}
