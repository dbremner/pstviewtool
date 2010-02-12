#include "StdAfx.h"
#include "NDBViewer.h"
#include "ndbport.h"

CMap<NID, NID, const WCHAR*, const WCHAR*> s_nidToString;
bool fMapInit = false;
void InitNidMap();

CFont* g_Font;
CFont* g_FontFixedWidth;
bool fFontFixedWidthInit = false;
bool fFontInit = false;

const WCHAR* GetPlatformString(WORD w)
{
	if(w & NDB_PLATFORM_NT) return L"NDB_PLATFORM_NT";
	if(w & NDB_PLATFORM_CHI) return L"NDB_PLATFORM_CHI";
	if(w & NDB_PLATFORM_WIN16) return L"NDB_PLATFORM_WIN16";
	if(w & NDB_PLATFORM_DOS) return L"NDB_PLATFORM_DOS";
	if(w & NDB_PLATFORM_MAC) return L"NDB_PLATFORM_MAC";

	return L"Unknown";
}

const WCHAR* GetVersionString(WORD w)
{
	if(w == NDB16K_VERSION) return L"NDB16K_VERSION";
	if(w == NDB64K_VERSION) return L"NDB64K_VERSION";
	if(w == NDBUNI_VERSION) return L"NDBUNI_VERSION";
	if(w == NDBUNI_HUGETBL_VERSION) return L"NDBUNI_HUGETBL_VERSION";
	if(w == NDBUNI_BIGHNALLOC_VERSION) return L"NDBUNI_BIGHNALLOC_VERSION";
	if(w == NDBUNI_BIGSBLOCKLEAF_VERSION) return L"NDBUNI_BIGSBLOCKLEAF_VERSION";

	return L"Unknown";
}

const WCHAR* GetClientVersionString(WORD w)
{
	if(w == PST_VERSION) return L"PST_VERSION";
	if(w == OST_VERSION) return L"OST_VERSION";
	if(w == OST_VERSION_55) return L"OST_VERSION_55";
	if(w == OST_VERSION_50) return L"OST_VERSION_50";
	if(w == XST_VERSION) return L"XST_VERSION";

	return L"Unknown";
}

const WCHAR* GetClientMagicNumString(WORD w)
{
	if(w == PST_MAGIC) return L"PST_MAGIC";
	if(w == OST_MAGIC) return L"OST_MAGIC";
	if(w == XST_MAGIC) return L"XST_MAGIC";
	return L"Unknown";
}

const WCHAR* GetMagicNumString(DWORD dw)
{
	if(dw == dwMagicHL) return L"dwMagicHL";
	if(dw == dwMagicLH) return L"dwMagicLH";

	return L"Unknown";
}

const WCHAR* GetAMapString(BYTE b)
{
	if(b == 0) return L"FALSE";
	if(b == 1) return L"TRUE";
	if(b == OL12_VALID_AMAP) return L"OL12_VALID_AMAP";

	return L"Unknown, TRUE?";
}

const WCHAR* GetPTypeString(BYTE b)
{
	if(b == ptypeBBT) return L"ptypeBBT";
	if(b == ptypeNBT) return L"ptypeNBT";
	if(b == ptypeFMap) return L"ptypeFMap";
	if(b == ptypePMap) return L"ptypePMap";
	if(b == ptypeAMap) return L"ptypeAMap";
	if(b == ptypeFPMap) return L"ptypeFPMap";
	if(b == ptypeFL) return L"ptypeFL";
	if(b == ptypeRBT) return L"ptypeRBT";
	if(b == ptypeNRBT) return L"ptypeNRBT";

	return L"Unknown";
}

CFont* GetGlobalFont()
{
	if(!fFontInit)
	{
		g_Font = new CFont();
		g_Font->CreatePointFont(80, L"Segoe UI");
		fFontInit = true;
	}
	return g_Font;
}

CFont* GetGlobalFontFixedWidth()
{
	if(!fFontFixedWidthInit)
	{
		g_FontFixedWidth = new CFont();
		g_FontFixedWidth->CreatePointFont(80, L"Courier New");
		fFontFixedWidthInit = true;
	}
	return g_FontFixedWidth;
}

void DestroyFonts()
{
	if(fFontInit)
		delete g_Font;
	if(fFontFixedWidthInit)
		delete g_FontFixedWidth;

	fFontInit = false;
	fFontFixedWidthInit = false;
}

NDBViewer::NDBViewer(LPCWSTR filename)
: m_ndb(filename, CFile::modeRead | CFile::shareExclusive)
{
	ASSERT(filename != NULL);
	InitNidMap();
	ReadVerifyHeader();
	m_cachedAMapBitmap = NULL;
	m_cachedBTBitmap = NULL;
}

NDBViewer::~NDBViewer(void)
{
	// free memory used by the page cache
	for(int i = 0; i < m_fileCacheAgeList.GetCount(); i++)
	{
		CACHEINFO* pCi = m_fileCache[m_fileCacheAgeList.GetAt(m_fileCacheAgeList.FindIndex(i))];
		delete [] pCi->pData;
		delete pCi;
	}

	// delete the cached bitmaps, if any
	if(m_cachedAMapBitmap)
		delete m_cachedAMapBitmap;

	if(m_cachedBTBitmap)
		delete m_cachedBTBitmap;
}

CB NDBViewer::IsFree(IB ib)
{
	// Is this IB free in the AMap, and if so, how much free space?
	IB ibAMap = ib - ((ib - ibAMapBase) % cbPerAMap);
	IB ibPMap = ib - ((ib - ibPMapBase) % cbPerPMap);
	UINT uiAMapSlot = (UINT)(ib - ibAMap) / cbPerSlot;
	UINT uiPMapSlot = (UINT)(ib - ibPMap) / cbPage;
	BOOL fAMapFree = FALSE;
	BOOL fPMapFree = FALSE;
	BYTE page[512];
	CB cbFreeSpace = 0;

	// Read AMap page
	ReadPage(page, sizeof(page), NULL, ibAMap);

	// Is this slot free?
	fAMapFree = !(BMapTestBit(page, uiAMapSlot));
	if(fAMapFree)
	{
		cbFreeSpace = cbPerSlot;
	}
	else
	{
		// if it's occupied, check to see if it's free in the PMap
		// if the IB is page aligned.
		if(uiAMapSlot % cbPerSlot == 0)
		{
			// sector aligned. check PMap
			ReadPage(page, sizeof(page), NULL, ibPMap);
			fPMapFree = !(BMapTestBit(page, uiPMapSlot));
			if(fPMapFree)
			{
				cbFreeSpace = cbPage;
			}
		}
	}
	return cbFreeSpace;

}
bool NDBViewer::ReadVerifyHeader(void)
{
	DWORD dwCRC;

	m_ndb.SeekToBegin();
	m_ndb.Read((void*)(&m_header), sizeof(m_header));
	
	// compute partial crc
	dwCRC =	ComputeCRC((((BYTE*)&m_header) + ibHeaderCRCPartialStart), cbHeaderCRCPartial);
	m_fPartialCRCPass = (dwCRC == m_header.dwCRCPartial);

	// compute full crc
	dwCRC = ComputeCRC((((BYTE*)&m_header) + ibHeaderCRCFullStart), cbHeaderCRCFull);
	m_fFullCRCPass = (dwCRC == m_header.dwCRCFull);
	return true;
}

void NDBViewer::PopulateNBT(CTreeCtrl *ptctrl)
{
	HTREEITEM hItem = NULL;
	NodeData * pNodeData = NULL;
	WCHAR nodeTitle[255];

	// Add root node to tree
	wsprintf(nodeTitle, L"Page BID: 0x%I64X, IB: %I64u", m_header.root.brefNBT.bid, m_header.root.brefNBT.ib);
	hItem = ptctrl->InsertItem(nodeTitle, TVI_ROOT);
	pNodeData = new NodeData;
	pNodeData->bref = m_header.root.brefNBT;
	pNodeData->nid = 0;
	pNodeData->btkey = 0;
	pNodeData->cb = 512;
	pNodeData->ulFlags = aOpenBTPage;
	ptctrl->SetItemData(hItem, (DWORD_PTR)pNodeData);
	ptctrl->SetItemImage(hItem, iconPage, iconPage);
	AddFalseChild(ptctrl, hItem);

	// Add children of root node
	AddBTNodeChildren(ptctrl, hItem);
}

void NDBViewer::PopulateBBT(CTreeCtrl *ptctrl)
{
	HTREEITEM hItem = NULL;
	NodeData * pNodeData = NULL;
	WCHAR nodeTitle[255];

	// Add root node to tree
	wsprintf(nodeTitle, L"Page BID: 0x%I64X, IB: %I64u", m_header.root.brefBBT.bid, m_header.root.brefBBT.ib);
	hItem = ptctrl->InsertItem(nodeTitle, TVI_ROOT);
	pNodeData = new NodeData;
	pNodeData->bref = m_header.root.brefBBT;
	pNodeData->btkey = 0;
	pNodeData->nid = 0;
	pNodeData->cb = 512;
	pNodeData->ulFlags = aOpenBTPage;
	ptctrl->SetItemData(hItem, (DWORD_PTR)pNodeData);
	ptctrl->SetItemImage(hItem, iconPage, iconPage);
	AddFalseChild(ptctrl, hItem);

	// Add children of root node
	AddBTNodeChildren(ptctrl, hItem);
}

void NDBViewer::PopulateSBlock(CTreeCtrl *ptctrl, const BREF& bref, CB cb)
{
	HTREEITEM hItem = NULL;
	NodeData * pNodeData = NULL;
	WCHAR nodeTitle[255];

	// Add root node to tree
	wsprintf(nodeTitle, L"SBlock BID: 0x%I64X, IB: %I64u", bref.bid, bref.ib);
	hItem = ptctrl->InsertItem(nodeTitle, TVI_ROOT);
	pNodeData = new NodeData;
	pNodeData->bref = bref;
	pNodeData->btkey = 0;
	pNodeData->nid = 0;
	pNodeData->cb = cb;
	pNodeData->ulFlags = aOpenBlock;
	ptctrl->SetItemData(hItem, (DWORD_PTR)pNodeData);
	ptctrl->SetItemImage(hItem, iconPage, iconPage);
	AddFalseChild(ptctrl, hItem);

	// Add children of root node
	AddSBlockChildren(ptctrl, hItem);
}

void NDBViewer::PopulateHeader(CTreeCtrl *ptctrl)
{
	WCHAR nodeTitle[255];
	HTREEITEM hItem = NULL;

	wsprintf(nodeTitle, L"dwMagic: 0x%X (%s)", m_header.dwMagic, GetMagicNumString(m_header.dwMagic));
	ptctrl->InsertItem(nodeTitle, TVI_ROOT);

	wsprintf(nodeTitle, L"dwCRCPartial: 0x%X", m_header.dwCRCPartial);
	ptctrl->InsertItem(nodeTitle, TVI_ROOT);

	wsprintf(nodeTitle, L"wMagicClient: 0x%X (%s)", m_header.wMagicClient, GetClientMagicNumString(m_header.wMagicClient));
	ptctrl->InsertItem(nodeTitle, TVI_ROOT);

	wsprintf(nodeTitle, L"wVer: %u (%s)", m_header.wVer, GetVersionString(m_header.wVer));
	ptctrl->InsertItem(nodeTitle, TVI_ROOT);

	wsprintf(nodeTitle, L"wVerClient: %u (%s)", m_header.wVerClient, GetClientVersionString(m_header.wVerClient));
	ptctrl->InsertItem(nodeTitle, TVI_ROOT);

	wsprintf(nodeTitle, L"bPlatformCreate: 0x%X (%s)", m_header.bPlatformCreate, GetPlatformString(m_header.bPlatformCreate));
	ptctrl->InsertItem(nodeTitle, TVI_ROOT);

	wsprintf(nodeTitle, L"bPlatformAccess: 0x%X (%s)", m_header.bPlatformAccess, GetPlatformString(m_header.bPlatformAccess));
	ptctrl->InsertItem(nodeTitle, TVI_ROOT);

	wsprintf(nodeTitle, L"dwOpenDBID: 0x%X", m_header.dwOpenDBID);
	ptctrl->InsertItem(nodeTitle, TVI_ROOT);

	wsprintf(nodeTitle, L"dwOpenClaimID: 0x%X", m_header.dwOpenClaimID);
	ptctrl->InsertItem(nodeTitle, TVI_ROOT);

	wsprintf(nodeTitle, L"bidUnused: 0x%I64X", m_header.bidUnused);
	ptctrl->InsertItem(nodeTitle, TVI_ROOT);

	wsprintf(nodeTitle, L"bidNextP: 0x%I64X", m_header.bidNextP);
	ptctrl->InsertItem(nodeTitle, TVI_ROOT);

	wsprintf(nodeTitle, L"dwUnique: 0x%X", m_header.dwUnique);
	ptctrl->InsertItem(nodeTitle, TVI_ROOT);

	hItem = ptctrl->InsertItem(L"rgnid", TVI_ROOT);
		for(int i = 0; i < cnidTypesMax; i++)
		{
			if(i == NID_TYPE_NORMAL_FOLDER ||
				i == NID_TYPE_SEARCH_FOLDER || 
				i == NID_TYPE_NORMAL_MESSAGE ||
				i == NID_TYPE_ASSOC_MESSAGE
				)
				wsprintf(nodeTitle, L"[%i] %s: 0x%X (%u allocated)", i, s_nidTypeToString[i], m_header.rgnid[i], (m_header.rgnid[i]+1) - GetMinNIDIndex(i));
			else
				wsprintf(nodeTitle, L"[%i] %s: 0x%X", i, s_nidTypeToString[i], m_header.rgnid[i]);
			ptctrl->InsertItem(nodeTitle, hItem);
		}

	hItem = ptctrl->InsertItem(L"root", TVI_ROOT);
		wsprintf(nodeTitle, L"cOrphans: %u", m_header.root.cOrphans);
		ptctrl->InsertItem(nodeTitle, hItem);

		wsprintf(nodeTitle, L"ibFileEof: %I64u", m_header.root.ibFileEof);
		ptctrl->InsertItem(nodeTitle, hItem);

		wsprintf(nodeTitle, L"ibAMapLast: %I64u", m_header.root.ibAMapLast);
		ptctrl->InsertItem(nodeTitle, hItem);

		wsprintf(nodeTitle, L"cbAMapFree: %u", m_header.root.cbAMapFree);
		ptctrl->InsertItem(nodeTitle, hItem);
		
		wsprintf(nodeTitle, L"cbPMapFree: %u", m_header.root.cbPMapFree);
		ptctrl->InsertItem(nodeTitle, hItem);

		wsprintf(nodeTitle, L"brefNBT Page BID: 0x%I64X, IB: %I64u", m_header.root.brefNBT.bid, m_header.root.brefNBT.ib);
		ptctrl->InsertItem(nodeTitle, hItem);

		wsprintf(nodeTitle, L"brefBBT Page BID: 0x%I64X, IB: %I64u", m_header.root.brefBBT.bid, m_header.root.brefBBT.ib);
		ptctrl->InsertItem(nodeTitle, hItem);

		wsprintf(nodeTitle, L"fAmapValid: 0x%X (%s)", m_header.root.fAMapValid, GetAMapString(m_header.root.fAMapValid));
		ptctrl->InsertItem(nodeTitle, hItem);

		wsprintf(nodeTitle, L"bARVec: 0x%X", m_header.root.bARVec);
		ptctrl->InsertItem(nodeTitle, hItem);

		wsprintf(nodeTitle, L"cARVec: %u", m_header.root.cARVec);
		ptctrl->InsertItem(nodeTitle, hItem);

	hItem = ptctrl->InsertItem(L"rgbFM", TVI_ROOT);
		for(int i = 0; i < cEntFMRoot; i++)
		{
			wsprintf(nodeTitle, L"[%i] 0x%X", i, m_header.rgbFM[i]);
			ptctrl->InsertItem(nodeTitle, hItem);
		}

	hItem = ptctrl->InsertItem(L"rgbFP", TVI_ROOT);
		for(int i = 0; i < cbFPRoot; i++)
		{
			wsprintf(nodeTitle, L"[%i] 0x%X", i, m_header.rgbFP[i]);
			ptctrl->InsertItem(nodeTitle, hItem);
		}

	wsprintf(nodeTitle, L"bSentinel: 0x%X", m_header.bSentinel);
	ptctrl->InsertItem(nodeTitle, TVI_ROOT);

	wsprintf(nodeTitle, L"bCryptMethod: 0x%X (%s)", m_header.bCryptMethod, CRYPTSTRING(m_header.bCryptMethod));
	ptctrl->InsertItem(nodeTitle, TVI_ROOT);

	wsprintf(nodeTitle, L"rgbReserved: 0x%X", *((DWORD*)&m_header.rgbReserved));
	ptctrl->InsertItem(nodeTitle, TVI_ROOT);

	wsprintf(nodeTitle, L"bidNextB: 0x%I64X", m_header.bidNextB);
	ptctrl->InsertItem(nodeTitle, TVI_ROOT);

	wsprintf(nodeTitle, L"dwCRCFull: 0x%X", m_header.dwCRCFull);
	ptctrl->InsertItem(nodeTitle, TVI_ROOT);

	hItem = ptctrl->InsertItem(L"rgbVersionEncoded", TVI_ROOT);
		for(int i = 0; i < 3; i++)
		{
			wsprintf(nodeTitle, L"[%i] 0x%X", i, m_header.rgbVersionEncoded[i]);
			ptctrl->InsertItem(nodeTitle, hItem);
		}

	wsprintf(nodeTitle, L"bLockSemaphore: 0x%X", m_header.bLockSemaphore);
	ptctrl->InsertItem(nodeTitle, TVI_ROOT);

	hItem = ptctrl->InsertItem(L"rgbLock", TVI_ROOT);
		for(int i = 0; i < cLockMax; i++)
		{
			wsprintf(nodeTitle, L"[%i] 0x%X", i, m_header.rgbLock[i]);
			ptctrl->InsertItem(nodeTitle, hItem);
		}
}

// Because node expansion (the trigger to add AddSBlockChildren) is either manual
// or otherwise guarded, we don't action on bad data here
void NDBViewer::AddSBlockChildren(CTreeCtrl* ptctrl, HTREEITEM parent)
{
	BLOCKTRAILER bt;
	BYTE * pBuffer = NULL;
	SBLOCK * pSBlock = NULL;
	DWORD dwCRC;
	WORD sig;
	bool fCorrectType;
	bool fCRCPass;
	bool fSigMatch;
	bool fValid; 
	HTREEITEM hItemChild = NULL;
	WCHAR nodeTitle[255];
	NodeData * pNodeData = (NodeData *)ptctrl->GetItemData(parent);
	NodeData * pChildNodeData = NULL;

	if(pNodeData && pNodeData->bref.bid && (!pNodeData->cb || !pNodeData->bref.ib))
	{
		// We need to do a BBT lookup to fill out the data
		UINT cRefs;
		if(!LookupBID(pNodeData->bref.bid, pNodeData->bref, pNodeData->cb, cRefs))
		{
			RemoveFalseChild(ptctrl, parent);
			wsprintf(nodeTitle, L"Could not find SBlock in BBT");
			hItemChild = ptctrl->InsertItem(nodeTitle, parent);
			pChildNodeData = new NodeData;
			memcpy(pChildNodeData, pNodeData, sizeof(NodeData));
			pChildNodeData->ulFlags |= fInfoNode;
			ptctrl->SetItemData(hItemChild, (DWORD_PTR)pChildNodeData);
			return;
		}
	}

	if(pNodeData && !(pNodeData->ulFlags & fChildrenLoaded))
	{
		RemoveFalseChild(ptctrl, parent);
		UINT cbAligned = (UINT)CbAlignDisk(pNodeData->cb);
		UINT cbUnaligned = BBufferSize((UINT)pNodeData->cb);
		pBuffer = new BYTE[cbUnaligned];
		pSBlock = (SBLOCK*)pBuffer;
		ReadBlock(pBuffer, cbUnaligned, &bt, pNodeData->bref.ib, cbAligned);

		dwCRC = ComputeCRC(pBuffer, cbUnaligned);
		fCRCPass = (dwCRC == bt.dwCRC);
		sig = ComputeSig(pNodeData->bref.ib, pNodeData->bref.bid);
		fCRCPass = (dwCRC == bt.dwCRC);
		fSigMatch = (sig == bt.wSig);
		fCorrectType = (pSBlock->btype == btypeSB);
		fValid = fCRCPass && fSigMatch && fCorrectType;

		if(!fValid)
		{
			ptctrl->SetItemImage(parent, iconCorrupt, iconCorrupt);
			pNodeData->ulFlags |= errCRCFailed;

			wsprintf(nodeTitle, L"CRC %s", fCRCPass ? L"Valid" : L"Invalid");
			hItemChild = ptctrl->InsertItem(nodeTitle, parent);
			pChildNodeData = new NodeData;
			memcpy(pChildNodeData, pNodeData, sizeof(NodeData));
			pChildNodeData->ulFlags |= fInfoNode;
			ptctrl->SetItemData(hItemChild, (DWORD_PTR)pChildNodeData);

			wsprintf(nodeTitle, L"Sig %s (0x%X)", fSigMatch ? L"Valid" : L"Invalid", bt.wSig);
			hItemChild = ptctrl->InsertItem(nodeTitle, parent);
			pChildNodeData = new NodeData;
			memcpy(pChildNodeData, pNodeData, sizeof(NodeData));
			pChildNodeData->ulFlags |= fInfoNode;
			ptctrl->SetItemData(hItemChild, (DWORD_PTR)pChildNodeData);

			wsprintf(nodeTitle, L"BType %s (%s)", fCorrectType ? L"Valid" : L"Invalid", BTYPESTRING(pSBlock->btype));
			hItemChild = ptctrl->InsertItem(nodeTitle, parent);
			pChildNodeData = new NodeData;
			memcpy(pChildNodeData, pNodeData, sizeof(NodeData));
			pChildNodeData->ulFlags |= fInfoNode;
			ptctrl->SetItemData(hItemChild, (DWORD_PTR)pChildNodeData);
		}

		// Add SBlock specific values
		wsprintf(nodeTitle, L"Depth: %u", pSBlock->cLevel);
		hItemChild = ptctrl->InsertItem(nodeTitle, parent);
		pChildNodeData = new NodeData;
		memcpy(pChildNodeData, pNodeData, sizeof(NodeData));
		pChildNodeData->ulFlags |= fInfoNode;
		ptctrl->SetItemData(hItemChild, (DWORD_PTR)pChildNodeData);

		wsprintf(nodeTitle, L"Count: %u", pSBlock->cEnt);
		hItemChild = ptctrl->InsertItem(nodeTitle, parent);
		pChildNodeData = new NodeData;
		memcpy(pChildNodeData, pNodeData, sizeof(NodeData));
		pChildNodeData->ulFlags |= fInfoNode;
		ptctrl->SetItemData(hItemChild, (DWORD_PTR)pChildNodeData);

		if(pSBlock->cLevel > 0)
		{
			SIENTRY * siEntry = pSBlock->rgsi;
			for(UINT i = 0; i < SBIEnt(*pSBlock, cbUnaligned); i++, siEntry++)
			{
				wsprintf(nodeTitle, L"NID (Key): 0x%I64X, BID: 0x%I64X", siEntry->nid, siEntry->bid);
				hItemChild = ptctrl->InsertItem(nodeTitle, parent);
				pChildNodeData = new NodeData;
				pChildNodeData->bref.bid = siEntry->bid;
				pChildNodeData->bref.ib = 0;
				pChildNodeData->btkey = siEntry->nid;
				pChildNodeData->cb = 0;
				pChildNodeData->nid = 0;
				pChildNodeData->ulFlags = aBrowseBBT;
				if(pSBlock->cLevel == 1) // child is a leaf page
				{
					ptctrl->SetItemImage(hItemChild, iconLeafPage, iconLeafPage);
				}
				else // child is another non-leaf page
				{
					ptctrl->SetItemImage(hItemChild, iconPage, iconPage);
				}
				ptctrl->SetItemData(hItemChild, (DWORD_PTR)pChildNodeData);
				AddFalseChild(ptctrl, hItemChild);
			}
		}
		else
		{
			// this is a leaf page, add nodes as subnodes
			SLENTRY * slEntry = pSBlock->rgsl;
			BOOL fGreatSuccess;
			HTREEITEM hItemChildChild = NULL;
			const WCHAR * pNidName;

			for(UINT i = 0; i < SBLEnt(*pSBlock, cbUnaligned); i++, slEntry++)
			{
				// Add the subnode NID
				fGreatSuccess = s_nidToString.Lookup((NID)slEntry->nid, pNidName);
				wsprintf(nodeTitle, L"NID: 0x%X (%s)", slEntry->nid, fGreatSuccess ? pNidName : TYPESTRINGFROMNID(slEntry->nid));
				hItemChild = ptctrl->InsertItem(nodeTitle, parent);
				pChildNodeData = new NodeData;
				pChildNodeData->bref.bid = slEntry->bidData;
				pChildNodeData->ulFlags = aBrowseBBT | fChildrenLoaded;
				pChildNodeData->btkey = slEntry->nid;
				pChildNodeData->nid = 0;
				ptctrl->SetItemData(hItemChild, (DWORD_PTR)pChildNodeData);
				ptctrl->SetItemImage(hItemChild, ICONFROMNID(slEntry->nid), ICONFROMNID(slEntry->nid));

				// Add the data and subnodes BIDs as children
				wsprintf(nodeTitle, L"Data BID: 0x%I64X", slEntry->bidData);
				hItemChildChild = ptctrl->InsertItem(nodeTitle, hItemChild);
				pChildNodeData = new NodeData;
				pChildNodeData->bref.bid = slEntry->bidData;
				pChildNodeData->nid = 0;
				pChildNodeData->ulFlags = aBrowseBBT | fChildrenLoaded | fInfoNode;
				ptctrl->SetItemData(hItemChildChild, (DWORD_PTR)pChildNodeData);

				wsprintf(nodeTitle, L"Subnodes BID: 0x%I64X", slEntry->bidSub);
				hItemChildChild = ptctrl->InsertItem(nodeTitle, hItemChild);
				pChildNodeData = new NodeData;
				pChildNodeData->bref.bid = slEntry->bidSub;
				pChildNodeData->nid = 0;
				pChildNodeData->ulFlags = aBrowseBBT | fChildrenLoaded | fInfoNode;
				ptctrl->SetItemData(hItemChildChild, (DWORD_PTR)pChildNodeData);
			}

			if(fValid)
				ptctrl->SetItemImage(parent, iconLeafPage, iconLeafPage);
		}
		pNodeData->ulFlags |= fChildrenLoaded;
	}
	delete [] pBuffer;
}

// Because node expansion (the trigger to add BTNodeChildren) is either manual
// or otherwise guarded, we don't action on bad data here
void NDBViewer::AddBTNodeChildren(CTreeCtrl *ptctrl, HTREEITEM hItem)
{
	PAGETRAILER pt;
	BTPAGE page;
	DWORD dwCRC;
	WORD sig;
	bool fCorrectType;
	bool fCRCPass;
	bool fBeforeEof;
	bool fSigMatch;
	bool fValid; 
	HTREEITEM hItemChild = NULL;
	WCHAR nodeTitle[255];
	NodeData * pNodeData = (NodeData *)ptctrl->GetItemData(hItem);
	NodeData * pChildNodeData = NULL;

	if(pNodeData && !(pNodeData->ulFlags & fChildrenLoaded))
	{
		RemoveFalseChild(ptctrl, hItem);
		// read in the page
		ReadPage((BYTE*)&page, sizeof(page), &pt, pNodeData->bref.ib);
		
		// validate
		dwCRC = ComputeCRC(&page, sizeof(page));
		sig = ComputeSig(pNodeData->bref.ib, pNodeData->bref.bid);
		fCRCPass = (dwCRC == pt.dwCRC);
		fSigMatch = (sig == pt.wSig);
		fBeforeEof = (pNodeData->bref.ib + sizeof(page) < GetFileEOF());
		fCorrectType = (pt.ptype == pt.ptypeRepeat);
		fValid = (fCRCPass && fSigMatch && fCorrectType && fBeforeEof);

		// add validation subnodes if necessary
		if(!fValid)
		{
			ptctrl->SetItemImage(hItem, iconCorrupt, iconCorrupt);
			pNodeData->ulFlags |= errCRCFailed;

			if(!fBeforeEof)
			{
				hItemChild = ptctrl->InsertItem(L"After EOF");
				pChildNodeData = new NodeData;
				memcpy(pChildNodeData, pNodeData, sizeof(NodeData));
				pChildNodeData->ulFlags |= fInfoNode;
				ptctrl->SetItemData(hItemChild, (DWORD_PTR)pChildNodeData);
			}

			wsprintf(nodeTitle, L"CRC %s", fCRCPass ? L"Valid" : L"Invalid");
			hItemChild = ptctrl->InsertItem(nodeTitle, hItem);
			pChildNodeData = new NodeData;
			memcpy(pChildNodeData, pNodeData, sizeof(NodeData));
			pChildNodeData->ulFlags |= fInfoNode;
			ptctrl->SetItemData(hItemChild, (DWORD_PTR)pChildNodeData);

			wsprintf(nodeTitle, L"PType %s (0x%X)", fCorrectType ? L"Valid" : L"Invalid", pt.ptype);
			hItemChild = ptctrl->InsertItem(nodeTitle, hItem);
			pChildNodeData = new NodeData;
			memcpy(pChildNodeData, pNodeData, sizeof(NodeData));
			pChildNodeData->ulFlags |= fInfoNode;
			ptctrl->SetItemData(hItemChild, (DWORD_PTR)pChildNodeData);

			wsprintf(nodeTitle, L"Sig %s (0x%X)", fSigMatch ? L"Valid" : L"Invalid", pt.wSig);
			hItemChild = ptctrl->InsertItem(nodeTitle, hItem);
			pChildNodeData = new NodeData;
			memcpy(pChildNodeData, pNodeData, sizeof(NodeData));
			pChildNodeData->ulFlags |= fInfoNode;
			ptctrl->SetItemData(hItemChild, (DWORD_PTR)pChildNodeData);
		}

		// add BTPage specific subnodes
		wsprintf(nodeTitle, L"Depth: %u", page.cLevel);
		hItemChild = ptctrl->InsertItem(nodeTitle, hItem);
		pChildNodeData = new NodeData;
		memcpy(pChildNodeData, pNodeData, sizeof(NodeData));
		pChildNodeData->ulFlags |= fInfoNode;
		ptctrl->SetItemData(hItemChild, (DWORD_PTR)pChildNodeData);

		wsprintf(nodeTitle, L"Count: %u", page.cEnt);
		hItemChild = ptctrl->InsertItem(nodeTitle, hItem);
		pChildNodeData = new NodeData;
		memcpy(pChildNodeData, pNodeData, sizeof(NodeData));
		pChildNodeData->ulFlags |= fInfoNode;
		ptctrl->SetItemData(hItemChild, (DWORD_PTR)pChildNodeData);

		if(page.cLevel > 0)
		{
			// add subpages
			BTENTRY *pbte = (BTENTRY*)(page.rgbte);
			for(int i = 0; i < (page.cEnt < cBTEntMax ? page.cEnt : (int)cBTEntMax); i++, pbte++)
			{
				NodeData * pNodeData = NULL;
				wsprintf(nodeTitle, L"BTKey: 0x%I64X, Page BID: 0x%I64X, IB: %I64u", pbte->btkey, pbte->bref.bid, pbte->bref.ib);
				hItemChild = ptctrl->InsertItem(nodeTitle, hItem);
				pNodeData = new NodeData;
				pNodeData->cb = 512;
				pNodeData->nid = 0;
				pNodeData->bref = pbte->bref;
				pNodeData->btkey = pbte->btkey;
				pNodeData->ulFlags = aOpenBTPage;

				if(page.cLevel == 1) // child is a leaf page
				{
					ptctrl->SetItemImage(hItemChild, iconLeafPage, iconLeafPage);
				}
				else // child is another non-leaf page
				{
					ptctrl->SetItemImage(hItemChild, iconPage, iconPage);
				}
				ptctrl->SetItemData(hItemChild, (DWORD_PTR)pNodeData);
				AddFalseChild(ptctrl, hItemChild);
			}
		} 
		else
		{
			// this is a leaf page - add the data in the page as subnodes
			if(pt.ptype == ptypeNBT)
			{
				AddNBTLeafNodes(ptctrl, hItem, page);
			} 
			else
			{
				AddBBTLeafNodes(ptctrl, hItem, page);
			}
			if(fValid)
				ptctrl->SetItemImage(hItem, iconLeafPage, iconLeafPage);
		}
		pNodeData->ulFlags |= fChildrenLoaded;
	}
}

void NDBViewer::AddNBTLeafNodes(CTreeCtrl* ptctrl, HTREEITEM parent, const BTPAGE& btpage)
{
	WCHAR nodeTitle[255];
	BOOL fGreatSuccess = FALSE;
	const WCHAR * pNidName;
	HTREEITEM hItem = NULL;
	HTREEITEM hItemChild = NULL;
	NBTENTRY *pnbtEntry = (NBTENTRY*)(btpage.rgbte);
	NodeData * pNodeData = NULL;

	for(UINT i = 0; i < NBTEnt(btpage); i++, pnbtEntry++)
	{
		// add this entry
		fGreatSuccess = s_nidToString.Lookup((NID)pnbtEntry->nid, pNidName);
		wsprintf(nodeTitle, L"NID: 0x%I64X (%s)", pnbtEntry->nid, fGreatSuccess ? pNidName : TYPESTRINGFROMNID(pnbtEntry->nid));
		hItem = ptctrl->InsertItem(nodeTitle, parent);
		pNodeData = new NodeData;
		pNodeData->nid = (NID)pnbtEntry->nid;
		pNodeData->bref.bid = pnbtEntry->bidData;
		pNodeData->ulFlags = aBrowseBBT | fChildrenLoaded;
		pNodeData->btkey = pnbtEntry->nid;
		ptctrl->SetItemData(hItem, (DWORD_PTR)pNodeData);
		ptctrl->SetItemImage(hItem, ICONFROMNID(pnbtEntry->nid), ICONFROMNID(pnbtEntry->nid));

		// add information about this entry
		wsprintf(nodeTitle, L"Data BID: 0x%I64X", pnbtEntry->bidData);
		hItemChild = ptctrl->InsertItem(nodeTitle, hItem);
		pNodeData = new NodeData;
		pNodeData->bref.bid = pnbtEntry->bidData;
		pNodeData->nid = (NID)pnbtEntry->nid;
		pNodeData->ulFlags = aBrowseBBT | fChildrenLoaded | fInfoNode;
		ptctrl->SetItemData(hItemChild, (DWORD_PTR)pNodeData);

		wsprintf(nodeTitle, L"Subnodes BID: 0x%I64X", pnbtEntry->bidSub);
		hItemChild = ptctrl->InsertItem(nodeTitle, hItem);
		pNodeData = new NodeData;
		pNodeData->bref.bid = pnbtEntry->bidSub;
		pNodeData->nid = (NID)pnbtEntry->nid;
		pNodeData->ulFlags = aBrowseBBT | fChildrenLoaded | fInfoNode;
		ptctrl->SetItemData(hItemChild, (DWORD_PTR)pNodeData);

		if(pnbtEntry->nidParent)
		{
			fGreatSuccess = s_nidToString.Lookup((NID)pnbtEntry->nidParent, pNidName);
			wsprintf(nodeTitle, L"Parent NID: 0x%X (%s)", pnbtEntry->nidParent, fGreatSuccess ? pNidName : TYPESTRINGFROMNID(pnbtEntry->nidParent));
		}
		else
		{
			wsprintf(nodeTitle, L"Parent NID: 0x0");
		}
		hItemChild = ptctrl->InsertItem(nodeTitle, hItem);
		pNodeData = new NodeData;
		pNodeData->nid = pnbtEntry->nidParent;
		pNodeData->ulFlags = aBrowseNBT | fChildrenLoaded | fInfoNode;
		ptctrl->SetItemData(hItemChild, (DWORD_PTR)pNodeData);
	}
}

void NDBViewer::AddBBTLeafNodes(CTreeCtrl* ptctrl, HTREEITEM parent, const BTPAGE& btpage)
{
	WCHAR nodeTitle[255];
	HTREEITEM hItem = NULL;
	HTREEITEM hItemChild = NULL;
	BBTENTRY *pbbtEntry = (BBTENTRY*)(btpage.rgbte);
	NodeData * pNodeData = NULL;

	for(UINT i = 0; i < BBTEnt(btpage); i++, pbbtEntry++)
	{
		// add this entry
		wsprintf(nodeTitle, L"BID: 0x%I64X, IB: %I64u", pbbtEntry->bref.bid, pbbtEntry->bref.ib);
		hItem = ptctrl->InsertItem(nodeTitle, parent);
		pNodeData = new NodeData;
		pNodeData->bref = pbbtEntry->bref;
		pNodeData->nid = 0;
		pNodeData->cb = pbbtEntry->cb;
		pNodeData->btkey = pbbtEntry->bref.bid;
		pNodeData->ulFlags = aOpenBlock | fChildrenLoaded;
		ptctrl->SetItemImage(hItem, BIDIsInternal(pbbtEntry->bref.bid) ? iconInternalBlock : iconBlock, BIDIsInternal(pbbtEntry->bref.bid) ? iconInternalBlock : iconBlock);
		ptctrl->SetItemData(hItem, (DWORD_PTR)pNodeData);

		// add information about this entry
		wsprintf(nodeTitle, L"Size: %u (%u)", pbbtEntry->cb, CbAlignDisk(pbbtEntry->cb));
		hItemChild = ptctrl->InsertItem(nodeTitle, hItem);
		pNodeData = new NodeData;
		pNodeData->bref = pbbtEntry->bref;
		pNodeData->nid = 0;
		pNodeData->cb = pbbtEntry->cb;
		pNodeData->ulFlags = aOpenBlock | fChildrenLoaded | fInfoNode;
		ptctrl->SetItemData(hItemChild, (DWORD_PTR)pNodeData);

		wsprintf(nodeTitle, L"Ref Count: %u", pbbtEntry->cRef);
		hItemChild = ptctrl->InsertItem(nodeTitle, hItem);
		pNodeData = new NodeData;
		pNodeData->bref = pbbtEntry->bref;
		pNodeData->nid = 0;
		pNodeData->cb = pbbtEntry->cb;
		pNodeData->ulFlags = aBrowseRefs | fChildrenLoaded | fInfoNode;
		ptctrl->SetItemData(hItemChild, (DWORD_PTR)pNodeData);
	}
}

void NDBViewer::ExpandSBlockNode(CTreeCtrl* ptctrl, HTREEITEM hItem)
{
	AddSBlockChildren(ptctrl, hItem);
}

void NDBViewer::DeleteNode(CTreeCtrl* ptctrl, HTREEITEM hItem)
{
	delete (NodeData *)ptctrl->GetItemData(hItem);
}

bool NDBViewer::LookupBID(BID bid, BREF& bref, CB& cb, UINT& cRef)
{
	BTPAGE btpage;
	ReadPage((BYTE*)&btpage, sizeof(btpage), NULL, m_header.root.brefBBT.ib);
	return LookupBIDImpl(&btpage, BIDStrip(bid), bref, cb, cRef);
}

bool NDBViewer::LookupNID(NID nid, BID& data, BID& sub)
{
	BTPAGE btpage;
	ReadPage((BYTE*)&btpage, sizeof(btpage), NULL, m_header.root.brefNBT.ib);
	return LookupNIDImpl(&btpage, nid, data, sub);
}

CB NDBViewer::XBGetSize(BID bid)
{
	CB cb = 0;
	UINT cRef = 0;
	BREF bref;

	if(LookupBID(bid, bref, cb, cRef))
	{
		// If this is an XBlock, read the header
		if(!BIDIsExternal(bid))
		{
			XBLOCK xb;
			ReadData((BYTE*)&xb, sizeof(XBLOCK), NULL, 0, bref.ib, sizeof(XBLOCK));
			cb = xb.lcbTotal;
		}
	}
	return cb;
}

bool NDBViewer::LookupSubnodeNID(NID nidParent, NID nidSubnode, BID& data, BID& sub)
{
	BID parentData = 0;
	BID parentSub = 0;
	BREF bref;
	CB cb;
	UINT cRef;
	BYTE * pBuffer = 0;
	BLOCKTRAILER bt;
	bool fSuccess = false;
	SBLOCK * pSBlock = 0;

	if(nidParent == 0)
		return LookupNID(nidSubnode, data, sub);

	if(!LookupNID(nidParent, parentData, parentSub))
		return false;

	if(!LookupBID(parentSub, bref, cb, cRef))
		return false;

	if(FValidBlock(bref.ib, cb, bref.bid))
	{
		pBuffer = new BYTE[BBufferSize((UINT)cb)];
		pSBlock = (SBLOCK*)pBuffer;

		ReadBlock(pBuffer, (UINT)BBufferSize((UINT)cb), &bt, bref.ib, CbAlignDisk(cb));
		fSuccess = LookupSubnodeNIDImpl(pSBlock, cb, nidSubnode, data, sub);

		delete [] pBuffer;
	}
	else
	{
		fSuccess = false;
	}

	return fSuccess;
}

bool NDBViewer::LookupSubnodeNIDImpl(SBLOCK * pSBlock, CB cbBlocksize, NID nidSubnode, BID& data, BID& sub)
{
	if(pSBlock->cLevel > 0)
	{
		// recursively call down to the next block
		SIENTRY * siEntry = pSBlock->rgsi;
		SIENTRY * siPrevious = NULL;
		for(UINT i = 0; i < SBIEnt(*pSBlock, BBufferSize((UINT)cbBlocksize)); i++, siEntry++)
		{
			if(siEntry->nid > nidSubnode)
				break;
			siPrevious = siEntry;
		}

		if(siPrevious)
		{
			BREF bref;
			CB cb;
			UINT cRef;
			BYTE * pBuffer = NULL;
			SBLOCK * pSBlockSub = NULL;

			if(!LookupBID(siPrevious->bid, bref, cb, cRef))
				return false;

			if(FValidBlock(bref.ib, cb, bref.bid))
			{
				pBuffer = new BYTE[BBufferSize((UINT)cb)];
				pSBlockSub = (SBLOCK*)pBuffer;
				BLOCKTRAILER bt;

				ReadBlock(pBuffer, (UINT)BBufferSize((UINT)cb), &bt, bref.ib, CbAlignDisk(cb));
				return LookupSubnodeNIDImpl(pSBlockSub, cb, nidSubnode, data, sub);
			}
			else
			{
				return false;
			}

		}
	}
	else
	{
		// search this block for the subnode
		SLENTRY * slEntry = pSBlock->rgsl;
		for(UINT i = 0; i < SBLEnt(*pSBlock, BBufferSize((UINT)cbBlocksize)); i++, slEntry++)
		{
			if(slEntry->nid == nidSubnode)
			{
				data = slEntry->bidData;
				sub = slEntry->bidSub;
				return true;
			}

			if(slEntry->nid > nidSubnode)
				return false;
		}
	}

	return false;
}

// To guard against infinite recursion, we only recurse on valid pages
bool NDBViewer::LookupBIDImpl(BTPAGE * pPage, BID bid, BREF& bref, CB& cb, UINT& cRefs)
{
	if(pPage->cLevel > 0)
	{
		// recursvely call down to the next page
		BTENTRY *pbtEntry = (BTENTRY*)pPage->rgbte;
		BTENTRY *pbtEntryPrev = NULL;
		for(UINT i = 0; i < BTEnt(*pPage); i++, pbtEntry++)
		{
			if(pbtEntry->btkey > bid)
				break;
			pbtEntryPrev = pbtEntry;
		}

		if(pbtEntryPrev)
		{
			BTPAGE btPage;
			ReadPage((BYTE*)&btPage, sizeof(btPage), NULL, pbtEntryPrev->bref.ib);

			if(FValidPage(pbtEntryPrev->bref.ib, ptypeBBT))
				return LookupBIDImpl(&btPage, bid, bref, cb, cRefs);
		}
	} 
	else
	{
		BBTENTRY *pbbtEntry = (BBTENTRY*)(pPage->rgbte);
		for(UINT i = 0; i < BBTEnt(*pPage); i++, pbbtEntry++)
		{
			if(pbbtEntry->bref.bid == bid)
			{
				bref.bid = pbbtEntry->bref.bid;
				bref.ib = pbbtEntry->bref.ib;
				cb = pbbtEntry->cb;
				cRefs = pbbtEntry->cRef;
				return true;
			}

			if(pbbtEntry->bref.bid > bid)
				return false;
		}
	}

	return false;
}

// To guard against infinite recursion, we only recurse on valid pages
bool NDBViewer::LookupNIDImpl(BTPAGE * pPage, NID nid, BID& data, BID& sub)
{
	if(pPage->cLevel > 0)
	{
		// recursvely call down to the next page
		BTENTRY *pbtEntry = (BTENTRY*)pPage->rgbte;
		BTENTRY *pbtEntryPrev = NULL;
		for(UINT i = 0; i < BTEnt(*pPage); i++, pbtEntry++)
		{
			if(pbtEntry->btkey > nid)
				break;
			pbtEntryPrev = pbtEntry;
		}

		if(pbtEntryPrev)
		{
			BTPAGE btPage;
			ReadPage((BYTE*)&btPage, sizeof(btPage), NULL, pbtEntryPrev->bref.ib);

			if(FValidPage(pbtEntryPrev->bref.ib, ptypeNBT))
				return LookupNIDImpl(&btPage, nid, data, sub);
		}
	} 
	else
	{
		NBTENTRY *pnbtEntry = (NBTENTRY*)(pPage->rgbte);
		for(UINT i = 0; i < NBTEnt(*pPage); i++, pnbtEntry++)
		{
			if(pnbtEntry->nid == nid)
			{
				data = pnbtEntry->bidData;
				sub = pnbtEntry->bidSub;
				return true;
			}

			if(pnbtEntry->nid > nid)
				return false;
		}
	}

	return false;
}

// The level check here guards against an infinite loop
BOOL NDBViewer::SelectNID(CTreeCtrl* ptctrl, NID nid)
{
	HTREEITEM hItem = ptctrl->GetRootItem();
	HTREEITEM hNextItem = NULL;
	NodeData * pNodeData = (NodeData *)ptctrl->GetItemData(hItem);
	NodeData * pNextNodeData = NULL;
	PAGETRAILER pt;
	BTPAGE page;
	UINT cLevels = 0;

	if(nid == 0) return true;

	// Read the root nbt page (just to find out how many levels)
	ReadPage((BYTE*)&page, sizeof(page), &pt, m_header.root.brefNBT.ib);
	cLevels = page.cLevel;

	while(true)
	{
		// First off, make sure this node has all child information
		if(!(pNodeData->ulFlags & fChildrenLoaded))
			AddBTNodeChildren(ptctrl, hItem);

		// Find the first "real" child node that isn't an info node
		hItem = ptctrl->GetChildItem(hItem);
		pNodeData = (NodeData *)ptctrl->GetItemData(hItem);
		while(pNodeData->ulFlags & fInfoNode)
		{
			hItem = ptctrl->GetNextSiblingItem(hItem);
			pNodeData = (NodeData *)ptctrl->GetItemData(hItem);
		}

		if(cLevels > 0)
		{
			// Loop over child nodes, find the one with the correct key
			while(1)
			{
				hNextItem = ptctrl->GetNextSiblingItem(hItem);
				if(hNextItem)
				{
					pNextNodeData = (NodeData *)ptctrl->GetItemData(hNextItem);
					ASSERT(nid >= pNodeData->btkey);

					// are we pointing at the correct page?
					if(nid < pNextNodeData->btkey)
						break;
					
					hItem = hNextItem;
					pNodeData = pNextNodeData;
				} 
				else
				{
					break;
				}
			}
			// We should be pointing at the correct page now.
			--cLevels;
		}
		else
		{
			while(hItem && pNodeData->btkey < nid)
			{
				hItem = ptctrl->GetNextSiblingItem(hItem);
				if(hItem)
					pNodeData = (NodeData *)ptctrl->GetItemData(hItem);
			}
			break;
		}
	}
	ptctrl->SelectItem(hItem);
	ptctrl->Expand(hItem, TVE_EXPAND);
	return (pNodeData->btkey == nid);
}

// The level check here guards against an infinite loop
BOOL NDBViewer::SelectBID(CTreeCtrl* ptctrl, BID bid)
{
	HTREEITEM hItem = ptctrl->GetRootItem();
	HTREEITEM hNextItem = NULL;
	NodeData * pNodeData = (NodeData *)ptctrl->GetItemData(hItem);
	NodeData * pNextNodeData = NULL;
	PAGETRAILER pt;
	BTPAGE page;
	UINT cLevels = 0;

	if(bid == 0) return true;
	bid = BIDStrip(bid);

	// Read the root bbt page (just to find out how many levels)
	ReadPage((BYTE*)&page, sizeof(page), &pt, m_header.root.brefBBT.ib);
	cLevels = page.cLevel;

	while(true)
	{
		// First off, make sure this node has all child information
		if(!(pNodeData->ulFlags & fChildrenLoaded))
			AddBTNodeChildren(ptctrl, hItem);

		// Find the first "real" child node that isn't an info node
		hItem = ptctrl->GetChildItem(hItem);
		pNodeData = (NodeData *)ptctrl->GetItemData(hItem);
		while(pNodeData->ulFlags & fInfoNode)
		{
			hItem = ptctrl->GetNextSiblingItem(hItem);
			pNodeData = (NodeData *)ptctrl->GetItemData(hItem);
		}

		if(cLevels > 0)
		{
			// Loop over child nodes, find the one with the correct key
			while(1)
			{
				hNextItem = ptctrl->GetNextSiblingItem(hItem);
				if(hNextItem)
				{
					pNextNodeData = (NodeData *)ptctrl->GetItemData(hNextItem);
					ASSERT(bid >= pNodeData->btkey);

					// are we pointing at the correct page?
					if(bid < pNextNodeData->btkey)
						break;
					
					hItem = hNextItem;
					pNodeData = pNextNodeData;
				} 
				else
				{
					break;
				}
			}
			// We should be pointing at the correct page now.
			--cLevels;
		}
		else
		{
			while(hItem && pNodeData->btkey < bid)
			{
				hItem = ptctrl->GetNextSiblingItem(hItem);
				if(hItem)
					pNodeData = (NodeData *)ptctrl->GetItemData(hItem);
			}
			break;
		}
	}
	ptctrl->SelectItem(hItem);
	ptctrl->Expand(hItem, TVE_EXPAND);
	return (pNodeData->btkey == bid);
}

void NDBViewer::ReadPage(BYTE* pPageData, UINT cbData, PAGETRAILER* pPageTrailer, IB ib)
{
	ASSERT(cbData <= cbPage);
	ReadData(pPageData, cbData, (BYTE*)pPageTrailer, sizeof(PAGETRAILER), ib, cbPage);
}

void NDBViewer::ReadBlock(BYTE* pBlockData, UINT cbData, BLOCKTRAILER* pBlockTrailer, IB ib, UINT cbBlock)
{
#ifdef SMALL_PST
	ASSERT(cbData <= (4*1024));
#else
	ASSERT(cbData <= (8*1024));
#endif

	ReadData(pBlockData, cbData, (BYTE*)pBlockTrailer, sizeof(BLOCKTRAILER), ib, cbBlock);
}

void NDBViewer::ReadData(BYTE* pBuffAll, UINT cbBuffAll, BYTE* pBuffTrailer, UINT cbBuffTrailer, IB ib, UINT cb)
{
	CACHEINFO * pData = NULL;

	// check the cache first
	if(m_fileCache.Lookup(ib, pData))
	{
		// is it large enough to fill the request?
		if(pData->cb < cb)
		{
			// recache with the new size
			delete [] pData->pData;
			pData->cb = cb;
			pData->pData = new BYTE[cb];

			// read in the data
			m_ndb.Seek(ib, CFile::begin);
			m_ndb.Read(pData->pData, cb);
		}

		// great success! bump this ib in the age list
		POSITION p = m_fileCacheAgeList.Find(ib);
		ASSERT(p != NULL); // finding it in the page cache implies it's in the age list

		m_fileCacheAgeList.RemoveAt(p);
		m_fileCacheAgeList.AddTail(ib);
	}
	else
	{
		// Not cached. Read it and cache it.
		if(m_fileCacheAgeList.GetSize() == c_pcMaxSize)
		{
			// Find the oldest element
			IB oldestIB = m_fileCacheAgeList.RemoveHead();
			
			// Remove from the page cache
			if(m_fileCache.Lookup(oldestIB, pData))
			{
				delete [] pData->pData;
				delete pData;
			}
			else
				ASSERT(FALSE);

			m_fileCache.RemoveKey(oldestIB);

			ASSERT(m_fileCacheAgeList.GetSize() == c_pcMaxSize-1);
			ASSERT(m_fileCache.GetCount() == c_pcMaxSize-1);
		}
		pData = new CACHEINFO;
		pData->pData = new BYTE[cb];
		pData->cb = cb;

		// read in the data
		m_ndb.Seek(ib, CFile::begin);
		m_ndb.Read(pData->pData, cb);

		// add it to the data cache
		m_fileCache[ib] = pData;
		m_fileCacheAgeList.AddTail(ib);
	}
	if(pBuffAll)
		memcpy(pBuffAll, pData->pData, cbBuffAll);
	if(pBuffTrailer && ((cb - cbBuffTrailer) > 0))
		memcpy(pBuffTrailer, (pData->pData + (cb - cbBuffTrailer)), cbBuffTrailer);
}

void InitNidMap()
{
	if(!fMapInit)
	{
		s_nidToString.InitHashTable(57);

		s_nidToString[NID_MESSAGE_STORE] = L"NID_MESSAGE_STORE";
		s_nidToString[NID_NAME_TO_ID_MAP] = L"NID_NAME_TO_ID_MAP";
		s_nidToString[NID_NORMAL_FOLDER_TEMPLATE] = L"NID_NORMAL_FOLDER_TEMPLATE";
		s_nidToString[NID_SEARCH_FOLDER_TEMPLATE] = L"NID_SEARCH_FOLDER_TEMPLATE";
		s_nidToString[NID_ROOT_FOLDER] = L"NID_ROOT_FOLDER";
		s_nidToString[NID_SEARCH_MANAGEMENT_QUEUE] = L"NID_SEARCH_MANAGEMENT_QUEUE";
		s_nidToString[NID_SEARCH_ACTIVITY_LIST] = L"NID_SEARCH_ACTIVITY_LIST";
		s_nidToString[NID_SEARCH_DOMAIN_ALTERNATE] = L"NID_SEARCH_DOMAIN_ALTERNATE";
		s_nidToString[NID_SEARCH_DOMAIN_OBJECT] = L"NID_SEARCH_DOMAIN_OBJECT";
		s_nidToString[NID_SEARCH_GATHERER_QUEUE] = L"NID_SEARCH_GATHERER_QUEUE";
		s_nidToString[NID_SEARCH_GATHERER_DESCRIPTOR] = L"NID_SEARCH_GATHERER_DESCRIPTOR";
		s_nidToString[NID_TABLE_REBUILD_QUEUE] = L"NID_TABLE_REBUILD_QUEUE";
		s_nidToString[NID_JUNK_MAIL_PIHSL] = L"NID_JUNK_MAIL_PIHSL";
		s_nidToString[NID_SEARCH_GATHERER_FOLDER_QUEUE] = L"NID_SEARCH_GATHERER_FOLDER_QUEUE";
		s_nidToString[NID_TC_SUB_PROPS] = L"NID_TC_SUB_PROPS";
		s_nidToString[NID_HIERARCHY_TABLE_TEMPLATE] = L"NID_HIERARCHY_TABLE_TEMPLATE";
		s_nidToString[NID_CONTENTS_TABLE_TEMPLATE] = L"NID_CONTENTS_TABLE_TEMPLATE";
		s_nidToString[NID_ASSOC_CONTENTS_TABLE_TEMPLATE] = L"NID_ASSOC_CONTENTS_TABLE_TEMPLATE";
		s_nidToString[NID_SEARCH_CONTENTS_TABLE_TEMPLATE] = L"NID_SEARCH_CONTENTS_TABLE_TEMPLATE";
		s_nidToString[NID_SMP_TEMPLATE] = L"NID_SMP_TEMPLATE";
		s_nidToString[NID_TOMBSTONE_TABLE_TEMPLATE] = L"NID_TOMBSTONE_TABLE_TEMPLATE";
		s_nidToString[NID_LREP_DUPS_TABLE_TEMPLATE] = L"NID_LREP_DUPS_TABLE_TEMPLATE";
		s_nidToString[NID_RECEIVE_FOLDERS] = L"NID_RECEIVE_FOLDERS";
		s_nidToString[NID_OUTGOING_QUEUE] = L"NID_OUTGOING_QUEUE";
		s_nidToString[NID_ATTACHMENT_TABLE] = L"NID_ATTACHMENT_TABLE";
		s_nidToString[NID_RECIPIENT_TABLE] = L"NID_RECIPIENT_TABLE";
		s_nidToString[NID_CHANGE_HISTORY_TABLE] = L"NID_CHANGE_HISTORY_TABLE";
		s_nidToString[NID_TOMBSTONE_TABLE] = L"NID_TOMBSTONE_TABLE";
		s_nidToString[NID_TOMBSTONE_DATE_TABLE] = L"NID_TOMBSTONE_DATE_TABLE";
		s_nidToString[NID_ALL_MESSAGE_SEARCH_FOLDER_OLD] = L"NID_ALL_MESSAGE_SEARCH_FOLDER_OLD";
		s_nidToString[NID_ALL_MESSAGE_SEARCH_FOLDER] = L"NID_ALL_MESSAGE_SEARCH_FOLDER";
		s_nidToString[NID_LREP_GMP] = L"NID_LREP_GMP";
		s_nidToString[NID_LREP_FOLDERS_SMP] = L"NID_LREP_FOLDERS_SMP";
		s_nidToString[NID_LREP_FOLDERS_TABLE] = L"NID_LREP_FOLDERS_TABLE";
		s_nidToString[NID_FOLDER_PATH_TOMBSTONE_TABLE] = L"NID_FOLDER_PATH_TOMBSTONE_TABLE";
		s_nidToString[NID_HST_HMP] = L"NID_HST_HMP";
		s_nidToString[NID_CRITERR_NOTIFICATION] = L"NID_CRITERR_NOTIFICATION";
		s_nidToString[NID_OBJECT_NOTIFICATION] = L"NID_OBJECT_NOTIFICATION";
		s_nidToString[NID_NEWMAIL_NOTIFICATION] = L"NID_NEWMAIL_NOTIFICATION";
		s_nidToString[NID_EXTENDED_NOTIFICATION] = L"NID_EXTENDED_NOTIFICATION";
		s_nidToString[NID_INDEXING_NOTIFICATION] = L"NID_INDEXING_NOTIFICATION";
		s_nidToString[NID_PRV_ROOT_FOLDER] = L"NID_PRV_ROOT_FOLDER";
		s_nidToString[NID_PUB_ROOT_FOLDER] = L"NID_PUB_ROOT_FOLDER";
		s_nidToString[NID_ALL_MESSAGE_SEARCH_CONTENTS] = L"NID_ALL_MESSAGE_SEARCH_CONTENTS";
		fMapInit = true;
	}
}

void NDBViewer::ForEachBTPage(PTYPE ptype, bool (*pfn)(BTPAGE*,PAGETRAILER*,IB,void*), int feType, void* pv)
{
	if(ptype == ptypeNBT)
	{
		ForEachBTPageImpl(m_header.root.brefNBT.ib, pfn, feType, pv);
	}
	else if(ptype == ptypeBBT)
	{
		ForEachBTPageImpl(m_header.root.brefBBT.ib, pfn, feType, pv);
	}
}

// To guard against infinite recursion, we only recurse on valid pages.
void NDBViewer::ForEachBTPageImpl(IB ib, bool (*pfn)(BTPAGE*,PAGETRAILER*,IB,void*), int feType, void* pv)
{
	bool fValid; 
	bool fContinue = true;
	BTPAGE btPage;
	PAGETRAILER pt;

	fValid = FValidPage(ib);
	if(!(fValid || (feType & feInvalid)))
		return;

	ReadPage((BYTE*)&btPage, sizeof(btPage), &pt, ib);

	if(btPage.cLevel == 0)
	{
		// Shouldn't be here if we don't want to look at leaf pages
		ASSERT((feType & feLeaf));

		// Don't care about the result, we're done anyway
		pfn(&btPage, &pt, ib, pv);
	}
	else
	{
		if(feType & feNonLeaf)
			fContinue = pfn(&btPage, &pt, ib, pv);

		if(fContinue && fValid)
		{
			if(btPage.cLevel > 1 || (feType & feLeaf))
			{
				// recurse to children
				BTENTRY *pbte = (BTENTRY*)(btPage.rgbte);
				for(int i = 0; i < (btPage.cEnt < cBTEntMax ? btPage.cEnt : (int)cBTEntMax); i++, pbte++)
				{
					ForEachBTPageImpl(pbte->bref.ib, pfn, feType, pv);
				}
			}
		}
	}
}

void NDBViewer::DecodeBlockInPlace(BYTE *pBlockData, UINT cbData, BID bid)
{
	return CryptData(&m_header, pBlockData, cbData, bid, FALSE);
}

bool NDBViewer::FValidPage(IB ib, PTYPE ptype)
{
	bool fValid = false;
	bool fValidCRC = false;
	bool fValidSig = false;
	bool fValidType = false;
	BYTE * pBuffer = NULL;
	PAGETRAILER pt;

	// if it's past EOF, it isn't valid
	if(ib+cbPage > GetFileEOF()) return false;

	// read the page and validate
	pBuffer = new BYTE[cbPage];

	ReadPage(pBuffer, cbPage, &pt, ib);

	fValidCRC = (ComputeCRC(pBuffer, cbPage - sizeof(PAGETRAILER)) == pt.dwCRC);
	fValidSig = (ComputeSig(ib, pt.bid) == pt.wSig);
	fValidType = (pt.ptype == pt.ptypeRepeat && (ptype == 0 ? true : ptype == pt.ptype));

	fValid = (fValidCRC && fValidSig && fValidType);

	delete [] pBuffer;

	return fValid;
}

bool NDBViewer::FFree(IB ib)
{
	IB ibOffset = (ib - ibAMapBase) % cbPerAMap;
	IB ibAMap = ib - ibOffset;
	UINT uiBit = CbToCs(ibOffset);
	BYTE amapPage[cbAMapPage];

	if(		(ib < GetHeader().root.ibFileEof)
		&&	(ib > ibAMapBase)
		&&	(GetHeader().root.fAMapValid)
		)
	{
		ReadPage(amapPage, cbAMapPage, NULL, ibAMap);
		return !BMapTestBit(amapPage, uiBit);
	}
	
	return false;
}

bool NDBViewer::FValidBlock(IB ib, CB cbUnaligned, BID bid)
{
	CB cbAligned = CbAlignDisk(cbUnaligned);
	BYTE * pBuffer = NULL;
	bool fValid = false;
	bool fValidCRC = false;
	bool fValidSig = false;
	bool fValidSize = false;
	BLOCKTRAILER bt;

	// if it's greater than the max block size, it can't possibly be valid
#ifdef SMALL_PST
	if(CbAlignDiskNDB(cbUnaligned) > (4*1024)) return false;
#else
	if(CbAlignDiskNDB(cbUnaligned) > (8*1024)) return false;
#endif

	// if it's past EOF, it isn't valid
	if(ib+cbAligned > GetFileEOF()) return false;

	// Read the block and validate
	pBuffer = new BYTE[(UINT)cbAligned];

	ReadBlock(pBuffer, (UINT)cbAligned, &bt, ib, (UINT)cbAligned);
	fValidCRC = (ComputeCRC(pBuffer, (UINT)cbUnaligned) == bt.dwCRC);
	fValidSig = ((ComputeSig(ib, bt.bid) == bt.wSig) && (bid == 0 ? true : bid == bt.bid)) ;
	fValidSize = (cbUnaligned == bt.cb);

	fValid =  (fValidCRC && fValidSig && fValidSize);

	delete [] pBuffer;
	return fValid;
}
CB NDBViewer::NODE_GetSize(NID nidParent, NID nid)
{
	BID data;
	BID sub;

	if(LookupSubnodeNID(nidParent, nid, data, sub))
	{
		return XBGetSize(data);
	}
	return 0;
}

CB NDBViewer::NODE_ReadData(NID nidParent, NID nid, BYTE* buffAll, UINT cbBuffAll, IB ib, UINT cb)
{
	BID bidData = 0;
	BID bidSub = 0;

	// Top level node?
	if(nidParent == 0)
	{
		if(!LookupNID(nid, bidData, bidSub))
			return 0;
	}
	// Subnode...
	else
	{
		if(!LookupSubnodeNID(nidParent, nid, bidData, bidSub))
			return 0;
	}

	// Ok. The data we want is ib bytes into bidData.
	return XBlockRead(bidData, buffAll, cbBuffAll, ib, cb);
}

CB NDBViewer::XBlockRead(BID bid, BYTE* buffAll, UINT cbBuffAll, IB ib, UINT cb)
{
	BREF bref;
	CB cbBlock;
	UINT cRef;
	CB cbTotal = 0;

	if(bid == 0)
	{
		ASSERT(cb <= cbNBDataMax);
		// unrealized block of an XBlock.. zero fill the memory
		ZeroMemory(buffAll, cb);
		return cb;
	}

	// we have real data to look up
	if(!LookupBID(bid, bref, cbBlock, cRef))
		return 0;

	if(FValidBlock(bref.ib, cbBlock, bref.bid))
	{
		BYTE *pByte = new BYTE[BBufferSize((UINT)cbBlock)];
		BLOCKTRAILER bt;

		ReadBlock(pByte, BBufferSize((UINT)cbBlock), &bt, bref.ib, CbAlignDisk(cbBlock));

		// what type of block are we?
		if(BIDIsExternal(bid))
		{
			if(ib > cbBlock) return 0;
			if(ib+cb > cbBlock)
				cb = (UINT)cbBlock - (UINT)ib;
			DecodeBlockInPlace(pByte, BBufferSize((UINT)cbBlock), bid);
			memcpy(buffAll, pByte+ib, cb);
			cbTotal = cb;
		}
		else
		{
			// XX/XBlock!
			XBLOCK * pBlock = (XBLOCK*)pByte;
			ASSERT(pBlock->btype == btypeXB);

			if(pBlock->cLevel == 2)
			{
				//XXBlock

				// what XBlock do we start on?
				UINT i = (UINT)ib / lcbXBDataMax;
				
				// start IB for any given XBlock in this XXBlock
				IB ibXBlockStart = i * lcbXBDataMax;

				BYTE * pBufferBase = buffAll;
				CB cbRemainingToRead = cb;

				for(; i < XBEnt(*pBlock, (UINT)cbBlock); i++)
				{
					IB ibBase = ib - ibXBlockStart;
					CB cbReadBase = ibBase + cbRemainingToRead > lcbXBDataMax ? lcbXBDataMax - ibBase : cbRemainingToRead;

					// call into XBlock to read this data
					cbTotal += XBlockRead(pBlock->rgbid[i], pBufferBase, (UINT)cbReadBase, ibBase, (UINT)cbReadBase);

					// adjust values for the next XBlock
					pBufferBase += cbReadBase; // pointer into our destination buffer
					cbRemainingToRead -= cbReadBase; // bytes left to read
					ib += cbReadBase; // address to start reading
					ibXBlockStart += lcbXBDataMax; // address of the next xblock in the node

					if(cbRemainingToRead == 0) break;
				}
			}
			else
			{
				// XBlock
				ASSERT(pBlock->cLevel == 1);

				// what Block do we start on?
				UINT i = (UINT)ib / cbNBDataMax;
				
				// start IB for any given Block in this XBlock
				IB ibBlockStart = i * cbNBDataMax;

				BYTE * pBufferBase = buffAll;
				CB cbRemainingToRead = cb;

				for(; i < XBEnt(*pBlock, (UINT)cbBlock); i++)
				{
					IB ibBase = ib - ibBlockStart;
					CB cbReadBase = ibBase + cbRemainingToRead > cbNBDataMax ? cbNBDataMax - ibBase : cbRemainingToRead;

					// call into the Block to read this data
					cbTotal += XBlockRead(pBlock->rgbid[i], pBufferBase, (UINT)cbReadBase, ibBase, (UINT)cbReadBase);

					// adjust values for the next block
					pBufferBase += cbReadBase; // pointer into our destination buffer
					cbRemainingToRead -= cbReadBase; // bytes left to read
					ib += cbReadBase; // address to start reading
					ibBlockStart += cbNBDataMax; // address of the next block in the node

					if(cbRemainingToRead == 0) break;
				}
			}
		}

		delete [] pByte;
	}

	return cbTotal;
}

UINT SBIEnt(const SBLOCK& block, UINT dataSize)
{
	if(dataSize > cbNBDataMax)
		dataSize = cbNBDataMax;

	UINT maxSize = UINT((dataSize - offsetof(SBLOCK, rgsl)) / sizeof(SIENTRY));
	return (block.cEnt > maxSize ? maxSize : block.cEnt);
}

UINT SBLEnt(const SBLOCK& block, UINT dataSize)
{
	if(dataSize > cbNBDataMax)
		dataSize = cbNBDataMax;

	UINT maxSize = UINT((dataSize - offsetof(SBLOCK, rgsl)) / sizeof(SLENTRY));
	return (block.cEnt > maxSize ? maxSize : block.cEnt);
}

UINT XBEnt(const XBLOCK& block, UINT dataSize)
{
	if(dataSize > cbNBDataMax)
		dataSize = cbNBDataMax;

	UINT maxSize = UINT((dataSize - offsetof(XBLOCK, rgbid)) / sizeof(BID));
	return (block.cEnt > maxSize ? maxSize : block.cEnt);
}

// ?BTEnt is a little simpler, as the dataSize is constant
UINT BTEnt(const BTPAGE& page)
{
	return (page.cEnt > cBTEntMax ? cBTEntMax : page.cEnt);
}

UINT BBTEnt(const BTPAGE& page)
{
	ASSERT(page.cLevel == 0);
	return (page.cEnt > cBBTEntMax ? cBBTEntMax : page.cEnt);
}

UINT NBTEnt(const BTPAGE& page)
{
	ASSERT(page.cLevel == 0);
	return (page.cEnt > cNBTEntMax ? cNBTEntMax : page.cEnt);
}

UINT BBufferSize(UINT blockUnaligned)
{
	return (blockUnaligned > cbNBDataMax ? cbNBDataMax : blockUnaligned);
}

ULONGLONG ULLSize(ULONGLONG ull)
{
	if(ull < 20000)
		return ull;
	if(ull < 20000000)
		return ull/1024;
	return ull/1048576;
}

const WCHAR* WSZSize(ULONGLONG ull)
{
	if(ull < 20000)
		return L"bytes";
	if(ull < 20000000)
		return L"KB";
	return L"MB";
}