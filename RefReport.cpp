// RefReport.cpp : implementation file
//

#include "stdafx.h"
#include "NDBView.h"
#include "RefReport.h"

extern CMap<NID, NID, const WCHAR*, const WCHAR*> s_nidToString;
// CRefReport dialog

IMPLEMENT_DYNAMIC(CRefReport, CDialog)

CRefReport::CRefReport(BID bid, NDBViewer * pNDBViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent /*=NULL*/)
	: CNDBViewChildDlg(pNDBViewDlg, CRefReport::IDD, pParent)
{
	m_pNDBViewer = pNDBViewer;
	m_bid = BIDStrip(bid);
	m_cDiskRefCount = (UINT)-1;
	m_cCalculatedCount = 0;
}

CRefReport::~CRefReport()
{
}

void CRefReport::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST3, m_lc);
}


BEGIN_MESSAGE_MAP(CRefReport, CDialog)
	ON_NOTIFY(LVN_DELETEITEM, IDC_LIST3, &CRefReport::OnLvnDeleteitemList3)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST3, &CRefReport::OnNMDblclkList3)
END_MESSAGE_MAP()


// CRefReport message handlers
bool NBTAddRefs(BTPAGE * pBTPage, PAGETRAILER* pPageTrailer, IB ib, void* pv)
{
	((CRefReport*)pv)->NBTPage(pBTPage, pPageTrailer, ib);
	return true;
}

bool BBTAddRefs(BTPAGE * pBTPage, PAGETRAILER* pPageTrailer, IB ib, void* pv)
{
	((CRefReport*)pv)->BBTPage(pBTPage, pPageTrailer, ib);
	return true;
}

BOOL CRefReport::OnInitDialog()
{
	RECT r;
	CBitmap cbm;
	WCHAR buffer[255];
	CDialog::OnInitDialog();
	CWaitCursor wait;
	
	cbm.LoadBitmap(IDB_BITMAP2);
	m_il.Create(16, 16, ILC_COLOR24, 10, 1);
	m_il.Add(&cbm, RGB(255, 0, 255));
	m_il.SetBkColor(CLR_NONE);
	m_lc.SetImageList(&m_il, LVSIL_SMALL);

	// TODO:  Add extra initialization here
	BREF bref;
	CB cb;
	m_pNDBViewer->LookupBID(m_bid, bref, cb, m_cDiskRefCount);

	m_lc.GetClientRect(&r);
	m_lc.InsertColumn(0, L"Type", LVCFMT_LEFT, r.right/8);
	m_lc.InsertColumn(1, L"BID", LVCFMT_LEFT, r.right/8);
	m_lc.InsertColumn(2, L"Info", LVCFMT_LEFT, r.right*3/4);

	m_pNDBViewer->ForEachBTPage(ptypeNBT, &NBTAddRefs, feLeaf, this);
	m_pNDBViewer->ForEachBTPage(ptypeBBT, &BBTAddRefs, feLeaf, this);

	if(m_cDiskRefCount != (UINT)-1)
	{
		wsprintf(buffer, L"Searching for %u refs, found %u refs", m_cDiskRefCount, m_cCalculatedCount);
		GetDlgItem(IDC_STATIC)->SetWindowText(buffer);
	}
	else
	{
		wsprintf(buffer, L"Found %u refs", m_cCalculatedCount);
		GetDlgItem(IDC_STATIC)->SetWindowText(buffer);
	}

	wsprintf(buffer, L"References to BID 0x%I64X", m_bid);
	SetWindowText(buffer);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CRefReport::NBTPage(BTPAGE * pBTPage, PAGETRAILER * pPT, IB ib)
{
	UINT icon = (m_pNDBViewer->FValidPage(ib, ptypeNBT) ? (pBTPage->cLevel == 0 ? iconLeafPage : iconPage ) : iconCorrupt);
	WCHAR buffer[255];
	const WCHAR * pNidName;
	BOOL fGreatSuccess;

	// add a ref for the data block and subnodes block of all entries
	NBTENTRY *pnbtEntry = (NBTENTRY*)(pBTPage->rgbte);
	for(UINT i = 0; i < NBTEnt(*pBTPage); i++, pnbtEntry++)
	{
		if(BIDStrip(pnbtEntry->bidData) == m_bid)
		{
			NodeData * pNodeData = new NodeData;
			pNodeData->bref.bid = pPT->bid;
			pNodeData->bref.ib = ib;
			pNodeData->cb = 512;
			pNodeData->btkey = 0;
			pNodeData->ulFlags = aOpenBTPage;

			m_lc.InsertItem(m_cCalculatedCount, L"Page", icon);

			wsprintf(buffer, L"0x%I64X", pPT->bid);
			m_lc.SetItemText(m_cCalculatedCount, 1, buffer);

			fGreatSuccess = s_nidToString.Lookup((NID)pnbtEntry->nid, pNidName);
			wsprintf(buffer, L"Data BID for NID 0x%I64X (%s)", pnbtEntry->nid, fGreatSuccess ? pNidName : TYPESTRINGFROMNID(pnbtEntry->nid));
			m_lc.SetItemText(m_cCalculatedCount, 2, buffer);

			m_lc.SetItemData(m_cCalculatedCount, (DWORD_PTR)pNodeData);
			m_cCalculatedCount++;
		}

		if(BIDStrip(pnbtEntry->bidSub) == m_bid)
		{
			NodeData * pNodeData = new NodeData;
			pNodeData->bref.bid = pPT->bid;
			pNodeData->bref.ib = ib;
			pNodeData->cb = 512;
			pNodeData->btkey = 0;
			pNodeData->ulFlags = aOpenBTPage;

			m_lc.InsertItem(m_cCalculatedCount, L"Page", icon);

			wsprintf(buffer, L"0x%I64X", pPT->bid);
			m_lc.SetItemText(m_cCalculatedCount, 1, buffer);

			fGreatSuccess = s_nidToString.Lookup((NID)pnbtEntry->nid, pNidName);
			wsprintf(buffer, L"Subnode BID for NID 0x%I64X (%s)", pnbtEntry->nid, fGreatSuccess ? pNidName : TYPESTRINGFROMNID(pnbtEntry->nid));
			m_lc.SetItemText(m_cCalculatedCount, 2, buffer);

			m_lc.SetItemData(m_cCalculatedCount, (DWORD_PTR)pNodeData);
			m_cCalculatedCount++;
		}
	}
}

void CRefReport::BBTPage(BTPAGE * pBTPage, PAGETRAILER * pPT, IB ib)
{
	UINT icon = (m_pNDBViewer->FValidPage(ib, ptypeBBT) ? (pBTPage->cLevel == 0 ? iconLeafPage : iconPage ) : iconCorrupt);
	BBTENTRY *pbbtEntry = (BBTENTRY*)(pBTPage->rgbte);
	WCHAR buffer[255];
	BOOL fGreatSuccess;
	const WCHAR * pNidName;

	for(int i = 0; i < pBTPage->cEnt; i++, pbbtEntry++)
	{
		if(BIDStrip(pbbtEntry->bref.bid) == m_bid)
		{
			NodeData * pNodeData = new NodeData;
			pNodeData->bref.bid = pPT->bid;
			pNodeData->bref.ib = ib;
			pNodeData->cb = 512;
			pNodeData->btkey = 0;
			pNodeData->ulFlags = aOpenBTPage;
			m_lc.InsertItem(m_cCalculatedCount, L"Page", icon);

			wsprintf(buffer, L"0x%I64X", pPT->bid);
			m_lc.SetItemText(m_cCalculatedCount, 1, buffer);

			m_lc.SetItemText(m_cCalculatedCount, 2, L"BBT Leaf Page Ref");

			m_lc.SetItemData(m_cCalculatedCount, (DWORD_PTR)pNodeData);
			m_cCalculatedCount++;
		}

		// if the bid is an internal bid, we have to open the block and add all of it's refs
		if(BIDIsInternal(pbbtEntry->bref.bid))
		{
			BYTE * blockData = new BYTE[BBufferSize(pbbtEntry->cb)];
			UINT iconBlock = (m_pNDBViewer->FValidBlock(pbbtEntry->bref.ib, pbbtEntry->cb, pbbtEntry->bref.bid) ? iconInternalBlock : iconCorrupt);
			BLOCKTRAILER bt;

			m_pNDBViewer->ReadBlock(blockData, BBufferSize(pbbtEntry->cb), &bt, pbbtEntry->bref.ib, CbAlignDisk(pbbtEntry->cb));

			// add all of the refs
			if(blockData[0] == btypeSB)
			{
				SBLOCK * psb = (SBLOCK*)blockData;
				if(psb->cLevel > 0)
				{
					SIENTRY * siEntry = psb->rgsi;
					for(UINT i = 0; i < SBIEnt(*psb, BBufferSize(pbbtEntry->cb)); i++, siEntry++)
					{
						if(BIDStrip(siEntry->bid) == m_bid)
						{
							NodeData * pNodeData = new NodeData;
							pNodeData->bref = pbbtEntry->bref;
							pNodeData->cb = pbbtEntry->cb;
							pNodeData->btkey = 0;
							pNodeData->ulFlags = aOpenBlock | aBrowseBBT;
							m_lc.InsertItem(m_cCalculatedCount, L"Block", iconBlock);

							wsprintf(buffer, L"0x%I64X", pbbtEntry->bref.bid);
							m_lc.SetItemText(m_cCalculatedCount, 1, buffer);

							wsprintf(buffer, L"SIENTRY %u", i);
							m_lc.SetItemText(m_cCalculatedCount, 2, buffer);

							m_lc.SetItemData(m_cCalculatedCount, (DWORD_PTR)pNodeData);
							m_cCalculatedCount++;
						}
					}
				}
				else
				{
					SLENTRY * slEntry = psb->rgsl;
					for(UINT i = 0; i < SBLEnt(*psb, BBufferSize(pbbtEntry->cb)); i++, slEntry++)
					{
						if(BIDStrip(slEntry->bidData) == m_bid)
						{
							NodeData * pNodeData = new NodeData;
							pNodeData->bref = pbbtEntry->bref;
							pNodeData->cb = pbbtEntry->cb;
							pNodeData->btkey = 0;
							pNodeData->ulFlags = aOpenBlock | aBrowseBBT;
							m_lc.InsertItem(m_cCalculatedCount, L"Block", iconBlock);

							wsprintf(buffer, L"0x%I64X", pbbtEntry->bref.bid);
							m_lc.SetItemText(m_cCalculatedCount, 1, buffer);

							fGreatSuccess = s_nidToString.Lookup((NID)slEntry->nid, pNidName);
							wsprintf(buffer, L"Data BID for Subnode NID 0x%X (%s)", slEntry->nid, fGreatSuccess ? pNidName : TYPESTRINGFROMNID(slEntry->nid));
							m_lc.SetItemText(m_cCalculatedCount, 2, buffer);
							
							m_lc.SetItemData(m_cCalculatedCount, (DWORD_PTR)pNodeData);
							m_cCalculatedCount++;
						}
						if(BIDStrip(slEntry->bidSub) == m_bid)
						{
							NodeData * pNodeData = new NodeData;
							pNodeData->bref = pbbtEntry->bref;
							pNodeData->cb = pbbtEntry->cb;
							pNodeData->btkey = 0;
							pNodeData->ulFlags = aOpenBlock | aBrowseBBT;
							m_lc.InsertItem(m_cCalculatedCount, L"Block", iconBlock);

							wsprintf(buffer, L"0x%I64X", pbbtEntry->bref.bid);
							m_lc.SetItemText(m_cCalculatedCount, 1, buffer);

							fGreatSuccess = s_nidToString.Lookup((NID)slEntry->nid, pNidName);
							wsprintf(buffer, L"Subnode BID for Subnode NID 0x%X (%s)", slEntry->nid, fGreatSuccess ? pNidName : TYPESTRINGFROMNID(slEntry->nid));
							m_lc.SetItemText(m_cCalculatedCount, 2, buffer);

							m_lc.SetItemData(m_cCalculatedCount, (DWORD_PTR)pNodeData);
							m_cCalculatedCount++;
						}
					}
				}
			}
			else if(blockData[0] == btypeXB)
			{
				XBLOCK *pxb = (XBLOCK*)blockData;
				// here we don't care about level, every level XBLOCK is just a list of BIDs
				for(UINT i = 0; i < XBEnt(*pxb, BBufferSize(pbbtEntry->cb)); i++)
				{
					if(BIDStrip(pxb->rgbid[i]) == m_bid)
					{
						NodeData * pNodeData = new NodeData;
						pNodeData->bref = pbbtEntry->bref;
						pNodeData->cb = pbbtEntry->cb;
						pNodeData->btkey = 0;
						pNodeData->ulFlags = aOpenBlock | aBrowseBBT;

						m_lc.InsertItem(m_cCalculatedCount, L"Block", iconBlock);

						wsprintf(buffer, L"0x%I64X", pbbtEntry->bref.bid);
						m_lc.SetItemText(m_cCalculatedCount, 1, buffer);

						wsprintf(buffer, L"XBLOCK entry %u", i);
						m_lc.SetItemText(m_cCalculatedCount, 2, buffer);

						m_lc.SetItemData(m_cCalculatedCount, (DWORD_PTR)pNodeData);
						m_cCalculatedCount++;
					}
				}
			}
			delete [] blockData;
		}
	}
}
void CRefReport::OnLvnDeleteitemList3(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	delete (NodeData*)m_lc.GetItemData(pNMLV->iItem);

	*pResult = 0;
}

void CRefReport::OnNMDblclkList3(NMHDR *pNMHDR, LRESULT *pResult)
{
	Unreferenced(pNMHDR);

	// TODO: Add your control notification handler code here
	int i = m_lc.GetSelectionMark();

	if(i >= 0)
	{
		NodeData * pNodeData = (NodeData*)m_lc.GetItemData(i);
		m_pNDBViewDlg->PerformAction(pNodeData);
	}
	*pResult = 0;
}
// RefReport.cpp : implementation file
//

#include "stdafx.h"
#include "NDBView.h"
#include "RefReport.h"


// CRefReportBID dialog

IMPLEMENT_DYNAMIC(CRefReportBID, CDialog)

CRefReportBID::CRefReportBID(CWnd* pParent /*=NULL*/)
	: CDialog(CRefReportBID::IDD, pParent)
	, m_cs(_T(""))
{

}

CRefReportBID::~CRefReportBID()
{
}

void CRefReportBID::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_cs);
}


BEGIN_MESSAGE_MAP(CRefReportBID, CDialog)
END_MESSAGE_MAP()
