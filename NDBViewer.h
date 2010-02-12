#pragma once

#include "ndbport.h"
#include "ltpport.h"

#define Unreferenced(_a) (_a);

struct NodeData
{
	BREF bref;
	NID nid;
	NID nidParent;
	HID hid;
	CB cb;
	BTKEY btkey;
	ULONG ulFlags;
};

enum
{
	fChildrenLoaded = 0x1,
	fInfoNode = 0x2,
	
	// open btpage inspector for bref.ib
	aOpenBTPage = 0x4, // open a bt page inspector
	aOpenPage = 0x8, // open a regular page inspector

	// open the appropriate block inspector for bref.ib
	aOpenBlock = 0x10,
	aOpenXBlock = 0x20,
	aOpenSBlock = 0x40,

	// open a binary inspector
	aOpenBinary = 0x80,

	// select a specific node in one of the trees
	aBrowseBBT = 0x100, // browse to bref.bid in bbt
	aBrowseNBT = 0x200, // browse to nid in nbt

	// try to find refs to bref.bid
	aBrowseRefs = 0x400,

	// open a node size dialog for nid, nidparent
	aViewNodeSize = 0x800,

	// open a HN inspector for nid, nidparent
	aOpenHN = 0x1000,

	// open an ES inspector for nid, nidparent
	aOpenES = 0x2000,

	// open a HID inspector for nid, nidparent, hid
	aOpenHID = 0x4000,

	// open a BTH inspector for nid, nidparent
	aOpenBTH = 0x10000,

	// open a TCV Index Root inspector for nid
	aOpenTCVIR = 0x20000,

	// open a PC inspector for nid, nidparent
	aOpenPC = 0x40000,

	// error state
	errCRCFailed = 0x80000000,
};

#define	feLeaf 0x01
#define feNonLeaf 0x02
#define feInvalid 0x04
#define	feAllValid (feLeaf|feNonLeaf)
#define feAll (feLeaf|feNonLeaf|feInvalid)

enum {
	iconNone,
	iconFolder,
	iconMessage,
	iconCorrupt,
	iconPass,
	iconInternal,
	iconSearchFolder,
	iconAttachment,
	iconLeafPage,
	iconAssociatedMessage,
	iconBlock,
	iconPage,
	iconInternalBlock,
	iconWarning
};

#define iconFail iconCorrupt
#define iconInfo iconInternal

UINT const s_nidTypeToIcon[NID_TYPE_LTP+1] = {
	iconNone, // NID_TYPE_NONE
	iconInternal, // NID_TYPE_INTERNAL
	iconFolder, //NID_TYPE_NORMAL_FOLDER
	iconSearchFolder, //NID_TYPE_SEARCH_FOLDER
	iconMessage, //NID_TYPE_NORMAL_MESSAGE
	iconAttachment, //NID_TYPE_ATTACHMENT
	iconInternal, //NID_TYPE_SEARCH_UPDATE_QUEUE
	iconInternal, //NID_TYPE_SEARCH_CRITERIA_OBJECT
	iconAssociatedMessage, //NID_TYPE_ASSOC_MESSAGE
	iconInternal, // NID_TYPE_STORAGE
	iconInternal, //NID_TYPE_CONTENTS_TABLE_INDEX
	iconFolder, //NID_TYPE_RECEIVE_FOLDER_TABLE
	iconFolder, //NID_TYPE_OUTGOING_QUEUE_TABLE
	iconFolder, //NID_TYPE_HIERARCHY_TABLE
	iconFolder, //NID_TYPE_CONTENTS_TABLE
	iconFolder, //NID_TYPE_ASSOC_CONTENTS_TABLE
	iconSearchFolder, //NID_TYPE_SEARCH_CONTENTS_TABLE
	iconFolder, //NID_TYPE_ATTACHMENT_TABLE
	iconFolder, //NID_TYPE_RECIPIENT_TABLE
	iconInternal, //NID_TYPE_SEARCH_TABLE_INDEX
	iconFolder, // NID_TYPE_CONTENTS_SMP
	iconFolder, // NID_TYPE_ASSOC_CONTENTS_SMP
	iconFolder, // NID_TYPE_CHANGE_HISTORY_TABLE
	iconFolder, // NID_TYPE_TOMBSTONE_TABLE
	iconFolder, // NID_TYPE_TOMBSTONE_DATE_TABLE
	iconInternal, // NID_TYPE_LREP_DUPS_TABLE
	iconCorrupt, // 1A
	iconCorrupt, // 1B
	iconCorrupt, // 1C
	iconCorrupt, // 1D
	iconCorrupt, // 1E
	iconInternal // NID_TYPE_LTP
};
#define ICONFROMNID(x) (NIDType(x) <= NID_TYPE_LTP ? s_nidTypeToIcon[NIDType(x)] : iconCorrupt)

const WCHAR* const s_nidTypeToString[] = {
	L"NID_TYPE_NONE",
	L"NID_TYPE_INTERNAL",
	L"NID_TYPE_NORMAL_FOLDER",
	L"NID_TYPE_SEARCH_FOLDER",
	L"NID_TYPE_NORMAL_MESSAGE",
	L"NID_TYPE_ATTACHMENT",
	L"NID_TYPE_SEARCH_UPDATE_QUEUE",
	L"NID_TYPE_SEARCH_CRITERIA_OBJECT",
	L"NID_TYPE_ASSOC_MESSAGE",
	L"NID_TYPE_STORAGE",
	L"NID_TYPE_CONTENTS_TABLE_INDEX",
	L"NID_TYPE_RECEIVE_FOLDER_TABLE",
	L"NID_TYPE_OUTGOING_QUEUE_TABLE",
	L"NID_TYPE_HIERARCHY_TABLE",
	L"NID_TYPE_CONTENTS_TABLE",
	L"NID_TYPE_ASSOC_CONTENTS_TABLE",
	L"NID_TYPE_SEARCH_CONTENTS_TABLE",
	L"NID_TYPE_ATTACHMENT_TABLE",
	L"NID_TYPE_RECIPIENT_TABLE",
	L"NID_TYPE_SEARCH_TABLE_INDEX",
	L"NID_TYPE_CONTENTS_SMP",
	L"NID_TYPE_ASSOC_CONTENTS_SMP",
	L"NID_TYPE_CHANGE_HISTORY_TABLE",
	L"NID_TYPE_TOMBSTONE_TABLE",
	L"NID_TYPE_TOMBSTONE_DATE_TABLE",
	L"NID_TYPE_LREP_DUPS_TABLE",
	L"Unknown Type 1A",
	L"Unknown Type 1B",
	L"Unknown Type 1C",
	L"Unknown Type 1D",
	L"Unknown Type 1E",
	L"NID_TYPE_LTP"
};
#define TYPESTRINGFROMNID(x) (NIDType(x) <= NID_TYPE_LTP ? s_nidTypeToString[NIDType(x)] : L"Unknown")

const WCHAR* const s_cryptTypeToString[] = {
	L"NDB_CRYPT_NONE",
	L"NDB_CRYPT_PERMUTE",
	L"NDB_CRYPT_CYCLIC"
};
#define CRYPTSTRING(x) (x <= NDB_CRYPT_CYCLIC ? s_cryptTypeToString[x] : L"Unknown")

const WCHAR* const s_btypeToString[] = {
	L"btypeNB",
	L"btypeXB",
	L"btypeSB"
};
#define BTYPESTRING(x) (x <= btypeSB ? s_btypeToString[x] : L"Unknown")

const WCHAR* GetVersionString(WORD w);
const WCHAR* GetPlatformString(WORD w);
const WCHAR* GetClientVersionString(WORD w);
const WCHAR* GetClientMagicNumString(WORD w);
const WCHAR* GetMagicNumString(DWORD dw);
const WCHAR* GetAMapString(BYTE b);
const WCHAR* GetPTypeString(BYTE b);
ULONGLONG ULLSize(ULONGLONG ull);
const WCHAR* WSZSize(ULONGLONG ull);

// Range checking functions
UINT SBIEnt(const SBLOCK& block, UINT dataSize);
UINT SBLEnt(const SBLOCK& block, UINT dataSize);
UINT XBEnt(const XBLOCK& block, UINT dataSize);
UINT BTEnt(const BTPAGE& page);
UINT BBTEnt(const BTPAGE& page);
UINT NBTEnt(const BTPAGE& page);
UINT BBufferSize(UINT blockUnaligned); // size block data buffers using this function

class NDBViewer
{
public:
	NDBViewer(LPCWSTR filename);
	~NDBViewer(void);

	void PopulateNBT(CTreeCtrl* ptctrl);
	BOOL SelectNID(CTreeCtrl* ptctrl, NID nid);

	void PopulateBBT(CTreeCtrl* ptctrl);
	BOOL SelectBID(CTreeCtrl* ptctrl, BID bid);

	void PopulateSBlock(CTreeCtrl *ptctrl, const BREF& bref, CB cb);
	void ExpandSBlockNode(CTreeCtrl* ptctrl, HTREEITEM hItem);

	void AddBTNodeChildren(CTreeCtrl* ptctrl, HTREEITEM parent);

	void PopulateHeader(CTreeCtrl* ptctrl);

	void ForEachBTPage(PTYPE ptype, bool (*pfn)(BTPAGE*,PAGETRAILER*,IB,void*), int btType, void* pv);
	void DeleteNode(CTreeCtrl* ptctrl, HTREEITEM hItem);

	void ReadPage(BYTE* pPageData, UINT cbData, PAGETRAILER* pPageTrailer, IB ib);
	void ReadBlock(BYTE* pBlockData, UINT cbData, BLOCKTRAILER* pBlockTrailer, IB ib, UINT cbBlock);
	void ReadData(BYTE* buffAll, UINT cbBuffAll, BYTE* buffTrailer, UINT cbBuffTrailer, IB ib, UINT cb);
	void DecodeBlockInPlace(BYTE * pBlockData, UINT cbData, BID bid);
	CB IsFree(IB ib);

	bool LookupBID(BID b, BREF& bref, CB& cb, UINT& cRef);
	bool LookupNID(NID nid, BID& data, BID& sub);
	bool LookupSubnodeNID(NID nidParent, NID nidSubnode, BID& data, BID& sub);

	CB NODE_ReadData(NID nidParent, NID nid, BYTE* buffAll, UINT cbBuffAll, IB ib, UINT cb);
	CB NODE_GetSize(NID nidParent, NID nid);
	CB XBlockRead(BID bid, BYTE* buffAll, UINT cbBuffAll, IB ib, UINT cb);
	CB XBGetSize(BID bid);

	HEADER GetHeader() { return m_header; }

	IB GetFileEOF() { return m_ndb.GetLength(); }
	bool FValidPartialCRC() { return m_fPartialCRCPass; }
	bool FValidCRC() { return m_fFullCRCPass; }

	bool FValidPage(IB ib, PTYPE ptype = 0);
	bool FValidBlock(IB ib, CB cbUnaligned, BID bid = 0);
	bool FFree(IB ib);

	void CacheAMapBitmap(CBitmap * pBitmap) { m_cachedAMapBitmap = pBitmap; }
	CBitmap * GetCachedAMapBitmap() { return m_cachedAMapBitmap; }
	void CacheBTBitmap(CBitmap * pBitmap) { m_cachedBTBitmap = pBitmap; }
	CBitmap * GetCachedBTBitmap() { return m_cachedBTBitmap; }
	CFile& GetFile() { return m_ndb; }
protected:
	CFile m_ndb;

	// header and CRC checks
	HEADER m_header;
	bool m_fPartialCRCPass;
	bool m_fFullCRCPass;

	// File Cache
	static const int c_pcMaxSize = 50;
	struct CACHEINFO
	{
		UINT cb;
		BYTE* pData;
	};
	CMap<IB, IB, CACHEINFO*, CACHEINFO*> m_fileCache;
	CList<IB> m_fileCacheAgeList;

	// Cached AMap bitmap
	CBitmap * m_cachedAMapBitmap;

	// Cached BT bitmap
	CBitmap * m_cachedBTBitmap;

protected:
	bool ReadVerifyHeader(void);
	void AddSBlockChildren(CTreeCtrl* ptctrl, HTREEITEM parent);
	void AddNBTLeafNodes(CTreeCtrl* ptctrl, HTREEITEM parent, const BTPAGE& btpage);
	void AddBBTLeafNodes(CTreeCtrl* ptctrl, HTREEITEM parent, const BTPAGE& btpage);
	void ForEachBTPageImpl(IB ib, bool (*pfn)(BTPAGE*,PAGETRAILER*,IB,void*), int btType, void* pv);
	bool LookupBIDImpl(BTPAGE * pPage, BID bid, BREF& bref, CB& cb, UINT& cRef);
	bool LookupNIDImpl(BTPAGE * pPage, NID nid, BID& data, BID& sub);
	bool LookupSubnodeNIDImpl(SBLOCK * pSBlock, CB cbBlocksize, NID nidSubnode, BID& data, BID& sub);
};

// Helper Functions
inline void RemoveFalseChild(CTreeCtrl * ptctrl, HTREEITEM hItem)
{
	HTREEITEM hFalseChild = ptctrl->GetChildItem(hItem);
	ptctrl->DeleteItem(hFalseChild);
}
inline void AddFalseChild(CTreeCtrl * ptctrl, HTREEITEM hItem)
{
	ptctrl->InsertItem(L"False Child", hItem);
}

CFont* GetGlobalFont();
CFont* GetGlobalFontFixedWidth();
void DestroyFonts();
