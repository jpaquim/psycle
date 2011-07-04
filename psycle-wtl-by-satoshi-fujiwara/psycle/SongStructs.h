#ifndef _SONGSTRUCTS_H
#define _SONGSTRUCTS_H
/** @file 
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.3 $
 */

#pragma pack(push, 1)

struct PatternEntry
{
	UCHAR _note;
	UCHAR _inst;
	UCHAR _mach;
	UCHAR _cmd;
	UCHAR _parameter;
	UCHAR _volcmd;
	UCHAR _volume;
	UCHAR _reserve;// 7byteだとアラインメントが．．．。
};
/*

class PatternEntry {

public:
	float DeltaTime()

private:
	float m_DeltaTime;
	DWORD m_Command;
	DWORD 
}

struct PatternEntry 
{
	float DeltaTime;// デルタタイム(ticks単位)
	DWORD Command; // Note
	DWORD Parameter;// ベロシティ
	float GateTime;// ゲートタイム (0は連続）
};

*/
// Patterns are organized in lines by rows,
// i.e. the first TRACK*sizeof(Entry) bytes
// belong to the first line.
#pragma warning(disable : 4200)
typedef struct
{
	PatternEntry _data[];
}
Pattern;

typedef enum
{
	MACH_MASTER = 0,
		MACH_SINE = 1,
		MACH_DIST = 2,
		MACH_SAMPLER = 3,
		MACH_DELAY = 4,
		MACH_2PFILTER = 5,
		MACH_GAIN = 6,
		MACH_FLANGER = 7,
		MACH_PLUGIN = 8,
		MACH_VST = 9,
		MACH_VSTFX = 10,
		MACH_SCOPE = 11,
		MACH_XMSAMPLER = 12,
		MACH_DUMMY = 255
}
MachineType;

typedef enum
{
	MACHMODE_GENERATOR = 0,
	MACHMODE_FX = 1,
	MACHMODE_MASTER = 2,
}
MachineMode;

struct PatternCmd {
	static const UCHAR VELOCITY = 0xc;
	static const UCHAR PATTERNBREAK = 0xd;
	static const UCHAR OLD_MIDI = 0x10;
	static const UCHAR NOTECUT = 0xe;
	static const UCHAR PAN = 0xf8; 
	static const UCHAR RETRIG_CONTINUE = 0xfa; 
	static const UCHAR RETRIG = 0xfb; 
	static const UCHAR VOLCHG = 0xfc;
	static const UCHAR DELAY = 0xfd;
	static const UCHAR CHGTPB = 0xfe;
	static const UCHAR CHGBPM = 0xff;
};

#pragma warning(default : 4200)
#pragma pack(pop)

#endif
