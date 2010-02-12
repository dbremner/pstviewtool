// NDBStats.cpp : implementation file
//

#include "stdafx.h"
#include "NDBView.h"
#include "NDBStats.h"

// CNDBStats dialog

IMPLEMENT_DYNAMIC(CNDBStats, CDialog)

CNDBStats::CNDBStats(NDBViewer * pNDBViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent)
	: CNDBViewChildDlg(pNDBViewDlg, CNDBStats::IDD, pParent),
	m_pNDBViewer(pNDBViewer),
	m_uiPages(0),
	m_uiBBTPages(0),
	m_uiNBTPages(0),
	m_uiAMapPages(0),
	m_uiFMapPages(0),
	m_uiPMapPages(0),
	m_uiFPMapPages(0),
	m_uiBlocks(0),
	m_cbBlocks(0),
	m_uiExternalBlocks(0),
	m_cbExternalBlocks(0),
	m_uiXBlocks(0),
	m_cbXBlocks(0),
	m_uiXXBlocks(0),
	m_cbXXBlocks(0),
	m_uiSBlocks(0),
	m_cbSBlocks(0),
	m_uiReusedBlocks(0),
	m_cbReusedBlocks(0),
	m_bidLargestBlock(0),
	m_cbLargestBlock(0),
	m_uiMessages(0),
	m_uiAssMessages(0),
	m_uiFolders(0),
	m_uiSearchFlds(0),
	m_nidLastModified(0),
	m_bidHighest(0)
{

}

CNDBStats::~CNDBStats()
{
}

void CNDBStats::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CNDBStats, CDialog)
END_MESSAGE_MAP()


// CNDBStats message handlers

bool CNDBStats::NBTStats(BTPAGE* pBTPage, PAGETRAILER* pPT, IB ib, void* pv)
{
	CNDBStats * pStats = (CNDBStats*)pv;
	pStats->m_uiPages++;
	pStats->m_uiNBTPages++;
	Unreferenced(pPT);

	if(pStats->m_pNDBViewer->FValidPage(ib, ptypeNBT) && pBTPage->cLevel == 0)
	{
		NBTENTRY *pnbtEntry = (NBTENTRY*)(pBTPage->rgbte);
		for(UINT i = 0; i < NBTEnt(*pBTPage); i++, pnbtEntry++)	
		{
			switch(NIDType(pnbtEntry->nid))
			{
			case NID_TYPE_NORMAL_FOLDER:
				if(pnbtEntry->nid > GetMinNIDIndex(NID_TYPE_NORMAL_FOLDER))
					pStats->m_uiFolders++;
				break;
			case NID_TYPE_SEARCH_FOLDER:
				if(pnbtEntry->nid > GetMinNIDIndex(NID_TYPE_SEARCH_FOLDER))
					pStats->m_uiSearchFlds++;
				break;
			case NID_TYPE_NORMAL_MESSAGE:
				if(pnbtEntry->nid > GetMinNIDIndex(NID_TYPE_NORMAL_MESSAGE))
					pStats->m_uiMessages++;
				break;
			case NID_TYPE_ASSOC_MESSAGE:
				if(pnbtEntry->nid > GetMinNIDIndex(NID_TYPE_ASSOC_MESSAGE))
					pStats->m_uiAssMessages++;
				break;
			}

			if(pnbtEntry->bidData > pStats->m_bidHighest)
			{
				pStats->m_bidHighest = pnbtEntry->bidData;
				pStats->m_nidLastModified = (NID)pnbtEntry->nid;
			}
			if(pnbtEntry->bidSub > pStats->m_bidHighest)
			{
				pStats->m_bidHighest = pnbtEntry->bidSub;
				pStats->m_nidLastModified = (NID)pnbtEntry->nid;
			}

		}
	}
	
	return true;
}

bool CNDBStats::BBTStats(BTPAGE* pBTPage, PAGETRAILER* pPT, IB ib, void* pv)
{
	CNDBStats * pStats = (CNDBStats*)pv;
	pStats->m_uiPages++;
	pStats->m_uiBBTPages++;
	Unreferenced(pPT);
	
	if(pStats->m_pNDBViewer->FValidPage(ib, ptypeBBT) && pBTPage->cLevel == 0)
	{
		BBTENTRY *pbbtEntry = (BBTENTRY*)(pBTPage->rgbte);
		for(UINT i = 0; i < BBTEnt(*pBTPage); i++, pbbtEntry++)
		{
			CB logicalSize = pbbtEntry->cb;

			pStats->m_uiBlocks++;
			pStats->m_cbBlocks += CbAlignDisk(pbbtEntry->cb);

			if(BIDIsExternal(pbbtEntry->bref.bid))
			{
				pStats->m_uiExternalBlocks++;
				pStats->m_cbExternalBlocks += CbAlignDisk(pbbtEntry->cb);
			}
			else
			{
				XBLOCK xblock;
				// It's either an xblock or sblock, we need to read it to determine
				pStats->m_pNDBViewer->ReadData((BYTE*)&xblock, sizeof(XBLOCK), NULL, 0, pbbtEntry->bref.ib, sizeof(XBLOCK));
				if(xblock.btype == btypeXB)
				{
					// level?
					if(xblock.cLevel == 1)
					{
						pStats->m_uiXBlocks++;
						pStats->m_cbXBlocks += CbAlignDisk(pbbtEntry->cb);
						logicalSize = xblock.lcbTotal;
					} 
					else
					{
						pStats->m_uiXXBlocks++;
						pStats->m_cbXXBlocks += CbAlignDisk(pbbtEntry->cb);
						logicalSize = xblock.lcbTotal;
					}
				}
				else
				{
					// sblock, no magic is needed.
					pStats->m_uiSBlocks++;
					pStats->m_cbSBlocks += CbAlignDisk(pbbtEntry->cb);
				}
			}

			// "largest logical block" check
			if(logicalSize > pStats->m_cbLargestBlock)
			{
				pStats->m_bidLargestBlock = pbbtEntry->bref.bid;
				pStats->m_cbLargestBlock = logicalSize;
			}

			// "reused space" check
			if(pbbtEntry->cRef > 2)
			{
				pStats->m_uiReusedBlocks += pbbtEntry->cRef - 2;
				pStats->m_cbReusedBlocks += pbbtEntry->cb * (pbbtEntry->cRef - 2);
			}
		}
	}

	return true;
}

BOOL CNDBStats::OnInitDialog()
{
	WCHAR buffer[255];
	CWaitCursor wait;

	CNDBViewChildDlg::OnInitDialog();
	for(IB ib = ibAMapBase; ib < m_pNDBViewer->GetFileEOF(); ib += cbPerAMap)
	{
		m_uiAMapPages++;
		m_uiPages++;
	}
	for(IB ib = ibPMapBase; ib < m_pNDBViewer->GetFileEOF(); ib += cbPerPMap)
	{
		m_uiPMapPages++;
		m_uiPages++;
	}
	for(IB ib = ibFMapBase; ib < m_pNDBViewer->GetFileEOF(); ib += cbPerFMap)
	{
		m_uiFMapPages++;
		m_uiPages++;
	}
	for(IB ib = ibFPMapBase; ib < m_pNDBViewer->GetFileEOF(); ib += cbPerFPMap)
	{
		m_uiFPMapPages++;
		m_uiPages++;
	}


	m_pNDBViewer->ForEachBTPage(ptypeNBT, &NBTStats, feAll, this);
	m_pNDBViewer->ForEachBTPage(ptypeBBT, &BBTStats, feAll, this);

	// page info
	wsprintf(buffer, L"%u (%I64u %s)", m_uiPages, ULLSize(m_uiPages*512), WSZSize(m_uiPages*512));
	GetDlgItem(IDC_ACTIVE_PAGES)->SetWindowText(buffer);

	swprintf_s(buffer, 255, L"%u (%I64u %s - %.2f%%)", m_uiBBTPages, ULLSize(m_uiBBTPages*512), WSZSize(m_uiBBTPages*512), (m_uiBBTPages*(double)100)/m_uiPages);
	GetDlgItem(IDC_BBT)->SetWindowTextW(buffer);

	swprintf_s(buffer, 255, L"%u (%I64u %s - %.2f%%)", m_uiNBTPages, ULLSize(m_uiNBTPages*512), WSZSize(m_uiNBTPages*512), (m_uiNBTPages*(double)100)/m_uiPages);
	GetDlgItem(IDC_NBT)->SetWindowTextW(buffer);

	swprintf_s(buffer, 255, L"%u (%I64u %s - %.2f%%)", m_uiAMapPages, ULLSize(m_uiAMapPages*512), WSZSize(m_uiAMapPages*512), (m_uiAMapPages*(double)100)/m_uiPages);
	GetDlgItem(IDC_AMAP)->SetWindowTextW(buffer);

	swprintf_s(buffer, 255, L"%u (%I64u %s - %.2f%%)", m_uiFMapPages, ULLSize(m_uiFMapPages*512), WSZSize(m_uiFMapPages*512), (m_uiFMapPages*(double)100)/m_uiPages);
	GetDlgItem(IDC_FMAP)->SetWindowTextW(buffer);

	swprintf_s(buffer, 255, L"%u (%I64u %s - %.2f%%)", m_uiPMapPages, ULLSize(m_uiPMapPages*512), WSZSize(m_uiPMapPages*512), (m_uiPMapPages*(double)100)/m_uiPages);
	GetDlgItem(IDC_PMAP)->SetWindowTextW(buffer);

	swprintf_s(buffer, 255, L"%u (%I64u %s - %.2f%%)", m_uiFPMapPages, ULLSize(m_uiFPMapPages*512), WSZSize(m_uiFPMapPages*512), (m_uiFPMapPages*(double)100)/m_uiPages);
	GetDlgItem(IDC_FPMAP)->SetWindowTextW(buffer);

	// block info
	wsprintf(buffer, L"%u (%I64u %s)", m_uiBlocks, ULLSize(m_cbBlocks), WSZSize(m_cbBlocks));
	GetDlgItem(IDC_ACTIVE_BLOCKS)->SetWindowTextW(buffer);

	swprintf_s(buffer, 255, L"%u (%I64u %s - %.2f%%)", m_uiExternalBlocks, ULLSize(m_cbExternalBlocks), WSZSize(m_cbExternalBlocks), m_cbExternalBlocks*(double)100/m_cbBlocks);
	GetDlgItem(IDC_EXTERNAL)->SetWindowTextW(buffer);

	swprintf_s(buffer, 255, L"%u (%I64u %s - %.2f%%)", m_uiXBlocks, ULLSize(m_cbXBlocks), WSZSize(m_cbXBlocks), m_cbXBlocks*(double)100/m_cbBlocks);
	GetDlgItem(IDC_XBLOCK)->SetWindowTextW(buffer);
	
	swprintf_s(buffer, 255, L"%u (%I64u %s - %.2f%%)", m_uiXXBlocks, ULLSize(m_cbXXBlocks), WSZSize(m_cbXXBlocks), m_cbXXBlocks*(double)100/m_cbBlocks);
	GetDlgItem(IDC_XXBLOCK)->SetWindowTextW(buffer);
	
	swprintf_s(buffer, 255, L"%u (%I64u %s - %.2f%%)", m_uiSBlocks, ULLSize(m_cbSBlocks), WSZSize(m_cbSBlocks), m_cbSBlocks*(double)100/m_cbBlocks);
	GetDlgItem(IDC_SBLOCK)->SetWindowTextW(buffer);
	
	swprintf_s(buffer, 255, L"%u (%I64u %s - %.2f%%)", m_uiReusedBlocks, ULLSize(m_cbReusedBlocks), WSZSize(m_cbReusedBlocks), m_cbReusedBlocks*(double)100/m_cbBlocks);
	GetDlgItem(IDC_REUSED)->SetWindowTextW(buffer);

	wsprintf(buffer, L"0x%I64X (%I64u %s)", m_bidLargestBlock, ULLSize(m_cbLargestBlock), WSZSize(m_cbLargestBlock));
	GetDlgItem(IDC_LARGEST)->SetWindowTextW(buffer);

	// node info
	double value;

	if(m_uiMessages)
	{
		value = (double)m_uiMessages * 100.0/(m_pNDBViewer->GetHeader().rgnid[NID_TYPE_NORMAL_MESSAGE] + 1 - GetMinNIDIndex(NID_TYPE_NORMAL_MESSAGE));
		if(value < 0.01) value = 0.0;
	} 
	else
	{
		value = 0.0;
	}
	swprintf_s(buffer, 255, L"%u (%.2f%%)", m_uiMessages, value);
	GetDlgItem(IDC_MESSAGES)->SetWindowTextW(buffer);

	if(m_uiFolders)
	{
		value = (double)m_uiFolders * 100.0/(m_pNDBViewer->GetHeader().rgnid[NID_TYPE_NORMAL_FOLDER] + 1 - GetMinNIDIndex(NID_TYPE_NORMAL_FOLDER));
		if(value < 0.01) value = 0.0;
	}
	else
	{
		value = 0.0;
	}
	swprintf_s(buffer, 255, L"%u (%.2f%%)", m_uiFolders, value);
	GetDlgItem(IDC_FOLDERS)->SetWindowTextW(buffer);

	if(m_uiAssMessages)
	{
		value = (double)m_uiAssMessages * 100.0/(m_pNDBViewer->GetHeader().rgnid[NID_TYPE_ASSOC_MESSAGE] + 1 - GetMinNIDIndex(NID_TYPE_ASSOC_MESSAGE));
		if(value < 0.01) value = 0.0;
	}
	else
	{
		value = 0.0;
	}
	swprintf_s(buffer, 255, L"%u (%.2f%%)", m_uiAssMessages, value);
	GetDlgItem(IDC_ASS_MESSAGES)->SetWindowTextW(buffer);

	if(m_uiSearchFlds)
	{
		value = (double)m_uiSearchFlds * 100.0/(m_pNDBViewer->GetHeader().rgnid[NID_TYPE_SEARCH_FOLDER] + 1 - GetMinNIDIndex(NID_TYPE_SEARCH_FOLDER));
		if(value < 0.01) value = 0.0;
	}
	else
	{
		value = 0.0;
	}
	swprintf_s(buffer, 255, L"%u (%.2f%%)", m_uiSearchFlds, value);
	GetDlgItem(IDC_SRC_FOLDERS)->SetWindowTextW(buffer);

	wsprintf(buffer, L"0x%X", m_nidLastModified);
	GetDlgItem(IDC_LAST_MODIFIED)->SetWindowTextW(buffer);

	// general info
	wsprintf(buffer, L"%I64u %s", ULLSize(m_pNDBViewer->GetHeader().root.ibFileEof), WSZSize(m_pNDBViewer->GetHeader().root.ibFileEof));
	GetDlgItem(IDC_FILE_SIZE)->SetWindowTextW(buffer);

	swprintf_s(buffer, 255, L"%.2f%%", ((m_pNDBViewer->GetHeader().root.cbAMapFree+m_pNDBViewer->GetHeader().root.cbPMapFree)*100)/(double)m_pNDBViewer->GetHeader().root.ibFileEof);
	GetDlgItem(IDC_PERCENT_FREE)->SetWindowTextW(buffer);

	swprintf_s(buffer, 255, L"%.12f%%", ((double)(m_pNDBViewer->GetHeader().bidNextB))/(double)BID_MAX * 100.00);
	GetDlgItem(IDC_PERCENT_BID)->SetWindowTextW(buffer);

	return TRUE;  // return TRUE unless you set the focus to a control
}
