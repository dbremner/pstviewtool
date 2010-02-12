// BTDialog.cpp : implementation file
//

#include "stdafx.h"
#include "NDBView.h"
#include "BTDialog.h"


// CBTDialog dialog

IMPLEMENT_DYNAMIC(CBTDialog, CDialog)

CBTDialog::CBTDialog(NDBViewer * pNDBViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent /*=NULL*/)
	: CNDBViewChildDlg(pNDBViewDlg, CBTDialog::IDD, pParent)
{
	m_pNDBViewer = pNDBViewer;
	m_pNDBViewDlg = pNDBViewDlg;
	if(m_pNDBViewer)
		m_pBTMap = m_pNDBViewer->GetCachedBTBitmap();
	m_fDoModal = false;
}

CBTDialog::~CBTDialog()
{
}

void CBTDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CBTDialog, CDialog)
	ON_WM_PAINT()
	ON_WM_CREATE()
END_MESSAGE_MAP()


// CBTDialog message handlers
int CBTDialog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	RECT client;
	int numPixel;
	int numPixelNeeded;
	int extraLines;
	CB cbPixel; 
	int retVal;
	WCHAR windowTitle[255];

	retVal = CDialog::OnCreate(lpCreateStruct);

	if(m_pNDBViewer)
	{
		GetClientRect(&client);
		numPixel = client.right * client.bottom;
		cbPixel = (m_pNDBViewer->GetHeader().root.ibFileEof / numPixel) + 1;
		wsprintf(windowTitle, L"BT Page Locations (1 pixel = %I64u bytes)", cbPixel);
		SetWindowText(windowTitle);

		// The visual effects of the above integer rounding can be dramatic
		// on small files
		numPixelNeeded = (int)(m_pNDBViewer->GetHeader().root.ibFileEof / cbPixel);
		extraLines = (numPixel - numPixelNeeded) / client.right;
		if(extraLines > 0)
		{
			RECT r;
			GetWindowRect(&r);

			SetWindowPos(NULL, 0, 0, r.right - r.left, r.bottom - r.top - extraLines, SWP_NOMOVE | SWP_NOZORDER);
		}
	}
	return retVal;
}

struct FEINFO {
	BYTE * space;
	CB cbPixel;
};
#define feBBT 0x1
#define feNBT 0x2

void MarkPageIB(BYTE * space, CB cbPixel, IB ib, PTYPE ptype)
{
	IB start, end;

	start = ib / cbPixel;
	end = (ib + 512) / cbPixel;
	for(; start <= end; start++)
	{
		space[start] |= (ptype == ptypeNBT ? feNBT : feBBT);
	}
}

bool PageExistsAt(BTPAGE* pBtpage, PAGETRAILER* pt, IB, void* pv)
{
	FEINFO * pFeInfo = (FEINFO*)pv;

	BTENTRY *pbte = (BTENTRY*)(pBtpage->rgbte);
	for(int i = 0; i < pBtpage->cEnt; i++, pbte++)
	{
		MarkPageIB(pFeInfo->space, pFeInfo->cbPixel, pbte->bref.ib, pt->ptype);
	}

	return true;
}

void CBTDialog::OnPaint()
{
	CPaintDC dc(this);
	CDC dcMem;
	RECT client;
	CBitmap * pOld = NULL;

	GetClientRect(&client);

	if(!m_pNDBViewer)
		return CDialog::OnPaint();

	dcMem.CreateCompatibleDC(&dc);
	if(!m_pBTMap)
	{
		// First paint, we need to walk the AMaps and create the bitmap.
		CWaitCursor wait;
		CPen p;
		int numPixel = client.right * client.bottom;
		BYTE * space = new BYTE[numPixel];
		CB cbPixel = (m_pNDBViewer->GetHeader().root.ibFileEof / numPixel) + 1; // round up
		FEINFO feI;

		feI.space = space;
		feI.cbPixel = cbPixel;
		
		for(int i = 0; i < numPixel; i++)
			space[i] = BYTE(0);

		// Iterate over BT's
		MarkPageIB(space, cbPixel, m_pNDBViewer->GetHeader().root.brefBBT.ib, ptypeBBT);
		m_pNDBViewer->ForEachBTPage(ptypeBBT, &PageExistsAt, feNonLeaf, &feI);
		MarkPageIB(space, cbPixel, m_pNDBViewer->GetHeader().root.brefNBT.ib, ptypeNBT);
		m_pNDBViewer->ForEachBTPage(ptypeNBT, &PageExistsAt, feNonLeaf, &feI);

		// allocate the bitmap
		m_pBTMap = new CBitmap();
		m_pBTMap->CreateCompatibleBitmap(&dc, client.right, client.bottom);
		pOld = dcMem.SelectObject(m_pBTMap);

		for(int j = 0; j < client.bottom; j++)
		{
			for(int i = 0; i < client.right; i++)
			{
				// if this pixel extends past eof, just draw the eof pixel
				// the confusing +1 is because we want the end of the region mapped
				// by this pixel, which is the start of the region mapped by the next
				// pixel
				if((j * client.right + i + 1) * cbPixel <= m_pNDBViewer->GetHeader().root.ibFileEof)
				{
					BYTE b = space[j * client.right + i];

					// does it contain both?
					if((b & feBBT) && (b & feNBT))
					{
						dcMem.SetPixel(i, j, c_BothPage);
					}
					// just a BBT page?
					else if(b & feBBT)
					{
						dcMem.SetPixel(i, j, c_BBTPage);
					}
					// just a NBT page?
					else if(b & feNBT)
					{
						dcMem.SetPixel(i, j, c_NBTPage);
					}
					else
					{
						dcMem.SetPixel(i, j, c_Empty);
					}
				}
				else
				{
					dcMem.SetPixel(i, j, c_BTPastEOF);
				}
			}
		}
		delete [] space;
		m_pNDBViewer->CacheBTBitmap(m_pBTMap);
	}
	else
	{
		pOld = dcMem.SelectObject(m_pBTMap);
	}

	dc.BitBlt(client.left, client.top, client.right, client.bottom, &dcMem, 0, 0, SRCCOPY);
	dcMem.SelectObject(pOld);
}
