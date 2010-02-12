// ConsistencyReport.cpp : implementation file
//

#include "stdafx.h"
#include "NDBView.h"
#include "ConsistencyReport.h"

ITest::ITest(NDBViewer* pNDBViewer)
{
	m_pNDBViewer = pNDBViewer;
	m_nFailures = 0;
	m_nWarnings = 0;
	m_summary = OK;
}

ITest::~ITest()
{
}

const WCHAR* ITest::GetSummaryResultString()
{
	wsprintf(m_resultsSummary, L"%u Failures, %u Warnings", m_nFailures, m_nWarnings);
	return m_resultsSummary;
}

// CConsistencyReport dialog

IMPLEMENT_DYNAMIC(CConsistencyReport, CDialog)

CConsistencyReport::CConsistencyReport(NDBViewer * pNDBViewer, CNDBViewDlg * pNDBDlg, CWnd* pParent /*=NULL*/)
	: CNDBViewChildDlg(pNDBDlg, CConsistencyReport::IDD, pParent)
{
	m_fDoModal = false;
	m_pNDBViewer = pNDBViewer;
	m_pNDBViewDlg = pNDBDlg;

	tests[0] = new HeaderTest(pNDBViewer);
	tests[1] = new BTreeTest(pNDBViewer);
	tests[2] = new AMapTest(pNDBViewer);
	tests[3] = new CRCTest(pNDBViewer);
	//tests[4] = new RefTest(pNDBViewer);

	m_finalResult = OK;
}

CConsistencyReport::~CConsistencyReport()
{
	for(int i = 0; i < NUM_TESTS; i++)
		delete tests[i];
}

void CConsistencyReport::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_lc);
	DDX_Control(pDX, IDC_TREE1, m_tc);
}


BEGIN_MESSAGE_MAP(CConsistencyReport, CDialog)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CConsistencyReport::OnTvnSelchangedTree1)
	ON_NOTIFY(LVN_DELETEITEM, IDC_LIST1, &CConsistencyReport::OnLvnDeleteitemList1)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CConsistencyReport::OnNMDblclkList1)
END_MESSAGE_MAP()

HeaderTest::HeaderTest(NDBViewer* pNDBViewer)
	: ITest(pNDBViewer)
{
}

HeaderTest::~HeaderTest(void)
{
}

void HeaderTest::PreTest()
{
	HEADER header = m_pNDBViewer->GetHeader();

	if((header.dwMagic == dwMagicHL) || (header.dwMagic == dwMagicLH))
		m_trMagic = OK;
	else
	{
		Failure();
		m_trMagic = FAILURE;
	}

	if(m_pNDBViewer->FValidPartialCRC())
		m_trCRCPartial = OK;
	else
	{
		Failure();
		m_trCRCPartial = FAILURE;
	}

	if((header.wMagicClient == PST_MAGIC) || (header.wMagicClient == OST_MAGIC) || (header.wMagicClient))
		m_trMagicClient = OK;
	else
	{
		Failure();
		m_trMagicClient = FAILURE;
	}

	if((header.wVer >= NDB16K_VERSION) && (header.wVer <= NDBUNI_BIGSBLOCKLEAF_VERSION))
		m_trVer = OK;
	else
	{
		Failure();
		m_trVer = FAILURE;
	}

	if(header.wVerClient <= PST_VERSION)
		m_trVerClient = OK;
	else
	{
		Failure();
		m_trVerClient = FAILURE;
	}

	if(header.root.ibFileEof == m_pNDBViewer->GetFileEOF())
		m_trEOF = OK;
	else if(header.root.ibFileEof < m_pNDBViewer->GetFileEOF())
	{
		Warning();
		m_trEOF = WARNING;
	}
	else
	{
		Failure();
		m_trEOF = FAILURE;
	}

	if(header.root.brefBBT.ib < m_pNDBViewer->GetFileEOF())
		m_trBBT = OK;
	else
	{
		Failure();
		m_trBBT = FAILURE;
	}

	if(header.root.brefNBT.ib < m_pNDBViewer->GetFileEOF())
		m_trNBT = OK;
	else
	{
		Failure();
		m_trNBT = FAILURE;
	}

	if(header.bSentinel == 0x80)
		m_trSentinel = OK;
	else
	{
		Failure();
		m_trSentinel = FAILURE;
	}

	if(header.bCryptMethod <= NDB_CRYPT_CYCLIC)
		m_trCryptMethod = OK;
	else
	{
		Failure();
		m_trCryptMethod = FAILURE;
	}

	if(m_pNDBViewer->FValidCRC())
		m_trCRCFull = OK;
	else
	{
		Failure();
		m_trCRCFull = FAILURE;
	}
}

void HeaderTest::ReportResults(ofstream& file)
{
	HEADER header = m_pNDBViewer->GetHeader();

	if(GetFailures() == 0 && GetWarnings() == 0)
	{
		GreatSuccess(file);
		return;
	}

	if(m_trMagic != OK)
	{
		file << "Fail: Unknown dwMagic value (0x%X)" <<  hex << header.dwMagic << dec << endl;
	}

	if(m_trCRCPartial != OK)
	{
		file << "Fail: Partial CRC Failure\n";
	}

	if(m_trMagicClient != OK)
	{
		file << "Fail: Unknown wMagicClient value " << header.wMagicClient << endl;
	}

	if(m_trVer != OK)
	{
		file << "Fail: Unknown wVer value " << header.wVer << endl;
	}

	if(m_trVerClient != OK)
	{
		file << "Fail: Unknown wVerClient value " << header.wVerClient << endl;
	}	

	if(m_trEOF == WARNING)
	{
		file << "Warning: File EOF > Header EOF (" << m_pNDBViewer->GetFileEOF() << " vs " << header.root.ibFileEof << ")\n";
	}

	else if(m_trEOF == FAILURE)
	{
		file << "Fail: Header EOF > File EOF (" <<  header.root.ibFileEof << " vs " << m_pNDBViewer->GetFileEOF() << ")\n";
	}

	if(m_trBBT != OK)
	{
		file << "Fail: BBT Root past EOF (" << header.root.brefBBT.ib << ")\n";
	}

	if(m_trNBT != OK)
	{
		file << "Fail: NBT Root past EOF (" << header.root.brefNBT.ib << ")\n";
	}

	if(m_trSentinel != OK)
	{
		file << "Fail: Unknown bSentinel value (0x%X) - should always be 0x80\n";
	}

	if(m_trCryptMethod != OK)
	{
		file << "Fail: Unknown bCryptMethod (0x" << hex << header.bCryptMethod << dec << ")\n";
	}

	if(m_trCRCFull != OK)
	{
		file << "Fail: Full CRC Failure\n";
	}
}


void HeaderTest::ReportResults(CListCtrl *lc)
{
	int nItem = 0;
	WCHAR buffer[255];
	RECT r;
	HEADER header = m_pNDBViewer->GetHeader();

	lc->GetClientRect(&r);
	lc->InsertColumn(0, L"Result", LVCFMT_LEFT, r.right);

	if(GetFailures() == 0 && GetWarnings() == 0)
	{
		GreatSuccess(lc);
		return;
	}

	if(m_trMagic != OK)
	{
		wsprintf(buffer, L"Unknown dwMagic value (0x%X)", header.dwMagic);
		lc->InsertItem(nItem++, buffer, iconFail);
	}

	if(m_trCRCPartial != OK)
	{
		lc->InsertItem(nItem++, L"Partial CRC Failure", iconFail);
	}

	if(m_trMagicClient != OK)
	{
		wsprintf(buffer, L"Unknown wMagicClient value (0x%X)", header.wMagicClient);
		lc->InsertItem(nItem++, buffer, iconFail);
	}

	if(m_trVer != OK)
	{
		wsprintf(buffer, L"Unknown wVer value (0x%X)", header.wVer);
		lc->InsertItem(nItem++, buffer, iconFail);
	}

	if(m_trVerClient != OK)
	{
		wsprintf(buffer, L"Unknown wVerClient value (0x%X)", header.wVerClient);
		lc->InsertItem(nItem++, buffer, iconFail);
	}	

	if(m_trEOF == WARNING)
	{
		wsprintf(buffer, L"File EOF > Header EOF (%I64u vs. %I64u)", m_pNDBViewer->GetFileEOF(), header.root.ibFileEof);
		lc->InsertItem(nItem++, buffer, iconWarning);
	}
	else if(m_trEOF == FAILURE)
	{
		wsprintf(buffer, L"Header EOF > File EOF (%I64u vs. %I64u)", header.root.ibFileEof, m_pNDBViewer->GetFileEOF());
		lc->InsertItem(nItem++, buffer, iconFail);
	}

	if(m_trBBT != OK)
	{
		wsprintf(buffer, L"BBT Root past EOF (%I64u)", header.root.brefBBT.ib);
		lc->InsertItem(nItem++, buffer, iconFail);
	}

	if(m_trNBT != OK)
	{
		wsprintf(buffer, L"NBT Root past EOF (%I64u)", header.root.brefNBT.ib);
		lc->InsertItem(nItem++, buffer, iconFail);
	}

	if(m_trSentinel != OK)
	{
		wsprintf(buffer, L"Unknown bSentinel value (0x%X) - should always be 0x80", header.bSentinel);
		lc->InsertItem(nItem++, buffer, iconFail);
	}

	if(m_trCryptMethod != OK)
	{
		wsprintf(buffer, L"Unknown bCryptMethod (0x%X)", header.bCryptMethod);
		lc->InsertItem(nItem++, buffer, iconFail);
	}

	if(m_trCRCFull != OK)
	{
		lc->InsertItem(nItem++, L"Full CRC Failure", iconFail);
	}
}

bool TestHelperNBT(BTPAGE * pBTPage, PAGETRAILER* pPageTrailer, IB ib, void* pv)
{
	CConsistencyReport * pCR = (CConsistencyReport*)pv;
	ITest ** tests = pCR->GetTests();
	bool failed = false;

	for(int i = 0; i < NUM_TESTS; i++)
		if(!(tests[i])->NBTPage(pBTPage, pPageTrailer, ib))
		{
			failed = true;
			break;
		}

	return !failed;
}

bool TestHelperBBT(BTPAGE * pBTPage, PAGETRAILER* pPageTrailer, IB ib, void* pv)
{
	CConsistencyReport * pCR = (CConsistencyReport*)pv;
	ITest ** tests = pCR->GetTests();
	bool failed = false;

	for(int i = 0; i < NUM_TESTS; i++)
		if(!(tests[i])->BBTPage(pBTPage, pPageTrailer, ib))
		{
			failed = true;
			break;
		}

	return !failed;
}

bool TestHelperStaticNBT(BTPAGE * pBTPage, PAGETRAILER* pPageTrailer, IB ib, void* pv)
{
	ITest ** tests = (ITest**)pv;
	bool failed = false;

	for(int i = 0; i < NUM_TESTS; i++)
		if(!(tests[i])->NBTPage(pBTPage, pPageTrailer, ib))
		{
			failed = true;
			break;
		}

	return !failed;
}

bool TestHelperStaticBBT(BTPAGE * pBTPage, PAGETRAILER* pPageTrailer, IB ib, void* pv)
{
	ITest ** tests = (ITest**)pv;
	bool failed = false;

	for(int i = 0; i < NUM_TESTS; i++)
		if(!(tests[i])->BBTPage(pBTPage, pPageTrailer, ib))
		{
			failed = true;
			break;
		}

	return !failed;
}

BOOL CConsistencyReport::JustRunReport(NDBViewer * pNDBViewer)
{
	ITest* tests[NUM_TESTS];
	tests[0] = new HeaderTest(pNDBViewer);
	tests[1] = new CRCTest(pNDBViewer);
	tests[2] = new BTreeTest(pNDBViewer);
	tests[3] = new RefTest(pNDBViewer);
	tests[4] = new AMapTest(pNDBViewer);
	TestResult finalResult;

	finalResult = OK;
	CString path = pNDBViewer->GetFile().GetFilePath();

	path.Append(L".report");
	ofstream file(path);

	// Run the tests!
	for(int i = 0; i < NUM_TESTS; i++)
		(tests[i])->PreTest();

	pNDBViewer->ForEachBTPage(ptypeNBT, &TestHelperStaticNBT, feAll, tests);
	pNDBViewer->ForEachBTPage(ptypeBBT, &TestHelperStaticBBT, feAll, tests);

	for(int i = 0; i < NUM_TESTS; i++)
		(tests[i])->PostTest();

	// Get our overall status
	for(int i = 0; i < NUM_TESTS; i++)
		if(tests[i]->GetSummaryResultCode() > finalResult) finalResult = tests[i]->GetSummaryResultCode();

	// Setup the result tree
	for(int i = 0; i < NUM_TESTS; i++)
	{
		file << "\n=====================================\n";

		file << convert(tests[i]->GetTitle()).c_str() << ": ";
		TestResult r = tests[i]->GetSummaryResultCode();
		if(r == OK)
			file << " PASS\n";
		else if(r == WARNING)
			file << " WARNING\n";
		else 
			file << " FAIL\n";

		file << convert(tests[i]->GetDescription()).c_str();

		file << endl << endl;

		tests[i]->ReportResults(file);
	}

	for(int i = 0; i < NUM_TESTS; i++)
		delete tests[i];

	return TRUE;  // return TRUE unless you set the focus to a control
}

BOOL CConsistencyReport::OnInitDialog()
{
	CBitmap cb;
	CString path = m_pNDBViewer->GetFile().GetFilePath();
	CWaitCursor wait;
	CDialog::OnInitDialog();

	path.Append(L".report");
	ofstream file(path);

	// Setup image lists of the trees
	cb.LoadBitmap(IDB_BITMAP2);
	m_il.Create(16, 16, ILC_COLOR24, 10, 1);
	m_il.Add(&cb, RGB(255, 0, 255));
	m_il.SetBkColor(CLR_NONE);

	m_lc.SetImageList(&m_il, LVSIL_SMALL);
	m_tc.SetImageList(&m_il, TVSIL_NORMAL);

	// Run the tests!
	for(int i = 0; i < NUM_TESTS; i++)
		(tests[i])->PreTest();

	m_pNDBViewer->ForEachBTPage(ptypeNBT, &TestHelperNBT, feAll, this);
	m_pNDBViewer->ForEachBTPage(ptypeBBT, &TestHelperBBT, feAll, this);

	for(int i = 0; i < NUM_TESTS; i++)
		(tests[i])->PostTest();

	// Get our overall status
	for(int i = 0; i < NUM_TESTS; i++)
		if(tests[i]->GetSummaryResultCode() > m_finalResult) m_finalResult = tests[i]->GetSummaryResultCode();

	// Setup the result tree
	int image = (m_finalResult == OK ? iconPass : (m_finalResult == WARNING ? iconWarning : iconFail));
	HTREEITEM root = m_tc.InsertItem(L"Summary", TVI_ROOT);
	m_tc.SetItemImage(root, image, image);

	for(int i = 0; i < NUM_TESTS; i++)
	{
		file << "\n=====================================\n";

		file << convert(tests[i]->GetTitle()).c_str() << ": ";
		TestResult r = tests[i]->GetSummaryResultCode();
		if(r == OK)
			file << " PASS\n";
		else if(r == WARNING)
			file << " WARNING\n";
		else 
			file << " FAIL\n";

		file << convert(tests[i]->GetDescription()).c_str();

		file << endl << endl;

		tests[i]->ReportResults(file);

		int childImage = (r == OK ? iconPass : (r == WARNING ? iconWarning : iconFail));

		HTREEITEM child = m_tc.InsertItem(tests[i]->GetTitle(), root);
		m_tc.SetItemImage(child, childImage, childImage);
		m_tc.SetItemData(child, (DWORD_PTR)tests[i]);
	}

	m_tc.Expand(root, TVE_EXPAND);

	Summary();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CConsistencyReport::OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	HTREEITEM newItem = pNMTreeView->itemNew.hItem;
	ITest * pTest = (ITest*)m_tc.GetItemData(newItem);

	// Clean out the results list
	m_lc.DeleteAllItems();

	int nColumnCount = m_lc.GetHeaderCtrl()->GetItemCount();
	for (int i=0;i < nColumnCount;i++)
	{
		m_lc.DeleteColumn(0);
	}
	
	if(pTest)
	{
		GetDlgItem(IDC_STATIC)->SetWindowText(pTest->GetTitle());
		GetDlgItem(IDC_STATIC1)->SetWindowText(pTest->GetDescription());

		pTest->ReportResults(&m_lc);
	}
	else // go back to summary mode
	{
		Summary();
	}

	*pResult = 0;
}

void CConsistencyReport::Summary()
{
	WCHAR buffer[255];
	RECT r;
	int image;

	GetDlgItem(IDC_STATIC)->SetWindowText(L"Summary");
	GetDlgItem(IDC_STATIC1)->SetWindowText(L"Select a test on the left for more details.");
	m_lc.GetClientRect(&r);
	m_lc.InsertColumn(0, L"Result", LVCFMT_LEFT, r.right);

	for(int i = 0; i < NUM_TESTS; i++)
	{
		wsprintf(buffer, L"%s: %s", tests[i]->GetTitle(), tests[i]->GetSummaryResultString());
		image = (tests[i]->GetSummaryResultCode() == OK ? iconPass : (tests[i]->GetSummaryResultCode() == WARNING ? iconWarning : iconFail));
		m_lc.InsertItem(i, buffer, image);
	}
}

CRCTest::CRCTest(NDBViewer * pNDBViewer) 
	: ITest(pNDBViewer)
{
}

CRCTest::~CRCTest()
{
}

void CRCTest::PreTest()
{
	BYTE page[cbPage];
	PAGETRAILER pt;
	BREF bref;
	DWORD dwCRC;
	WORD wSig;
	bool fAMapValid = (m_pNDBViewer->GetHeader().root.fAMapValid != 0);
	CList<BREF> * pList = &m_invalidAMapFailedPages;
	IB eof = m_pNDBViewer->GetHeader().root.ibFileEof;

	// Validate that all AMap pages pass CRC. For reporting purposes, we are
	// going to cheat and put the expected ptype of the page in bref.bid for
	// failed pages.
	for(IB ib = ibAMapBase; ib < eof; ib += cbPerAMap)
	{
		m_pNDBViewer->ReadPage(page, sizeof(page), &pt, ib);
		dwCRC = ComputeCRC(page, cbPageData);
		wSig = ComputeSig(ib, ib);

		if(		(dwCRC != pt.dwCRC)
			||	(wSig != pt.wSig)
			||	(pt.ptype != pt.ptypeRepeat)
			||	(pt.ptype != ptypeAMap)
			)
		{
			bref.ib = ib;
			bref.bid = (BID)ptypeAMap;
			pList->AddTail(bref);
			if(fAMapValid)
				Failure();
		}
	}

	// Validate that all PMap pages pass CRC
	IB ibPMap;
	for(IB ib = ibPMapBase; ib < eof; ib += cbPerPMap)
	{
		ibPMap = ib + cbPage;
		m_pNDBViewer->ReadPage(page, sizeof(page), &pt, ibPMap);
		dwCRC = ComputeCRC(page, cbPageData);
		wSig = ComputeSig(ibPMap, ibPMap);

		if(		(dwCRC != pt.dwCRC)
			||	(wSig != pt.wSig)
			||	(pt.ptype != pt.ptypeRepeat)
			||	(pt.ptype != ptypePMap)
			)
		{
			bref.ib = ibPMap;
			bref.bid = (BID)ptypePMap;
			pList->AddTail(bref);
			if(fAMapValid)
				Failure();
		}
	}
	// Validate that all FMap pages pass CRC
	IB ibFMap = 0;
	for(IB ib = ibFMapBase; ib < eof; ib += cbPerFMap)
	{
		ibFMap = ib + (cbPage*2);

		m_pNDBViewer->ReadPage(page, sizeof(page), &pt, ibFMap);
		dwCRC = ComputeCRC(page, cbPageData);
		wSig = ComputeSig(ibFMap, ibFMap);

		if(		(dwCRC != pt.dwCRC)
			||	(wSig != pt.wSig)
			||	(pt.ptype != pt.ptypeRepeat)
			||	(pt.ptype != ptypeFMap)
			)
		{
			bref.ib = ibFMap;
			bref.bid = (BID)ptypeFMap;
			pList->AddTail(bref);
			if(fAMapValid)
				Failure();
		}
	}
	// Validate that all FPMap pages pass CRC
	IB ibFPMap;
	for(IB ib = ibFPMapBase; ib < eof; ib += cbPerFPMap)
	{
		ibFPMap = ib + (cbPage * (ib % cbPerFMap == 0 ? 3 : 2));
		m_pNDBViewer->ReadPage(page, sizeof(page), &pt, ibFPMap);
		dwCRC = ComputeCRC(page, cbPageData);
		wSig = ComputeSig(ibFPMap, ibFPMap);

		if(		(dwCRC != pt.dwCRC)
			||	(wSig != pt.wSig)
			||	(pt.ptype != pt.ptypeRepeat)
			||	(pt.ptype != ptypeFPMap)
			)
		{
			bref.ib = ibFPMap;
			bref.bid = (BID)ptypeFPMap;
			pList->AddTail(bref);
			if(fAMapValid)
				Failure();
		}
	}
}

BOOL CRCTest::NBTPage(BTPAGE * pBTPage, PAGETRAILER * pPT, IB ib)
{
	DWORD dwCRC;
	WORD wSig;
	BREF bref;

	dwCRC = ComputeCRC(pBTPage, cbPageData);
	wSig = ComputeSig(ib, pPT->bid);

	bref.ib = ib;
	bref.bid = ptypeNBT;

	if(		(dwCRC != pPT->dwCRC) 
		||	(wSig != pPT->wSig)
		||	(pPT->ptype != ptypeNBT)
		||	(pPT->ptype != pPT->ptypeRepeat)
		)
	{
		Failure();
		m_failedPages.AddTail(bref);
		return FALSE;
	}
	return TRUE;
}

BOOL CRCTest::BBTPage(BTPAGE * pBTPage, PAGETRAILER * pPT, IB ib)
{
	DWORD dwCRC;
	WORD wSig;
	BREF bref;
	BOOL valid = TRUE;

	dwCRC = ComputeCRC(pBTPage, cbPageData);
	wSig = ComputeSig(ib, pPT->bid);

	bref.ib = ib;
	bref.bid = ptypeBBT;

	if(		(dwCRC != pPT->dwCRC) 
		||	(wSig != pPT->wSig)
		||	(pPT->ptype != ptypeBBT)
		||	(pPT->ptype != pPT->ptypeRepeat)
		)
	{
		Failure();
		m_failedPages.AddTail(bref);
		valid = FALSE;
	}
	else if(pBTPage->cLevel == 0)
	{
		// we also have to validate each block on this page
		BBTENTRY *pbbtEntry = (BBTENTRY*)(pBTPage->rgbte);
		for(int i = 0; i < pBTPage->cEnt; i++, pbbtEntry++)
		{
			BYTE * buffer = new BYTE[BBufferSize(pbbtEntry->cb)];
			BLOCKTRAILER bt;

			if(pbbtEntry->bref.ib < m_pNDBViewer->GetFileEOF())
			{
				m_pNDBViewer->ReadBlock(buffer, BBufferSize(pbbtEntry->cb), &bt, pbbtEntry->bref.ib, CbAlignDisk(pbbtEntry->cb));
				dwCRC = ComputeCRC(buffer, BBufferSize(pbbtEntry->cb));
				wSig = ComputeSig(pbbtEntry->bref.ib, pbbtEntry->bref.bid);

				if(		(dwCRC != bt.dwCRC)
					||	(wSig != bt.wSig)
					)
				{
					Failure();
					m_failedBlocks.AddTail(*pbbtEntry);
					valid = FALSE;
				}
			} else {
				Failure();
				m_failedBlocks.AddTail(*pbbtEntry);
			}

			delete [] buffer;
		}
	}
	return valid;
}

void CRCTest::ReportResults(ofstream& file)
{
	POSITION pos;
	bool fAMapValid = (m_pNDBViewer->GetHeader().root.fAMapValid != 0);

	if(GetFailures() == 0 && GetWarnings() == 0 && m_invalidAMapFailedPages.GetCount() == 0)
	{
		GreatSuccess(file);
		return;
	}

	pos = m_invalidAMapFailedPages.GetHeadPosition();
	for(int i = 0; i < m_invalidAMapFailedPages.GetCount(); i++)
	{
		BREF b = m_invalidAMapFailedPages.GetNext(pos);

		if(fAMapValid)
			file << "Fail: ";
		else
			file << "Info: ";

		file << "AMap Page at " << b.ib << " failed CRC Check\n";
	}

	pos = m_failedPages.GetHeadPosition();
	for(int i = 0; i < m_failedPages.GetCount(); i++)
	{
		BREF b = m_failedPages.GetNext(pos);

		file << "Fail: " << convert(GetPTypeString((BYTE)b.bid)).c_str() << " Page at " << b.ib << " failed CRC Check\n";
	}
	
	pos = m_failedBlocks.GetHeadPosition();
	for(int i = 0; i < m_failedBlocks.GetCount(); i++)
	{
		BBTENTRY be = m_failedBlocks.GetNext(pos);

		file << "Fail: Block 0x" << hex << be.bref.bid << dec << " at " << be.bref.ib << " failed CRC Check\n";
	}
}

void CRCTest::ReportResults(CListCtrl *lc)
{
	int nItem = 0;
	WCHAR buffer[255];
	RECT r;
	POSITION pos;
	bool fAMapValid = (m_pNDBViewer->GetHeader().root.fAMapValid != 0);

	lc->GetClientRect(&r);
	if(GetFailures() == 0 && GetWarnings() == 0 && m_invalidAMapFailedPages.GetCount() == 0)
	{
		lc->InsertColumn(0, L"Result", LVCFMT_LEFT, r.right);
		GreatSuccess(lc);
		return;
	}
	lc->InsertColumn(0, L"Type", LVCFMT_LEFT, r.right/3);
	lc->InsertColumn(1, L"IB", LVCFMT_LEFT, r.right/3);
	lc->InsertColumn(2, L"BID", LVCFMT_LEFT, r.right/3);

	// Remember, we stored the ptype of the pages in bref.bid - because the actual BID of a
	// page isn't that interesting (especially if the CRC failed).
	pos = m_invalidAMapFailedPages.GetHeadPosition();
	for(int i = 0; i < m_invalidAMapFailedPages.GetCount(); i++)
	{
		BREF b = m_invalidAMapFailedPages.GetNext(pos);
		NodeData * pNodeData = new NodeData;

		if(fAMapValid)
			lc->InsertItem(nItem, GetPTypeString((BYTE)b.bid), iconFail);
		else
			lc->InsertItem(nItem, GetPTypeString((BYTE)b.bid), iconInfo);

		wsprintf(buffer, L"%I64u", b.ib);
		lc->SetItemText(nItem, 1, buffer);
		
		pNodeData->bref = b;
		pNodeData->bref.bid = 0;
		pNodeData->btkey = 0;
		pNodeData->cb = 512;
		pNodeData->nid = 0;
		pNodeData->ulFlags = aOpenPage;

		lc->SetItemData(nItem++, (DWORD_PTR)pNodeData);
	}

	pos = m_failedPages.GetHeadPosition();
	for(int i = 0; i < m_failedPages.GetCount(); i++)
	{
		BREF b = m_failedPages.GetNext(pos);
		NodeData * pNodeData = new NodeData;

		lc->InsertItem(nItem, GetPTypeString((BYTE)b.bid), iconFail);

		wsprintf(buffer, L"%I64u", b.ib);
		lc->SetItemText(nItem, 1, buffer);
		
		pNodeData->bref = b;
		pNodeData->bref.bid = 0;
		pNodeData->btkey = 0;
		pNodeData->cb = 512;
		pNodeData->nid = 0;
		pNodeData->ulFlags = aOpenBTPage;

		lc->SetItemData(nItem++, (DWORD_PTR)pNodeData);
	}

	pos = m_failedBlocks.GetHeadPosition();
	for(int i = 0; i < m_failedBlocks.GetCount(); i++)
	{
		BBTENTRY be = m_failedBlocks.GetNext(pos);
		NodeData * pNodeData = new NodeData;

		lc->InsertItem(nItem, L"Block", iconFail);

		wsprintf(buffer, L"%I64u", be.bref.ib);
		lc->SetItemText(nItem, 1, buffer);

		wsprintf(buffer, L"0x%I64X", be.bref.bid);
		lc->SetItemText(nItem, 2, buffer);
		
		pNodeData->bref = be.bref;
		pNodeData->btkey = 0;
		pNodeData->cb = be.cb;
		pNodeData->nid = 0;
		pNodeData->ulFlags = aOpenBlock | aBrowseBBT;

		lc->SetItemData(nItem++, (DWORD_PTR)pNodeData);
	}


}
void CConsistencyReport::OnLvnDeleteitemList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	delete (NodeData*)m_lc.GetItemData(pNMLV->iItem);

	*pResult = 0;
}

void CConsistencyReport::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	int i = m_lc.GetSelectionMark();
	NodeData * pNodeData = (NodeData*)m_lc.GetItemData(i);
	Unreferenced(pNMHDR);

	if(pNodeData)
		m_pNDBViewDlg->PerformAction(pNodeData);

	*pResult = 0;
}


RefTest::RefTest(NDBViewer *pNDBViewer) 
	: ITest(pNDBViewer)
{
}

RefTest::~RefTest()
{
}

RefTest::REFINFO RefTest::GetRefInfo(BID b)
{
	REFINFO ri;
	if(!m_refMap.Lookup(b, ri))
	{
		ri.calculatedCount = 0;
		ri.diskCount = (UINT)-1;
	}
	return ri;
}

BOOL RefTest::BBTPage(BTPAGE *pBTPage, PAGETRAILER *pPT, IB ib)
{
	Unreferenced(pPT);
	Unreferenced(ib);

	if(pBTPage->cLevel != 0) return TRUE;

	// add a ref for all entries
	BBTENTRY *pbbtEntry = (BBTENTRY*)(pBTPage->rgbte);
	for(UINT i = 0; i < BBTEnt(*pBTPage); i++, pbbtEntry++)
	{
		REFINFO ri = GetRefInfo(BIDStrip(pbbtEntry->bref.bid));

		// set the "disk count"
		ri.diskCount = pbbtEntry->cRef;

		// add a ref for this page
		ri.calculatedCount++;

		// if the bid is an internal bid, we have to open the block and add all of it's refs
		if(BIDIsInternal(pbbtEntry->bref.bid))
		{
			BYTE * blockData = new BYTE[BBufferSize(pbbtEntry->cb)];
			BLOCKTRAILER bt;

			m_pNDBViewer->ReadBlock(blockData, BBufferSize(pbbtEntry->cb), &bt, pbbtEntry->bref.ib, CbAlignDisk(pbbtEntry->cb));

			// verify CRC of this block

			// add all of the refs
			if(blockData[0] == btypeSB)
			{
				SBLOCK * psb = (SBLOCK*)blockData;
				if(psb->cLevel > 0)
				{
					SIENTRY * siEntry = psb->rgsi;
					for(UINT i = 0; i < SBIEnt(*psb, BBufferSize(pbbtEntry->cb)); i++, siEntry++)
					{
						REFINFO sri = GetRefInfo(BIDStrip(siEntry->bid));
						sri.calculatedCount++;
						m_refMap[ BIDStrip(siEntry->bid) ] = sri;
					}
				}
				else
				{
					SLENTRY * slEntry = psb->rgsl;
					for(UINT i = 0; i < SBLEnt(*psb, BBufferSize(pbbtEntry->cb)); i++, slEntry++)
					{
						if(slEntry->bidData)
						{
							REFINFO sri = GetRefInfo(BIDStrip(slEntry->bidData));
							sri.calculatedCount++;
							m_refMap[BIDStrip(slEntry->bidData)] = sri;
						}
						if(slEntry->bidSub)
						{
							REFINFO sri = GetRefInfo(BIDStrip(slEntry->bidSub));
							sri.calculatedCount++;
							m_refMap[BIDStrip(slEntry->bidSub)] = sri;
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
					if(pxb->rgbid[i])
					{
						REFINFO sri = GetRefInfo(BIDStrip(pxb->rgbid[i]));
						sri.calculatedCount++;
						m_refMap[ BIDStrip(pxb->rgbid[i]) ] = sri;
					}
				}
			}
			delete [] blockData;
		}
		m_refMap[BIDStrip(pbbtEntry->bref.bid)] = ri;
	}

	return TRUE;
}

BOOL RefTest::NBTPage(BTPAGE *pBTPage, PAGETRAILER *pPT, IB ib)
{
	Unreferenced(pPT);
	Unreferenced(ib);

	if(pBTPage->cLevel != 0) return TRUE;

	// add a ref for the data block and subnodes block of all entries
	NBTENTRY *pnbtEntry = (NBTENTRY*)(pBTPage->rgbte);
	for(UINT i = 0; i < NBTEnt(*pBTPage); i++, pnbtEntry++)
	{
		if(pnbtEntry->bidData)
		{
			REFINFO ri = GetRefInfo(BIDStrip(pnbtEntry->bidData));
			ri.calculatedCount++;
			m_refMap[BIDStrip(pnbtEntry->bidData)] = ri;
		}

		if(pnbtEntry->bidSub)
		{
			REFINFO ri = GetRefInfo(BIDStrip(pnbtEntry->bidSub));
			ri.calculatedCount++;
			m_refMap[BIDStrip(pnbtEntry->bidSub)] = ri;
		}
	}

	return TRUE;
}

void RefTest::PostTest()
{
	CList<BID> lBid;
	POSITION mapPos;
	POSITION listPos;

	// Our goal here is to log failures, warnings, and keep
	// track of correct refs (which are uninteresting) so we
	// can remove them.
	mapPos = m_refMap.GetStartPosition();
	while(mapPos != NULL)
	{
		BID b;
		REFINFO ri;
		m_refMap.GetNextAssoc(mapPos, b, ri);

		// -1 is a sentinel value, meaning we didn't see it in the BBT
		if(ri.diskCount == (UINT)-1)
			Failure();
		// If the disk refcount is higher, all we did is leak space
		else if(ri.calculatedCount < ri.diskCount)
			Warning();
		// If the disk count is lower, then the store will eventually
		// be corrupted as this block will be prematurely deleted
		else if(ri.calculatedCount > ri.diskCount)
			Failure();
		// otherwise, things are fine and remove this from our map
		else if(ri.calculatedCount == ri.diskCount)
			lBid.AddTail(b);
	}

	listPos = lBid.GetHeadPosition();
	while(listPos != NULL)
	{
		BID b = lBid.GetNext(listPos);
		m_refMap.RemoveKey(b);
	}
}

void RefTest::ReportResults(ofstream& file)
{
	POSITION pos;

	if(GetFailures() == 0 && GetWarnings() == 0)
	{
		GreatSuccess(file);
		return;
	}

	pos = m_refMap.GetStartPosition();
	while(pos != NULL)
	{
		BID b;
		REFINFO ri;
		UINT icon;

		m_refMap.GetNextAssoc(pos, b, ri);

		icon = (ri.diskCount == (UINT)-1 ? iconFail : (ri.calculatedCount > ri.diskCount ? iconFail : iconWarning));
		if(icon == iconFail)
			file << "Fail: ";
		else
			file << "Warning: ";
		file << "0x" << hex << b << dec;

		if(ri.diskCount != UINT(-1))
		{
			file << "\tdiskcount " << ri.diskCount << "\t"; 
		}
		else
		{
			file << "\tNot In BBT!\t";
		}

		file << "calculated count " << ri.calculatedCount << endl;
	}
}

void RefTest::ReportResults(CListCtrl *lc)
{
	int nItem = 0;
	WCHAR buffer[255];
	RECT r;
	POSITION pos;

	lc->GetClientRect(&r);
	if(GetFailures() == 0 && GetWarnings() == 0)
	{
		lc->InsertColumn(0, L"Result", LVCFMT_LEFT, r.right);
		GreatSuccess(lc);
		return;
	}
	lc->InsertColumn(0, L"BID", LVCFMT_LEFT, r.right/4);
	lc->InsertColumn(1, L"Disk Ref", LVCFMT_LEFT, r.right/3);
	lc->InsertColumn(2, L"Calculated Ref", LVCFMT_LEFT, r.right/3);

	pos = m_refMap.GetStartPosition();
	while(pos != NULL)
	{
		BID b;
		REFINFO ri;
		UINT icon;
		NodeData * pNodeData = new NodeData;

		m_refMap.GetNextAssoc(pos, b, ri);

		icon = (ri.diskCount == (UINT)-1 ? iconFail : (ri.calculatedCount > ri.diskCount ? iconFail : iconWarning));
		wsprintf(buffer, L"0x%I64X", b);
		lc->InsertItem(nItem, buffer, icon);

		if(ri.diskCount != UINT(-1))
		{
			wsprintf(buffer, L"%u", ri.diskCount);
			lc->SetItemText(nItem, 1, buffer);
		}
		else
		{
			lc->SetItemText(nItem, 1, L"Not in BBT!");
		}

		wsprintf(buffer, L"%u", ri.calculatedCount);
		lc->SetItemText(nItem, 2, buffer);

		pNodeData->bref.bid = b;
		pNodeData->bref.ib = 0;
		pNodeData->cb = 0;
		pNodeData->btkey = 0;
		pNodeData->ulFlags = aBrowseBBT | aBrowseRefs;
		lc->SetItemData(nItem++, (DWORD_PTR)pNodeData);
	}
}

BTreeTest::BTreeTest(NDBViewer *pNDBViewer) 
	: ITest(pNDBViewer)
{
}

BOOL BTreeTest::NBTPage(BTPAGE* pBTPage, PAGETRAILER* pPt, IB ib)
{
	DWORD dwCRC = ComputeCRC(pBTPage, cbPageData);
	WORD wSig = ComputeSig(ib, pPt->bid);

	if(		(dwCRC != pPt->dwCRC) 
		||	(wSig != pPt->wSig)
		)
	{
		return FALSE;
	}

	// only interested in non-leaf pages
	if(pBTPage->cLevel == 0)
	{
		return TRUE;
	}

	BREF me;
	me.ib = ib;
	me.bid = pPt->bid;

	for(int i = 0; i < pBTPage->cEnt; ++i)
	{
		// have we seen this page before??
		if(m_seen_pages.find(pBTPage->rgbte[i].bref.bid) != m_seen_pages.end())
		{
			Failure();
			m_nbt_failures.push_back(std::pair<BREF,BREF>(pBTPage->rgbte[i].bref, me));
			return FALSE;
		}
		else if(m_seen_pages_ib.find(pBTPage->rgbte[i].bref.ib) != m_seen_pages_ib.end())
		{
			Failure();
			m_nbt_failures.push_back(std::pair<BREF,BREF>(pBTPage->rgbte[i].bref, me));
			return FALSE;
		}
		else
		{
			BREF bref;
			bref.bid = pPt->bid;
			bref.ib = ib;
			ASSERT(ib != 0);
			m_seen_pages[pBTPage->rgbte[i].bref.bid] = bref;
			m_seen_pages_ib[pBTPage->rgbte[i].bref.ib] = bref;
		}
	}

	return TRUE;
}

void BTreeTest::PreTest()
{
	BREF header;
	header.ib = 31337;
	header.bid = 0xbadc0de;

	// we've "seen" the root pages
	m_seen_pages[m_pNDBViewer->GetHeader().root.brefNBT.bid] = header;
	m_seen_pages[m_pNDBViewer->GetHeader().root.brefBBT.bid] = header;
	m_seen_pages_ib[m_pNDBViewer->GetHeader().root.brefNBT.ib] = header;
	m_seen_pages_ib[m_pNDBViewer->GetHeader().root.brefBBT.ib] = header;
}

BOOL BTreeTest::BBTPage(BTPAGE* pBTPage, PAGETRAILER* pPt, IB ib)
{
	DWORD dwCRC = ComputeCRC(pBTPage, cbPageData);
	WORD wSig = ComputeSig(ib, pPt->bid);

	if(		(dwCRC != pPt->dwCRC) 
		||	(wSig != pPt->wSig)
		)
	{
		return FALSE;
	}

	// only interested in non-leaf pages
	if(pBTPage->cLevel == 0)
	{
		return TRUE;
	}

	BREF me;
	me.ib = ib;
	me.bid = pPt->bid;

	for(int i = 0; i < pBTPage->cEnt; ++i)
	{
		// have we seen this page before??
		if(m_seen_pages.find(pBTPage->rgbte[i].bref.bid) != m_seen_pages.end())
		{
			Failure();
			m_bbt_failures.push_back(std::pair<BREF,BREF>(pBTPage->rgbte[i].bref, me));
			return FALSE;
		}
		else if(m_seen_pages_ib.find(pBTPage->rgbte[i].bref.ib) != m_seen_pages_ib.end())
		{
			Failure();
			m_bbt_failures.push_back(std::pair<BREF,BREF>(pBTPage->rgbte[i].bref, me));
			return FALSE;
		}
		else
		{
			BREF bref;
			bref.bid = pPt->bid;
			bref.ib = ib;
			ASSERT(ib != 0);
			m_seen_pages[pBTPage->rgbte[i].bref.bid] = bref;
			m_seen_pages_ib[pBTPage->rgbte[i].bref.ib] = bref;
		}
	}

	return TRUE;
}


void BTreeTest::ReportResults(CListCtrl *lc)
{
	WCHAR buffer[255];
	RECT r;
	int nItem = 0;

	lc->GetClientRect(&r);
	if(GetFailures() == 0 && GetWarnings() == 0)
	{
		lc->InsertColumn(0, L"Result", LVCFMT_LEFT, r.right);
		GreatSuccess(lc);
		return;
	}
	lc->InsertColumn(0, L"Type", LVCFMT_LEFT, r.right/6);
	lc->InsertColumn(1, L"IB", LVCFMT_LEFT, r.right/6);
	lc->InsertColumn(2, L"BID", LVCFMT_LEFT, r.right/6);
	lc->InsertColumn(3, L"Seen At", LVCFMT_LEFT, r.right/3);
	lc->InsertColumn(4, L"First Seen At", LVCFMT_LEFT, r.right / 3);

	for(unsigned int i = 0; i < m_nbt_failures.size(); ++i)
	{
		BREF failureChild = m_nbt_failures[i].first;
		BREF failureParent = m_nbt_failures[i].second;

		NodeData * pNodeData = new NodeData;
		pNodeData->bref = failureChild;
		pNodeData->btkey = 0;
		pNodeData->cb = 512;
		pNodeData->nid = 0;
		pNodeData->ulFlags = aOpenBTPage;

		lc->InsertItem(nItem, L"NBT Page", iconPage);
		wsprintf(buffer, L"%I64u", failureChild.ib);
		lc->SetItemText(nItem, 1, buffer);
		wsprintf(buffer, L"0x%I64X", failureChild.bid);
		lc->SetItemText(nItem, 2, buffer);
		wsprintf(buffer, L"%I64u", failureParent.ib);
		lc->SetItemText(nItem, 3, buffer);
		if(m_seen_pages.find(failureChild.bid) != m_seen_pages.end())
		{
			BREF seenat = m_seen_pages[failureChild.bid];
			wsprintf(buffer, L"(by bid) %I64u", m_seen_pages[failureChild.bid].ib);
		}
		else
		{
			BREF seenat = m_seen_pages_ib[failureChild.ib];
			wsprintf(buffer, L"(by address) %I64u", m_seen_pages_ib[failureChild.ib].ib);
		}
		lc->SetItemText(nItem, 4, buffer);
		lc->SetItemData(nItem++, (DWORD_PTR)pNodeData);
	}

	for(unsigned int i = 0; i < m_bbt_failures.size(); ++i)
	{
		BREF failureChild = m_bbt_failures[i].first;
		BREF failureParent = m_bbt_failures[i].second;

		NodeData * pNodeData = new NodeData;
		pNodeData->bref = failureChild;
		pNodeData->btkey = 0;
		pNodeData->cb = 512;
		pNodeData->nid = 0;
		pNodeData->ulFlags = aOpenBTPage;

		lc->InsertItem(nItem, L"BBT Page", iconPage);
		wsprintf(buffer, L"%I64u", failureChild.ib);
		lc->SetItemText(nItem, 1, buffer);
		wsprintf(buffer, L"0x%I64X", failureChild.bid);
		lc->SetItemText(nItem, 2, buffer);
		wsprintf(buffer, L"%I64u", failureParent.ib);
		lc->SetItemText(nItem, 3, buffer);
		if(m_seen_pages.find(failureChild.bid) != m_seen_pages.end())
		{
			//BREF seenat = m_seen_pages[failureChild.bid];
			std::map<BID,BREF>::iterator iter = m_seen_pages.find(failureChild.bid);
			ASSERT(iter != m_seen_pages.end());
			wsprintf(buffer, L"(by bid) %I64u", m_seen_pages[failureChild.bid].ib);
		}
		else
		{
			BREF seenat = m_seen_pages_ib[failureChild.ib];
			wsprintf(buffer, L"(by address) %I64u", m_seen_pages_ib[failureChild.ib].ib);
		}		
		lc->SetItemText(nItem, 4, buffer);
		lc->SetItemData(nItem++, (DWORD_PTR)pNodeData);
	}
}

void BTreeTest::ReportResults(ofstream& file)
{
	for(unsigned int i = 0; i < m_nbt_failures.size(); ++i)
	{
		BREF failureChild = m_nbt_failures[i].first;
		BREF failureParent = m_nbt_failures[i].second;

		file << "Fail: NBT Page at " << failureChild.ib << " (bid " << hex << failureChild.bid << dec << ") child of page at " << failureParent.ib << " first seen as child of page at ";
		if(m_seen_pages.find(failureChild.bid) != m_seen_pages.end())
			file << m_seen_pages[failureChild.bid].ib << "(by bid)" << endl;
		else
			file << m_seen_pages_ib[failureChild.ib].ib << "(by ib)" << endl;
	}

	for(unsigned int i = 0; i < m_bbt_failures.size(); ++i)
	{
		BREF failureChild = m_bbt_failures[i].first;
		BREF failureParent = m_bbt_failures[i].second;

		file << "Fail: NBT Page at " << failureChild.ib << " (bid " << hex << failureChild.bid << dec << ") child of page at " << failureParent.ib << " first seen as child of page at ";
		if(m_seen_pages.find(failureChild.bid) != m_seen_pages.end())
			file << m_seen_pages[failureChild.bid].ib << "(by bid)" << endl;
		else
			file << m_seen_pages_ib[failureChild.ib].ib << "(by ib)" << endl;

	}
}

AMapTest::AMapTest(NDBViewer *pNDBViewer) 
	: ITest(pNDBViewer)
{
	m_fValidAMap = pNDBViewer->GetHeader().root.fAMapValid != 0;
	if(!m_fValidAMap)
	{
		AMAPREBUILDRESUMEINFO amri;
		BOOL fNBTValid;
		BOOL fBBTValid;
		BOOL fEofValid;
		BOOL fResumeValid;

		m_pNDBViewer->ReadData((BYTE*)&amri, sizeof(AMAPREBUILDRESUMEINFO), NULL, NULL, ibAMapBase, sizeof(AMAPREBUILDRESUMEINFO));
		fNBTValid = amri.bidNBTRoot == m_pNDBViewer->GetHeader().root.brefNBT.bid;
		fBBTValid = amri.bidBBTRoot == m_pNDBViewer->GetHeader().root.brefNBT.bid;
		fEofValid = amri.ibAMapRebuildWatermark < m_pNDBViewer->GetHeader().root.ibFileEof;
		fResumeValid = fNBTValid && fBBTValid && fEofValid;		

		if(fResumeValid)
		{
			m_ibWatermark = amri.ibAMapRebuildWatermark;
		}
		else
		{
			m_ibWatermark = m_pNDBViewer->GetHeader().root.ibFileEof;
		}
	}
	else
	{
		m_ibWatermark = ibAMapBase;
	}
}

AMapTest::~AMapTest()
{
}

void AMapTest::PreTest()
{
	// allocate memory for the in memory AMap
	for(IB ib = ibAMapBase; ib < m_pNDBViewer->GetFileEOF(); ib += cbPerAMap)
	{
		BYTE * pAMap = new BYTE[cbAMapPage];
		ZeroMemory(pAMap, cbAMapPage);

		m_observedAMaps[ib] = pAMap;
		TestRangeMem(ib, cbPage); // mark the AMap as having itself allocated
	}

	// mark each pmap page, fmap page, and fpmap page as allocated in the
	// in memory amaps
	for(IB ib = ibPMapBase; ib < m_pNDBViewer->GetFileEOF(); ib += cbPerPMap)
	{
		TestRangeMem(ib+cbPage, cbPage);
	}
	for(IB ib = ibFMapBase; ib < m_pNDBViewer->GetFileEOF(); ib += cbPerFMap)
	{
		TestRangeMem(ib+(2*cbPage), cbPage);
	}
	for(IB ib = ibFPMapBase; ib < m_pNDBViewer->GetFileEOF(); ib += cbPerFPMap)
	{
		// does this area also contain an FMap?
		if((ib-ibFMapBase)%cbPerFMap == 0)
			TestRangeMem(ib+(3*cbPage), cbPage);
		else
			TestRangeMem(ib+(2*cbPage), cbPage);
	}

	// Test that the AMap and PMaps have consistent allocations

	// for each AMap page in the file..
	for(IB ibAMap = m_ibWatermark; ibAMap < m_pNDBViewer->GetFileEOF(); ibAMap += cbPerAMap)
	{
		BYTE amapPage[cbAMapPage];
		m_pNDBViewer->ReadPage(amapPage, cbAMapPage, NULL, ibAMap);
		
		// for each byte in the AMap...
		for(UINT i = 0; i < cbAMapPage; i++)
		{
			// if there is free space...
			if(amapPage[i] != 0xFF)
			{
				// there better not be a zero in the corresponding PMap slot
				BYTE pmapPage[cbAMapPage];
				PAGETRAILER pt;
				IB ibPMap = (ibAMap - ((ibAMap-ibPMapBase)%cbPerPMap)) + cbPage;
				int iSlot = (int)((ibAMap + (i*csPerByte*cbPerSlot)) - (ibPMap-cbPage)) / cbPage;

				m_pNDBViewer->ReadPage(pmapPage, cbAMapPage, &pt, ibPMap);
				//ASSERT(pt.ptype == ptypePMap);

				if(!BMapTestBit(pmapPage, iSlot))
				{
					AddResult(E_INCONSISTENT_MAPS, ibAMap + (i*csPerByte*cbPerSlot));
				}
			}
		}
	}

	if(!m_fValidAMap)
	{
		AddResult(I_INVALID_AMAP);
	}
}

BOOL AMapTest::NBTPage(BTPAGE* pBTPage, PAGETRAILER* pPt, IB ib)
{
	Unreferenced(pBTPage);

	if(ib+cbPage > m_pNDBViewer->GetFileEOF())
	{
		AddResult(E_ALLOCATED_PAST_EOF_PAGE, ib, cbPage);
		return TRUE;
	}

	if(TestRangeMem(ib, cbPage))
	{
		AddResult(E_OCCUPIED_SPACE_PAGE, ib, cbPage, pPt->bid);
	}

	if(!TestRangeDisk(ib, cbPage))
	{
		AddResult(E_FREE_SPACE_PAGE, ib, cbPage, pPt->bid);
	}

	return TRUE;
}

BOOL AMapTest::BBTPage(BTPAGE* pBTPage, PAGETRAILER* pPt, IB ib)
{
	if(ib+cbPage > m_pNDBViewer->GetFileEOF())
	{
		AddResult(E_ALLOCATED_PAST_EOF_PAGE, ib, cbPage);
		return TRUE;
	}

	if(TestRangeMem(ib, cbPage))
	{
		AddResult(E_OCCUPIED_SPACE_PAGE, ib, cbPage, pPt->bid);
	}

	if(!TestRangeDisk(ib, cbPage))
	{
		AddResult(E_FREE_SPACE_PAGE, ib, cbPage, pPt->bid);
	}

	// If this is a valid, zero level page check the blocks as well
	if(m_pNDBViewer->FValidPage(ib, ptypeBBT) && pBTPage->cLevel == 0)
	{
		// add a ref for all entries
		BBTENTRY *pbbtEntry = (BBTENTRY*)(pBTPage->rgbte);
		for(UINT i = 0; i < BBTEnt(*pBTPage); i++, pbbtEntry++)
		{
			if(pbbtEntry->bref.ib+pbbtEntry->cb > m_pNDBViewer->GetFileEOF())
			{
				AddResult(E_ALLOCATED_PAST_EOF_BLOCK, pbbtEntry->bref.ib, pbbtEntry->cb, pbbtEntry->bref.bid);
				continue;
			}

			if(TestRangeMem(pbbtEntry->bref.ib, CbAlignDisk(pbbtEntry->cb)))
			{
				AddResult(E_OCCUPIED_SPACE_BLOCK, pbbtEntry->bref.ib, pbbtEntry->cb, pbbtEntry->bref.bid);
			}
			
			if(!TestRangeDisk(pbbtEntry->bref.ib, CbAlignDisk(pbbtEntry->cb)))
			{
				AddResult(E_FREE_SPACE_BLOCK, pbbtEntry->bref.ib, pbbtEntry->cb, pbbtEntry->bref.bid);
			}
		}
	}

	return TRUE;
}

void AMapTest::PostTest()
{
	// check for leaked space
	for(IB ib = m_ibWatermark; ib < m_pNDBViewer->GetFileEOF(); ib += cbPerAMap)
	{
		BYTE amapPage[cbAMapPage];
		BYTE * pAMapPageMem = (BYTE*)m_observedAMaps[ib];

		m_pNDBViewer->ReadPage(amapPage, cbAMapPage, NULL, ib);

		for(int i = 0; i < cbAMapPage; i++)
		{
			if(amapPage[i] != pAMapPageMem[i])
			{
				if(amapPage[i] == 0xFF && pAMapPageMem[i] == 0x0)
				{
					// This probably means this area is free in the PMap.
					BYTE pmapPage[cbAMapPage];
					IB ibPMap = (ib - ((ib-ibPMapBase)%cbPerPMap)) + cbPage;
					int iSlot = (int)((ib + (i*csPerByte*cbPerSlot)) - (ibPMap-cbPage)) / cbPage;

					m_pNDBViewer->ReadPage(pmapPage, cbAMapPage, NULL, ibPMap);
					
					if(!BMapTestBit(pmapPage, iSlot))
						continue;
				}

				// have to check each bit...
				for(int j = 0; j < 8; j++)
				{
					// "leaked space" is when the disk amap has something marked
					// as allocated, but the memory amap does not. The reverse
					// has already been tested in the btree walk, when we have some
					// context as to the specific block or page that isn't marked as
					// allocated
					BYTE mask = 0x80 >> j;
					if(	((amapPage[i] & mask) != 0)
					&&	((pAMapPageMem[i] & mask) == 0)
					)
					{
						AddResult(W_LEAKED_SPACE, (ib + (i*csPerByte+j)*cbPerSlot), cbPerSlot);
					}
				}
			}
		}
	}

	// free the in memory AMap
	for(IB ib = ibAMapBase; ib < m_pNDBViewer->GetFileEOF(); ib += cbPerAMap)
	{
		delete (void*)(m_observedAMaps[ib]);
	}
	m_observedAMaps.RemoveAll();
}

void AMapTest::ReportResults(ofstream& file)
{
	POSITION pos;

	if(GetFailures() == 0 && GetWarnings() == 0 && m_fValidAMap)
	{
		GreatSuccess(file);
		return;
	}

	pos = m_results.GetHeadPosition();
	while(pos != NULL)
	{
		ResultList rl = m_results.GetNext(pos);

		switch(rl.result)
		{
		case E_OCCUPIED_SPACE_BLOCK:
			file << "Fail: Block @" << rl.bref.ib << " bid " << hex << rl.bref.bid << dec << " Address double allocation\n";
			break;
		case E_OCCUPIED_SPACE_PAGE:
			file << "Fail: Page @" << rl.bref.ib << " bid " << hex << rl.bref.bid << dec << " Address double allocation\n";
			break;
		case E_ALLOCATED_PAST_EOF_BLOCK:
			file << "Fail: Block @" << rl.bref.ib << " bid " << hex << rl.bref.bid << dec << " Allocated past EOF\n";
			break;
		case E_ALLOCATED_PAST_EOF_PAGE:
			file << "Fail: Page @" << rl.bref.ib << " bid " << hex << rl.bref.bid << dec << " Allocated past EOF\n";
			break;
		case E_FREE_SPACE_BLOCK:
			file << "Fail: Block @" << rl.bref.ib << " bid " << hex << rl.bref.bid << dec << " Address marked as free\n";
			break;
		case E_FREE_SPACE_PAGE:
			file << "Fail: Page @" << rl.bref.ib << " bid " << hex << rl.bref.bid << dec << " Address marked as free\n";
			break;
		case E_INCONSISTENT_MAPS:
			file << "Fail: AMap/PMap @" << rl.bref.ib << " bid " << hex << rl.bref.bid << dec << " Address marked as free in both the AMap and PMap\n";
			break;
		case W_LEAKED_SPACE:
			file << "Warning: AMap/PMap @" << rl.bref.ib << " bid " << hex << rl.bref.bid << dec << " Address leaked\n";
			break;
		case I_INVALID_AMAP:
			file << "Info: Invalid AMap - disk AMap checks have been skipped\n";
			break;
		default:
			ASSERT(0);
		}
	}

}

IB IbAMapFromIb(IB ib)
{
	IB ibOffset = (ib - ibAMapBase) % cbPerAMap;
	IB ibAMap   = ib - ibOffset;
	return ibAMap;
}

void AMapTest::ReportResults(CListCtrl *lc)
{
	int nItem = 0;
	WCHAR buffer[255];
	RECT r;
	POSITION pos;
	int total_warnings = 0;

	lc->GetClientRect(&r);
	if(GetFailures() == 0 && GetWarnings() == 0 && m_fValidAMap)
	{
		lc->InsertColumn(0, L"Result", LVCFMT_LEFT, r.right);
		GreatSuccess(lc);
		return;
	}
	lc->InsertColumn(0, L"Type", LVCFMT_LEFT, r.right/5);
	lc->InsertColumn(1, L"IB", LVCFMT_LEFT, r.right/5);
	lc->InsertColumn(2, L"BID", LVCFMT_LEFT, r.right/5);
	lc->InsertColumn(3, L"AMap Page Address", LVCFMT_LEFT, r.right/5);
	lc->InsertColumn(4, L"Problem", LVCFMT_LEFT, r.right/2);

	pos = m_results.GetHeadPosition();
	while(pos != NULL)
	{
		ResultList rl = m_results.GetNext(pos);
		NodeData * pNodeData = new NodeData;
		ZeroMemory(pNodeData, sizeof(NodeData));
		pNodeData->bref = rl.bref;
		pNodeData->cb = rl.cb;
		switch(rl.result)
		{
		case E_OCCUPIED_SPACE_BLOCK:
			lc->InsertItem(nItem, L"Block", iconCorrupt);
			wsprintf(buffer, L"%I64u", rl.bref.ib);
			lc->SetItemText(nItem, 1, buffer);
			wsprintf(buffer, L"0x%I64X", rl.bref.bid);
			lc->SetItemText(nItem, 2, buffer);
			wsprintf(buffer, L"%I64u", IbAMapFromIb(rl.bref.ib));
			lc->SetItemText(nItem, 3, buffer);
			lc->SetItemText(nItem, 4, L"Address double allocation");
			pNodeData->ulFlags = aBrowseBBT | aOpenBlock;
			lc->SetItemData(nItem++, (DWORD_PTR)pNodeData);
			break;
		case E_OCCUPIED_SPACE_PAGE:
			lc->InsertItem(nItem, L"Page", iconCorrupt);
			wsprintf(buffer, L"%I64u", rl.bref.ib);
			lc->SetItemText(nItem, 1, buffer);
			wsprintf(buffer, L"0x%I64X", rl.bref.bid);
			lc->SetItemText(nItem, 2, buffer);
			wsprintf(buffer, L"%I64u", IbAMapFromIb(rl.bref.ib));
			lc->SetItemText(nItem, 3, buffer);
			lc->SetItemText(nItem, 4, L"Address double allocation");
			pNodeData->ulFlags = aOpenBTPage;
			lc->SetItemData(nItem++, (DWORD_PTR)pNodeData);
			break;
		case E_ALLOCATED_PAST_EOF_BLOCK:
			lc->InsertItem(nItem, L"Block", iconCorrupt);
			wsprintf(buffer, L"%I64u", rl.bref.ib);
			lc->SetItemText(nItem, 1, buffer);
			wsprintf(buffer, L"0x%I64X", rl.bref.bid);
			lc->SetItemText(nItem, 2, buffer);
			wsprintf(buffer, L"%I64u", IbAMapFromIb(rl.bref.ib));
			lc->SetItemText(nItem, 3, buffer);
			lc->SetItemText(nItem, 4, L"Allocated past EOF");
			pNodeData->ulFlags = aBrowseBBT;
			lc->SetItemData(nItem++, (DWORD_PTR)pNodeData);
			break;
		case E_ALLOCATED_PAST_EOF_PAGE:
			lc->InsertItem(nItem, L"Page", iconCorrupt);
			wsprintf(buffer, L"%I64u", rl.bref.ib);
			lc->SetItemText(nItem, 1, buffer);
			wsprintf(buffer, L"0x%I64X", rl.bref.bid);
			lc->SetItemText(nItem, 2, buffer);
			wsprintf(buffer, L"%I64u", IbAMapFromIb(rl.bref.ib));
			lc->SetItemText(nItem, 3, buffer);
			lc->SetItemText(nItem, 4, L"Allocated past EOF");
			pNodeData->ulFlags = 0;
			lc->SetItemData(nItem++, (DWORD_PTR)pNodeData);
			break;
		case E_FREE_SPACE_BLOCK:
			lc->InsertItem(nItem, L"Block", iconCorrupt);
			wsprintf(buffer, L"%I64u", rl.bref.ib);
			lc->SetItemText(nItem, 1, buffer);
			wsprintf(buffer, L"0x%I64X", rl.bref.bid);
			lc->SetItemText(nItem, 2, buffer);
			wsprintf(buffer, L"%I64u", IbAMapFromIb(rl.bref.ib));
			lc->SetItemText(nItem, 3, buffer);
			lc->SetItemText(nItem, 4, L"Address marked as free");
			pNodeData->ulFlags = aBrowseBBT | aOpenBlock;
			lc->SetItemData(nItem++, (DWORD_PTR)pNodeData);
			break;
		case E_FREE_SPACE_PAGE:
			lc->InsertItem(nItem, L"Page", iconCorrupt);
			wsprintf(buffer, L"%I64u", rl.bref.ib);
			lc->SetItemText(nItem, 1, buffer);
			wsprintf(buffer, L"0x%I64X", rl.bref.bid);
			lc->SetItemText(nItem, 2, buffer);
			wsprintf(buffer, L"%I64u", IbAMapFromIb(rl.bref.ib));
			lc->SetItemText(nItem, 3, buffer);
			lc->SetItemText(nItem, 4, L"Address marked as free");
			pNodeData->ulFlags = aOpenBTPage;
			lc->SetItemData(nItem++, (DWORD_PTR)pNodeData);
			break;
		case E_INCONSISTENT_MAPS:
			lc->InsertItem(nItem, L"AMap/PMap", iconCorrupt);
			wsprintf(buffer, L"%I64u", rl.bref.ib);
			lc->SetItemText(nItem, 1, buffer);
			wsprintf(buffer, L"0x%I64X", rl.bref.bid);
			lc->SetItemText(nItem, 2, buffer);
			wsprintf(buffer, L"%I64u", IbAMapFromIb(rl.bref.ib));
			lc->SetItemText(nItem, 3, buffer);
			lc->SetItemText(nItem, 4, L"Address marked as free in both the AMap and PMap");
			pNodeData->ulFlags = 0;
			lc->SetItemData(nItem++, (DWORD_PTR)pNodeData);
			break;
		case W_LEAKED_SPACE:
			total_warnings++;
			if(total_warnings < 100)
			{
				lc->InsertItem(nItem, L"AMap/PMap", iconWarning);
				wsprintf(buffer, L"%I64u", rl.bref.ib);
				lc->SetItemText(nItem, 1, buffer);
				wsprintf(buffer, L"0x%I64X", rl.bref.bid);
				lc->SetItemText(nItem, 2, buffer);
				wsprintf(buffer, L"%I64u", IbAMapFromIb(rl.bref.ib));
				lc->SetItemText(nItem, 3, buffer);
				lc->SetItemText(nItem, 4, L"Address Leaked");
				pNodeData->ulFlags = aOpenBinary;
				lc->SetItemData(nItem++, (DWORD_PTR)pNodeData);
			}
			else
			{
				delete pNodeData;
			}
			break;
		case I_INVALID_AMAP:
			lc->InsertItem(nItem, L"Info", iconInfo);
			lc->SetItemText(nItem, 4, L"Invalid AMap - disk AMap checks have been skipped");
			lc->SetItemData(nItem++, (DWORD_PTR)pNodeData);
			break;
		default:
			ASSERT(0);
		}
	}

	if(total_warnings >= 100)
	{
		lc->InsertItem(nItem, L"AMap/PMap: More Warnings", iconWarning);
		wsprintf(buffer, L"%I64u", 0);
		lc->SetItemText(nItem, 1, buffer);
		wsprintf(buffer, L"0x%I64X", 0);
		lc->SetItemText(nItem, 2, buffer);
		wsprintf(buffer, L"%u additional warnings not displayed", total_warnings-100);
		lc->SetItemText(nItem, 4, buffer);
	}

}

string convert(const WCHAR* wstr)
{
	std::wstring temp_string(wstr);
	std::string temp_s2(temp_string.begin(), temp_string.end());
	return temp_s2;
}


bool AMapTest::TestRangeMem(IB ib, UINT size)
{
	IB ibAMapPage = (ib - ((ib-ibAMapBase)%cbPerAMap));
	UINT iSlotStart = (UINT)(ib - ibAMapPage) / cbPerSlot;
	UINT iNumSlots = size / cbPerSlot;
	BYTE * pAMapPage = (BYTE*)m_observedAMaps[ibAMapPage];
	bool fOccupied = false;

	ASSERT(pAMapPage);
	for(UINT i = 0; i < iNumSlots; i++)
	{
		if(BMapTestBit(pAMapPage, iSlotStart+i))
		{
			fOccupied = true;
			break;
		}
	}

	BMapSetBits(pAMapPage, iSlotStart, iNumSlots);

	return fOccupied;
}

bool AMapTest::TestRangeDisk(IB ib, UINT size)
{
	IB ibAMapPage = (ib - ((ib-ibAMapBase)%cbPerAMap));
	UINT iSlotStart = (UINT)(ib - ibAMapPage) / cbPerSlot;
	UINT iNumSlots = size / cbPerSlot;
	BYTE amapPage[cbAMapPage];
	bool fOccupied = true;

	if(ib < m_ibWatermark)
		return true;

	m_pNDBViewer->ReadPage(amapPage, cbAMapPage, NULL, ibAMapPage);
	for(UINT i = 0; i < iNumSlots; i++)
	{
		if(!BMapTestBit(amapPage, iSlotStart+i))
		{
			fOccupied = false;
			break;
		}
	}

	return fOccupied;
}


void AMapTest::AddResult(AMapTest::Result r, IB ib, UINT cb, BID bid)
{
	ResultList rl;

	rl.bref.ib = ib;
	rl.bref.bid = bid;
	rl.cb = cb;
	rl.result = r;
	m_results.AddTail(rl);

	if(r < W_LEAKED_SPACE)
		Failure();
	else if(r < I_INVALID_AMAP)
		Warning();
}

