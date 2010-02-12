#pragma once

#define HEAP_SIGNATURE				((BYTE) 0xEC)
#define CB_MAX_ALLOC_SIZE			((UINT) 3580)	// Largest size of allocation = 3.5K - 4
#define CB_MAX_ALLOC_SIZE_16K		((UINT) 3068)	// for backward compatibility with 16K tables

typedef ULONG			HID,	FAR * PHID,		FAR * FAR * PPHID;
typedef DWORD			HNID,	FAR * PHNID,	FAR * FAR * PPHNID;
typedef UINT			PAGET;			// Page Type

#define PAGE_TYPE_FIRST				((PAGET) 0)
#define PAGE_TYPE_NORMAL			((PAGET) 1)
#define PAGE_TYPE_FILL_BITMAP		((PAGET) 2)

#define FILL_LEVEL_EMPTY			((BYTE) 0x00)
#define FILL_LEVEL_1				((BYTE) 0x01)
#define FILL_LEVEL_2				((BYTE) 0x02)
#define FILL_LEVEL_3				((BYTE) 0x03)
#define FILL_LEVEL_4				((BYTE) 0x04)
#define FILL_LEVEL_5				((BYTE) 0x05)
#define FILL_LEVEL_6				((BYTE) 0x06)
#define FILL_LEVEL_7				((BYTE) 0x07)
#define FILL_LEVEL_8				((BYTE) 0x08)
#define FILL_LEVEL_9				((BYTE) 0x09)
#define FILL_LEVEL_10				((BYTE) 0x0A)
#define FILL_LEVEL_11				((BYTE) 0x0B)
#define FILL_LEVEL_12				((BYTE) 0x0C)
#define FILL_LEVEL_13				((BYTE) 0x0D)
#define FILL_LEVEL_14				((BYTE) 0x0E)
#define FILL_LEVEL_FULL				((BYTE) 0x0F)
#define FILL_LEVEL_LAST				FILL_LEVEL_FULL
#define FILL_LEVEL_MASK				FILL_LEVEL_FULL

#define CB_FILL_PAGES				((UINT) 64)		// Number of bytes in the
													// fill page bitmap
#define CB_HEADER_FILL_PAGES		((UINT) 4)		// Number of bytes in the
													// header's fill page bitmap

#define MakeHID(_iPage, _iIndex)	((((HID) (_iPage)) << 16) | \
											(((_iIndex) + 1) << 5))
#define HIDIndex(_hid)				((UINT) ((((_hid) >> 5) - 1) & \
											0x000007FF))
#define HIDPage(_hid)				((UINT) ((_hid) >> 16))

/*
 *	PagetFromIPage
 *
 *	Purpose:
 *		Given an index to a page in the heap, calculates and
 *		returns what type of page it is.
 *
 *	Arguments:
 *		iPage	Index of the page.
 *
 *	Returns:
 *		PAGET.	Page type.	Possible types are:
 *			PAGE_TYPE_FIRST			First page.
 *			PAGE_TYPE_NORMAL		Page without a fill bitmap.
 *			PAGE_TYPE_FILL_BITMAP	Page (other than the first)
 *									with a fill bitmap.
 *
 *	Side effects:
 *		None.
 *
 *	Errors:
 *		None.
 */

#define CBitsByte(_b)					(	!!((_b) & 0x01) \
										+	!!((_b) & 0x02) \
										+	!!((_b) & 0x04) \
										+	!!((_b) & 0x08) \
										+	!!((_b) & 0x10) \
										+	!!((_b) & 0x20) \
										+	!!((_b) & 0x40) \
										+	!!((_b) & 0x80))

#define CBITS_PER_FILL_LEVEL			CBitsByte(FILL_LEVEL_MASK)
#define CPAGES_PER_FILL_BYTE			(8 / CBITS_PER_FILL_LEVEL)
#define CPAGES_PER_FILL_BITMAP			(CB_FILL_PAGES * \
												CPAGES_PER_FILL_BYTE)
#define CPAGES_PER_HEADER_FILL_BITMAP	(CB_HEADER_FILL_PAGES * \
												CPAGES_PER_FILL_BYTE)

/*------------------------------------------------------------------------------
	%%Id: 8badc67b-1f8c-4c83-a940-cb0ae7daa6c2
------------------------------------------------------------------------------*/
inline PAGET
PagetFromIPage(UINT iPage)
{																	/*lint -save -e778 -e504 -e506 -e514*/
	if (iPage == 0)
		return PAGE_TYPE_FIRST;
	if ((iPage % CPAGES_PER_FILL_BITMAP) == CPAGES_PER_HEADER_FILL_BITMAP)
		return PAGE_TYPE_FILL_BITMAP;

	return PAGE_TYPE_NORMAL;										/*lint -restore*/
}

typedef struct _HNHDR			// Header of heap (beginning of first page)
{
	WORD	ibHnpm;
	BYTE	bSig;
	BYTE	bClientSig;
	HID		hidUserRoot;
	BYTE	rgbFillLevel[CB_HEADER_FILL_PAGES];
} HNHDR, * PHNHDR;

typedef struct _HNPAGEHDR		// Header of normal pages
{
	WORD	ibHnpm;
} HNPAGEHDR, * PHNPAGEHDR;

typedef struct _HNBITMAPHDR		// Header of pages with page fill bitmaps
{
	WORD	ibHnpm;
	BYTE	rgbFillLevel[CB_FILL_PAGES];
} HNBITMAPHDR, * PHNBITMAPHDR;

typedef struct _HNPAGEMAP		// Allocation map in each page
{
	WORD	cAlloc;
	WORD	cFree;
	WORD	rgibAlloc[1];		// The real size of this array will be
								// cAlloc + 1 (dummy at the end).
} HNPAGEMAP, * PHNPAGEMAP;

// Definitions ----------------------------------------------------------------
#define bMagicGMP		0x6C
#define bMagicTC		0x7C
#define bMagicSMP		0x8C
#define bMagicHMP		0x9C
#define bMagicCH		0xA5	// column heap
#define bMagicCHTC		0xAC
#define bMagicBTH		0xB5
#define bMagicPC		0xBC


// BTH
typedef struct {
	BYTE			bMagic;			// bMagicBTH
	BYTE			cbKey;			// size of key in the B-Tree
	BYTE			cbEnt;			// bytes per leaf entry (not including key)
	BYTE			bIdxLevels;		// Number of index levels in the btree
	HID				hidRoot;		// HID of root B-Tree block
} BTHHEADER, * PBTHHEADER;

#define TBL_INDEX_MAX_SORT	16

#define SizedSSortOrderSet(_csort, _name) \
struct _SSortOrderSet_ ## _name \
{ \
	ULONG	  		cSorts;			\
	ULONG			cCategories;	\
	ULONG			cExpanded;		\
	SSortOrder		aSort[_csort];	\
} _name

typedef struct _SSortOrder
{
	ULONG	ulPropTag;			/* Column to sort on */
	ULONG	ulOrder;			/* Ascending, descending, combine to left */
} SSortOrder, FAR * LPSSortOrder;

#pragma pack(2)
typedef struct TCVINDEXINFO {
	NID					nidSub;			// sub-node containing this index
	ULONG				tagInst;		// Instancing column (MVI only)
	ULONG				cRest;			// Count of restriction structures
	DWORD				dwRestF;		// Fixed-size restriction CRC
	DWORD				dwRestV;		// Variable-size restriction CRC
	LCID				lcid;			// Locale for the index
	SizedSSortOrderSet(TBL_INDEX_MAX_SORT,ssos);	// Sort order set	
} TCVINDEXINFO, *PTCVINDEXINFO;

// number of unique indexes that we will save for a given table
#define TBL_INDEX_COUNT		5
typedef struct TCVINDEXROOT {
	BYTE				bPlatform;		// Platform which encoded the index
	BYTE				bVersion;		// Platform-specific index version
	TCVINDEXINFO		rgtcvii[TBL_INDEX_COUNT]; // information on available indexes
} TCVINDEXROOT, * PTCVINDEXROOT;
#pragma pack()

#define TCVHUGE_INDEX_VERSION		7
#define TCV64K_INDEX_VERSION		6

// the version number of the root of an index (the array of TCVINDEXINFO
// structures describing each saved index)
#define TCVROOT_INDEX_VERSION			129
#define TCVROOT_NONLOCALE_INDEX_VERSION	128


// 
// PC Stuff
//

#pragma pack(2)

typedef struct {
	WORD		wType;
	HNID		hnid;
} PROPENTRY, * PPROPENTRY;

typedef struct {
	WORD		wAlign;
	PROPENTRY	pe;
} PROPALIGN, * PPROPALIGN;

#pragma pack()