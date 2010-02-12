#pragma once

#include "NDBViewer.h"
#include "NDBViewDlg.h"
#include "NDBViewChildDlg.h"
#include "afxcmn.h"
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <set>

using namespace std;

std::string convert(const WCHAR* wstr);

enum TestResult
{
	OK,
	WARNING,
	FAILURE
};

#define NUM_TESTS 4

class ITest
{
public:
	ITest(NDBViewer* pNDBViewer);
	virtual ~ITest();

// test functions
	// called before the BT walk. Do any setup here (or your whole test, if you're quick)
	virtual void PreTest() {}
	// called for each NBT page
	virtual BOOL NBTPage(BTPAGE* pBTPage, PAGETRAILER* pPt, IB ib) 
		{ Unreferenced(pBTPage);Unreferenced(pPt);Unreferenced(ib);return TRUE;}
	// called for each BBT page
	virtual BOOL BBTPage(BTPAGE* pBTPage, PAGETRAILER* pPt, IB ib) 
		{ Unreferenced(pBTPage);Unreferenced(pPt);Unreferenced(ib);return TRUE;}
	// called after the BT walk. Prepare results.
	virtual void PostTest() {}

// reporting functions
	// these can be called at any time
	virtual const WCHAR* GetTitle() = 0;
	virtual const WCHAR* GetDescription() = 0;

	// these are guarenteed to be only called after PostTest()
	virtual const TestResult GetSummaryResultCode() { return m_summary; }
	virtual const WCHAR* GetSummaryResultString();
	// populate the list contrl with any failures, and set the item 
	// data to be a NodeData* which can be passed to PerformAction
	virtual void ReportResults(CListCtrl * lc) = 0;
	virtual void ReportResults(ofstream& file) = 0;

// helper functions
	// call when you have no failures
	void GreatSuccess(CListCtrl * lc) { lc->InsertItem(0, L"No problems detected", iconPass); }
	void GreatSuccess(ofstream& file) { file << "No problems detected\n"; }
	void Failure() { m_nFailures++; m_summary = FAILURE; }
	UINT GetFailures() { return m_nFailures; }
	void Warning() { m_nWarnings++; if(m_summary == OK) m_summary = WARNING; }
	UINT GetWarnings() { return m_nWarnings; }

protected:
	NDBViewer * m_pNDBViewer;

private:
	TestResult m_summary;
	UINT m_nFailures;
	UINT m_nWarnings;
	WCHAR m_resultsSummary[255];
};

// CConsistencyReport dialog

class CConsistencyReport : public CNDBViewChildDlg
{
	DECLARE_DYNAMIC(CConsistencyReport)

public:
	static BOOL CConsistencyReport::JustRunReport(NDBViewer * pNDBViewer);
	CConsistencyReport(NDBViewer * pNDBViewer, CNDBViewDlg * pNDBDlg, CWnd* pParent = NULL);   // standard constructor
	virtual ~CConsistencyReport();
	ITest** GetTests() { return tests; }

// Dialog Data
	enum { IDD = IDD_CONSISTENCY_REPORT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	NDBViewer * m_pNDBViewer;
private:
	CListCtrl m_lc;
	CTreeCtrl m_tc;
	CImageList m_il;
	ITest* tests[NUM_TESTS];

	TestResult m_finalResult;

public:
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult);
public:
	afx_msg void OnLvnDeleteitemList1(NMHDR *pNMHDR, LRESULT *pResult);
public:
	afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
private:
	void Summary();
};

class BTreeTest :
	public ITest
{
public:
	BTreeTest(NDBViewer* pNDBViewer);
	~BTreeTest() {}

	const WCHAR* GetTitle() { return L"BTree Test"; }
	const WCHAR* GetDescription() { return L"The BTree Test verifies that the BTrees appear consistent"; }

	void PreTest();
	BOOL NBTPage(BTPAGE* pBTPage, PAGETRAILER* pPt, IB ib);
	BOOL BBTPage(BTPAGE* pBTPage, PAGETRAILER* pPt, IB ib);

	void ReportResults(CListCtrl *lc);
	void ReportResults(ofstream& file);

private:
	std::map<BID,BREF> m_seen_pages; // page BID to parent BREF that first saw it
	std::map<IB,BREF> m_seen_pages_ib; // page IB to parent BREF that first saw it
	std::vector<std::pair<BREF,BREF>> m_bbt_failures; // child/parent pairs (besides first)
	std::vector<std::pair<BREF,BREF>> m_nbt_failures; 
};

class HeaderTest :
	public ITest
{
public:
	HeaderTest(NDBViewer* pNDBViewer);
	~HeaderTest(void);

	void PreTest();
	
	const WCHAR* GetTitle() { return L"File Header"; }
	const WCHAR* GetDescription() { return L"The File Header check ensures that the information contained in the file header makes sense."; }

	void ReportResults(CListCtrl *lc);
	void ReportResults(ofstream& file);

private:
// header validation
	TestResult m_trMagic;
	TestResult m_trCRCPartial;
	TestResult m_trMagicClient;
	TestResult m_trVer;
	TestResult m_trVerClient;
	TestResult m_trEOF;
	TestResult m_trBBT;
	TestResult m_trNBT;
	TestResult m_trSentinel;
	TestResult m_trCryptMethod;
	TestResult m_trCRCFull;
};

class CRCTest :
	public ITest
{
public:
	CRCTest(NDBViewer* pNDBViewer);
	~CRCTest();

	const WCHAR* GetTitle() { return L"CRC Test"; }
	const WCHAR* GetDescription() { return L"The CRC Test checks that every page and every block in the database has a valid CRC."; }

	void PreTest();
	BOOL NBTPage(BTPAGE* pBTPage, PAGETRAILER* pPt, IB ib);
	BOOL BBTPage(BTPAGE* pBTPage, PAGETRAILER* pPt, IB ib);

	void ReportResults(CListCtrl *lc);
	void ReportResults(ofstream& file);
private:
	CList<BREF> m_invalidAMapFailedPages;
	CList<BREF> m_failedPages;
	CList<BBTENTRY> m_failedBlocks;
};

class RefTest : 
	public ITest
{
public:
	RefTest(NDBViewer* pNDBViewer);
	~RefTest();

	const WCHAR* GetTitle() { return L"Reference Test"; }
	const WCHAR* GetDescription() { return L"The Reference Test validates that every block referenced by another block or page in the database exists, and has the correct ref count."; }

	BOOL NBTPage(BTPAGE* pBTPage, PAGETRAILER* pPt, IB ib);
	BOOL BBTPage(BTPAGE* pBTPage, PAGETRAILER* pPt, IB ib);

	void ReportResults(CListCtrl *lc);
	void ReportResults(ofstream& file);

	void PostTest();

private:
	struct REFINFO
	{
		UINT diskCount;
		UINT calculatedCount;
	};

	// Helper function to access refinfo for a bid (or create an empty one)
	REFINFO GetRefInfo(BID b);

	CMap<BID, BID, REFINFO, REFINFO> m_refMap;
};

class AMapTest :
	public ITest
{
public:
	AMapTest(NDBViewer* pNDBViewer);
	~AMapTest();

	const WCHAR* GetTitle() { return L"Allocation Test"; }
	const WCHAR* GetDescription() { return L"The Allocation Test validates the AMaps contained on disk are consistent with the data allocated in the BTrees, with themselves, and that no area is double allocated."; }

	void PreTest();
	BOOL NBTPage(BTPAGE* pBTPage, PAGETRAILER* pPt, IB ib);
	BOOL BBTPage(BTPAGE* pBTPage, PAGETRAILER* pPt, IB ib);
	void PostTest();

	void ReportResults(CListCtrl *lc);
	void ReportResults(ofstream& file);

private:

	bool m_fValidAMap;
	IB m_ibWatermark;
	CMap<IB, IB, void*, void*> m_observedAMaps; // to check double allocations

	// this test actually has 5 different subtests:
	// 1. No block or page claims to share the same IB as another block/page
	// 2. No block or page is allocated past EOF
	// 3. All blocks/pages are marked as allocated in the AMap/PMap
	// 4. All areas of the AMap marked as occupied actually are occupied (warning only)
	// 4. The PMap and AMaps are consistent with eachother (no area is free in both)
	//
	// If the AMap is invalid, only the first two checks are done (using an in memory version
	// of the AMap). The third check is done during the BTree walk, and checks the AMap
	// on the disk. The fourth check is done during PostTest, by comparing the on disk AMap
	// to the in memory AMap. The fifth check is done during the PreTest.

	typedef enum
	{
		E_OCCUPIED_SPACE_BLOCK,
		E_OCCUPIED_SPACE_PAGE,
		E_ALLOCATED_PAST_EOF_BLOCK,
		E_ALLOCATED_PAST_EOF_PAGE,
		E_FREE_SPACE_BLOCK,
		E_FREE_SPACE_PAGE,
		E_INCONSISTENT_MAPS,
		W_LEAKED_SPACE,
		I_INVALID_AMAP
	} Result;

	struct ResultList
	{
		BREF bref;
		UINT cb;
		Result result;
	};

	CList<ResultList> m_results;

// Helper functions
	// returns TRUE if the space is already marked as occupied in memory.
	// has the side effect of marking the space as occupied in memory.
	bool TestRangeMem(IB ib, UINT size);
	// returns TRUE if the space is marked as occupied on disk.
	bool TestRangeDisk(IB ib, UINT size);

	void AddResult(Result r, IB ib = 0, UINT cb = 0, BID bid = 0);
};