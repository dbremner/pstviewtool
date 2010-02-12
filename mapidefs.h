#pragma once

const ULONG MV_FLAG = 0x1000;			/* Multi-value flag */
const ULONG PT_UNSPECIFIED = ((ULONG)  0);	/* (Reserved for interface use) type doesn't matter to caller */
const ULONG PT_NULL = ((ULONG)  1);	/* NULL property value */
const ULONG	PT_I2 = ((ULONG)  2);	/* Signed 16-bit value */
const ULONG PT_LONG = ((ULONG)  3);	/* Signed 32-bit value */
const ULONG	PT_R4 = ((ULONG)  4);	/* 4-byte floating point */
const ULONG PT_DOUBLE = ((ULONG)  5);	/* Floating point double */
const ULONG PT_CURRENCY = ((ULONG)  6);	/* Signed 64-bit int (decimal w/	4 digits right of decimal pt) */
const ULONG	PT_APPTIME = ((ULONG)  7);	/* Application time */
const ULONG PT_ERROR = ((ULONG) 10);	/* 32-bit error value */
const ULONG PT_BOOLEAN = ((ULONG) 11);	/* 16-bit boolean (non-zero true) */
const ULONG PT_OBJECT = ((ULONG) 13);	/* Embedded object in a property */
const ULONG	PT_I8 = ((ULONG) 20);	/* 8-byte signed integer */
const ULONG PT_STRING8 = ((ULONG) 30);	/* Null terminated 8-bit character string */
const ULONG PT_UNICODE = ((ULONG) 31);	/* Null terminated Unicode string */
const ULONG PT_SYSTIME = ((ULONG) 64);	/* FILETIME 64-bit int w/ number of 100ns periods since Jan 1,1601 */
const ULONG	PT_CLSID = ((ULONG) 72);	/* OLE GUID */
const ULONG PT_BINARY = ((ULONG) 258);	/* Uninterpreted (counted byte array) */

#define PROP_TYPE_MASK					((ULONG)0x0000FFFF)	/* Mask for Property type */
#define PROP_TYPE(ulPropTag)			(((ULONG)(ulPropTag))&PROP_TYPE_MASK)
#define PROP_ID(ulPropTag)				(((ULONG)(ulPropTag))>>16)
#define PROP_TAG(ulPropType,ulPropID)	((((ULONG)(ulPropID))<<16)|((ULONG)(ulPropType)))

typedef struct _SBinary
{
	ULONG		cb;
	LPBYTE 		lpb;
} SBinary, FAR *LPSBinary;

typedef	struct _SShortArray
{
	ULONG		cValues;
	short int	FAR *lpi;
} SShortArray;

typedef struct _SGuidArray
{
	ULONG		cValues;
	GUID		FAR *lpguid;
} SGuidArray;

typedef	struct _SRealArray
{
	ULONG		cValues;
	float		FAR *lpflt;
} SRealArray;

typedef struct _SLongArray
{
	ULONG		cValues;
	LONG 		FAR *lpl;
} SLongArray;

typedef struct _SLargeIntegerArray
{
	ULONG		cValues;
	LARGE_INTEGER	FAR *lpli;
} SLargeIntegerArray;

typedef struct _SDateTimeArray
{
	ULONG		cValues;
	FILETIME	FAR *lpft;
} SDateTimeArray;

typedef struct _SAppTimeArray
{
	ULONG		cValues;
	double		FAR *lpat;
} SAppTimeArray;

typedef struct _SCurrencyArray
{
	ULONG		cValues;
	CURRENCY	FAR *lpcur;
} SCurrencyArray;

typedef struct _SBinaryArray
{
	ULONG		cValues;
	SBinary		FAR *lpbin;
} SBinaryArray;

typedef struct _SDoubleArray
{
	ULONG		cValues;
	double		FAR *lpdbl;
} SDoubleArray;

typedef struct _SWStringArray
{
	ULONG		cValues;
	LPWSTR		FAR *lppszW;
} SWStringArray;

typedef struct _SLPSTRArray
{
	ULONG		cValues;
	LPSTR		FAR *lppszA;
} SLPSTRArray;

typedef union _PV
{
	short int			i;			/* case PT_I2 */
	LONG				l;			/* case PT_LONG */
	ULONG				ul;			/* alias for PT_LONG */
	LPVOID				lpv;		/* alias for PT_PTR */
	float				flt;		/* case PT_R4 */
	double				dbl;		/* case PT_DOUBLE */
	unsigned short int	b;			/* case PT_BOOLEAN */
	CURRENCY			cur;		/* case PT_CURRENCY */
	double				at;			/* case PT_APPTIME */
	FILETIME			ft;			/* case PT_SYSTIME */
	LPSTR				lpszA;		/* case PT_STRING8 */
	SBinary				bin;		/* case PT_BINARY */
	LPWSTR				lpszW;		/* case PT_UNICODE */
	LPGUID				lpguid;		/* case PT_CLSID */
	LARGE_INTEGER		li;			/* case PT_I8 */
	SShortArray			MVi;		/* case PT_MV_I2 */
	SLongArray			MVl;		/* case PT_MV_LONG */
	SRealArray			MVflt;		/* case PT_MV_R4 */
	SDoubleArray		MVdbl;		/* case PT_MV_DOUBLE */
	SCurrencyArray		MVcur;		/* case PT_MV_CURRENCY */
	SAppTimeArray		MVat;		/* case PT_MV_APPTIME */
	SDateTimeArray		MVft;		/* case PT_MV_SYSTIME */
	SBinaryArray		MVbin;		/* case PT_MV_BINARY */
	SLPSTRArray			MVszA;		/* case PT_MV_STRING8 */
	SWStringArray		MVszW;		/* case PT_MV_UNICODE */
	SGuidArray			MVguid;		/* case PT_MV_CLSID */
	SLargeIntegerArray	MVli;		/* case PT_MV_I8 */
	SCODE				err;		/* case PT_ERROR */
	LONG				x;			/* case PT_NULL, PT_OBJECT (no usable value) */
} __UPV;

typedef struct _SPropValue
{
	ULONG		ulPropTag;
	ULONG		dwAlignPad;
	union _PV	Value;
} SPropValue, FAR * LPSPropValue;