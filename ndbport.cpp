#include "stdafx.h"
#include "ndbport.h"

DWORD rgdwCRC[] = {
	0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F,
	0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
	0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2,
	0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
	0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9,
	0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
	0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C,
	0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
	0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423,
	0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
	0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x01DB7106,
	0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
	0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D,
	0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
	0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
	0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
	0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7,
	0xA4D1C46D, 0xD3D6F4FB,	0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
	0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,	0x5005713C, 0x270241AA,
	0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
	0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
	0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
	0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84,
	0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
	0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB,
	0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
	0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E,
	0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
	0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55,
	0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
	0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28,
	0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
	0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F,
	0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
	0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242,
	0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
	0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69,
	0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
	0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC,
	0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
	0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693,
	0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
	0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

DWORD ComputeCRCEx(DWORD dwCRC, LPCVOID pv, UINT cb)
{
	const DWORD 	*pdw = (const DWORD *) pv;
	DWORD			dwCurr;
	const BYTE 		*pb = (const BYTE *) pv;
	BYTE			b;

	// if we have more than 4-bytes worth of data then try to CRC it with
	// aligned reads
	if (cb >= sizeof(DWORD))
	{
		// advance one byte at a time until we reach a 4-byte-aligned boundary
		while (0 != (((DWORD_PTR) pb) % sizeof(DWORD)))
		{
			dwCRC = rgdwCRC[(int)((BYTE)dwCRC ^ *pb++)] ^ (dwCRC >> 8);
			cb--;
		}

		// continue through four-bytes at a time with aligned reads
		pdw = (const DWORD *) pb;
		for (; cb >= 4; cb -= 4)
		{
			dwCurr = *pdw;
			
			b = (BYTE) (dwCurr & 0xFF);
			dwCRC = rgdwCRC[(int)((BYTE)dwCRC ^ b)] ^ (dwCRC >> 8);

			dwCurr = dwCurr >> 8;		
			b = (BYTE) (dwCurr & 0xFF);
			dwCRC = rgdwCRC[(int)((BYTE)dwCRC ^ b)] ^ (dwCRC >> 8);
			
			dwCurr = dwCurr >> 8;		
			b = (BYTE) (dwCurr  & 0xFF);
			dwCRC = rgdwCRC[(int)((BYTE)dwCRC ^ b)] ^ (dwCRC >> 8);
			
			dwCurr = dwCurr >> 8;		
			b = (BYTE) (dwCurr  & 0xFF);
			dwCRC = rgdwCRC[(int)((BYTE)dwCRC ^ b)] ^ (dwCRC >> 8);			

			pdw++;
		}

		// we'll pick up the rest with 1-byte reads
		pb = (const BYTE *) pdw;
	}

	ASSERT(sizeof(DWORD) > cb);
	// we're down to less than 4 bytes, we can't help but read unaligned data
	for (; cb > 0; --cb)
		dwCRC = rgdwCRC[(int)((BYTE)dwCRC ^ *pb++)] ^ (dwCRC >> 8);

	return(dwCRC);
}

BYTE mpbbCrypt[] =
{
	// The 'R' table begins here

	  65,  54,  19,  98, 168,  33, 110, 187,
	 244,  22, 204,   4, 127, 100, 232,  93,
	  30, 242, 203,  42, 116, 197,  94,  53,
	 210, 149,  71, 158, 150,  45, 154, 136,
	  76, 125, 132,  63, 219, 172,  49, 182,
	  72,  95, 246, 196, 216,  57, 139, 231,
	  35,  59,  56, 142, 200, 193, 223,  37,
	 177,  32, 165,  70,  96,  78, 156, 251,
	 170, 211,  86,  81,  69, 124,  85,   0,
	   7, 201,  43, 157, 133, 155,   9, 160,
	 143, 173, 179,  15,  99, 171, 137,  75,
	 215, 167,  21,  90, 113, 102,  66, 191,
	  38,  74, 107, 152, 250, 234, 119,  83,
	 178, 112,   5,  44, 253,  89,  58, 134,
	 126, 206,   6, 235, 130, 120,  87, 199,
	 141,  67, 175, 180,  28, 212,  91, 205,
	 226, 233,  39,  79, 195,   8, 114, 128,
	 207, 176, 239, 245,  40, 109, 190,  48,
	  77,  52, 146, 213,  14,  60,  34,  50,
	 229, 228, 249, 159, 194, 209,  10, 129,
	  18, 225, 238, 145, 131, 118, 227, 151,
	 230,  97, 138,  23, 121, 164, 183, 220,
	 144, 122,  92, 140,   2, 166, 202, 105,
	 222,  80,  26,  17, 147, 185,  82, 135,
	  88, 252, 237,  29,  55,  73,  27, 106,
	 224,  41,  51, 153, 189, 108, 217, 148,
	 243,  64,  84, 111, 240, 198, 115, 184,
	 214,  62, 101,  24,  68,  31, 221, 103,
	  16, 241,  12,  25, 236, 174,   3, 161,
	  20, 123, 169,  11, 255, 248, 163, 192,
	 162,   1, 247,  46, 188,  36, 104, 117,
	  13, 254, 186,  47, 181, 208, 218,  61,

	// The 'S' table begins here

	  20,  83,  15,  86, 179, 200, 122, 156,
	 235, 101,  72,  23,  22,  21, 159,   2,
	 204,  84, 124, 131,   0,  13,  12,  11,
	 162,  98, 168, 118, 219, 217, 237, 199,
	 197, 164, 220, 172, 133, 116, 214, 208,
	 167, 155, 174, 154, 150, 113, 102, 195,
	  99, 153, 184, 221, 115, 146, 142, 132,
	 125, 165,  94, 209,  93, 147, 177,  87,
	  81,  80, 128, 137,  82, 148,  79,  78,
	  10, 107, 188, 141, 127, 110,  71,  70,
	  65,  64,  68,   1,  17, 203,   3,  63,
	 247, 244, 225, 169, 143,  60,  58, 249,
	 251, 240,  25,  48, 130,   9,  46, 201,
	 157, 160, 134,  73, 238, 111,  77, 109,
	 196,  45, 129,  52,  37, 135,  27, 136,
	 170, 252,   6, 161,  18,  56, 253,  76,
	  66, 114, 100,  19,  55,  36, 106, 117,
	 119,  67, 255, 230, 180,  75,  54,  92,
	 228, 216,  53,  61,  69, 185,  44, 236,
	 183,  49,  43,  41,   7, 104, 163,  14,
	 105, 123,  24, 158,  33,  57, 190,  40,
	  26,  91, 120, 245,  35, 202,  42, 176,
	 175,  62, 254,   4, 140, 231, 229, 152,
	  50, 149, 211, 246,  74, 232, 166, 234,
	 233, 243, 213,  47, 112,  32, 242,  31,
	   5, 103, 173,  85,  16, 206, 205, 227,
	  39,  59, 218, 186, 215, 194,  38, 212,
	 145,  29, 210,  28,  34,  51, 248, 250,
	 241,  90, 239, 207, 144, 182, 139, 181,
	 189, 192, 191,   8, 151,  30, 108, 226,
	  97, 224, 198, 193,  89, 171, 187,  88,
	 222,  95, 223,  96, 121, 126, 178, 138,

	// The 'I' table begins here

	  71, 241, 180, 230,  11, 106, 114,  72,
	 133,  78, 158, 235, 226, 248, 148,  83,
	 224, 187, 160,   2, 232,  90,   9, 171,
	 219, 227, 186, 198, 124, 195,  16, 221,
	  57,   5, 150,  48, 245,  55,  96, 130,
	 140, 201,  19,  74, 107,  29, 243, 251,
	 143,  38, 151, 202, 145,  23,   1, 196,
	  50,  45, 110,  49, 149, 255, 217,  35,
	 209,   0,  94, 121, 220,  68,  59,  26,
	  40, 197,  97,  87,  32, 144,  61, 131,
	 185,  67, 190, 103, 210,  70,  66, 118,
	 192, 109,  91, 126, 178,  15,  22,  41,
	  60, 169,   3,  84,  13, 218,  93, 223,
	 246, 183, 199,  98, 205, 141,   6, 211,
	 105,  92, 134, 214,  20, 247, 165, 102,
	 117, 172, 177, 233,  69,  33, 112,  12,
	 135, 159, 116, 164,  34,  76, 111, 191,
	  31,  86, 170,  46, 179, 120,  51,  80,
	 176, 163, 146, 188, 207,  25,  28, 167,
	  99, 203,  30,  77,  62,  75,  27, 155,
	  79, 231, 240, 238, 173,  58, 181,  89,
	   4, 234,  64,  85,  37,  81, 229, 122,
	 137,  56, 104,  82, 123, 252,  39, 174,
	 215, 189, 250,   7, 244, 204, 142,  95,
	 239,  53, 156, 132,  43,  21, 213, 119,
	  52,  73, 182,  18,  10, 127, 113, 136,
	 253, 157,  24,  65, 125, 147, 216,  88,
	  44, 206, 254,  36, 175, 222, 184,  54,
	 200, 161, 128, 166, 153, 152, 168,  47,
	  14, 129, 101, 115, 228, 194, 162, 138,
	 212, 225,  17, 208,   8, 139,  42, 242,
	 237, 154, 100,  63, 193, 108, 249, 236
};

#define mpbbR	(mpbbCrypt)
#define mpbbS	(mpbbCrypt + 256)
#define mpbbI	(mpbbCrypt + 512)

/*------------------------------------------------------------------------------
	%%Owner: (BOBN)
	%%Id: d8b57c6a-bce2-43da-ae86-eda6169895a7
------------------------------------------------------------------------------*/
void CryptPermute(PVOID pv, int cb, BOOL fEncrypt)
{
	BYTE * 			pb		= (BYTE *)pv;
	BYTE * 			pbTable	= fEncrypt ? mpbbR : mpbbI;
	const DWORD *	pdw 	= (const DWORD *) pv;
	DWORD			dwCurr;
	BYTE			b;

	// if we have more than 4-bytes worth of data then try to crypt it with
	// aligned reads
	if (cb >= sizeof(DWORD))
	{
		// advance one byte at a time until we reach a 4-byte-aligned boundary
		while (0 != (((DWORD_PTR) pb) % sizeof(DWORD)))
		{
			*pb = pbTable[*pb];
			pb++;
			cb--;
		}

		// continue through four-bytes at a time with aligned reads
		pdw = (const DWORD *) pb;
		for (; cb >= 4; cb -= 4)
		{
			dwCurr = *pdw;
			
			b = (BYTE) (dwCurr & 0xFF);
			*pb = pbTable[b];
			pb++;

			dwCurr = dwCurr >> 8;		
			b = (BYTE) (dwCurr & 0xFF);
			*pb = pbTable[b];
			pb++;
			
			dwCurr = dwCurr >> 8;		
			b = (BYTE) (dwCurr  & 0xFF);
			*pb = pbTable[b];
			pb++;
			
			dwCurr = dwCurr >> 8;		
			b = (BYTE) (dwCurr  & 0xFF);
			*pb = pbTable[b];
			pb++;

			pdw++;
		}

		// we'll pick up the rest with 1-byte reads
		pb = (BYTE *) pdw;
	}

	ASSERT(sizeof(DWORD) > cb);
	// we're down to less than 4 bytes, we can't help but read unaligned data
	for (; --cb >= 0; ++pb)
		*pb = pbTable[*pb];
}

/*------------------------------------------------------------------------------
	%%Id: 76a5e544-c4af-4223-944a-080c9e015443
------------------------------------------------------------------------------*/
void CryptCyclic(PVOID pv, int cb, DWORD dwKey)
{
#if defined(_X86_)

		_asm	cld

		_asm	mov		edi, pv
		_asm	mov		ecx, cb
		_asm	jecxz	done
		_asm	mov		edx, dwKey
		_asm	mov		eax, edx
		_asm	shr		eax, 16
		_asm	xor		edx, eax
		_asm	mov		ebx, OFFSET mpbbCrypt

	next:
		_asm	mov		al, [edi]
		_asm	add		al, dl
		_asm	xlatb
		_asm	add		ebx, 256
		_asm	add		al, dh
		_asm	xlatb
		_asm	add		ebx, 256
		_asm	sub		al, dh
		_asm	xlatb
		_asm	sub		ebx, 512
		_asm	sub		al, dl
		_asm	stosb
		_asm	inc		dx
		_asm	loop	next

	done: ;

#else

	BYTE * pb = (BYTE *)pv;
	BYTE b;
	WORD w;

	w = (WORD)(dwKey ^ (dwKey >> 16));

	while (--cb >= 0) {
		b = *pb;
		b = (BYTE)(b + (BYTE)w);
		b = mpbbR[b];
		b = (BYTE)(b + (BYTE)(w >> 8));
		b = mpbbS[b];
		b = (BYTE)(b - (BYTE)(w >> 8));
		b = mpbbI[b];
		b = (BYTE)(b - (BYTE)w);
		*pb++ = b;

		w = (WORD)(w + 1);
	}

#endif
}																								/*lint -restore*/

void BMapSetBits(PVOID pv, int is, int cs)
{
	register BYTE *pb = (BYTE *)pv;

	// Advance to byte of first bit
	pb += (is >> 3);

	// Set the rightmost bits of the first byte
	if (is &= 7) {
		// See if entire bit field is in first byte
		if (is + cs <= 8) {
			*pb |= ((0xFF >> (8 - cs)) << (8 - is - cs));
			return;
		}
		*pb++ |= (0xFF >> is);
		cs -= 8 - is;
	}

	// Set whole bytes
	is = (cs >> 3);
	while (--is >= 0)
		*pb++ = 0xFF;
	
	// Set the leftmost bits of the last byte
	if (cs &= 7)
		*pb |= (0xFF << (8 - cs));
}