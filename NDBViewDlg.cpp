// NDBViewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NDBView.h"
#include "NDBViewDlg.h"
#include "AMapDialog.h"
#include "TestDialog.h"
#include "BTDialog.h"
#include "Inspectors.h"
#include "LTPInspectors.h"
#include "AMRResumeDlg.h"
#include "RefReport.h"
#include "NidSize.h"
#include "NDBStats.h"
#include "ConsistencyReport.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BOOL DoConsistencyCheck(CString filename)
{
	BOOL ret = TRUE;
	try {
		LTPViewer pViewer(filename);
		CConsistencyReport::JustRunReport(&pViewer);
	}
	catch(...)
	{
		ret = FALSE;
	}

	return ret;
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CNDBViewDlg dialog




CNDBViewDlg::CNDBViewDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNDBViewDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_fClosingChildren = false;
	m_ndbViewer = NULL;
}

void CNDBViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE1, m_nbt);
	DDX_Control(pDX, IDC_TREE2, m_bbt);
	DDX_Control(pDX, IDC_TREE4, m_header);
}

BEGIN_MESSAGE_MAP(CNDBViewDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FILE_OPEN32771, &CNDBViewDlg::OnFileOpen32771)
	ON_COMMAND(ID_VIEW_ADDRESS, &CNDBViewDlg::OnViewAddress)
	ON_COMMAND(ID_FILE_CLOSE32775, &CNDBViewDlg::OnFileClose32775)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE1, &CNDBViewDlg::OnTvnItemexpandingTree1)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE2, &CNDBViewDlg::OnTvnItemexpandingTree2)
	ON_NOTIFY(TVN_DELETEITEM, IDC_TREE1, &CNDBViewDlg::OnTvnDeleteitemTree1)
	ON_NOTIFY(TVN_DELETEITEM, IDC_TREE2, &CNDBViewDlg::OnTvnDeleteitemTree2)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE1, &CNDBViewDlg::OnNMDblclkTree1)
	ON_COMMAND(ID_VIEW_ALLOCATIONMAP, &CNDBViewDlg::OnViewAllocationmap)
	ON_COMMAND(ID_VIEW_BTREEPAGEDISTRIBUTION, &CNDBViewDlg::OnViewBtreepagedistribution)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE2, &CNDBViewDlg::OnNMDblclkTree2)
	ON_COMMAND(ID_VIEW_SCAVENGE, &CNDBViewDlg::OnViewScavenge)
	ON_COMMAND(ID_VIEW_CONSISTENCYREPORT, &CNDBViewDlg::OnViewConsistencyreport)
	ON_COMMAND(ID_VIEW_REFREPORT, &CNDBViewDlg::OnViewRefreport)
	ON_COMMAND(ID_VIEW_NODESIZE, &CNDBViewDlg::OnViewNodesize)
	ON_COMMAND(ID_VIEW_STATISTICS, &CNDBViewDlg::OnViewStatistics)
	ON_MESSAGE(WM_DELAY_PERFORM_ACTION, &CNDBViewDlg::PerformActionHandler)
	ON_COMMAND(ID_HELP_ABOUTNDBVIEW, &CNDBViewDlg::OnHelpAboutndbview)
	ON_COMMAND(ID_FILE_EXIT, &CNDBViewDlg::OnFileExit)
	ON_COMMAND(ID_VIEW_AMRINFO, &CNDBViewDlg::OnViewAmrinfo)
END_MESSAGE_MAP()


// CNDBViewDlg message handlers

BOOL CNDBViewDlg::OnInitDialog()
{
	CBitmap cb;

	CDialog::OnInitDialog();

	cb.LoadBitmap(IDB_BITMAP2);
	m_il.Create(16, 16, ILC_COLOR24, 10, 1);
	m_il.Add(&cb, RGB(255, 0, 255));
	m_il.SetBkColor(CLR_NONE);
	m_bbt.SetImageList(&m_il, TVSIL_NORMAL);
	m_nbt.SetImageList(&m_il, TVSIL_NORMAL);

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CNDBViewDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CNDBViewDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CNDBViewDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CNDBViewDlg::OnDestroy()
{
	Clear();
	CDialog::OnDestroy();
}

void CNDBViewDlg::OnFileOpen32771()
{
	OPENFILENAME ofn;
	WCHAR filename[255];

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFile = filename;

	ofn.lpstrFile[0] = L'\0';
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrFilter = L"*.OST, *.PST\0*.PST;*.OST\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if(GetOpenFileName(&ofn) == TRUE)
	{
		WCHAR buffer[256];
		CWaitCursor wait;
		Clear();

		wsprintf(buffer, L"NDBView - %s", ofn.lpstrFile);
		SetWindowText(buffer);
		m_ndbViewer = new LTPViewer(ofn.lpstrFile);
		m_ndbViewer->PopulateNBT(&m_nbt);
		m_ndbViewer->PopulateBBT(&m_bbt);
		m_ndbViewer->PopulateHeader(&m_header);
#ifndef SMALL_PST
		if(m_ndbViewer->GetHeader().wVer != NDBUNI_BIGSBLOCKLEAF_VERSION)
		{
			::MessageBox(m_hWnd, L"This does not appear to be a unicode store. This version of NDBView works on unicode stores only.", L"ANSI store detected", MB_OK | MB_ICONWARNING);
		} else
#endif
		if(!m_ndbViewer->FValidCRC() || !m_ndbViewer->FValidPartialCRC())
		{
			::MessageBox(m_hWnd, L"The header failed a CRC check. Access to data in this store is limited", L"Header CRC Failure", MB_OK | MB_ICONWARNING);
		}
		GetMenu()->EnableMenuItem(1, MF_ENABLED | MF_BYPOSITION);
		DrawMenuBar(); 
	}
}

void CNDBViewDlg::Clear(void)
{
	CWaitCursor wait;
	
	CloseAllChildDialogs();

	SetWindowText(L"NDBView");
	if(m_ndbViewer)
	{
		delete m_ndbViewer;
		m_ndbViewer = NULL;
	}

	m_nbt.DeleteAllItems();
	m_bbt.DeleteAllItems();
	m_header.DeleteAllItems();
	GetMenu()->EnableMenuItem(1, MF_GRAYED | MF_BYPOSITION);
	DrawMenuBar();
}

void CNDBViewDlg::OnViewAddress()
{
	InspectorPicker ip(this);

	CWnd * pFocus = GetFocus();
	if(pFocus->GetDlgCtrlID() == IDC_TREE1 
		|| pFocus->GetDlgCtrlID() == IDC_TREE2 
		|| pFocus->GetDlgCtrlID() == IDC_TREE4)
	{
		CTreeCtrl* pTree = (CTreeCtrl *)pFocus;
		HTREEITEM hItem = pTree->GetSelectedItem();
		NodeData * pNodeData = NULL;

		if(hItem != NULL)
			pNodeData = (NodeData *)pTree->GetItemData(hItem);

		if(pNodeData)
		{
			ip.m_start = pNodeData->bref.ib;
			ip.m_length = (UINT)pNodeData->cb;
		}
	}

	if(IDOK == ip.DoModal())
	{
		NodeData nd;

		nd.bref.ib = ip.m_start;
		nd.bref.bid = 0;
		nd.cb = ip.m_length;

		if(ip.m_type == inspAutoDetect)
		{
		//	m_ndbViewer.PickInspector(&type, &address, &size);
		}

		switch(ip.m_type)
		{
		case inspPage:
			nd.ulFlags = aOpenPage;
			break;
		case inspBlock:
			nd.ulFlags = aOpenBlock;
			break;
		case inspXBlock:
			nd.ulFlags = aOpenXBlock;
			break;
		case inspSBlock:
			nd.ulFlags = aOpenSBlock;
			break;
		case inspBinaryData:
			nd.ulFlags = aOpenBinary;
			break;
		case inspBTreePage:
			nd.ulFlags = aOpenBTPage;
			break;
		default:
			::AfxMessageBox(L"Unknown Inspector Type");
			break;
		}

		PerformAction(&nd);
	}
}

void CNDBViewDlg::OnFileClose32775()
{
	Clear();
}

void CNDBViewDlg::OnTvnItemexpandingTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	if(pNMTreeView->hdr.code == TVN_ITEMEXPANDING)
	{
		m_ndbViewer->AddBTNodeChildren(&m_nbt, pNMTreeView->itemNew.hItem);
	}

	*pResult = 0;
}

void CNDBViewDlg::OnTvnItemexpandingTree2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	if(pNMTreeView->hdr.code == TVN_ITEMEXPANDING)
	{
		m_ndbViewer->AddBTNodeChildren(&m_bbt, pNMTreeView->itemNew.hItem);
	}

	*pResult = 0;
}

void CNDBViewDlg::OnTvnDeleteitemTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	
	// TODO: Add your control notification handler code here
	if(pNMTreeView->hdr.code == TVN_DELETEITEM)
	{
		m_ndbViewer->DeleteNode(&m_nbt, pNMTreeView->itemOld.hItem);
	}

	*pResult = 0;
}

void CNDBViewDlg::OnTvnDeleteitemTree2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	
	if(pNMTreeView->hdr.code == TVN_DELETEITEM)
	{
		m_ndbViewer->DeleteNode(&m_nbt, pNMTreeView->itemOld.hItem);
	}

	*pResult = 0;
}

void CNDBViewDlg::OnNMDblclkTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	HTREEITEM hItem = m_nbt.GetSelectedItem();
	NodeData * pNodeData = NULL;
	Unreferenced(pNMHDR);
	
	// Do we have an item?
	if(hItem)
	{
		// Does it have data?
		pNodeData = (NodeData *)m_nbt.GetItemData(hItem);
	}

	PerformAction(pNodeData);

	*pResult = 0;
}

void CNDBViewDlg::OnOK(void)
{
	// Eat "OnOK", otherwise hitting Enter will quit the app
}

void CNDBViewDlg::OnViewAllocationmap()
{
	CDialog * d = new CAMapDialog(m_ndbViewer, this);
	d->Create(CAMapDialog::IDD);
	d->ShowWindow(SW_SHOW);
}

void CNDBViewDlg::OnViewBtreepagedistribution()
{
	CDialog * d = new CBTDialog(m_ndbViewer, this);
	d->Create(CBTDialog::IDD);
	d->ShowWindow(SW_SHOW);
}

void CNDBViewDlg::OnNMDblclkTree2(NMHDR *pNMHDR, LRESULT *pResult)
{
	HTREEITEM hItem = m_bbt.GetSelectedItem();
	NodeData * pNodeData = NULL;
	Unreferenced(pNMHDR);
	
	// Do we have an item?
	if(hItem)
	{
		// Does it have data?
		pNodeData = (NodeData *)m_bbt.GetItemData(hItem);
	}

	PerformAction(pNodeData);

	*pResult = 0;
}

void CNDBViewDlg::PerformAction(NodeData * pNodeData)
{
	// A tree control sets focus on itself after broadcasting a double click
	// message. This is really annoying when you're trying to create dialogs 
	// inside of double click handlers. Insert standard clever work around.
	if(pNodeData)
	{
		NodeData * pNodeDataCopy = new NodeData;
		memcpy(pNodeDataCopy, pNodeData, sizeof(NodeData));
		::PostMessage(m_hWnd, WM_DELAY_PERFORM_ACTION, NULL, (LPARAM)pNodeDataCopy);
	}
}

afx_msg LRESULT CNDBViewDlg::PerformActionHandler(WPARAM wParam, LPARAM lParam)
{
	NodeData * pNodeData = (NodeData *)lParam;
	WCHAR errorString[255];
	CDialog * pDialog = NULL;
	Unreferenced(wParam);

	if(!pNodeData || !pNodeData->ulFlags) 
	{
		delete pNodeData;
		return 0;
	}

	// Browse to a specific block in the BBT, and set focus on that tree
	if(pNodeData->ulFlags & aBrowseBBT)
	{
		if(pNodeData->bref.bid)
		{
			if(!m_ndbViewer->SelectBID(&m_bbt, pNodeData->bref.bid))
			{
				wsprintf(errorString, L"Could not find BID 0x%I64X in BBT", pNodeData->bref.bid);
				::MessageBox(m_hWnd, errorString, L"BBT Lookup Error", MB_OK | MB_ICONSTOP);
			}
			::SendMessage(m_hWnd, WM_NEXTDLGCTL, (WPARAM)m_bbt.m_hWnd, TRUE);
		}
	}

	// Browse to a specific node in the NBT, and set focus on that tree
	if(pNodeData->ulFlags & aBrowseNBT)
	{
		if(pNodeData->nid)
		{
			if(!m_ndbViewer->SelectNID(&m_nbt, pNodeData->nid))
			{
				wsprintf(errorString, L"Could not find NID 0x%X in NBT", pNodeData->nid);
				::MessageBox(m_hWnd, errorString, L"NBT Lookup Error", MB_OK | MB_ICONSTOP);
			}
			::SendMessage(m_hWnd, WM_NEXTDLGCTL, (WPARAM)m_nbt.m_hWnd, TRUE);
		}
	}

	// Open an inspector for a block
	if(pNodeData->ulFlags & aOpenBlock)
	{
		BYTE b;

		m_ndbViewer->ReadBlock(&b, 1, NULL, pNodeData->bref.ib, CbAlignDisk(pNodeData->cb));
		if(BIDIsExternal(pNodeData->bref.bid) || (b != btypeXB && b != btypeSB))
		{
			pDialog = new CBlockInspector(pNodeData->bref, (UINT)pNodeData->cb, m_ndbViewer, this);
			pDialog->Create(CBlockInspector::IDD);
		}
		else
		{
			if(b == btypeXB)
			{
				pDialog = new CXBlockInspector(pNodeData->bref, (UINT)pNodeData->cb, m_ndbViewer, this);
				pDialog->Create(CXBlockInspector::IDD);
			}
			else
			{
				pDialog = new CSBlockInspector(pNodeData->bref, (UINT)pNodeData->cb, m_ndbViewer, this);
				pDialog->Create(CSBlockInspector::IDD);
			}
		}
	}

	// Open an sblock inspector (don't try to detect)
	if(pNodeData->ulFlags & aOpenSBlock)
	{
		pDialog = new CSBlockInspector(pNodeData->bref, (UINT)pNodeData->cb, m_ndbViewer, this);
		pDialog->Create(CSBlockInspector::IDD);
	}

	// Open an xblock inspector (don't try to detect)
	if(pNodeData->ulFlags & aOpenXBlock)
	{
		pDialog = new CXBlockInspector(pNodeData->bref, (UINT)pNodeData->cb, m_ndbViewer, this);
		pDialog->Create(CXBlockInspector::IDD);
	}

	// Open an inspector for a BT page
	if(pNodeData->ulFlags & aOpenBTPage)
	{
		pDialog = new CBTPageInspector(pNodeData->bref.ib, m_ndbViewer, this);
		pDialog->Create(CBTPageInspector::IDD);
	}

	// Open an inspector for a page
	if(pNodeData->ulFlags & aOpenPage)
	{
		pDialog = new CPageInspector(pNodeData->bref.ib, m_ndbViewer, this);
		pDialog->Create(CPageInspector::IDD);
	}

	// Last resort: Binary inspector
	if(pNodeData->ulFlags & aOpenBinary)
	{
		pDialog = new CBinaryInspector(pNodeData->bref.ib, (UINT)pNodeData->cb, m_ndbViewer, this);
		pDialog->Create(CBinaryInspector::IDD);
	}

	// Browse refs for this BID
	if(pNodeData->ulFlags & aBrowseRefs)
	{
		pDialog = new CRefReport(pNodeData->bref.bid, m_ndbViewer, this);
		pDialog->Create(CRefReport::IDD);
	}

	if(pNodeData->ulFlags & aViewNodeSize)
	{
		pDialog = new CNidSize(pNodeData->nidParent, pNodeData->nid, m_ndbViewer, this);
		pDialog->Create(CNidSize::IDD);
	}

	if(pNodeData->ulFlags & aOpenHN)
	{
		pDialog = new HNInspector(pNodeData->nidParent, pNodeData->nid, m_ndbViewer, this);
		pDialog->Create(HNInspector::IDD);
	}

	if(pNodeData->ulFlags & aOpenES)
	{
		pDialog = new CLogicalBlockInspector(pNodeData->nidParent, pNodeData->nid, m_ndbViewer, this);
		pDialog->Create(CLogicalBlockInspector::IDD);
	}

	if(pNodeData->ulFlags & aOpenHID)
	{
		pDialog = new CHIDInspector(pNodeData->nidParent, pNodeData->nid, pNodeData->hid, m_ndbViewer, this);
		pDialog->Create(CHIDInspector::IDD);
	}

	if(pNodeData->ulFlags & aOpenBTH)
	{
		pDialog = new BTHInspector(pNodeData->nidParent, pNodeData->nid, pNodeData->hid, m_ndbViewer, this);
		pDialog->Create(BTHInspector::IDD);
	}

	if(pNodeData->ulFlags & aOpenTCVIR)
	{
		pDialog = new IndexRootInspector(pNodeData->nid, m_ndbViewer, this);
		pDialog->Create(IndexRootInspector::IDD);
	}

	if(pNodeData->ulFlags & aOpenPC)
	{
		pDialog = new PCInspector(pNodeData->nidParent, pNodeData->nid, m_ndbViewer, this);
		pDialog->Create(PCInspector::IDD);
	}

	if(pDialog)
	{
		pDialog->ShowWindow(SW_SHOW);
		pDialog->SetFocus();
	}

	delete pNodeData;
	return 0;
}
void CNDBViewDlg::OnViewScavenge()
{
	CScavengeRange sr(this);

	CWnd * pFocus = GetFocus();
	if(pFocus->GetDlgCtrlID() == IDC_TREE1 
		|| pFocus->GetDlgCtrlID() == IDC_TREE2 
		|| pFocus->GetDlgCtrlID() == IDC_TREE4)
	{
		CTreeCtrl* pTree = (CTreeCtrl *)pFocus;
		HTREEITEM hItem = pTree->GetSelectedItem();
		NodeData * pNodeData = NULL;

		if(hItem != NULL)
			pNodeData = (NodeData *)pTree->GetItemData(hItem);

		if(pNodeData && (pNodeData->ulFlags & (aOpenPage|aOpenBlock|aBrowseRefs)))
		{
			sr.m_start = pNodeData->bref.ib;
			sr.m_length = (UINT)pNodeData->cb;
		}
	}

	if(IDOK == sr.DoModal())
	{
		CDialog * pDialog = new CScavengeResults(sr.m_start, sr.m_length, sr.m_fFreeSpaceOnly != 0, m_ndbViewer, this);
		pDialog->Create(CScavengeResults::IDD);
		pDialog->ShowWindow(SW_SHOW);
	}
}

void CNDBViewDlg::OnViewConsistencyreport()
{
	CDialog * pDialog = new CConsistencyReport(m_ndbViewer, this);
	pDialog->Create(CConsistencyReport::IDD);
	pDialog->ShowWindow(SW_SHOW);
}

void CNDBViewDlg::OnViewRefreport()
{
	CRefReportBID d;
	NodeData * pNodeData = NULL;
	CTreeCtrl * pTree = (CTreeCtrl*)GetDlgItem(IDC_TREE2);
	HTREEITEM hItem = pTree->GetSelectedItem();
	WCHAR buffer[255];
	bool fDone = false;

	if(hItem != NULL)
		pNodeData = (NodeData*)pTree->GetItemData(hItem);

	// If a block (or an info node below a block) is selected, prepopulate
	if(pNodeData && (pNodeData->ulFlags & (aOpenBlock|aBrowseRefs)))
	{
		wsprintf(buffer, L"0x%I64X", pNodeData->bref.bid);
		d.m_cs = buffer;
	}

	while(!fDone)
	{
		if(IDOK == d.DoModal())
		{
			CString cs = d.m_cs;
			BID bid = 0;

			// try hex first
			if(swscanf_s(cs, L"0x%X", &bid) > 0 && BIDStrip(bid) != 0)
			{
				fDone = true;
			}
			else if(swscanf_s(cs, L"%u", &bid) > 0 && BIDStrip(bid) != 0)
			{
				fDone = true;
			}
			else
			{
				::MessageBox(m_hWnd, L"Please enter a valid BID. Prefix hex with \"0x\".", L"Invalid Input", MB_OK | MB_ICONSTOP);
			}

			if(fDone)
			{
				NodeData n;
				n.bref.ib = 0;
				n.bref.bid = bid;
				n.ulFlags = aBrowseRefs;
				PerformAction(&n);
			}
		}
		else
		{
			fDone = true;
		}
	}
}

void CNDBViewDlg::OnViewNodesize()
{
	CGetNid gn;
	bool fDone = false;
	WCHAR buffer[255];
	CWnd * pFocus = GetFocus();

	if(pFocus->GetDlgCtrlID() == IDC_TREE1 
		|| pFocus->GetDlgCtrlID() == IDC_TREE2 
		|| pFocus->GetDlgCtrlID() == IDC_TREE4)
	{
		CTreeCtrl* pTree = (CTreeCtrl *)pFocus;
		HTREEITEM hItem = pTree->GetSelectedItem();
		NodeData * pNodeData = NULL;

		if(hItem != NULL)
			pNodeData = (NodeData *)pTree->GetItemData(hItem);

		wsprintf(buffer, L"0x0");
		gn.m_parentNid = buffer;
		gn.m_hid = buffer;

		if(pNodeData && (pNodeData->nid != 0))
		{
			wsprintf(buffer, L"0x%X", pNodeData->nid);
			gn.m_nid = buffer;
		}
	}

	while(!fDone)
	{
		if(IDOK == gn.DoModal())
		{
			CString cs = gn.m_nid;
			CString csP = gn.m_parentNid;
			CString csH = gn.m_hid;
			bool fValidNid = false;
			bool fValidParentNid = false;
			bool fValidHID = false;
			NID nid = 0;
			NID parentNid = 0;
			NID hid = 0;

			// try hex first
			if(swscanf_s(cs, L"0x%X", &nid) > 0 && nid != 0)
			{
				fValidNid = true;
			}
			else if(swscanf_s(cs, L"%u", &nid) > 0 && nid != 0)
			{
				fValidNid = true;
			}
			else
			{
				::MessageBox(m_hWnd, L"Please enter a valid NID. Prefix hex with \"0x\".", L"Invalid Input", MB_OK | MB_ICONSTOP);
			}

			if(swscanf_s(csP, L"0x%X", &parentNid) > 0)
			{
				fValidParentNid = true;
			}
			else if(swscanf_s(csP, L"%u", &parentNid) > 0)
			{
				fValidParentNid = true;
			}
			else
			{
				::MessageBox(m_hWnd, L"Please enter a valid parent NID (or 0, for a root node). Prefix hex with \"0x\".", L"Invalid Input", MB_OK | MB_ICONSTOP);
			}

			if(swscanf_s(csH, L"0x%X", &hid) > 0)
			{
				fValidHID = true;
			}
			else if(swscanf_s(csH, L"%u", &hid) > 0)
			{
				fValidHID = true;
			}
			else
			{
				::MessageBox(m_hWnd, L"Please enter a valid HID (or 0, for a root HID). Prefix hex with \"0x\".", L"Invalid Input", MB_OK | MB_ICONSTOP);
			}

			if(fValidNid && fValidParentNid && fValidHID)
			{
				NodeData nd;
				ZeroMemory(&nd, sizeof(NodeData));
				nd.nidParent = parentNid;
				nd.nid = nid;
				nd.hid = hid;

				if(gn.m_type == inspLTPAutoDetect)
				{
					//
				}
				switch(gn.m_type)
				{
				case inspNodeSize:
					nd.ulFlags = aViewNodeSize;
					break;
				case inspHN:
					nd.ulFlags = aOpenHN;
					break;
				case inspES:
					nd.ulFlags = aOpenES;
					break;
				case inspBTH:
					nd.ulFlags = aOpenBTH;
					break;
				case inspTCVRI:
					nd.ulFlags = aOpenTCVIR;
					break;
				case inspPC:
					nd.ulFlags = aOpenPC;
					break;
				default:
					::AfxMessageBox(L"Unknown Inspector Type");
					break;
				}

				PerformAction(&nd);
				fDone = true;
			}
		}
		else
		{
			fDone = true;
		}
	}
	
}

void CNDBViewDlg::OnViewStatistics()
{
	CDialog * pDialog = new CNDBStats(m_ndbViewer, this);
	pDialog->Create(CNDBStats::IDD);
	pDialog->ShowWindow(SW_SHOW);
}

void CNDBViewDlg::OnHelpAboutndbview()
{
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}

void CNDBViewDlg::OnFileExit()
{
	Clear();
	OnCancel();
}

void CNDBViewDlg::OnViewAmrinfo()
{
	CDialog * pDialog = new CAMRResumeDlg(m_ndbViewer, this);
	pDialog->Create(CAMRResumeDlg::IDD);
	pDialog->ShowWindow(SW_SHOW);
}
