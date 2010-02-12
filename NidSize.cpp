// NidSize.cpp : implementation file
//

#include "stdafx.h"
#include "NDBView.h"
#include "NidSize.h"


// CNidSize dialog

IMPLEMENT_DYNAMIC(CNidSize, CDialog)

CNidSize::CNidSize(NID parentNid, NID nid, NDBViewer * pNDBViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent)
	: CNDBViewChildDlg(pNDBViewDlg, CNidSize::IDD, pParent)
{
	m_nid = nid;
	m_nidParent = parentNid;
	m_pNDBViewer = pNDBViewer;
	m_pBitmap = 0;
	m_fLookupError = false;

	if(parentNid == 0)
	{
		if(!m_pNDBViewer->LookupNID(m_nid, m_dataBID, m_subnodeBID))
		{
			m_dataBID = 0;
			m_subnodeBID = 0;
			m_fLookupError = true;
		}
	}
	else
	{
		if(!m_pNDBViewer->LookupSubnodeNID(parentNid, nid, m_dataBID, m_subnodeBID))
		{
			m_dataBID = 0;
			m_subnodeBID = 0;
			m_fLookupError = true;
		}
	}
}

CNidSize::~CNidSize()
{
	if(m_pBitmap)
		delete m_pBitmap;
}

void CNidSize::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CNidSize, CDialog)
	ON_WM_PAINT()
	ON_WM_CREATE()
END_MESSAGE_MAP()


// CNidSize message handlers
void MarkSpaceAsOccupied(CB * usedSpace, // array of pixels
						  CB cbPerPixel, // number of bytes each pixel represents in the file
						  int, // number of pixels, or the dimension of freeSpace
						  IB startAddress, // start area to mark allocated
						  IB endAddress // end of area to mark allocated (not inclusive)
						  ) 
{
	IB current = startAddress;
	if(startAddress >= endAddress) return; // no op

	while(current < endAddress)
	{
		IB end = (endAddress <= ((current / cbPerPixel) * cbPerPixel + cbPerPixel) ? endAddress : ((current / cbPerPixel) * cbPerPixel + cbPerPixel));
		usedSpace[current / cbPerPixel] += (end - current);
		current = end;
	}
}

// recursive function to mark the space occupied by a BID as used, and to 
// recurse on child BIDs for S, X and XXBlocks
void CNidSize::EachBID(BID b, CB * pRunningTotal, int * pNumBlocks, CB * usedSpace, CB cbPerPixel, int numPixel)
{
	BREF bref;
	CB cb;
	UINT cRef;
	BYTE * pBlock;
	BLOCKTRAILER bt;

	if(b == 0) return;

	// If we can't find the referenced BID, just skip it
	if(!m_pNDBViewer->LookupBID(b, bref, cb, cRef))
		return;

	MarkSpaceAsOccupied(usedSpace, cbPerPixel, numPixel, bref.ib, bref.ib+CbAlignDisk(cb));
	*pRunningTotal += CbAlignDisk(cb);
	(*pNumBlocks)++;

	if(!BIDIsExternal(b) && m_pNDBViewer->FValidBlock(bref.ib, cb, bref.bid))
	{
		pBlock = new BYTE[(UINT)cb];
		m_pNDBViewer->ReadBlock(pBlock, (UINT)cb, &bt, bref.ib, CbAlignDisk(cb));

		if(pBlock[0] == btypeXB)
		{
			// recurse on BID's referenced by this XBlock
			XBLOCK * pxb = (XBLOCK*)pBlock;
			for(UINT i = 0; i < XBEnt(*pxb, (UINT)cb); i++)
			{
				EachBID(pxb->rgbid[i], pRunningTotal, pNumBlocks, usedSpace, cbPerPixel, numPixel);
			}
		}
		else
		{
			ASSERT(pBlock[0] == btypeSB);
			// recurse on BID's referenced by this SBlock
			SBLOCK * pSBlock = (SBLOCK*)pBlock;
			if(pSBlock->cLevel == 0)
			{
				SLENTRY * slEntry = pSBlock->rgsl;

				for(UINT i = 0; i < SBLEnt(*pSBlock, (UINT)cb); i++, slEntry++)
				{
					EachBID(slEntry->bidData, pRunningTotal, pNumBlocks, usedSpace, cbPerPixel, numPixel);
					EachBID(slEntry->bidSub, pRunningTotal, pNumBlocks, usedSpace, cbPerPixel, numPixel);
				}
			}
			else
			{
				SIENTRY * siEntry = pSBlock->rgsi;
				for(UINT i = 0; i < SBIEnt(*pSBlock, (UINT)cb); i++, siEntry++)
				{
					EachBID(siEntry->bid, pRunningTotal, pNumBlocks, usedSpace, cbPerPixel, numPixel);
				}
			}

		}
		delete [] pBlock;
	}
}

void CNidSize::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CDC dcMem;
	RECT client;
	CBitmap * pOld = NULL;

	GetClientRect(&client);

	if(!m_pNDBViewer)
		return CDialog::OnPaint();

	dcMem.CreateCompatibleDC(&dc);

	if(!m_pBitmap)
	{
		// First paint, we do the actual work
		CWaitCursor wait;
		WCHAR buffer[255];
		int numPixel = client.right * (client.bottom * 3/4);
		CB * usedSpace = new CB[numPixel];
		CB cbPixel = (m_pNDBViewer->GetHeader().root.ibFileEof / numPixel) + 1; // round up
		CB dataBID = 0; // running totals
		CB subnodeBID = 0; // running totals
		int dataBIDnum = 0;
		int subnodeBIDnum = 0;
		int iFullR = GetRValue(c_FullNID);
		int iFullG = GetGValue(c_FullNID);
		int iFullB = GetBValue(c_FullNID);
		int iFreeR = GetRValue(c_FreeNID);
		int iFreeG = GetGValue(c_FreeNID);
		int iFreeB = GetBValue(c_FreeNID);

		for(int i = 0; i < numPixel; i++)
			usedSpace[i] = CB(0);

		EachBID(m_dataBID, &dataBID, &dataBIDnum, usedSpace, cbPixel, numPixel);
		EachBID(m_subnodeBID, &subnodeBID, &subnodeBIDnum, usedSpace, cbPixel, numPixel);

		if(!m_fLookupError)
		{
			wsprintf(buffer, L"%I64u %s (%u blocks)", ULLSize(dataBID), WSZSize(dataBID), dataBIDnum);
			GetDlgItem(IDC_STATIC1)->SetWindowText(buffer);
			wsprintf(buffer, L"%I64u %s (%u blocks)", ULLSize(subnodeBID), WSZSize(subnodeBID), subnodeBIDnum);
			GetDlgItem(IDC_STATIC2)->SetWindowText(buffer);
			wsprintf(buffer, L"%I64u %s (%u blocks)", ULLSize(dataBID + subnodeBID), WSZSize(dataBID + subnodeBID), dataBIDnum+subnodeBIDnum);
			GetDlgItem(IDC_STATIC3)->SetWindowText(buffer);
		}
		else
		{
			GetDlgItem(IDC_STATIC1)->SetWindowText(L"Lookup Error");
			GetDlgItem(IDC_STATIC2)->SetWindowText(L"Lookup Error");
			GetDlgItem(IDC_STATIC3)->SetWindowText(L"Lookup Error");
		}

		// allocate the bitmap
		m_pBitmap = new CBitmap();
		m_pBitmap->CreateCompatibleBitmap(&dc, client.right, client.bottom * 3/4);
		pOld = dcMem.SelectObject(m_pBitmap);
		
		for(int j = 0; j < (client.bottom*3/4); j++)
		{
			for(int i = 0; i < client.right; i++)
			{
				// if this pixel extends past eof, just draw the eof pixel
				// the confusing +1 is because we want the end of the region mapped
				// by this pixel, which is the start of the region mapped by the next
				// pixel
				if((j * client.right + i + 1) * cbPixel <= m_pNDBViewer->GetHeader().root.ibFileEof)
				{
					double diff = (double)usedSpace[(j * client.right) + i] / (double)cbPixel;

					if(diff < 0) diff = 0.0;
					// make sure even the smallest bit of free space shows up
					if(diff > 0.0 && diff < c_minShadeNID) diff = c_minShadeNID;
					if(diff > 1.0) diff = 1.0;
					COLORREF c = RGB(
						(int)((double)iFreeR - (iFreeR*diff) + (iFullR*diff)) & 0x00FF,
						(int)((double)iFreeG - (iFreeG*diff) + (iFullG*diff)) & 0x00FF,
						(int)((double)iFreeB - (iFreeB*diff) + (iFullB*diff)) & 0x00FF
						);
					dcMem.SetPixel(i, j, c);
				}
				else
				{
					dcMem.SetPixel(i, j, c_NIDPastEOF);
				}
				
			}
		}
		delete [] usedSpace;
	}
	else
	{
		pOld = dcMem.SelectObject(m_pBitmap);
	}
	dc.BitBlt(client.left, client.bottom/4, client.right, client.bottom, &dcMem, 0, 0, SRCCOPY);
	dcMem.SelectObject(pOld);
}

int CNidSize::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	WCHAR buffer[255];
	RECT client;
	int numPixel;
	int numPixelNeeded;
	int extraLines;
	CB cbPixel; 
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	if(m_pNDBViewer)
	{
		GetClientRect(&client);
		numPixel = client.right * (client.bottom * 3 / 4);
		cbPixel = (m_pNDBViewer->GetHeader().root.ibFileEof / numPixel) + 1;
		wsprintf(buffer, L"NID 0x%X Size (1 pixel = %I64u bytes)", m_nid, cbPixel);
		SetWindowText(buffer);

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

	return 0;
}
// NidSize.cpp : implementation file
//

#include "stdafx.h"
#include "NDBView.h"
#include "NidSize.h"


// CGetNid dialog

IMPLEMENT_DYNAMIC(CGetNid, CDialog)

CGetNid::CGetNid(CWnd* pParent /*=NULL*/)
	: CDialog(CGetNid::IDD, pParent)
	, m_nid(_T(""))
	, m_parentNid (_T(""))
	, m_hid(_T(""))
{

}

CGetNid::~CGetNid()
{
}

void CGetNid::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_parentNid);
	DDX_Text(pDX, IDC_EDIT4, m_nid);
	DDX_Text(pDX, IDC_EDIT2, m_hid);
	DDX_Control(pDX, IDC_COMBO1, m_ccb);
	DDX_CBIndex(pDX, IDC_COMBO1, m_type);
	DDX_Control(pDX, IDC_EDIT2, m_hidCtrl);
}


BEGIN_MESSAGE_MAP(CGetNid, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CGetNid::OnCbnSelchangeCombo1)
END_MESSAGE_MAP()


// CGetNid message handlers

BOOL CGetNid::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add list of page inspectors to the drop down box
	for(int i = 0; i < inspLTPMaxInspector; i++)
	{
		m_ccb.InsertString(i, ltplistvalues[i]);
	}
	m_ccb.SetCurSel(0);
	m_hidCtrl.EnableWindow(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CGetNid::OnCbnSelchangeCombo1()
{
	if(m_ccb.GetCurSel() != inspBTH)
	{
		m_hidCtrl.EnableWindow(FALSE);
	}
	else
	{
		m_hidCtrl.EnableWindow(TRUE);
	}
}
