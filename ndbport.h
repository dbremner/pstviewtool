#pragma once
#define cnidTypesMax		32			// max number of node types
#define	cEntFMRoot			UINT(128)	// number of FMap entries in header
#define cbFPRoot			UINT(128)	// number of bytes in free PMap bitmap
#define cLockMax			UINT(32)	// number of file lock entries
#define	cbAMapPage			UINT(cbPageData & ~7L)
#define cbAMapFill			UINT(cbPageData - cbAMapPage)
#define	sSlotShift			UINT(6)
#define	csPerByte			UINT(8)
#define	cbPerSlot			UINT(64)
#define csPage				LcbToCs(cbPage)

#define ibHeader			IB(0)
#define ibPageReserved		IB(ibHeader + cbPage)		// one extra page of overflow area for the header
#define ibARVec0			IB(ibPageReserved + cbPage)
#define cbARVec				UINT(8 * 1024)
#define ibARVec1			IB(ibARVec0 + cbARVec)
#define ibAMapBase			IB(ibARVec1 + cbARVec)
#define	csPerAMap			UINT(cbAMapPage * csPerByte)
#define	cbPerAMap			CB(csPerPMap * cbPerSlot)

#define ibPMapBase			ibAMapBase
#define	csPerPMap			csPerAMap
#define cAMapPerPMap		csPerByte
#define cbPMapPerAMap		(cbAMapPage / cAMapPerPMap)
#define cbPerPMap			CB(cbPerAMap * csPerByte)

#define cbFMapPage	cbPageData
#define	cEntFMPage	UINT(cbFMapPage & ~7)
#define cbFMapFill	UINT(cbPageData - cEntFMPage)
#define ibFMapBase	IB(ibAMapBase + cEntFMRoot * cbPerAMap)
#define cbPerFMap	CB(cbPerAMap * cEntFMPage)

#define cEntFPMRoot		(cbFPRoot * csPerByte)
#define cbFPMapPage		cbPageData
#define	cEntFPMPage		(cbFPMapPage * csPerByte)
#define csPerFPMap		cEntFPMPage
#define cbFPMapFill		UINT(cbPageData - cbFPMapPage)
#define ibFPMapBase		IB(ibAMapBase + cEntFPMRoot * cbPerPMap)
#define cbPerFPMap		CB(((CB) cbPerPMap) * cEntFPMPage)

#define cPagesPerCluster	UINT(8)
#define cbPerCluster		(cbPage * cPagesPerCluster)
#define csPerCluster		CbToCs(cbPerCluster)
#define ibHeaderCRCPartialStart		8		// offsetof(SmallPST::HEADER, wMagicClient)
#define ibHeaderCRCPartialEnd		479		// ofssetof(SmallPST::HEADER, bLockSemaphore)
#define cbHeaderCRCPartial			(ibHeaderCRCPartialEnd - ibHeaderCRCPartialStart)
#define ibHeaderCRCFullStart		offsetof(HEADER, wMagicClient)
#ifdef SMALL_PST
#define ibHeaderCRCFullEnd			offsetof(HEADER, bLockSemaphore)
#else
#define ibHeaderCRCFullEnd			offsetof(HEADER, dwCRCFull)
#endif
#define cbHeaderCRCFull				(ibHeaderCRCFullEnd - ibHeaderCRCFullStart)
#define ibHeaderCRCVersionStart		offsetof(HEADER, wMagicClient)
#define ibHeaderCRCVersionEnd		offsetof(HEADER, rgbReserved)
#define cbHeaderCRCVersion			(ibHeaderCRCVersionEnd - ibHeaderCRCVersionStart)

#define CsToCb(cs)			(CB(cs) << sSlotShift)
#define CbToCs(cb)			(((ULONG)(cb)) >> sSlotShift)
#define CsToLcb(cs)			(LCB(cs) << sSlotShift)
#define LcbToCs(cb)			UINT((cb) >> sSlotShift)
#define CbAlignSlot(cb)		((UINT(cb) + cbPerSlot - 1) & ~(cbPerSlot - 1))
#define CbAlignSlotIB(ib)		((IB(ib) + (IB)cbPerSlot - 1) & ~((IB)cbPerSlot - 1))
#define CbAlignDiskNDB(cb)		CbAlignSlot((cb) + sizeof(BLOCKTRAILER))
#ifdef SMALL_PST
#define CbAlignDisk(cb)		(CbAlignDiskNDB(cb) < (4*1024) ? CbAlignDiskNDB(cb) : (4*1024)) 
#else
#define CbAlignDisk(cb)		(CbAlignDiskNDB(cb) < (8*1024) ? CbAlignDiskNDB(cb) : (8*1024)) 
#endif

#define	cbPage			UINT(512)
#define	cbPageData		UINT(cbPage - sizeof(PAGETRAILER))
#ifdef SMALL_PST
#define	cbBTData	UINT(cbPageData - (4*sizeof(BYTE)))
#else
#define	cbBTData	UINT(cbPageData - (8*sizeof(BYTE))) 
#endif
#define	cBTEntMax	UINT((cbBTData / sizeof(BTENTRY)))
#define	cbBTEnt		UINT(sizeof(BTENTRY))

// bid stuff
#define bidAttachedBit				BID(0x00000001L)
#define bidInternalBit				BID(0x00000002L)
#define bidIncrement				BID(0x00000004L)
#define bidInvalid					BID(-1)

#define BIDIsAttached(bid)			(!!((bid) & bidAttachedBit))
#define BIDIsInternal(bid)			(!!((bid) & bidInternalBit))
#define BIDIsExternal(bid)			(! ((bid) & bidInternalBit))
#define BIDStrip(bid)				((bid) & ~bidAttachedBit)

// Block Types ----------------------------------------------------------------
#define btypeNB			BTYPE(0x00)		// external data block
#define btypeXB			BTYPE(0x01)		// internal XBLOCK
#define btypeSB			BTYPE(0x02)		// internal SBLOCK

// pages that are generally persisted to disk (next one should have id 0x86)
#define	ptypeBBT		PTYPE(0x80)		// blocks b-tree page
#define	ptypeNBT		PTYPE(0x81)		// nodes b-tree page
#define	ptypeFMap		PTYPE(0x82)		// csLimit hints page
#define	ptypePMap		PTYPE(0x83)		// page allocation map bits page
#define	ptypeAMap		PTYPE(0x84)		// allocation map bits page
#define ptypeFPMap		PTYPE(0x85)		// continuation of rgbFP in the HEADER structure
#define ptypeMaxWrite	ptypeFPMap		// the maximum page type for pages that are persisted to disk

// pages that are generally not persisted to disk (next one should have id 0x8C)
#define	ptypeFL			PTYPE(0x8D)		// free list page
#define	ptypeRBT		PTYPE(0x8E)		// refcount b-tree page (PSTRCVR)
#define ptypeNRBT		PTYPE(0x8F)		// NID refcount b-tree page (PSTRCVR)

#ifdef SMALL_PST
typedef ULONG NDBIDX;
#else
typedef ULONGLONG NDBIDX;
#endif
typedef NDBIDX BTKEY;				// B-Tree key
typedef NDBIDX IB;
typedef NDBIDX BID;
typedef NDBIDX CB;

typedef DWORD	NID;
typedef BYTE	PTYPE;
typedef BYTE	BTYPE;				// block type identifier
typedef ULONG	LCB;			// count of bytes
typedef WORD	CREF;				// reference count

struct BREF {
	BID				bid;				// block identifier of the page block
	IB				ib;					// file location of the page block
};

struct ROOT {							// LIB
	ULONG			cOrphans;			//	count of BBT blocks with no refs
	IB				ibFileEof;			//	current EOF of the file
	IB				ibAMapLast;			//	file location of last AMap page
	CB				cbAMapFree;			//	count of free bytes in the AMap
	CB				cbPMapFree;			//	count of free bytes in the PMap
	BREF			brefNBT;			//	reference to root NBT page
	BREF			brefBBT;			//	reference to root BBT page
	BYTE			fAMapValid;			//	allocation map is valid
	BYTE			bARVec;				//	current BBT AddRef vector in use
	WORD			cARVec;				//	count of entries in vector
};

struct HEADER {
	DWORD			dwMagic;			//	   file type identification
	DWORD			dwCRCPartial;		//	   CRC of area between ibHeaderCRCPartialStart and 
										// 		ibHeaderCRCPartialEnd irrespective of 
										//		actual header size
	WORD			wMagicClient;		//	   client file type identification
	WORD			wVer;				//	   file format version
	WORD			wVerClient;			//	   client file format version
	BYTE			bPlatformCreate;	//	   platform which created database
	BYTE			bPlatformAccess;	//	   platforms accessing database
	DWORD			dwOpenDBID;			//	   database id currently assigned
	DWORD			dwOpenClaimID;		//	   random number assigned at open
#ifdef SMALL_PST
	BID				bidNextB;			//	   next BID to assigned for blocks
#else
	BID				bidUnused;			//	   required for backwards compatibility
										//	   see bug O11:63190
#endif
	BID				bidNextP;			//	   next BID to assigned for pages
	DWORD			dwUnique;			//	   highest unique number assigned
	NID				rgnid[cnidTypesMax];//	   maximum NIDs assigned
	ROOT			root;				//     root structure
	BYTE			rgbFM[cEntFMRoot];	//     csLimit for first AMap pages
	BYTE			rgbFP[cbFPRoot];	//     bitmap of candidate PMap pages
	BYTE			bSentinel;			//     sentinel byte terminates bitmap
	BYTE			bCryptMethod;		//     encryption method in use
	BYTE			rgbReserved[2];	//     reserved (must be zero)
#ifdef SMALL_PST
	ULONGLONG		ullReserved;		//	  reserved (must be zero)
	DWORD			dwReserved;			//	  reserved (must be zero)
#else
#pragma pack(push, pack_prev)
#pragma pack(4)
	BID				bidNextB;			//	  next BID to assigned for blocks
#pragma pack(pop, pack_prev)
	DWORD			dwCRCFull;			//	  CRC of area between ibHeaderCRCFullStart and
										//		ibHeaderCRCFullEnd, can change from
										//		version to version
#endif
	BYTE			rgbVersionEncoded[3];	//		version (XOR'ed with CRC when written to disk)
	BYTE			bLockSemaphore;		//     file locking semaphore slot
	BYTE			rgbLock[cLockMax];	//     file locking client slots
};

void	CryptPermute(PVOID pv, int cb, BOOL fEncrypt);
void	CryptCyclic(PVOID pv, int cb, DWORD dwKey);

struct PAGETRAILER {
	PTYPE			ptype;				// page type
	PTYPE			ptypeRepeat;		// page type (always matches ptype)
	WORD			wSig;				// block signature
#ifndef SMALL_PST
	DWORD			dwCRC;				// CRC of data area
	BID				bid;				// block identifier
#else
	BID				bid;				// block identifier
	DWORD			dwCRC;				// CRC of data area
#endif
};

struct BLOCKTRAILER {
	WORD			cb;					// count of bytes in data area
	WORD			wSig;				// block signature
#ifndef SMALL_PST
	DWORD			dwCRC;				// CRC of data area
	BID				bid;				// block identifier
#else
	BID				bid;				// block identifier
	DWORD			dwCRC;				// CRC of data area
#endif
};

#ifndef SMALL_PST
#define cXBEntMax			UINT((8192L - sizeof(BLOCKTRAILER) - \
								offsetof(XBLOCK, rgbid)) / sizeof(BID))
#else
#define cXBEntMax			UINT((4096L - sizeof(BLOCKTRAILER) - \
								offsetof(XBLOCK, rgbid)) / sizeof(BID))
#endif
#define lcbXBDataMax		(LCB(cbNBDataMax) * cXBEntMax)
#define cbXBMin				offsetof(XBLOCK, lcbTotal)

#define	cNBTEntMax	UINT(cbBTData / sizeof(NBTENTRY))
#define	cbNBTEnt	UINT(sizeof(NBTENTRY))

#define	cBBTEntMax			UINT(cbBTData / sizeof(BBTENTRY))
#define	cbBBTEnt			UINT(sizeof(BBTENTRY))

#define BBT_CREF_ORPHAN		0
#define BBT_CREF_ZERO		1
#define BBT_CREF_ONE		2
#define BBT_CREF_MAX		0xFFFF

#define cSBDepthMax			1
#ifdef SMALL_PST
#define cSBLEntMax			UINT(128)
#else
#define cSBLEntMax			UINT((cbNBDataMax - offsetof(SBLOCK, rgsl)) / \
								sizeof(SLENTRY))
#endif
#define cSBIEntMax			UINT((cbNBDataMax - offsetof(SBLOCK, rgsl)) / \
								sizeof(SIENTRY))

struct XBLOCK {
	BTYPE			btype;		/*lint !e123 */
	BYTE			cLevel;
	WORD			cEnt;
	LCB				lcbTotal;
	BID				rgbid[1];
};

struct SLENTRY {
	NID				nid;
	BID				bidData;
	BID				bidSub;
};

struct SIENTRY {
	NID				nid;
	BID				bid;
};

struct SBLOCK {
	BTYPE			btype;			/*lint !e123*/
	BYTE			cLevel;
	WORD			cEnt;
	union {
		SLENTRY		rgsl[1];
		SIENTRY		rgsi[1];
	};
};
struct BTENTRY {
	BTKEY			btkey;				// key for this entry
	BREF			bref;				// reference to the page block
};

struct BBTENTRY {
	BREF			bref;
	WORD			cb;
	CREF			cRef;
};

struct NBTENTRY {
	NDBIDX			nid;	// this is not of type NID because the first 64 bits of the structure
							// must form a unique key of the same width as NDBIDX in order 
							// to be able to cast it to a BTENTRY structure
	BID				bidData;
	BID				bidSub;
	NID				nidParent;
};

struct BTPAGE {
	union {
		BTENTRY		rgbte[cBTEntMax];
		BYTE		rgb[cbBTData];
	};
	BYTE			cEnt;
	BYTE			cEntMax;
	BYTE			cbEnt;
	BYTE			cLevel;
};

// This struct is persisted in the first amap page on disk, to support resuamble rebuilds
struct AMAPREBUILDRESUMEINFO {
	BID		bidNBTRoot;					// The nbt root of the header
	BID		bidBBTRoot;					// The bbt root of the header
	IB		ibAMapRebuildWatermark;	// Above this location is known to be good
	BTKEY	btKeyMax;					// Point to stop in BBT traversals
};

#define ComputeCRC(pv, cb)			ComputeCRCEx(0, (pv), (cb))
DWORD	ComputeCRCEx(DWORD dwCRC, LPCVOID pv, UINT cb);

inline WORD ComputeSig(IB ib, BID bid)
{
	ib ^= bid;

	return(WORD(WORD(ib >> 16) ^ WORD(ib)));
}

inline bool BMapTestBit(const PVOID pv, int is)
{
	return((*((BYTE *)pv + (is >> 3)) & (0x80 >> (is & 7))) != 0);
}
void BMapSetBits(PVOID pv, int is, int cs);

// NID Related things.

#define GetPrvPubNid(_idx)	\
	MakeNID(NID_TYPE_NORMAL_FOLDER, NID_INDEX_PRV_PUB_BASE + (_idx))
#define NID_INDEX_PRV_PUB_BASE	0x100

#define cnidTypesMax				32				// max number of node types
#define dwNIDTypeMask				0x0000001FL
#define dwMinNIDIndex				1025L			// NIDs 0 through 1024 are reserved for all NIDTypes
#define dwMinMsgNIDIndex			65537L			// Messages reserver 0 through 65536
#define dwMinAssocMsgNIDIndex		32769L			// Assocated Messages reserve 0 through 32768
#define dwMinSrchNIDIndex			16385L			// Search Folders reserve 0 through 16384
#define dwMaxNIDIndex				0x07FFFFFFL

#define NIDType(nid)				((NID)(nid) & dwNIDTypeMask)
#define NIDIndex(nid)				((NID)(nid) >> 5)
#define MakeNID(nidType, nidIndex)	(NIDType(nidType) | ((NID)(nidIndex) << 5))
#define SetNidType(nid, nidType)	(((NID)(nid) & ~dwNIDTypeMask) | (NIDType(nidType)))
#define SetNidIndex(nid, nidIndex)	(NIDType(nid) | ((NID)(nidIndex) << 5))

#define BID_MIN				(bidIncrement)
#ifdef SMALL_PST
#define BID_MAX				((BID) 0xFFFFFFFF)
#else
#define BID_MAX				((BID) 0xFFFFFFFFFFFFFFFF)
#endif
#define IB_MAX				BID_MAX

#define NID_MIN				((NID) 0)
#define NID_MAX				((NID) 0xFFFFFFFF)

#define NID_TYPE_NONE						0x00
#define NID_TYPE_INTERNAL					0x01
#define NID_TYPE_NORMAL_FOLDER				0x02
#define NID_TYPE_SEARCH_FOLDER				0x03
#define NID_TYPE_NORMAL_MESSAGE				0x04
#define NID_TYPE_ATTACHMENT					0x05
#define NID_TYPE_SEARCH_UPDATE_QUEUE		0x06
#define NID_TYPE_SEARCH_CRITERIA_OBJECT		0x07
#define NID_TYPE_ASSOC_MESSAGE				0x08
#define NID_TYPE_STORAGE					0x09
#define NID_TYPE_CONTENTS_TABLE_INDEX		0x0A
#define NID_TYPE_RECEIVE_FOLDER_TABLE		0x0B
#define NID_TYPE_OUTGOING_QUEUE_TABLE		0x0C
#define NID_TYPE_HIERARCHY_TABLE			0x0D
#define NID_TYPE_CONTENTS_TABLE				0x0E
#define NID_TYPE_ASSOC_CONTENTS_TABLE		0x0F
#define NID_TYPE_SEARCH_CONTENTS_TABLE		0x10
#define NID_TYPE_ATTACHMENT_TABLE			0x11
#define NID_TYPE_RECIPIENT_TABLE			0x12
#define NID_TYPE_SEARCH_TABLE_INDEX			0x13
#define NID_TYPE_CONTENTS_SMP				0x14
#define NID_TYPE_ASSOC_CONTENTS_SMP			0x15
#define NID_TYPE_CHANGE_HISTORY_TABLE		0x16
#define NID_TYPE_TOMBSTONE_TABLE			0x17
#define NID_TYPE_TOMBSTONE_DATE_TABLE		0x18
#define NID_TYPE_LREP_DUPS_TABLE			0x19
//#define	NID_TYPE_FOLDER_PATH_TOMBSTONE_TABLE	0x1A
#define NID_TYPE_LTP						0x1F

#define NID_TYPE_TABLE_MIN					NID_TYPE_RECEIVE_FOLDER_TABLE
#define NID_TYPE_TABLE_MAX					NID_TYPE_RECIPIENT_TABLE

#define NID_MESSAGE_STORE					MakeNID(NID_TYPE_INTERNAL, 0x01)
// was: NID_RECEIVE_FOLDERS					MakeNID(NID_TYPE_INTERNAL, 0x02)
#define NID_NAME_TO_ID_MAP					MakeNID(NID_TYPE_INTERNAL, 0x03)
// was: NID_NAME_TO_ID_HASH					MakeNID(NID_TYPE_INTERNAL, 0x04)
// was: NID_OUTGOING_QUEUE					MakeNID(NID_TYPE_INTERNAL, 0x05)
#define NID_NORMAL_FOLDER_TEMPLATE			MakeNID(NID_TYPE_NORMAL_FOLDER,	0x06)
#define NID_SEARCH_FOLDER_TEMPLATE			MakeNID(NID_TYPE_SEARCH_FOLDER, 0x07)
// was: NID_MESSAGE_TEMPLATE				MakeNID(NID_TYPE_NORMAL_MESSAGE, 0x08)

#define NID_ROOT_FOLDER						MakeNID(NID_TYPE_NORMAL_FOLDER, 0x09)
// was: NID_CONTENTS_TABLE					MakeNID(NID_TYPE_INTERNAL, 0x0A)
// was: NID_ASSOCIATED_TABLE				MakeNID(NID_TYPE_INTERNAL, 0x0B)
// was: NID_HIERARCHY_TABLE					MakeNID(NID_TYPE_INTERNAL, 0x0C)
// was: NID_RECIPIENT_TABLE					MakeNID(NID_TYPE_INTERNAL, 0x0D)
// was: NID_ATTACHMENT_TABLE				MakeNID(NID_TYPE_INTERNAL, 0x0E)

#define NID_SEARCH_MANAGEMENT_QUEUE			MakeNID(NID_TYPE_INTERNAL, 0x0F)
#define NID_SEARCH_ACTIVITY_LIST			MakeNID(NID_TYPE_INTERNAL, 0x10)
// was: NID_SEARCH_DOMAIN_OBJECT			MakeNID(NID_TYPE_INTERNAL, 0x11)
#define NID_SEARCH_DOMAIN_ALTERNATE			MakeNID(NID_TYPE_INTERNAL, 0x12)
#define NID_SEARCH_DOMAIN_OBJECT			MakeNID(NID_TYPE_INTERNAL, 0x13)
#define NID_SEARCH_GATHERER_QUEUE			MakeNID(NID_TYPE_INTERNAL, 0x14)
#define NID_SEARCH_GATHERER_DESCRIPTOR		MakeNID(NID_TYPE_INTERNAL, 0x15)
// was: NID_SEARCH_GATHERER_BLOB			MakeNID(NID_TYPE_INTERNAL, 0x16)

#define NID_TABLE_REBUILD_QUEUE				MakeNID(NID_TYPE_INTERNAL, 0x17)

#define NID_JUNK_MAIL_PIHSL					MakeNID(NID_TYPE_INTERNAL, 0x18)

#define NID_SEARCH_GATHERER_FOLDER_QUEUE	MakeNID(NID_TYPE_INTERNAL, 0x19)

// was:	NID_HIERARCHY_TABLE_TEMPLATE		MakeNID(NID_TYPE_INTERNAL,	0x20)
// was:	NID_CONTENTS_TABLE_TEMPLATE			MakeNID(NID_TYPE_INTERNAL,	0x21)
// was:	NID_SEARCH_CONTENTS_TABLE_TEMPLATE	MakeNID(NID_TYPE_INTERNAL,	0x22)
// was:	NID_ASSOC_CONTENTS_TABLE_TEMPLATE	MakeNID(NID_TYPE_INTERNAL,	0x23)
// was:	NID_RECIPIENT_TABLE_TEMPLATE		MakeNID(NID_TYPE_INTERNAL,	0x24)
// was:	NID_ATTACHMENT_TABLE_TEMPLATE		MakeNID(NID_TYPE_INTERNAL,	0x25)
// was:	NID_ATTACHMENT_TEMPLATE				MakeNID(NID_TYPE_ATTACHMENT,0x26)

#define NID_TC_SUB_PROPS					MakeNID(NID_TYPE_INTERNAL, 0x27)

#define NID_INDEX_TEMPLATE					0x30
#define NID_HIERARCHY_TABLE_TEMPLATE		MakeNID(NID_TYPE_HIERARCHY_TABLE, NID_INDEX_TEMPLATE)
#define NID_CONTENTS_TABLE_TEMPLATE			MakeNID(NID_TYPE_CONTENTS_TABLE, NID_INDEX_TEMPLATE)
#define NID_ASSOC_CONTENTS_TABLE_TEMPLATE	MakeNID(NID_TYPE_ASSOC_CONTENTS_TABLE, NID_INDEX_TEMPLATE)
#define NID_SEARCH_CONTENTS_TABLE_TEMPLATE	MakeNID(NID_TYPE_SEARCH_CONTENTS_TABLE, NID_INDEX_TEMPLATE)
#define NID_SMP_TEMPLATE					MakeNID(NID_TYPE_CONTENTS_SMP, NID_INDEX_TEMPLATE)
#define NID_TOMBSTONE_TABLE_TEMPLATE		MakeNID(NID_TYPE_TOMBSTONE_TABLE, NID_INDEX_TEMPLATE)
#define NID_LREP_DUPS_TABLE_TEMPLATE		MakeNID(NID_TYPE_LREP_DUPS_TABLE, NID_INDEX_TEMPLATE)

#define NID_RECEIVE_FOLDERS					MakeNID(NID_TYPE_RECEIVE_FOLDER_TABLE, 0x31)
#define NID_OUTGOING_QUEUE					MakeNID(NID_TYPE_OUTGOING_QUEUE_TABLE, 0x32)
#define NID_ATTACHMENT_TABLE				MakeNID(NID_TYPE_ATTACHMENT_TABLE, 0x33)
#define NID_RECIPIENT_TABLE					MakeNID(NID_TYPE_RECIPIENT_TABLE, 0x34)
#define NID_CHANGE_HISTORY_TABLE			MakeNID(NID_TYPE_CHANGE_HISTORY_TABLE, 0x35)
#define NID_TOMBSTONE_TABLE					MakeNID(NID_TYPE_TOMBSTONE_TABLE, 0x36)
#define NID_TOMBSTONE_DATE_TABLE			MakeNID(NID_TYPE_TOMBSTONE_DATE_TABLE, 0x37)
#define NID_ALL_MESSAGE_SEARCH_FOLDER_OLD	MakeNID(NID_TYPE_SEARCH_FOLDER, 0x38)
#define NID_ALL_MESSAGE_SEARCH_FOLDER		MakeNID(NID_TYPE_SEARCH_FOLDER, 0x39)
#define NID_LREP_GMP						MakeNID(NID_TYPE_INTERNAL, 0x40)
#define NID_LREP_FOLDERS_SMP				MakeNID(NID_TYPE_INTERNAL, 0x41)
#define NID_LREP_FOLDERS_TABLE				MakeNID(NID_TYPE_INTERNAL, 0x42)
#define NID_FOLDER_PATH_TOMBSTONE_TABLE		MakeNID(NID_TYPE_INTERNAL, 0x43)

#define NID_HST_HMP							MakeNID(NID_TYPE_INTERNAL, 0x60)

#define NID_CRITERR_NOTIFICATION			MakeNID(NID_TYPE_INTERNAL,	0x3FD)
#define NID_OBJECT_NOTIFICATION				MakeNID(NID_TYPE_INTERNAL,	0x3FE)
#define NID_NEWMAIL_NOTIFICATION			MakeNID(NID_TYPE_INTERNAL,	0x3FF)
#define NID_EXTENDED_NOTIFICATION			MakeNID(NID_TYPE_INTERNAL,	0x400)
#define NID_INDEXING_NOTIFICATION			MakeNID(NID_TYPE_INTERNAL,	0x401)

#define idxPrvRoot				5
#define idxPubRoot				0
#define NID_PRV_ROOT_FOLDER					GetPrvPubNid(idxPrvRoot)
#define NID_PUB_ROOT_FOLDER					GetPrvPubNid(idxPubRoot)

#define OL12_VALID_AMAP	0x02	// Tells us that the AMAP was marked valid in a 12 or greater client

//#define NID_ALL_MESSAGE_SEARCH_CONTENTS	MakeNID(NID_TYPE_SEARCH_CONTENTS_TABLE, NIDIndex(NID_ALL_MESSAGE_SEARCH_FOLDER))
// changed NID for changing the format of the all messages table node.
#define NID_ALL_MESSAGE_SEARCH_CONTENTS	MakeNID(NID_TYPE_SEARCH_CONTENTS_TABLE, NIDIndex(NID_ALL_MESSAGE_SEARCH_FOLDER))

inline NID GetMinNIDIndex(NID nidType)
{
	switch (nidType)
	{
		case NID_TYPE_NORMAL_MESSAGE:
			return dwMinMsgNIDIndex;
		case NID_TYPE_ASSOC_MESSAGE:
			return dwMinAssocMsgNIDIndex;
		case NID_TYPE_SEARCH_FOLDER:
			return dwMinSrchNIDIndex;
		default:
			return dwMinNIDIndex;
	};
}

// PST/OST Version --------------------------------------------------------------------
#define PST_MAGIC		0x4D53	/* 'MS' */
#define PST_VERSION			19

#define OST_MAGIC		0x4F53	/* 'OS' */
#define OST_VERSION			12
#define OST_VERSION_55		11
#define OST_VERSION_50		10  /* Exchange 5.0 version number */

#define XST_MAGIC		0x5853	/* 'XS' */
#define XST_VERSION			1

#define dwMagicHL			DWORD(0x4E444221)
#define	dwMagicLH			DWORD(0x2142444E)
#define	cbEofAlign			CB(16 * 1024L)
#define	ibEofMin			IB(ibAMapBase + cbPerAMap)
#define cbNBDataMax			UINT((8 * 1024) - sizeof(BLOCKTRAILER))
#define lcbNBDataMax		LCB(cbNBDataMax)
#define cbNBDiskMax			CbAlignDisk(cbNBDataMax)
#define ibCompactPMap		IB(ibAMapBase + (10*cbPerPMap))

#define NDB16K_VERSION					14 // ndb version prior to the 64K limit change - 4.0 RTM build
#define NDB64K_VERSION					15 // keep this around to catch future version changes
#define NDBUNI_VERSION					20 // designates a UNICODE PST. - OBSOLETE, never created.
#define NDBUNI_HUGETBL_VERSION			21 // support tables with 28-bit row indeces
#define NDBUNI_BIGHNALLOC_VERSION		22 // support large allocations in the heap
#define NDBUNI_BIGSBLOCKLEAF_VERSION	23 // support large matrix allocations in the heap

#define NDB_PLATFORM_NT		0x0001
#define NDB_PLATFORM_CHI	0x0002
#define NDB_PLATFORM_WIN16	0x0004
#define NDB_PLATFORM_DOS	0x0008
#define NDB_PLATFORM_MAC	0x0010

#define NDB_CRYPT_NONE		0		// No encryption at all
#define NDB_CRYPT_PERMUTE	1		// Simple permuation encryption
#define NDB_CRYPT_CYCLIC	2		// Bullet-style encryption
#define LOWULONG(x)				((ULONG) ((x) & ((LONGLONG) 0x00000000FFFFFFFF)))
#define HILONG(x)				((LONG) (((x) & ((LONGLONG) 0xFFFFFFFF00000000)) >> 32 ))

inline void CryptData(HEADER * pHeader, PVOID pvData, UINT cb, BID bid, BOOL fEncrypt)
{
	switch (pHeader->bCryptMethod) {
		case NDB_CRYPT_PERMUTE:
			CryptPermute(pvData, cb, fEncrypt);													/*lint !e713*/
			break;
		case NDB_CRYPT_CYCLIC:					
			CryptCyclic(pvData, cb, LOWULONG((ULONGLONG) bid));														/*lint !e713*/
			break;
		default: break;
	}
}


