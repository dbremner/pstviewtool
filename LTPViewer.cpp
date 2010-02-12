#include "StdAfx.h"
#include "LTPViewer.h"
#include "ltpport.h"
#include "ptags.h"
#include "mapidefs.h"
#include "strsafe.h"

const WCHAR* GetPropTagString(ULONG ul)
{
	for(UINT i = 0; i < cptAll; ++i)
	{
		if(ul == rgptAll[i].ulptag)
			return rgptAll[i].pszptag;
	}
	return L"Unknown";
}

const WCHAR* GetPropTypeString(WORD w)
{
	if(w == PT_UNSPECIFIED) return L"PT_UNSPECIFIED";
	if(w == PT_NULL) return L"PT_NULL";
	if(w == PT_I2) return L"PT_I2";
	if(w == PT_LONG) return L"PT_LONG";
	if(w == PT_R4) return L"PT_R4";
	if(w == PT_DOUBLE) return L"PT_DOUBLE";
	if(w == PT_CURRENCY) return L"PT_CURRENCY";
	if(w == PT_APPTIME) return L"PT_APPTIME";
	if(w == PT_ERROR) return L"PT_ERROR";
	if(w == PT_BOOLEAN) return L"PT_BOOLEAN";
	if(w == PT_OBJECT) return L"PT_OBJECT";
	if(w == PT_I8) return L"PT_I8";
	if(w == PT_STRING8) return L"PT_STRING8";
	if(w == PT_UNICODE) return L"PT_UNICODE";
	if(w == PT_SYSTIME) return L"PT_SYSTIME";
	if(w == PT_CLSID) return L"PT_CLSID";
	if(w == PT_BINARY) return L"PT_BINARY";

	return L"Unknown";
}

const WCHAR* GetIndexVersionString(BYTE b)
{
	if(b == TCVHUGE_INDEX_VERSION) return L"TCVHUGE_INDEX_VERSION";
	if(b == TCV64K_INDEX_VERSION) return L"TCV64K_INDEX_VERSION";

	return L"Unknown";
}

const WCHAR* GetIndexRootVersionString(BYTE b)
{
	if(b == TCVROOT_INDEX_VERSION) return L"TCVROOT_INDEX_VERSION";
	if(b == TCVROOT_NONLOCALE_INDEX_VERSION) return L"TCVROOT_NONLOCALE_INDEX_VERSION";

	return L"Unknown";
}
const WCHAR* GetClientMagicSigString(WORD w)
{
	if(w == bMagicGMP) return L"bMagicGMP";
	if(w == bMagicTC) return L"bMagicTC";
	if(w == bMagicSMP) return L"bMagicSMP";
	if(w == bMagicHMP) return L"bMagicHMP";
	if(w == bMagicCH) return L"bMagicCH";
	if(w == bMagicCHTC) return L"bMagicCHTC";
	if(w == bMagicBTH) return L"bMagicBTH";
	if(w == bMagicPC) return L"bMagicPC";

	return L"Unknown";
}

const WCHAR* GetFillLevelString(WORD w)
{
	if(w == FILL_LEVEL_EMPTY) return L"FILL_LEVEL_EMPTY";
	if(w == FILL_LEVEL_1) return L"FILL_LEVEL_1";
	if(w == FILL_LEVEL_2) return L"FILL_LEVEL_2";
	if(w == FILL_LEVEL_3) return L"FILL_LEVEL_3";
	if(w == FILL_LEVEL_4) return L"FILL_LEVEL_4";
	if(w == FILL_LEVEL_5) return L"FILL_LEVEL_5";
	if(w == FILL_LEVEL_6) return L"FILL_LEVEL_6";
	if(w == FILL_LEVEL_7) return L"FILL_LEVEL_7";
	if(w == FILL_LEVEL_8) return L"FILL_LEVEL_8";
	if(w == FILL_LEVEL_9) return L"FILL_LEVEL_9";
	if(w == FILL_LEVEL_10) return L"FILL_LEVEL_10";
	if(w == FILL_LEVEL_11) return L"FILL_LEVEL_11";
	if(w == FILL_LEVEL_12) return L"FILL_LEVEL_12";
	if(w == FILL_LEVEL_13) return L"FILL_LEVEL_13";
	if(w == FILL_LEVEL_14) return L"FILL_LEVEL_14";
	if(w == FILL_LEVEL_FULL) return L"FILL_LEVEL_FULL";

	return L"Unknown";
}

const WCHAR* GetPageTypeString(PAGET w)
{
	if(w == PAGE_TYPE_FIRST) return L"PAGE_TYPE_FIRST";
	if(w == PAGE_TYPE_NORMAL) return L"PAGE_TYPE_NORMAL";
	if(w == PAGE_TYPE_FILL_BITMAP) return L"PAGE_TYPE_FILL_BITMAP";

	return L"Unknown";
}

LTPViewer::LTPViewer(LPCWSTR filename)
: NDBViewer(filename)
{
}

CB LTPViewer::HN_HeapSize(NID nidParent, NID nid)
{
	BID data;
	BID sub;
	CB cb;
	BREF bref;
	UINT cRef;

	if(	LookupSubnodeNID(nidParent, nid, data, sub) &&
		LookupBID(data, bref, cb, cRef)
		)
		return XBGetSize(data);
	else
		return 0;
}

UINT LTPViewer::HN_PageCount(NID nidParent, NID nid)
{
	return((WORD) ((HN_HeapSize(nidParent, nid) + cbNBDataMax - 1) / cbNBDataMax));
}

HID LTPViewer::HN_GetRootHID(NID nidParent, NID nid)
{
	HNHDR hnhdr;

	NODE_ReadData(nidParent, nid, (BYTE*)&hnhdr, sizeof(HNHDR), 0, sizeof(HNHDR));
	return hnhdr.hidUserRoot;
}

BOOL LTPViewer::HN_FValidHID(NID nidParent, NID nid, HID hid)
{
	BYTE pageBuffer[cbNBDataMax];
	HNPAGEHDR * pHdr = NULL;
	HNPAGEMAP * pMap = NULL;

	if(HIDPage(hid) >= HN_PageCount(nidParent, nid))
		return false;

	HN_GetPage(nidParent, nid, HIDPage(hid), pageBuffer, cbNBDataMax);
	pHdr = (PHNPAGEHDR)pageBuffer;
	pMap = (HNPAGEMAP*)(pageBuffer + pHdr->ibHnpm);

	if(HIDIndex(hid) >= pMap->cAlloc)
		return false;

	if((pMap->rgibAlloc[HIDIndex(hid)+1] - pMap->rgibAlloc[HIDIndex(hid)]) == 0)
		return false;

	return true;
}
void LTPViewer::BuildBTHTree(NID nidParent, NID nid, BTHHEADER * pHeader, CTreeCtrl * pTree)
{
	WCHAR buffer[255];
	HTREEITEM hItem = NULL;
	UINT icon = pHeader->bIdxLevels == 0 ? iconLeafPage : iconPage;

	// Add Root Node
	StringCchPrintf(buffer, 255, L"Root HID: 0x%X", pHeader->hidRoot);
	hItem = pTree->InsertItem(buffer, TVI_ROOT);
	pTree->SetItemData(hItem, (DWORD_PTR)pHeader->hidRoot);
	pTree->SetItemImage(hItem, icon, icon);

	// Recursively add children
	BuildBTHTreeImpl(nidParent, nid, hItem, pHeader->hidRoot, pTree, pHeader, pHeader->bIdxLevels);
}

void BinaryToString(BYTE * pData, UINT cbData, WCHAR * pBuffer, UINT cchDest)
{
	if(cbData == 2)
	{
		WORD w = *((WORD*)pData);
		StringCchPrintf(pBuffer, cchDest, L"word - 0x%X", w);
	}
	else if(cbData == 4)
	{
		DWORD dw = *((DWORD*)pData);
		StringCchPrintf(pBuffer, cchDest, L"dword - 0x%X", dw);
	}
	else if(cbData == 8)
	{
		ULONGLONG ull = *((ULONGLONG*)pData);
		StringCchPrintf(pBuffer, cchDest, L"quad - 0x%I64X", ull);
	}
	else
	{
		// some odd amount, do just a byte at a time
		StringCchPrintfEx(pBuffer, cchDest, &pBuffer, &cchDest, 0, L"binary - [");
		while(cbData > 0)
		{
			BYTE b = *pData;
			StringCchPrintfEx(pBuffer, cchDest, &pBuffer, &cchDest, 0, L"%02X ", b);
			pData++;
			cbData--;
		}
		StringCchPrintf(pBuffer, cchDest, L"]");
	}
}
void LTPViewer::BuildBTHTreeImpl(NID nidParent, NID nid, HTREEITEM hItem, HID hid, CTreeCtrl * pTree, BTHHEADER * pHeader, UINT iLevel)
{
	WCHAR buffer[255];
	int cb;
	HTREEITEM hItemChild = NULL;
	BYTE * pBuffer = NULL;
	BYTE * pCur = NULL;
	BYTE keyBuffer[16];
	BYTE entBuffer[32];
	WCHAR keyString[255];
	WCHAR entString[255];
	HID hidNext;

	ZeroMemory(keyBuffer, sizeof(keyBuffer));
	ZeroMemory(entBuffer, sizeof(entBuffer));

	if(!HN_FValidHID(nidParent, nid, hid))
	{
		pTree->SetItemImage(hItem, iconCorrupt, iconCorrupt);
		pTree->InsertItem(L"Invalid HID", hItem);
		return;
	}
	// Read this page in
	cb = HN_ReadHID(nidParent, nid, hid, &pBuffer);

	// INDEX page, each entry is sizeof(key)+sizeof(HID)
	if(iLevel > 0)
	{
		pCur = pBuffer;
		while(cb >= (pHeader->cbKey + (int)sizeof(HID)))
		{
			memcpy(keyBuffer, pCur, pHeader->cbKey);
			pCur += pHeader->cbKey;
			cb -= pHeader->cbKey;
			hidNext = *((HID*)pCur);
			pCur += sizeof(HID);
			cb -= sizeof(HID);

			// Add this entry to the tree
			UINT icon = iLevel-1 > 0 ? iconPage : iconLeafPage;
			BinaryToString(keyBuffer, pHeader->cbKey, keyString, 255);
			StringCchPrintf(buffer, 255, L"Key: %s, HID: 0x%X", keyString, hidNext);
			hItemChild = pTree->InsertItem(buffer, hItem);
			pTree->SetItemImage(hItemChild, icon, icon);
			pTree->SetItemData(hItemChild, (DWORD_PTR)hidNext);

			// Recursively call to add it's children
			BuildBTHTreeImpl(nidParent, nid, hItemChild, hidNext, pTree, pHeader, iLevel-1);
		}
	}
	// LEAF page, each entry is sizeof(key)+sizeof(entry)
	else
	{
		ASSERT((cb % (pHeader->cbKey + pHeader->cbEnt)) == 0);
		pCur = pBuffer;
		while(cb >= (pHeader->cbKey + pHeader->cbEnt))
		{
			memcpy(keyBuffer, pCur, pHeader->cbKey);
			pCur += pHeader->cbKey;
			cb -= pHeader->cbKey;

			memcpy(entBuffer, pCur, pHeader->cbEnt);
			pCur += pHeader->cbEnt;
			cb -= pHeader->cbEnt;

			BinaryToString(keyBuffer, pHeader->cbKey, keyString, 255);
			BinaryToString(entBuffer, pHeader->cbEnt, entString, 255);

			// Add this entry to the tree
			UINT icon = iconBlock;
			StringCchPrintf(buffer, 255, L"Key: %s, Value: %s", keyString, entString);
			hItemChild = pTree->InsertItem(buffer, hItem);
			pTree->SetItemImage(hItemChild, icon, icon);
			pTree->SetItemData(hItemChild, pHeader->cbEnt == sizeof(HID) ? (DWORD_PTR)(*((HID*)entBuffer)) : 0);
		}
	}
	delete [] pBuffer;
}

BOOL LTPViewer::BTH_Enum(NID nidParent, NID nid, BTHHEADER * pHeader, void* pCtx, bool (*pfn)(void*,void*,void*))
{
	return BTHEnumImpl(nidParent, nid, pHeader, pHeader->hidRoot, pHeader->bIdxLevels, pCtx, pfn);
}

BOOL LTPViewer::BTHEnumImpl(NID nidParent, NID nid, BTHHEADER * pHeader, HID hid, UINT iLevel, void* pCtx, bool (*pfn)(void*,void*,void*))
{
	int cb;
	BYTE * pBuffer = NULL;
	BYTE * pCur = NULL;
	HID hidNext;
	BYTE keyBuffer[16];
	BYTE entBuffer[32];

	if(!HN_FValidHID(nidParent, nid, hid))
	{
		return false;
	}

	// Read this page in
	cb = HN_ReadHID(nidParent, nid, hid, &pBuffer);

	// INDEX page, each entry is sizeof(key)+sizeof(HID)
	if(iLevel > 0)
	{
		pCur = pBuffer;
		while(cb >= (pHeader->cbKey + (int)sizeof(HID)))
		{
			// skip over the key for this page, we don't care
			pCur += pHeader->cbKey;
			cb -= pHeader->cbKey;

			hidNext = *((HID*)pCur);
			pCur += sizeof(HID);
			cb -= sizeof(HID);

			// Recursively call to children
			if(!BTHEnumImpl(nidParent, nid, pHeader, hidNext, iLevel-1, pCtx, pfn))
				return false;
		}
	}
	// LEAF page, each entry is sizeof(key)+sizeof(entry)
	else
	{
		ASSERT((cb % (pHeader->cbKey + pHeader->cbEnt)) == 0);
		pCur = pBuffer;
		while(cb >= (pHeader->cbKey + pHeader->cbEnt))
		{
			memcpy(keyBuffer, pCur, pHeader->cbKey);
			pCur += pHeader->cbKey;
			cb -= pHeader->cbKey;

			memcpy(entBuffer, pCur, pHeader->cbEnt);
			pCur += pHeader->cbEnt;
			cb -= pHeader->cbEnt;

			// Make the callback
			pfn(keyBuffer, entBuffer, pCtx);
		}
	}

	delete [] pBuffer;

	return true;
}

BOOL LTPViewer::BTH_Lookup(NID nidParent, NID nid, BTHHEADER * pHeader, BYTE* pKey, BYTE* pValue)
{
	return BTHLookupImpl(nidParent, nid, pHeader, pHeader->hidRoot, pHeader->bIdxLevels, pKey, pValue);
}

BOOL LTPViewer::BTHLookupImpl(NID nidParent, NID nid, BTHHEADER * pHeader, HID hid, UINT iLevel, BYTE* pKey, BYTE* pValue)
{
	int cb;
	int compValue;
	BYTE * pBuffer = NULL;
	BYTE * pCur = NULL;
	HID hidNext = 0;
	HID hidPrev = 0;
	BYTE keyBuffer[16];

	if(!HN_FValidHID(nidParent, nid, hid))
	{
		return false;
	}

	// Read this page in
	cb = HN_ReadHID(nidParent, nid, hid, &pBuffer);

	// INDEX page, each entry is sizeof(key)+sizeof(HID)
	if(iLevel > 0)
	{
		pCur = pBuffer;
		while(cb >= (pHeader->cbKey + (int)sizeof(HID)))
		{
			memcpy(keyBuffer, pCur, pHeader->cbKey);
			pCur += pHeader->cbKey;
			cb -= pHeader->cbKey;
			hidPrev = hidNext;
			hidNext = *((HID*)pCur);
			pCur += sizeof(HID);
			cb -= sizeof(HID);

			switch(pHeader->cbKey)
			{
				// compare word values
				case 2:
					compValue = ((*(WORD*)(keyBuffer) > *(WORD*)(pKey))) ? 1 : ((*(WORD*)(keyBuffer) < *(WORD*)(pKey))) ? -1 : 0;
					break;
				// compare dword values
				case 4:
					compValue = ((*(DWORD*)(keyBuffer) > *(DWORD*)(pKey))) ? 1 : ((*(DWORD*)(keyBuffer) < *(DWORD*)(pKey))) ? -1 : 0;
					break;
				// compare ULONGLONG values
				case 8:
					compValue = ((*(ULONGLONG*)(keyBuffer) > *(ULONGLONG*)(pKey))) ? 1 : ((*(ULONGLONG*)(keyBuffer) < *(ULONGLONG*)(pKey))) ? -1 : 0;
					break;
				// do a memcompare
				default:
					compValue = memcmp(keyBuffer, pKey, pHeader->cbKey);
					break;
			};
			// Recursively call to children

			if(compValue > 0)
			{
				delete [] pBuffer;
				return BTHLookupImpl(nidParent, nid, pHeader, hidPrev, iLevel-1, pKey, pValue);
			}
		}
	}
	// LEAF page, each entry is sizeof(key)+sizeof(entry)
	else
	{

		ASSERT((cb % (pHeader->cbKey + pHeader->cbEnt)) == 0);
		pCur = pBuffer;
		while(cb >= (pHeader->cbKey + pHeader->cbEnt))
		{
			memcpy(keyBuffer, pCur, pHeader->cbKey);
			pCur += pHeader->cbKey;
			cb -= pHeader->cbKey;

			if(memcmp(keyBuffer, pKey, pHeader->cbKey) == 0)
			{
				memcpy(pValue, pCur, pHeader->cbEnt);
				delete [] pBuffer;
				return true;
			}

			pCur += pHeader->cbEnt;
			cb -= pHeader->cbEnt;
		}
	}

	delete [] pBuffer;

	return false;
}

BOOL LTPViewer::PC_GetProp(NID nidParent, NID nid, LPSPropValue lpspv)
{
	BTHHEADER header;
	WORD tag = (WORD)PROP_ID(lpspv->ulPropTag);
	UINT cb;
	PROPENTRY pe;
	
	HN_ReadHID(nidParent, nid, HN_GetRootHID(nidParent, nid), (BYTE*)&header, sizeof(BTHHEADER));

	if(BTH_Lookup(nidParent, nid, &header, (BYTE*)&tag, (BYTE*)&pe))
	{
		switch(pe.wType)
		{
			case PT_I2:
			case PT_LONG:
			case PT_R4:
			case PT_DOUBLE:
			case PT_BOOLEAN:
				// the hnid value is the actual value we want
				lpspv->Value.ul = pe.hnid;
				break;
			case PT_CURRENCY:
			case PT_APPTIME:
			case PT_I8:
				// the hnid values points to what we want
				ASSERT(NIDType(pe.hnid) == 0);
				cb = HN_ReadHID(nidParent, nid, (HID)pe.hnid, (BYTE*)&(lpspv->Value.li), 8);
				ASSERT(cb == 8);
				break;
			case PT_STRING8:
			case PT_UNICODE:
				// the hnid values points to what we want
				PBYTE temp;
				UINT cbNew;

				if(NIDType(pe.hnid) == NID_TYPE_LTP)
				{
					// subnode
					cb = (ULONG)NODE_GetSize(nid, pe.hnid);
					cbNew = cb + ((pe.wType == PT_STRING8) ? sizeof(char) : sizeof(WCHAR));
					lpspv->Value.lpszW = (LPWSTR)(new BYTE[cbNew]);
					NODE_ReadData(nid, pe.hnid, (BYTE*)lpspv->Value.lpszW, cb, 0, cb);
				}
				else
				{
					// in the heap
					cb = HN_ReadHID(nidParent, nid, (HID)pe.hnid, (BYTE**)&(lpspv->Value.lpszW));

					// add space for null terminator... sigh.
					cbNew = cb + ((pe.wType == PT_STRING8) ? sizeof(char) : sizeof(WCHAR));
					temp = new BYTE[cbNew];
					if(cb)
					{
						memcpy(temp, lpspv->Value.lpszW, cb);
						delete [] lpspv->Value.lpszW;
					}
					lpspv->Value.lpszW = (LPWSTR)temp;
				}

				// add null terminator at the end
				for(UINT i = cb; i < cbNew; i++)
					((BYTE*)lpspv->Value.lpszW)[i] = 0;

				break;
			case PT_BINARY:
				// the hnid values points to what we want
				if(NIDType(pe.hnid) == NID_TYPE_LTP)
				{
					// subnode
					lpspv->Value.bin.cb = (ULONG)NODE_GetSize(nid, pe.hnid);
					lpspv->Value.bin.lpb = new BYTE[lpspv->Value.bin.cb];
					NODE_ReadData(nid, pe.hnid, lpspv->Value.bin.lpb, lpspv->Value.bin.cb, 0, lpspv->Value.bin.cb);
				}
				else
				{
					// in the heap
					cb = HN_ReadHID(nidParent, nid, (HID)pe.hnid, &(lpspv->Value.bin.lpb));
					lpspv->Value.bin.cb = cb;
				}
				break;
			default:
				lpspv->ulPropTag = PROP_TAG(PT_ERROR, PROP_ID(lpspv->ulPropTag));
		}
	}
	else
	{
		lpspv->ulPropTag = PROP_TAG(PT_ERROR, PROP_ID(lpspv->ulPropTag));
	}

	return (PROP_TYPE(lpspv->ulPropTag) == PT_ERROR);
}

CB LTPViewer::HN_GetPage(NID nidParent, NID nid, UINT uiPage, BYTE * pBuffer, UINT cbBuff)
{
	CB cbHeap = HN_HeapSize(nidParent, nid);
	UINT uiStartRead = uiPage * cbNBDataMax;
	UINT uiEndRead = (UINT)cbHeap - uiStartRead > cbBuff ? uiStartRead + cbBuff : (UINT)cbHeap;

	ASSERT(uiPage <= HN_PageCount(nidParent, nid));
	ASSERT(cbBuff <= cbNBDataMax);

	return NODE_ReadData(nidParent, nid, pBuffer, cbBuff, uiStartRead, uiEndRead - uiStartRead);	
}

UINT LTPViewer::HN_ReadHID(NID nidParent, NID nid, HID hid, BYTE* pBuffer, UINT cbBuffer)
{
	UINT uiPage = HIDPage(hid);
	BYTE pageBuffer[cbNBDataMax];
	HNPAGEHDR * pHdr = NULL;
	HNPAGEMAP * pMap = NULL;

	if(hid == 0)
		return 0;

	HN_GetPage(nidParent, nid, uiPage, pageBuffer, cbNBDataMax);
	pHdr = (PHNPAGEHDR)pageBuffer;
	pMap = (HNPAGEMAP*)(pageBuffer + pHdr->ibHnpm);

	return (UINT)NODE_ReadData(nidParent, nid, pBuffer, cbBuffer, uiPage * cbNBDataMax + pMap->rgibAlloc[HIDIndex(hid)], pMap->rgibAlloc[HIDIndex(hid)+1] - pMap->rgibAlloc[HIDIndex(hid)] > (int)cbBuffer ? cbBuffer : pMap->rgibAlloc[HIDIndex(hid)+1] - pMap->rgibAlloc[HIDIndex(hid)]);
}

UINT LTPViewer::HN_ReadHID(NID nidParent, NID nid, HID hid, BYTE** ppBuffer)
{
	UINT uiPage = HIDPage(hid);
	BYTE pageBuffer[cbNBDataMax];
	HNPAGEHDR * pHdr = NULL;
	HNPAGEMAP * pMap = NULL;
	UINT size = 0;

	if(uiPage >= HN_PageCount(nidParent, nid))
		return 0;

	if(hid == 0)
	{
		*ppBuffer = NULL;
		return 0;
	}

	HN_GetPage(nidParent, nid, uiPage, pageBuffer, cbNBDataMax);
	pHdr = (PHNPAGEHDR)pageBuffer;
	pMap = (HNPAGEMAP*)(pageBuffer + pHdr->ibHnpm);
	size = pMap->rgibAlloc[HIDIndex(hid)+1] - pMap->rgibAlloc[HIDIndex(hid)];
	*ppBuffer = new BYTE[size];
	return (UINT)NODE_ReadData(nidParent, nid, *ppBuffer, size, uiPage * cbNBDataMax + pMap->rgibAlloc[HIDIndex(hid)], size);
}


// making this function iterative because i'm bored
BID LTPViewer::HN_GetPageBID(NID nidParent, NID nid, UINT uiPage)
{
	BID data;
	BID sub;
	CB cb;
	BREF bref;
	UINT cRef;
	if(!LookupSubnodeNID(nidParent, nid, data, sub) || !LookupBID(data, bref, cb, cRef))
		return (BID)0;

	if(BIDIsExternal(data))
	{
		ASSERT(uiPage == 0);
		return data;
	}

	// else, we are looking at an XBLOCK
	{
		BYTE blockBuffer[cbNBDataMax];
		XBLOCK *pXB = NULL;
		BLOCKTRAILER bt;
		ReadBlock(blockBuffer, (UINT)cb, &bt, bref.ib, (UINT)CbAlignDiskNDB(cb));
		
		pXB = (XBLOCK*)blockBuffer;

		// XXBlock. Get the correct XBLOCK and adjust uiPage
		if(pXB->cLevel == 2)
		{
			UINT uiXBPage = uiPage / cXBEntMax;
			uiPage = uiPage % cXBEntMax;
			BID bidXB = pXB->rgbid[uiXBPage];
			LookupBID(bidXB, bref, cb, cRef);
			ReadBlock(blockBuffer, (UINT)cb, &bt, bref.ib, (UINT)CbAlignDisk(cb));
		}

		ASSERT(pXB->cLevel == 1);

		return pXB->rgbid[uiPage];
	}

}

BYTE LTPViewer::HN_GetPageFillLevel(NID nidParent, NID nid, UINT uiPage)
{
	BYTE pageBuffer[cbNBDataMax];
	BYTE * pFillBM = NULL;
	UINT ib;
	UINT ibit;
	BYTE bMaskShift;
	UINT uiFillIndexForPage = 0;

	// The header is a special case, because it's fill bitmap is a different size
	if(uiPage < CPAGES_PER_HEADER_FILL_BITMAP)
	{
		// The appropriate fillmap is in the header
		HN_GetPage(nidParent, nid, 0, pageBuffer, cbNBDataMax);
		pFillBM = ((HNHDR*)pageBuffer)->rgbFillLevel;
		uiFillIndexForPage = uiPage;
	}
	else
	{
		// We need to read the appropriate fillmap page
		UINT uiBMPage = uiPage - ((uiPage - CPAGES_PER_HEADER_FILL_BITMAP) % CPAGES_PER_FILL_BITMAP);
		HN_GetPage(nidParent, nid, uiBMPage, pageBuffer, cbNBDataMax);
		pFillBM = ((HNBITMAPHDR*)pageBuffer)->rgbFillLevel;
		uiFillIndexForPage = uiPage - uiBMPage;
	}

	// Now we have a pointer to the fill bitmap, and the index into it, return the value.
	//b = pFillBM[uiFillIndexForPage * CBITS_PER_FILL_LEVEL / 8];
	//if(uiFillIndexForPage % 2) b >>= CBITS_PER_FILL_LEVEL;
	ib = uiFillIndexForPage / CPAGES_PER_FILL_BYTE;
	ibit = (uiFillIndexForPage % CPAGES_PER_FILL_BYTE) * CBITS_PER_FILL_LEVEL;
	bMaskShift = (BYTE) (FILL_LEVEL_MASK << ibit);
	
	return ((pFillBM[ib] & bMaskShift) >> ibit);
}

WCHAR* SPVtoString(LPSPropValue lpspv)
{
	WCHAR* retVal = new WCHAR[64];

	switch(PROP_TYPE(lpspv->ulPropTag))
	{
		case PT_BOOLEAN:
			StringCchPrintf(retVal, 64, L"%s", lpspv->Value.b == 0 ? L"FALSE" : L"TRUE");
			break;
		case PT_I2:
		case PT_LONG:
		case PT_R4:
		case PT_DOUBLE:
			StringCchPrintf(retVal, 64, L"%i", lpspv->Value.ul);
			break;
		case PT_STRING8:
			StringCchPrintf(retVal, 64, L"%S", lpspv->Value.lpszA);
			StringCchPrintf(&(retVal[60]), 4, L"...");
			break;
		case PT_UNICODE:
			StringCchPrintf(retVal, 64, L"%s", lpspv->Value.lpszA);
			StringCchPrintf(&(retVal[60]), 4, L"...");
			break;
		case PT_ERROR:
			StringCchPrintf(retVal, 64, L"<error>");
			break;
		default:
			StringCchPrintf(retVal, 64, L"<...>");
	}

	return retVal;
}