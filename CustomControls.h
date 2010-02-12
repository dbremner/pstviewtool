#pragma once
#include "afxwin.h"
#include "ndbport.h"
#include "LTPViewer.h"

inline void RegisterClass(LPCTSTR name)
{
    WNDCLASS wndcls;
    HINSTANCE hInst = AfxGetInstanceHandle();

    if (!(::GetClassInfo(hInst, name, &wndcls)))
    {
        // otherwise we need to register a new class
        wndcls.style            = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wndcls.lpfnWndProc      = ::DefWindowProc;
        wndcls.cbClsExtra       = wndcls.cbWndExtra = 0;
        wndcls.hInstance        = hInst;
        wndcls.hIcon            = NULL;
        wndcls.hCursor          = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
        wndcls.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
        wndcls.lpszMenuName     = NULL;
        wndcls.lpszClassName    = name;

        if (!AfxRegisterClass(&wndcls))
        {
            AfxThrowResourceException();
        }
    }
}

// CBlockTrailerControl
class CBlockTrailerControl : public CWnd
{
	DECLARE_DYNAMIC(CBlockTrailerControl)

public:
	CBlockTrailerControl();
	CBlockTrailerControl(const BREF& bref, CB cb, NDBViewer * pNDBViewer);
	virtual ~CBlockTrailerControl();

protected:
	DECLARE_MESSAGE_MAP()
	virtual void PreSubclassWindow();
private:
	CButton m_groupBox;
	CStatic m_labels;
	CStatic m_values;

	BLOCKTRAILER m_bt;
	BOOL m_fValidCRC;
	WORD m_wSig;
};

// CBlockTrailerControl
class CPageTrailerControl : public CWnd
{
	DECLARE_DYNAMIC(CPageTrailerControl)

public:
	CPageTrailerControl();
	CPageTrailerControl(IB ib, NDBViewer * pNDBViewer);
	virtual ~CPageTrailerControl();

protected:
	DECLARE_MESSAGE_MAP()
	virtual void PreSubclassWindow();
private:
	CButton m_groupBox;
	CStatic m_labels;
	CStatic m_values;

	PAGETRAILER m_pt;
	BOOL m_fValidCRC;
};

#pragma once


// CBinaryListBox
enum BinaryListBoxMode
{
	MODE_BINARY = 1,
	MODE_HEX = 2,
	MODE_UNICODE = 3
};

#define HEX_BYTES_PER_LINE 12
#define BINARY_BYTES_PER_LINE 4
#define UNICODE_BYTES_PER_LINE 64

#define NULL_WCHAR_REPLACEMENT L'?'

class CBinaryListBox : public CListBox
{
	DECLARE_DYNAMIC(CBinaryListBox)

public:
	CBinaryListBox();
	void SetData(BYTE * pData, UINT size, IB ibStart);
	void SetMode(BinaryListBoxMode m) { m_mode = m; Populate(); }
	virtual ~CBinaryListBox();

protected:
	void Populate();
	void ByteToString(LPWSTR pBuffer, BYTE b);
	BinaryListBoxMode m_mode;
	BYTE * m_pData;
	UINT m_dataSize;
	IB m_ibStart;

	DECLARE_MESSAGE_MAP()
};


#pragma once


// CFileRangeControl
enum RangeType
{
	TYPE_BLOCK = 1,
	TYPE_PAGE,
	TYPE_META
};

struct RangeData
{
	CB start;
	CB end;
	RangeType type;
};

class CFileRangeControl : public CWnd
{
	DECLARE_DYNAMIC(CFileRangeControl)

public:
	CFileRangeControl(CB startRange, CB endRange, NDBViewer * pNDBViewer);
	CFileRangeControl(NDBViewer * pNDBViewer);
	virtual ~CFileRangeControl();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();

	void SetNewLimits(CB startRange, CB endRange);
	void AddRange(CB start, CB end, RangeType rt);
	void SelectRange(CB start, CB end, RangeType rt);
private:
	CB m_cbStart;
	CB m_cbEnd;
	LONG m_minWidth;
	RangeData selection;

	NDBViewer * m_pNDBViewer;
	CStatic m_staticStart;
	CStatic m_staticEnd;
	CList<RangeData> m_data;
	CBitmap * m_pBM;
protected:
	virtual void PreSubclassWindow();
};


