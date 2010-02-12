#pragma once
#include "customcontrols.h"
#include "afxwin.h"
#include "NDBViewChildDlg.h"
#include "afxcmn.h"


// HNInspector dialog

class HNInspector : public CNDBViewChildDlg
{
	DECLARE_DYNAMIC(HNInspector)

public:
	HNInspector(NID nidParent, NID nid, LTPViewer * pLTPViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent = NULL);
	virtual ~HNInspector();

// Dialog Data
	enum { IDD = IDD_HN_INSPECTOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CFileRangeControl m_frc;
public:
	CListBox m_pageList;
public:
	CListBox m_allocationList;

private:
	LTPViewer * m_pLTPViewer;
	NID m_nidParent;
	NID m_nid;
	virtual BOOL OnInitDialog();
	afx_msg void OnLbnSelchangeList2();
	afx_msg void OnLbnDblclkList2();
	afx_msg void OnLbnDblclkList4();
	afx_msg void OnLbnSelchangeList4();
};
#pragma once


// BTHInspector dialog

class BTHInspector : public CNDBViewChildDlg
{
	DECLARE_DYNAMIC(BTHInspector)

public:
	BTHInspector(NID nidParent, NID nid, HID hidRoot, LTPViewer * pLTPViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent = NULL);	virtual ~BTHInspector();

// Dialog Data
	enum { IDD = IDD_BTH_INSPECTOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	LTPViewer * m_pLTPViewer;
	NID m_nidParent;
	NID m_nid;
	HID m_hidRoot;
	CFileRangeControl m_frc;
	CImageList m_il;
	CTreeCtrl m_tree;
	virtual BOOL OnInitDialog();
	afx_msg void OnNMDblclkTree1(NMHDR *pNMHDR, LRESULT *pResult);
};
#pragma once


// IndexRootInspector dialog

class IndexRootInspector : public CNDBViewChildDlg
{
	DECLARE_DYNAMIC(IndexRootInspector)

public:
	IndexRootInspector(NID nid, LTPViewer * pLTPViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent = NULL);   // standard constructor
	virtual ~IndexRootInspector();

// Dialog Data
	enum { IDD = IDD_INDEXROOT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	LTPViewer * m_pLTPViewer;
	NID m_nid;
	virtual BOOL OnInitDialog();
	CTreeCtrl m_tree;
	afx_msg void OnNMDblclkTree1(NMHDR *pNMHDR, LRESULT *pResult);
};
#pragma once


// PCInspector dialog

class PCInspector : public CNDBViewChildDlg
{
	DECLARE_DYNAMIC(PCInspector)

public:
	PCInspector(NID nidParent, NID nid, LTPViewer * pLTPViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent = NULL);   // standard constructor
	virtual ~PCInspector();

// Dialog Data
	enum { IDD = IDD_PC_INSPECTOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	NID m_nidParent;
	NID m_nid;
	LTPViewer * m_pLTPViewer;

	DECLARE_MESSAGE_MAP()
public:
	CFileRangeControl m_frc;
public:
	virtual BOOL OnInitDialog();
public:
	CListCtrl m_lc;
};
