// LTPInspectors.cpp : implementation file
//

#include "stdafx.h"
#include "NDBView.h"
#include "LTPInspectors.h"


// HNInspector dialog

IMPLEMENT_DYNAMIC(HNInspector, CDialog)

HNInspector::HNInspector(NID nidParent, NID nid, LTPViewer * pLTPViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent)
	: CNDBViewChildDlg(pNDBViewDlg, HNInspector::IDD, pParent),
	m_frc(pLTPViewer)
{
	m_nidParent = nidParent;
	m_nid = nid;
	m_pLTPViewer = pLTPViewer;
}

HNInspector::~HNInspector()
{
}

void HNInspector::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CUSTOM1, m_frc);
	DDX_Control(pDX, IDC_LIST2, m_pageList);
	DDX_Control(pDX, IDC_LIST4, m_allocationList);
}


BEGIN_MESSAGE_MAP(HNInspector, CDialog)
	ON_LBN_SELCHANGE(IDC_LIST2, &HNInspector::OnLbnSelchangeList2)
	ON_LBN_DBLCLK(IDC_LIST2, &HNInspector::OnLbnDblclkList2)
	ON_LBN_DBLCLK(IDC_LIST4, &HNInspector::OnLbnDblclkList4)
	ON_LBN_SELCHANGE(IDC_LIST4, &HNInspector::OnLbnSelchangeList4)
END_MESSAGE_MAP()


// HNInspector message handlers

BOOL HNInspector::OnInitDialog()
{
	HNHDR	hnhdr;
	WCHAR	buffer[255];

	CNDBViewChildDlg::OnInitDialog();

	// TODO:  Add extra initialization here

	// Setup HN header info
	m_pLTPViewer->NODE_ReadData(m_nidParent, m_nid, (BYTE*)(&hnhdr), sizeof(HNHDR), 0, sizeof(HNHDR));
	GetDlgItem(IDC_LABELS)->SetWindowText(L"ibHnpm:\n\nbSig:\n\nbClientSig:\n\nhidUserRoot:");

	wsprintf(buffer, L"%u\n\n0x%X (%s)\n\n0x%X (%s)\n\n0x%X", hnhdr.ibHnpm, hnhdr.bSig, (hnhdr.bSig == HEAP_SIGNATURE ? L"Passed" : L"Failed"), hnhdr.bClientSig, GetClientMagicSigString(hnhdr.bClientSig), hnhdr.hidUserRoot);
	GetDlgItem(IDC_VALUES)->SetWindowText(buffer);

	// Setup Size Control (just the size of the node)
	m_frc.SetNewLimits(0, m_pLTPViewer->HN_HeapSize(m_nidParent, m_nid));

	// Populate Page List
	UINT cPages = m_pLTPViewer->HN_PageCount(m_nidParent, m_nid);
	for(UINT i = 0; i < cPages; i++)
	{
		BYTE pageBuffer[cbNBDataMax];
		BYTE fill = 0;
		int start, end;
		PHNPAGEHDR pHeader = NULL;
		PHNPAGEMAP pMap = NULL;

		m_pLTPViewer->HN_GetPage(m_nidParent, m_nid, i, pageBuffer, cbNBDataMax);
		fill = m_pLTPViewer->HN_GetPageFillLevel(m_nidParent, m_nid, i);

		pHeader = (PHNPAGEHDR)pageBuffer;
		pMap = (PHNPAGEMAP)(pageBuffer + pHeader->ibHnpm);

		wsprintf(buffer, L"Page %u, %s", i, GetFillLevelString(fill));
		m_pageList.AddString(buffer);

		// Add ranges to the file range control
		start = i * cbNBDataMax;
		switch(PagetFromIPage(i))
		{
		case PAGE_TYPE_FIRST:
			m_frc.AddRange(start, start + sizeof(HNHDR), TYPE_META);
			break;
		case PAGE_TYPE_NORMAL:
			m_frc.AddRange(start, start + sizeof(HNPAGEHDR), TYPE_META);
			break;
		case PAGE_TYPE_FILL_BITMAP:
			m_frc.AddRange(start, start + sizeof(HNBITMAPHDR), TYPE_META);
			break;
		}

		start = i * cbNBDataMax + pHeader->ibHnpm;
		m_frc.AddRange(start, start + sizeof(HNPAGEMAP) + (sizeof(WORD)*pMap->cAlloc), TYPE_META);

		for(int j = 0; j < pMap->cAlloc; j++)
		{
			start = i * cbNBDataMax + pMap->rgibAlloc[j];
			end = i * cbNBDataMax + pMap->rgibAlloc[j+1];
			if(start != end)
				m_frc.AddRange(start, end, TYPE_BLOCK);
		}
	}

	GetDlgItem(IDC_PAGE_INFO)->SetWindowText(L"");

	wsprintf(buffer, L"HN Inspector (NID: 0x%X, Parent NID: 0x%X)", m_nid, m_nidParent);
	SetWindowText(buffer);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void HNInspector::OnLbnSelchangeList2()
{
	int iPage = m_pageList.GetCurSel();
	BYTE pageBuffer[cbNBDataMax];
	PHNPAGEHDR pHeader = NULL;
	PHNPAGEMAP pMap = NULL;
	WCHAR buffer[255];
	CB cb = 0;

	m_allocationList.ResetContent();
	
	if(iPage >= 0)
	{

		cb = m_pLTPViewer->HN_GetPage(m_nidParent, m_nid, iPage, pageBuffer, cbNBDataMax);
		pHeader = (PHNPAGEHDR)pageBuffer;
		pMap = (PHNPAGEMAP)(pageBuffer + pHeader->ibHnpm);

		wsprintf(buffer, L"Page Type: %u (%s)\n\nibHnpm: %u\n\ncAlloc: %u\n\ncFree: %u", PagetFromIPage(iPage), GetPageTypeString(PagetFromIPage(iPage)), pHeader->ibHnpm, pMap->cAlloc, pMap->cFree);
		GetDlgItem(IDC_PAGE_INFO)->SetWindowText(buffer);

		for(int i = 0; i < pMap->cAlloc; i++)
		{
			wsprintf(buffer, L"HID 0x%X, IB %u, CB %u", MakeHID(iPage, i), pMap->rgibAlloc[i], pMap->rgibAlloc[i+1] - pMap->rgibAlloc[i]);
			m_allocationList.AddString(buffer);
		}
	}
}

void HNInspector::OnLbnDblclkList2()
{
	BID bid = m_pLTPViewer->HN_GetPageBID(m_nidParent, m_nid, m_pageList.GetCurSel());
	NodeData nd;
	BREF bref;
	CB cb;
	UINT cRef;

	ZeroMemory(&nd, sizeof(NodeData));

	m_pLTPViewer->LookupBID(bid, bref, cb, cRef);
	nd.bref = bref;
	nd.cb = cb;
	nd.ulFlags |= aOpenBlock;

	m_pNDBViewDlg->PerformAction(&nd);
}

void HNInspector::OnLbnDblclkList4()
{
	UINT uiPage = m_pageList.GetCurSel();
	UINT uiIndex = m_allocationList.GetCurSel();
	NodeData nd;

	ZeroMemory(&nd, sizeof(NodeData));
	nd.ulFlags |= aOpenHID;
	nd.nid = m_nid;
	nd.nidParent = m_nidParent;
	nd.hid = MakeHID(uiPage, uiIndex);

	m_pNDBViewDlg->PerformAction(&nd);
}

void HNInspector::OnLbnSelchangeList4()
{
	UINT uiPage = m_pageList.GetCurSel();
	UINT uiIndex = m_allocationList.GetCurSel();
	BYTE pageBuffer[cbNBDataMax];
	PHNPAGEHDR pHeader = NULL;
	PHNPAGEMAP pMap = NULL;

	m_pLTPViewer->HN_GetPage(m_nidParent, m_nid, uiPage, pageBuffer, cbNBDataMax);
	pHeader = (PHNPAGEHDR)pageBuffer;
	pMap = (PHNPAGEMAP)(pageBuffer + pHeader->ibHnpm);

	m_frc.SelectRange(uiPage*cbNBDataMax + pMap->rgibAlloc[uiIndex], uiPage*cbNBDataMax + pMap->rgibAlloc[uiIndex+1], TYPE_BLOCK);
}

// C:\projects\NDBView\NDBView\LTPInspectors.cpp : implementation file
//

#include "stdafx.h"
#include "NDBView.h"
#include "LTPInspectors.h"


// BTHInspector dialog

IMPLEMENT_DYNAMIC(BTHInspector, CDialog)

BTHInspector::BTHInspector(NID nidParent, NID nid, HID hidRoot, LTPViewer * pLTPViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent)
	: CNDBViewChildDlg(pNDBViewDlg, BTHInspector::IDD, pParent),
	m_frc(pLTPViewer)
{
	m_nidParent = nidParent;
	m_nid = nid;
	m_hidRoot = hidRoot;
	m_pLTPViewer = pLTPViewer;
}

BTHInspector::~BTHInspector()
{
}

void BTHInspector::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CUSTOM3, m_frc);
	DDX_Control(pDX, IDC_TREE1, m_tree);
}


BEGIN_MESSAGE_MAP(BTHInspector, CDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE1, &BTHInspector::OnNMDblclkTree1)
END_MESSAGE_MAP()


// BTHInspector message handlers

BOOL BTHInspector::OnInitDialog()
{
	CBitmap cb;
	BTHHEADER bth;
	UINT cbHeader = 0;
	HID hidRoot = m_hidRoot == 0 ? m_pLTPViewer->HN_GetRootHID(m_nidParent, m_nid) : m_hidRoot;
	WCHAR buffer[255];
	CNDBViewChildDlg::OnInitDialog();

	cb.LoadBitmap(IDB_BITMAP2);
	m_il.Create(16, 16, ILC_COLOR24, 10, 1);
	m_il.Add(&cb, RGB(255, 0, 255));
	m_il.SetBkColor(CLR_NONE);
	m_tree.SetImageList(&m_il, TVSIL_NORMAL);

	// TODO:  Add extra initialization here
	cbHeader = m_pLTPViewer->HN_ReadHID(m_nidParent, m_nid, hidRoot, (BYTE*)&bth, sizeof(BTHHEADER));

	GetDlgItem(IDC_LABELS)->SetWindowText(L"bMagic:\n\ncbKey:\n\ncbEnt:\n\nbIdxLevels:\n\nhidRoot");

	wsprintf(buffer, L"0x%X (%s)\n\n%u\n\n%u\n\n%u\n\n0x%X", bth.bMagic, (bth.bMagic == bMagicBTH ? L"Passed" : L"Failed"), bth.cbKey, bth.cbEnt, bth.bIdxLevels, bth.hidRoot);
	GetDlgItem(IDC_VALUES)->SetWindowText(buffer);

	if(cbHeader >= sizeof(BTHHEADER))
		m_pLTPViewer->BuildBTHTree(m_nidParent, m_nid, &bth, &m_tree);

	wsprintf(buffer, L"BTH Inspector (HID: 0x%X, NID: 0x%X, Parent NID: 0x%X)", hidRoot, m_nid, m_nidParent);
	SetWindowText(buffer);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void BTHInspector::OnNMDblclkTree1(NMHDR*, LRESULT*)
{
	HTREEITEM hItem = m_tree.GetSelectedItem();
	HID hid;

	if(hItem)
	{
		hid = (HID)m_tree.GetItemData(hItem);

		if(hid != 0)
		{
			NodeData nd;
			ZeroMemory(&nd, sizeof(NodeData));
			
			nd.nid = m_nid;
			nd.nidParent = m_nidParent;
			nd.hid = hid;
			nd.ulFlags = aOpenHID;

			m_pNDBViewDlg->PerformAction(&nd);
		}
	}
}// LTPInspectors.cpp : implementation file
//

#include "stdafx.h"
#include "NDBView.h"
#include "LTPInspectors.h"


// IndexRootInspector dialog

IMPLEMENT_DYNAMIC(IndexRootInspector, CDialog)

IndexRootInspector::IndexRootInspector(NID nid, LTPViewer * pLTPViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent)
	: CNDBViewChildDlg(pNDBViewDlg, IndexRootInspector::IDD, pParent)
{
	m_pLTPViewer = pLTPViewer;
	m_nid = nid;
}

IndexRootInspector::~IndexRootInspector()
{
}

void IndexRootInspector::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE1, m_tree);
}


BEGIN_MESSAGE_MAP(IndexRootInspector, CDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE1, &IndexRootInspector::OnNMDblclkTree1)
END_MESSAGE_MAP()


// IndexRootInspector message handlers

BOOL IndexRootInspector::OnInitDialog()
{
	CNDBViewChildDlg::OnInitDialog();
	PTCVINDEXROOT pRoot = NULL;
	HTREEITEM hItem = NULL;
	HTREEITEM hItemChild = NULL;
	ULONG ulSize = 0;
	BYTE * pData = NULL;
	WCHAR buffer[255];

	ulSize = (ULONG)m_pLTPViewer->NODE_GetSize(0, m_nid);
	pData = new BYTE[ulSize];
	m_pLTPViewer->NODE_ReadData(0, m_nid, pData, ulSize, 0, ulSize);
	pRoot = (PTCVINDEXROOT)pData;

	wsprintf(buffer, L"0x%X (%s)\n\n0x%X (%s)", pRoot->bPlatform, GetPlatformString(pRoot->bPlatform), pRoot->bVersion, GetIndexRootVersionString(pRoot->bVersion));
	GetDlgItem(IDC_VALUES)->SetWindowTextW(buffer);

	for(int i = 0; i < TBL_INDEX_COUNT; i++)
	{
		wsprintf(buffer, L"rgtcvii[%u]", i);
		hItem = m_tree.InsertItem(buffer, TVI_ROOT);

		// Add rgtcvii children
		wsprintf(buffer, L"nidSub: 0x%X", pRoot->rgtcvii[i].nidSub);
		hItemChild = m_tree.InsertItem(buffer, hItem);
		m_tree.SetItemData(hItemChild, (DWORD_PTR)pRoot->rgtcvii[i].nidSub);

		wsprintf(buffer, L"tagInst: 0x%X", pRoot->rgtcvii[i].tagInst);
		hItemChild = m_tree.InsertItem(buffer, hItem);

		wsprintf(buffer, L"cRest: %u", pRoot->rgtcvii[i].cRest);
		hItemChild = m_tree.InsertItem(buffer, hItem);
		
		wsprintf(buffer, L"dwRestF: 0x%X", pRoot->rgtcvii[i].dwRestF);
		hItemChild = m_tree.InsertItem(buffer, hItem);
		
		wsprintf(buffer, L"dwRestV: 0x%X", pRoot->rgtcvii[i].dwRestV);
		hItemChild = m_tree.InsertItem(buffer, hItem);
		
		wsprintf(buffer, L"lcid: 0x%X", pRoot->rgtcvii[i].lcid);
		hItemChild = m_tree.InsertItem(buffer, hItem);

		HTREEITEM hItemSOS = m_tree.InsertItem(L"ssos", hItem);

		wsprintf(buffer, L"cSorts: %u", pRoot->rgtcvii[i].ssos.cSorts);
		hItemChild = m_tree.InsertItem(buffer, hItemSOS);
			
		wsprintf(buffer, L"cCategories: %u", pRoot->rgtcvii[i].ssos.cCategories);
		hItemChild = m_tree.InsertItem(buffer, hItemSOS);
			
		wsprintf(buffer, L"cExpanded: %u", pRoot->rgtcvii[i].ssos.cExpanded);
		hItemChild = m_tree.InsertItem(buffer, hItemSOS);

		for(int j = 0; j < TBL_INDEX_MAX_SORT; j++)
		{
			wsprintf(buffer, L"aSort[%u]", j);
			hItemChild = m_tree.InsertItem(buffer, hItemSOS);

			wsprintf(buffer, L"ulPropTag: 0x%X", pRoot->rgtcvii[i].ssos.aSort[j].ulPropTag);
			(void)m_tree.InsertItem(buffer, hItemChild);
			
			wsprintf(buffer, L"ulOrder: 0x%X", pRoot->rgtcvii[i].ssos.aSort[j].ulOrder);
			(void)m_tree.InsertItem(buffer, hItemChild);
		}
	}

	wsprintf(buffer, L"TCV Index Root (nid: 0x%X)", m_nid);
	SetWindowText(buffer);
	delete [] pData;
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void IndexRootInspector::OnNMDblclkTree1(NMHDR*, LRESULT *pResult)
{
	NodeData nd;
	NID nid;
	ZeroMemory(&nd, sizeof(NodeData));

	nid = (NID)m_tree.GetItemData(m_tree.GetSelectedItem());

	if(nid != 0)
	{
		nd.nid = nid;
		nd.nidParent = m_nid;
		nd.ulFlags = aOpenES;
		m_pNDBViewDlg->PerformAction(&nd);
	}

	*pResult = 0;
}
// LTPInspectors.cpp : implementation file
//

#include "stdafx.h"
#include "NDBView.h"
#include "LTPInspectors.h"


// PCInspector dialog

IMPLEMENT_DYNAMIC(PCInspector, CDialog)

PCInspector::PCInspector(NID nidParent, NID nid, LTPViewer * pLTPViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent)   // standard constructor
	: CNDBViewChildDlg(pNDBViewDlg, IndexRootInspector::IDD, pParent),
	m_frc(pLTPViewer)
{
	m_nidParent = nidParent;
	m_nid = nid;
	m_pLTPViewer = pLTPViewer;
}

PCInspector::~PCInspector()
{
}

void PCInspector::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CUSTOM1, m_frc);
	DDX_Control(pDX, IDC_LIST1, m_lc);
}


BEGIN_MESSAGE_MAP(PCInspector, CDialog)
END_MESSAGE_MAP()


// PCInspector message handlers

struct CallbackCtx
{
	CListCtrl *plc;
	LTPViewer *pltpv;
	NID nid;
	NID nidParent;
};

bool PropertyCallback(void* pKey, void* pValue, void* pCtx)
{
	CallbackCtx *pcbctx = (CallbackCtx*)pCtx;
	CListCtrl *plc = pcbctx->plc;
	PPROPENTRY ppe = (PPROPENTRY)pValue;
	WORD tag = *((WORD*)pKey);
	SPropValue spv;
	int nItems = plc->GetItemCount();
	WCHAR* pDescription = NULL;
	WCHAR buffer[255];

	wsprintf(buffer, L"0x%04X (%s)", tag, GetPropTagString(PROP_TAG(ppe->wType, tag)));
	plc->InsertItem(nItems, buffer);
	wsprintf(buffer, L"0x%04X (%s%s)", ppe->wType, ppe->wType & MV_FLAG ? L"MV_FLAG|":L"", GetPropTypeString(ppe->wType & ~MV_FLAG));
	plc->SetItemText(nItems, 1, buffer);

//	wsprintf(buffer, L"0x%04X", ppe->hnid);
//	plc->SetItemText(nItems, 2, buffer);

	spv.ulPropTag = PROP_TAG(ppe->wType, tag);
	pcbctx->pltpv->PC_GetProp(pcbctx->nidParent, pcbctx->nid, &spv);
	pDescription = SPVtoString(&spv);

	plc->SetItemText(nItems, 2, pDescription);

	delete [] pDescription;
	if(PROP_TYPE(spv.ulPropTag) == PT_BINARY)
		delete [] spv.Value.bin.lpb;
	else if(PROP_TYPE(spv.ulPropTag) == PT_UNICODE)
		delete [] spv.Value.lpszW;
	else if(PROP_TYPE(spv.ulPropTag) == PT_STRING8)
		delete [] spv.Value.lpszA;
	return true;
}

BOOL PCInspector::OnInitDialog()
{
	RECT r;
	BTHHEADER header;
	CallbackCtx ctx;

	CNDBViewChildDlg::OnInitDialog();

	m_lc.GetClientRect(&r);
	// leave some room for the scroll bar
	r.right = r.right * 95 / 100;
	m_pLTPViewer->HN_ReadHID(m_nidParent, m_nid, m_pLTPViewer->HN_GetRootHID(m_nidParent, m_nid), (BYTE*)&header, sizeof(BTHHEADER));

	m_lc.InsertColumn(0, L"Property", LVCFMT_LEFT, r.right / 3);
	m_lc.InsertColumn(1, L"Type", LVCFMT_LEFT, r.right / 3);
	m_lc.InsertColumn(2, L"Value", LVCFMT_LEFT, r.right / 3);

	ctx.nid = m_nid;
	ctx.nidParent = m_nidParent;
	ctx.plc = &m_lc;
	ctx.pltpv = m_pLTPViewer;

	m_pLTPViewer->BTH_Enum(m_nidParent, m_nid, &header, &ctx, &PropertyCallback);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
