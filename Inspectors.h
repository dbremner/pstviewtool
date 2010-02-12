#pragma once
#include "afxwin.h"
#include "customcontrols.h"
#include "afxcmn.h"
#include "ndbviewdlg.h"
#include "NDBViewChildDlg.h"

enum {
	inspBlock,
	inspXBlock,
	inspSBlock,
	inspBinaryData,
	inspPage,
	inspBTreePage,
	inspAutoDetect,
	inspMaxInspector
};

static const WCHAR*	listvalues[] = {
	L"Block",
	L"XBlock",
	L"SBlock",
	L"Binary Data",
	L"Page",
	L"BTree Page",
	L"Auto Detect",
};

// InspectorPicker dialog

class InspectorPicker : public CDialog
{
	DECLARE_DYNAMIC(InspectorPicker)

public:
	InspectorPicker(CWnd* pParent = NULL);   // standard constructor
	virtual BOOL OnInitDialog();
	virtual ~InspectorPicker();

// Dialog Data
	enum { IDD = IDD_INSPECTOR_PICKER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	ULONGLONG m_start;
public:
	UINT m_length;
public:
	int m_type;
	CComboBox m_typepick;
public:
	afx_msg void OnCbnSelchangeCombo1();
public:
	CEdit m_sizecontrol;
};
#pragma once


// CBinaryInspector dialog

class CBinaryInspectorBase : public CNDBViewChildDlg
{
	DECLARE_DYNAMIC(CBinaryInspectorBase)

public:
	CBinaryInspectorBase(NDBViewer * pNDBViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent = NULL);
	virtual ~CBinaryInspectorBase();

	virtual void SetBinaryData() = 0;
	virtual void SetWindowTitle() = 0;
	virtual void SetupFRC() = 0;
// Dialog Data
	enum { IDD = IDD_BINARY_DATA_INSPECTOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
protected:
	CFileRangeControl m_frc;
	NDBViewer * m_pNDBViewer;

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio3();
protected:
	CBinaryListBox m_blb;
};
#pragma once

class CBinaryInspector : public CBinaryInspectorBase
{

public:
	CBinaryInspector(IB ib, UINT cb, NDBViewer * pNDBViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent = NULL);   // standard constructor
	virtual ~CBinaryInspector();

	void SetBinaryData();
	void SetWindowTitle();
	void SetupFRC();

private:
	UINT m_cb;
	IB m_ib;
};

class CLogicalBlockInspector : public CBinaryInspectorBase
{
public:
	CLogicalBlockInspector(NID nidParent, NID nid, NDBViewer * pNDBViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent = NULL);   // standard constructor
	~CLogicalBlockInspector();

	void SetBinaryData();
	void SetWindowTitle();
	void SetupFRC();

private:
	UINT m_cb;
	NID m_nid;
	NID m_nidParent;
};

class CHIDInspector : public CBinaryInspectorBase
{
public:
	CHIDInspector(NID nidParent, NID nid, HID hid, LTPViewer * pNDBViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent = NULL);   // standard constructor
	~CHIDInspector();

	void SetBinaryData();
	void SetWindowTitle();
	void SetupFRC();

private:
	UINT m_cb;
	NID m_nid;
	NID m_nidParent;
	HID m_hid;
};
// CBlockInspector dialog

class CBlockInspector : public CNDBViewChildDlg
{
	DECLARE_DYNAMIC(CBlockInspector)

public:
	CBlockInspector(const BREF& bref, UINT cb, NDBViewer * pNDBViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent = NULL);   // standard constructor
	virtual ~CBlockInspector();

// Dialog Data
	enum { IDD = IDD_BLOCK_INSPECTOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CFileRangeControl m_frc;
	BREF m_bref;
	UINT m_cb;
	NDBViewer * m_pNDBViewer;

private:
	CBlockTrailerControl m_btc;
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio3();
	virtual BOOL OnInitDialog();
private:
	CBinaryListBox m_blb;
};
#pragma once


// CPageInspector dialog

class CPageInspector : public CNDBViewChildDlg
{
	DECLARE_DYNAMIC(CPageInspector)

public:
	CPageInspector(IB ib, NDBViewer * pNDBViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent = NULL);   // standard constructor
	virtual ~CPageInspector();
// Dialog Data
	enum { IDD = IDD_PAGE_INSPECTOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedRadio1();
public:
	afx_msg void OnBnClickedRadio2();
private:
	CFileRangeControl m_frc;
	CPageTrailerControl m_ptc;
	CBinaryListBox m_blb;
	NDBViewer * m_pNDBViewer;
	IB m_ib;
	virtual BOOL OnInitDialog();
};
#pragma once


// CXBlockInspector dialog

class CXBlockInspector : public CNDBViewChildDlg
{
	DECLARE_DYNAMIC(CXBlockInspector)

public:
	CXBlockInspector(const BREF& bref, UINT cb, NDBViewer * pNDBViewer, CNDBViewDlg * pNDBDlg, CWnd* pParent = NULL);   // standard constructor
	virtual ~CXBlockInspector();

// Dialog Data
	enum { IDD = IDD_XBLOCK_INSPECTOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CBlockTrailerControl m_btc;
	CFileRangeControl m_frc;
	CListBox m_lb;
	BREF m_bref;
	UINT m_cbUnaligned;
	UINT m_cbAligned;
	BYTE * m_pData;
	NDBViewer * m_pNDBViewer;

public:
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnLbnDblclkList1();
};
#pragma once


// CSBlockInspector dialog

class CSBlockInspector : public CNDBViewChildDlg
{
	DECLARE_DYNAMIC(CSBlockInspector)

public:
	CSBlockInspector(const BREF& bref, UINT cb, NDBViewer * pNDBViewer, CNDBViewDlg * pNDBDlg, CWnd* pParent = NULL);   // standard constructor
	virtual ~CSBlockInspector();
// Dialog Data
	enum { IDD = IDD_SBLOCK_INSPECTOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CFileRangeControl m_frc;
	CBlockTrailerControl m_btc;
	CTreeCtrl m_tc;
	CImageList m_il;
	BREF m_bref;
	UINT m_cbAligned;
	UINT m_cbUnaligned;
	BYTE * m_pData;
	NDBViewer * m_pNDBViewer;
	virtual BOOL OnInitDialog();
	afx_msg void OnTvnDeleteitemTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnItemexpandingTree1(NMHDR *pNMHDR, LRESULT *pResult);
public:
	afx_msg void OnNMDblclkTree1(NMHDR *pNMHDR, LRESULT *pResult);
};
#pragma once


// CBTPageInspector dialog

class CBTPageInspector : public CNDBViewChildDlg
{
	DECLARE_DYNAMIC(CBTPageInspector)

public:
	CBTPageInspector(IB ib, NDBViewer * pNDBViewer, CNDBViewDlg * pNDBDlg, CWnd* pParent = NULL);   // standard constructor
	virtual ~CBTPageInspector();

// Dialog Data
	enum { IDD = IDD_BT_INSPECTOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CFileRangeControl m_frc;
	CPageTrailerControl m_ptc;
	CTreeCtrl m_tc;
	CImageList m_il;
	PAGETRAILER m_pt;
	IB m_ib;
	NDBViewer * m_pNDBViewer;
public:
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnTvnDeleteitemTree1(NMHDR *pNMHDR, LRESULT *pResult);
public:
	afx_msg void OnNMDblclkTree1(NMHDR *pNMHDR, LRESULT *pResult);
public:
	afx_msg void OnTvnItemexpandingTree1(NMHDR *pNMHDR, LRESULT *pResult);
};
#pragma once


// CScavengeRange dialog

class CScavengeRange : public CDialog
{
	DECLARE_DYNAMIC(CScavengeRange)

public:
	CScavengeRange(CWnd* pParent = NULL);   // standard constructor
	virtual ~CScavengeRange();

// Dialog Data
	enum { IDD = IDD_SCAVENGE_RANGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	long m_length;
public:
	ULONGLONG m_start;
public:
	int m_fFreeSpaceOnly;
};
#pragma once


// CScavengeResults dialog

class CScavengeResults : public CNDBViewChildDlg
{
	DECLARE_DYNAMIC(CScavengeResults)

public:
	CScavengeResults(IB start, UINT cb, bool fFreeOnly, NDBViewer * pNDBViewer, CNDBViewDlg * pNDBDlg, CWnd* pParent = NULL);   // standard constructor
	virtual ~CScavengeResults();

// Dialog Data
	enum { IDD = IDD_SCAVENGE_RESULTS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CFileRangeControl m_frc;
	IB m_start;
	UINT m_length;
	bool m_fFreeOnly;
	NDBViewer * m_pNDBViewer;
	CImageList m_il;
	CListCtrl m_lc;

public:
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
public:
	afx_msg void OnLvnDeleteitemList1(NMHDR *pNMHDR, LRESULT *pResult);
public:
	afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
};
