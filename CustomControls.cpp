// CustomControls.cpp : implementation file
//

#include "stdafx.h"
#include "NDBView.h"
#include "CustomControls.h"


// CBlockTrailerControl

IMPLEMENT_DYNAMIC(CBlockTrailerControl, CWnd)

CBlockTrailerControl::CBlockTrailerControl(const BREF& bref, CB cb, NDBViewer * pNDBViewer)
{
	BYTE * pBlockData = new BYTE[BBufferSize((UINT)cb)];

	pNDBViewer->ReadBlock(pBlockData, BBufferSize((UINT)cb), &m_bt, bref.ib, CbAlignDisk(cb));
	RegisterClass(L"CBlockTrailerControl");

	// pBlockData is only here to verify the CRC
	m_fValidCRC = (ComputeCRC(pBlockData, BBufferSize((UINT)cb)) == m_bt.dwCRC);
	m_wSig = ComputeSig(bref.ib, bref.bid);
	delete [] pBlockData;
}

CBlockTrailerControl::CBlockTrailerControl()
{
	RegisterClass(L"CBlockTrailerControl");
	m_fValidCRC = false;
}

CBlockTrailerControl::~CBlockTrailerControl()
{
}


BEGIN_MESSAGE_MAP(CBlockTrailerControl, CWnd)
END_MESSAGE_MAP()



// CBlockTrailerControl message handlers



void CBlockTrailerControl::PreSubclassWindow()
{
	RECT client;
	GetClientRect(&client);
	// TODO: Add your specialized code here and/or call the base class

	// Set the window font
	SetFont(GetGlobalFont());

	// Setup the group box
	m_groupBox.Create(L"Block Trailer", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, client, this, 0);
	m_groupBox.SetFont(GetGlobalFont());

	// Setup "label" static control
	RECT labelRect = client;
	labelRect.right /= 2;
	labelRect.top += 25;
	labelRect.bottom -= 25;
	labelRect.left += 25;
	labelRect.right -= 10;
	m_labels.Create(L"Text", WS_VISIBLE | WS_CHILD, labelRect, this, 0);
	m_labels.SetFont(GetGlobalFont());
	m_labels.SetWindowTextW(L"cb (Disk Aligned):\n\nwSig:\n\ndwCRC:\n\nbid:");

	// Setup "values" static control
	RECT valuesRect = client;
	WCHAR windowText[255];

	valuesRect.right -= 25;
	valuesRect.top += 25;
	valuesRect.bottom -= 25;
	valuesRect.left = client.right/2;
	valuesRect.left += 10;
	m_values.Create(L"Text", WS_VISIBLE | WS_CHILD, valuesRect, this, 0);
	m_values.SetFont(GetGlobalFont());

	wsprintf(windowText, L"%u (%u)\n\n0x%X (%s)\n\n0x%X (%s)\n\n0x%I64X", m_bt.cb, CbAlignDisk(m_bt.cb), m_bt.wSig, (m_wSig == m_bt.wSig ? L"Passed" : L"Failed"), m_bt.dwCRC, (m_fValidCRC ? L"Passed" : L"Failed"), m_bt.bid);
	m_values.SetWindowTextW(windowText);

	m_groupBox.BringWindowToTop();

	CWnd::PreSubclassWindow();
}

// CBlockTrailerControl

IMPLEMENT_DYNAMIC(CPageTrailerControl, CWnd)

CPageTrailerControl::CPageTrailerControl(IB ib, NDBViewer * pNDBViewer)
{
	BYTE * pPageData = new BYTE[cbPageData];

	pNDBViewer->ReadPage(pPageData, cbPageData, &m_pt, ib);
	RegisterClass(L"CPageTrailerControl");

	// pPageData is only here to verify the CRC
	m_fValidCRC = (ComputeCRC(pPageData, cbPageData) == m_pt.dwCRC);
	delete [] pPageData;
}

CPageTrailerControl::CPageTrailerControl()
{
	RegisterClass(L"CPageTrailerControl");
	m_fValidCRC = false;
}

CPageTrailerControl::~CPageTrailerControl()
{
}


BEGIN_MESSAGE_MAP(CPageTrailerControl, CWnd)
END_MESSAGE_MAP()



// CBlockTrailerControl message handlers



void CPageTrailerControl::PreSubclassWindow()
{
	RECT client;
	GetClientRect(&client);
	// TODO: Add your specialized code here and/or call the base class

	// Set the window font
	SetFont(GetGlobalFont());

	// Setup the group box
	m_groupBox.Create(L"Page Trailer", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, client, this, 0);
	m_groupBox.SetFont(GetGlobalFont());

	// Setup "label" static control
	RECT labelRect = client;
	labelRect.right /= 2;
	labelRect.top += 25;
	labelRect.bottom -= 25;
	labelRect.left += 25;
	labelRect.right -= 10;
	m_labels.Create(L"Text", WS_VISIBLE | WS_CHILD, labelRect, this, 0);
	m_labels.SetFont(GetGlobalFont());
	m_labels.SetWindowTextW(L"ptype:\n\nptypeRepeat:\n\nwSig:\n\ndwCRC:\n\nbid:");

	// Setup "values" static control
	RECT valuesRect = client;
	WCHAR windowText[255];

	valuesRect.right -= 25;
	valuesRect.top += 25;
	valuesRect.bottom -= 25;
	valuesRect.left = client.right/2;
	valuesRect.left += 10;
	m_values.Create(L"Text", WS_VISIBLE | WS_CHILD, valuesRect, this, 0);
	m_values.SetFont(GetGlobalFont());

	wsprintf(windowText, L"0x%X (%s)\n\n0x%X (%s)\n\n0x%X\n\n0x%X (%s)\n\n0x%I64X", m_pt.ptype, GetPTypeString(m_pt.ptype), m_pt.ptypeRepeat, GetPTypeString(m_pt.ptype), m_pt.wSig, m_pt.dwCRC, (m_fValidCRC ? L"Passed" : L"Failed"), m_pt.bid);
	m_values.SetWindowTextW(windowText);

	m_groupBox.BringWindowToTop();

	CWnd::PreSubclassWindow();
}

// CBinaryListBox

IMPLEMENT_DYNAMIC(CBinaryListBox, CListBox)

CBinaryListBox::CBinaryListBox()
{
	m_pData = 0;
	m_ibStart = 0;
	m_mode = MODE_HEX;
	m_dataSize = 0;
}

void CBinaryListBox::SetData(BYTE *pData, UINT size, IB ibStart)
{
	m_pData = new BYTE[size];
	m_dataSize = size;
	m_ibStart = ibStart;
	memcpy(m_pData, pData, size);

	Populate();
}
CBinaryListBox::~CBinaryListBox()
{
	if(m_pData)
		delete [] m_pData;
}

void CBinaryListBox::Populate()
{	
	WCHAR lineBuffer[255];
	WCHAR ibFormat[10];
	int longLen;

	wsprintf(lineBuffer, L"%I64u", m_ibStart+m_dataSize);
	longLen = lstrlen(lineBuffer);
	wsprintf(ibFormat, L"%%%uI64u: ", longLen);

	ResetContent();
	SetFont(GetGlobalFontFixedWidth());
	
	// no data is simple
	if(m_dataSize == 0)
	{
		wsprintf(lineBuffer, L"%I64u: ", m_ibStart);
		AddString(lineBuffer);
	}
	// unicode mode is pretty easy
	else if(m_mode == MODE_UNICODE)
	{
		UINT i = 0;

		while(i < m_dataSize)
		{
			WCHAR* pCurChar;
			UINT cbLineLength;

			wsprintf(lineBuffer, ibFormat, m_ibStart+i);
			pCurChar = &(lineBuffer[ lstrlen(lineBuffer) ]);
			ASSERT(*pCurChar == L'\0');

			// Grab UNICODE_BYTES_PER_LINE and make a string out of it
			cbLineLength = (i + UNICODE_BYTES_PER_LINE < m_dataSize ? UNICODE_BYTES_PER_LINE : m_dataSize - i);
			memcpy(pCurChar, &(m_pData[i]), cbLineLength);
			i += cbLineLength;

			// if the block happens to be an odd number of bytes, this will round down on the last line.
			// We'll end up nuking (intentionally) that half a WCHAR with the null character
			pCurChar += cbLineLength / 2;

			// replace null characters so the string isn't prematurely terminated
			for(WCHAR* pWchar = lineBuffer; pWchar != pCurChar; pWchar++)
			{
				if(*pWchar == L'\0') *pWchar = NULL_WCHAR_REPLACEMENT;
			}

			// Add the null terminator
			wsprintf(pCurChar, L"");

			AddString(lineBuffer);
		}
	}
	// we can treat hex and binary almost the same, with a few differences
	else
	{
		UINT uiLineSize = (m_mode == MODE_HEX ? HEX_BYTES_PER_LINE : BINARY_BYTES_PER_LINE);
		UINT i = 0;
		WCHAR byteBuffer[9];


		// each loop reads uiLineSize bytes out of the buffer. If the buffer isn't
		// uiLineSize aligned, we get the stray bytes at the end.
		if(m_dataSize > uiLineSize)
		{
			for(i = 0; i <= (m_dataSize - uiLineSize); i += uiLineSize)
			{
				wsprintf(lineBuffer, ibFormat, m_ibStart+i);
				for(UINT j = 0; j < uiLineSize; j++)
				{
					ByteToString(byteBuffer, m_pData[i+j]);
					lstrcat(lineBuffer, byteBuffer);
					lstrcat(lineBuffer, L" ");
				}
				AddString(lineBuffer);
			}
		}

		// if the data wasn't aligned to uiLineSize, deal with that now
		if(i < m_dataSize)
		{
			wsprintf(lineBuffer, ibFormat, m_ibStart+i);
			while(i < m_dataSize)
			{
				ByteToString(byteBuffer, m_pData[i++]);
				lstrcat(lineBuffer, byteBuffer);
				lstrcat(lineBuffer, L" ");
			}

			AddString(lineBuffer);
		}
	}
}

void CBinaryListBox::ByteToString(LPWSTR pBuffer, BYTE b)
{
	if(m_mode == MODE_HEX)
	{
		wsprintf(pBuffer, L"%02X", b);
	}
	else
	{
		// i'm surprised wsprintf doesn't suppose this directly? oh well.
		for(int i = 0; i < 8; i++)
			wsprintf(&(pBuffer[i]), L"%s", (b & 0x80 >> i) ? L"1" : L"0");
	}
}

BEGIN_MESSAGE_MAP(CBinaryListBox, CListBox)
END_MESSAGE_MAP()



// CBinaryListBox message handlers


// CustomControls.cpp : implementation file
//

#include "stdafx.h"
#include "NDBView.h"
#include "CustomControls.h"


// CFileRangeControl

IMPLEMENT_DYNAMIC(CFileRangeControl, CWnd)

CFileRangeControl::CFileRangeControl(CB startRange, CB endRange, NDBViewer * pNDBViewer)
{
	RegisterClass(L"CFileRangeControl");
	m_cbStart = startRange;
	m_cbEnd = endRange;
	m_pNDBViewer = pNDBViewer;
	selection.start = 0;
	selection.end = 0;
	m_pBM = NULL;
}

CFileRangeControl::CFileRangeControl(NDBViewer * pNDBViewer)
{
	RegisterClass(L"CFileRangeControl");
	m_cbStart = 0;
	m_cbEnd = 1;
	m_pNDBViewer = pNDBViewer;
	selection.start = 0;
	selection.end = 0;
	m_pBM = NULL;
}

CFileRangeControl::~CFileRangeControl()
{
	if(m_pBM)
		delete m_pBM;
}

void CFileRangeControl::SetNewLimits(CB startRange, CB endRange)
{
	WCHAR textBuffer[255];

	m_cbStart = startRange;
	m_cbEnd = endRange;

	if(m_cbStart > 0)
	{
		wsprintf(textBuffer, L"%I64u", m_cbStart);
		m_staticStart.SetWindowText(textBuffer);
	}

	if(m_cbEnd < m_pNDBViewer->GetHeader().root.ibFileEof)
	{
		wsprintf(textBuffer, L"%I64u", m_cbEnd);
		m_staticEnd.SetWindowText(textBuffer);
	}
	if(m_pBM)
	{
		delete m_pBM;
		m_pBM = NULL;
	}
}

BEGIN_MESSAGE_MAP(CFileRangeControl, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// CFileRangeControl message handlers

COLORREF s_blockColor = RGB(175,60,60);
COLORREF s_pageColor = RGB(60,175,175);
COLORREF s_metaColor = RGB(60,175,60);

void CFileRangeControl::OnPaint()
{
	CPaintDC dcPaint(this); // device context for painting
	CDC dc;
	CBitmap * pOld = NULL;
	RECT client;
	RECT fileBar;
	BLENDFUNCTION bf;
	// pens
	CPen blackPen;
	CPen * pOriginalPen;
	// brushes
	CBrush blackBrush;
	CPoint pointElipse(6,6);

	bool fLeftArrow = false;
	bool fRightArrow = false;
	bool fSelection = false;

	GetClientRect(&client);
	dc.CreateCompatibleDC(&dcPaint);
	dc.SetBkColor(dcPaint.GetBkColor());

	if(!m_pBM)
	{
		// create/initialize everything here
		blackPen.CreateStockObject(BLACK_PEN);
		blackBrush.CreateSolidBrush(RGB(0,0,0));
		bf.BlendOp = AC_SRC_OVER;
		bf.BlendFlags = 0;
		bf.SourceConstantAlpha = 170;
		bf.AlphaFormat = 0;

		m_pBM = new CBitmap();
		m_pBM->CreateCompatibleBitmap(&dcPaint, client.right, client.bottom);
		pOld = dc.SelectObject(m_pBM);

		// Get the right background color
		dc.BitBlt(client.left, client.top, client.right, client.bottom, &dcPaint, 0, 0, SRCCOPY);

		// start with client area, adjust to make 33% shorter
		fileBar = client;
		fileBar.bottom = client.bottom / 2;
		fileBar.top += 2; // give it a little room for the selection

		pOriginalPen = dc.SelectObject(&blackPen);
		dc.RoundRect(&fileBar, pointElipse);

		// draw all the ranges
		POSITION pos = m_data.GetHeadPosition();
		for(int i = 0; i < m_data.GetCount(); i++)
		{
			RangeData rd = m_data.GetNext(pos);
			RECT range;
			CDC srcDC;
			CBitmap bmp;
			CBitmap *pOldBmp;
			CBrush rdBrush;
			CRgn rdRgn;

			if(rd.type == TYPE_BLOCK)
				rdBrush.CreateSolidBrush(s_blockColor);
			else if(rd.type == TYPE_PAGE)
				rdBrush.CreateSolidBrush(s_pageColor);
			else if(rd.type == TYPE_META)
				rdBrush.CreateSolidBrush(s_metaColor);

			srcDC.CreateCompatibleDC(&dc);
			bmp.CreateCompatibleBitmap(&dc, client.right, client.bottom);
			pOldBmp = srcDC.SelectObject(&bmp);

			range.top = fileBar.top + 1;
			range.bottom = fileBar.bottom - 1;
			range.left = (LONG)(((fileBar.right - fileBar.left) * ((LONGLONG)rd.start - (LONGLONG)m_cbStart)) / ((LONGLONG)m_cbEnd - (LONGLONG)m_cbStart));
			range.right = (LONG)(((fileBar.right - fileBar.left) * ((LONGLONG)rd.end - (LONGLONG)m_cbStart)) / ((LONGLONG)m_cbEnd - (LONGLONG)m_cbStart));

			if(range.left < fileBar.left)
			{
				range.left = fileBar.left;
				fLeftArrow = true;
			}
			if(range.right > fileBar.right)
			{
				range.right = fileBar.right;
				fRightArrow = true;
			}

			// if this is the "selected" block, we draw it bigger and last
			if(rd.start == selection.start && rd.end == selection.end)
			{
				fSelection = true;
				srcDC.SelectObject(pOldBmp);
				continue;
			}

			if(range.right - range.left < m_minWidth)
			{
				range.left -= m_minWidth / 2;
				range.right += m_minWidth / 2;
				if(range.left < fileBar.left)
				{
					range.right += fileBar.left - range.left;
					range.left = fileBar.left;
				}
				if(range.right > fileBar.right)
				{
					range.left -= range.right - fileBar.right;
					range.right = fileBar.right;
				}
			}

			// now that we got the coords all figured out, draw it
			srcDC.FillRect(&range, &rdBrush);
			dc.AlphaBlend(range.left, range.top, range.right-range.left, range.bottom-range.top, &srcDC, range.left, range.top, range.right-range.left, range.bottom-range.top, bf);
			srcDC.SelectObject(pOldBmp);
		}

		if(fSelection)
		{
			CBrush rdBrush;
			RECT range;

			if(selection.type == TYPE_BLOCK)
				rdBrush.CreateSolidBrush(s_blockColor);
			else if(selection.type == TYPE_PAGE)
				rdBrush.CreateSolidBrush(s_pageColor);
			else if(selection.type == TYPE_META)
				rdBrush.CreateSolidBrush(s_metaColor);

			range.top = fileBar.top - 2;
			range.bottom = fileBar.bottom + 2;
			range.left = (LONG)(((fileBar.right - fileBar.left) * ((LONGLONG)selection.start - (LONGLONG)m_cbStart)) / ((LONGLONG)m_cbEnd - (LONGLONG)m_cbStart)) - 2;
			range.right = (LONG)(((fileBar.right - fileBar.left) * ((LONGLONG)selection.end - (LONGLONG)m_cbStart)) / ((LONGLONG)m_cbEnd - (LONGLONG)m_cbStart)) + 2;

			if(range.left < fileBar.left)
				range.left = fileBar.left;
			if(range.right > fileBar.right)
				range.right = fileBar.right;

			if(range.right - range.left < m_minWidth)
			{
				range.left -= m_minWidth / 2;
				range.right += m_minWidth / 2;
				if(range.left < fileBar.left)
				{
					range.right += fileBar.left - range.left;
					range.left = fileBar.left;
				}
				if(range.right > fileBar.right)
				{
					range.left -= range.right - fileBar.right;
					range.right = fileBar.right;
				}
			}

			// now that we got the coords all figured out, draw it
			dc.FillRect(&range, &rdBrush);
		}

		// do we need to draw a left arrow?
		if(fLeftArrow)
		{
			CRgn leftTriangle;
			POINT points[3];
			points[0].y = fileBar.bottom / 2;
			points[1].y = fileBar.bottom / 4;
			points[2].y = fileBar.bottom * 3 / 4;
			points[0].x = (points[2].y - points[1].y) / 2;
			points[1].x = points[2].x = points[0].x * 2;

			leftTriangle.CreatePolygonRgn(points, 3, WINDING);

			dc.FillRgn(&leftTriangle, &blackBrush);
		}

		// do we need to draw a right arrow?
		if(fRightArrow)
		{
			CRgn rightTriangle;
			POINT points[3];
			points[0].y = fileBar.bottom / 2;
			points[1].y = fileBar.bottom / 4;
			points[2].y = fileBar.bottom * 3 / 4;
			points[0].x = fileBar.right - ((points[2].y - points[1].y) / 2);
			points[1].x = points[2].x = points[0].x - (fileBar.right - points[0].x);

			rightTriangle.CreatePolygonRgn(points, 3, WINDING);

			dc.FillRgn(&rightTriangle, &blackBrush);		
		}

		// select back old objects..
		dc.SelectObject(pOriginalPen);
	}
	else
	{
		pOld = dc.SelectObject(m_pBM);
	}

	dcPaint.BitBlt(client.left, client.top, client.right, client.bottom, &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOld);
}

void CFileRangeControl::PreSubclassWindow()
{
	RECT client;
	RECT leftStatic;
	RECT rightStatic;
	WCHAR textBuffer[255];

	GetClientRect(&client);

	// Setup start label
	leftStatic.top = client.bottom / 2 + 2;
	leftStatic.bottom = client.bottom;
	leftStatic.left = client.left;
	leftStatic.right = client.right / 2;
	m_staticStart.Create(L"0", WS_VISIBLE | WS_CHILD | SS_LEFT, leftStatic, this, 0);
	m_staticStart.SetFont(GetGlobalFont());
	if(m_cbStart > 0)
	{
		wsprintf(textBuffer, L"%I64u", m_cbStart);
		m_staticStart.SetWindowText(textBuffer);
	}
	
	// Setup end label
	rightStatic.top = client.bottom / 2 + 2;
	rightStatic.bottom = client.bottom;
	rightStatic.left = client.right / 2;
	rightStatic.right = client.right;
	m_staticEnd.Create(L"EOF", WS_VISIBLE | WS_CHILD | SS_RIGHT, rightStatic, this, 0);
	m_staticEnd.SetFont(GetGlobalFont());
	if(m_cbEnd < m_pNDBViewer->GetHeader().root.ibFileEof)
	{
		wsprintf(textBuffer, L"%I64u", m_cbEnd);
		m_staticEnd.SetWindowText(textBuffer);
	}

	// Establish min-width
	m_minWidth = client.right / 50;

	CWnd::PreSubclassWindow();
}

void CFileRangeControl::AddRange(CB start, CB end, RangeType rt)
{
	RangeData rd;
	rd.start = start;
	rd.end = end;
	rd.type = rt;
	m_data.AddHead(rd);
	if(m_pBM)
	{
		delete m_pBM;
		m_pBM = NULL;
	}
}

void CFileRangeControl::SelectRange(CB start, CB end, RangeType rt)
{
	selection.start = start;
	selection.end = end;
	selection.type = rt;
	if(m_pBM)
	{
		delete m_pBM;
		m_pBM = NULL;
	}
	Invalidate();
}