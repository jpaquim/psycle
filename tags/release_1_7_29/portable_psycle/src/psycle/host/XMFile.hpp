#pragma once
#pragma unmanaged
/** @file
 *  @brief ***** [bohan] iso-(10)646 encoding only please! *****.
 *  $Date$
 *  $Revision$
 *  ***** [bohan] iso-(10)646 encoding only please! *****
 */

#pragma pack(1)
namespace SF {

	struct XMFILEHEADER
	{
		DWORD size;
		WORD norder;
		WORD restartpos;
		WORD channels;
		WORD patterns;
		WORD instruments;
		WORD flags;
		WORD speed;
		WORD tempo;
		BYTE order[256];
	};


	struct XMINSTRUMENTHEADER
	{
		DWORD size;
		CHAR name[22];
		BYTE type;
		BYTE samples;
		BYTE samplesh;
	};

	struct XMSAMPLEHEADER
	{
		DWORD shsize;
		BYTE snum[96];
		WORD venv[24];
		WORD penv[24];
		BYTE vnum, pnum;
		BYTE vsustain, vloops, vloope, psustain, ploops, ploope;
		BYTE vtype, ptype;
		BYTE vibtype, vibsweep, vibdepth, vibrate;
		WORD volfade;
		WORD res;
		BYTE reserved1[20];
	};

	struct XMSAMPLESTRUCT
	{
		DWORD samplen;
		DWORD loopstart;
		DWORD looplen;
		BYTE vol;
		signed char finetune;
		BYTE type;
		BYTE pan;
		signed char relnote;
		BYTE res;
		char name[22];
	};
}
#pragma pack()