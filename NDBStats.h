#pragma once

#include "NDBViewChildDlg.h"
#include "NDBViewer.h"
// CNDBStats dialog

class CNDBStats : public CNDBViewChildDlg
{
	DECLARE_DYNAMIC(CNDBStats)

public:
	CNDBStats(NDBViewer * pNDBViewer, CNDBViewDlg * pNDBViewDlg, CWnd* pParent = NULL);   // standard constructor
	virtual ~CNDBStats();

// Dialog Data
	enum { IDD = IDD_STATS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	
	DECLARE_MESSAGE_MAP()

private:
	static bool BBTStats(BTPAGE* pBTPage, PAGETRAILER* pPT, IB ib, void* pv);
	static bool NBTStats(BTPAGE* pBTPage, PAGETRAILER* pPT, IB ib, void* pv);

	NDBViewer * m_pNDBViewer;

// page stats. Constant size, so we don't need a seperate counter
	UINT m_uiPages;
	UINT m_uiBBTPages;
	UINT m_uiNBTPages;
	UINT m_uiAMapPages;
	UINT m_uiFMapPages;
	UINT m_uiPMapPages;
	UINT m_uiFPMapPages;

// block stats. Variable size
	UINT m_uiBlocks;
	CB m_cbBlocks;
	UINT m_uiExternalBlocks;
	CB m_cbExternalBlocks;
	UINT m_uiXBlocks;
	CB m_cbXBlocks;
	UINT m_uiXXBlocks;
	CB m_cbXXBlocks;
	UINT m_uiSBlocks;
	CB m_cbSBlocks;

// "reused blocks" is a count of all refs > 2. For example, if all blocks
// in the NDB had a refcount of 2 except one block with a refcount of 4,
// the count of reused blocks would be 2 (one per extra ref), and the size of
// the reused blocks would be twice that reused block.
	UINT m_uiReusedBlocks;
	CB m_cbReusedBlocks;

// The "largest" block, as reported by either the XXBlock header or the BBT
// (if this is a tiny database with no XBlocks)
	BID m_bidLargestBlock;
	CB m_cbLargestBlock;

// node stats
	UINT m_uiMessages;
	UINT m_uiAssMessages;
	UINT m_uiFolders;
	UINT m_uiSearchFlds;
	NID m_nidLastModified; // whatever node has the largest BID as data or subnode
	BID m_bidHighest;

// general info can all be read directly from the header
	virtual BOOL OnInitDialog();
};
