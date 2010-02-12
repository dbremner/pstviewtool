#include "NDBViewer.h"
#include "ltpport.h"
#include "mapidefs.h"
#pragma once
class LTPViewer : public NDBViewer 
{
public:
	LTPViewer(LPCWSTR filename);

	//
	// Heap-on-Node Functions
	//

	// returns the total size of the heap
	CB HN_HeapSize(NID nidParent, NID nid);
	// returns the (client) root HID
	HID HN_GetRootHID(NID nidParent, NID nid);
	// number of pages in the heap
	UINT HN_PageCount(NID nidParent, NID nid);
	// returns the specified page (zero based index)
	CB HN_GetPage(NID nidParent, NID nid, UINT uiPage, BYTE * pBuffer, UINT cbBuff);
	// BID of the specified page (zero based index)
	BID HN_GetPageBID(NID nidParent, NID nid, UINT uiPage);
	// fill level of the specified page (zero based index)
	BYTE HN_GetPageFillLevel(NID nidParent, NID nid, UINT uiPage);
	// gets the allocation, up to cbBuffer size. Returns bytes read into pBuffer
	UINT HN_ReadHID(NID nidParent, NID nid, HID hid, BYTE* pBuffer, UINT cbBuffer);
	// Gets the entire HID allocation, dynamically allocated. Caller must free *ppBuffer.
	UINT HN_ReadHID(NID nidParent, NID nid, HID hid, BYTE** ppBuffer);
	// Is this a valid HID for this heap?
	BOOL HN_FValidHID(NID nidParent, NID nid, HID hid);

	//
	// BTH Functions
	//
	void BuildBTHTree(NID nidParent, NID nid, BTHHEADER * pHeader, CTreeCtrl * pTree);
	void BuildBTHTreeImpl(NID nidParent, NID nid, HTREEITEM hItem, HID hid, CTreeCtrl * pTree, BTHHEADER * pHeader, UINT iLevel);

	BOOL BTH_Enum(NID nidParent, NID nid, BTHHEADER * pHeader, void* pCtx, bool (*pfn)(void*,void*,void*));
	BOOL BTHEnumImpl(NID nidParent, NID nid, BTHHEADER * pHeader, HID hid, UINT iLevel, void* pCtx, bool (*pfn)(void*,void*,void*));

	BOOL BTH_Lookup(NID nidParent, NID nid, BTHHEADER * pHeader, BYTE* pKey, BYTE* pValue);
	BOOL BTHLookupImpl(NID nidParent, NID nid, BTHHEADER * pHeader, HID hid, UINT iLevel, BYTE* pKey, BYTE* pValue);

	//
	// PC Functions
	//
	BOOL PC_GetProp(NID nidParent, NID nid, LPSPropValue lpspv);
//	BOOL PC_GetPropAsString(NID nidParent, NID nid, ULONG tag, PBYTE pBuffer, UINT cbBuffer);
//	BOOL PCDecodePPE(NID nidParent, NID nid, PPROPENTRY ppe, LPSPropValue lpspv);
};

const WCHAR* GetClientMagicSigString(WORD w);
const WCHAR* GetIndexVersionString(BYTE b);
const WCHAR* GetIndexRootVersionString(BYTE b);
const WCHAR* GetFillLevelString(WORD w);
const WCHAR* GetPageTypeString(PAGET w);
const WCHAR* GetPropTagString(ULONG ul);
const WCHAR* GetPropTypeString(WORD w);
WCHAR* SPVtoString(LPSPropValue lpspv);