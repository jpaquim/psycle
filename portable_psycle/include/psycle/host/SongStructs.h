#ifndef _SONGSTRUCTS_H
#define _SONGSTRUCTS_H

#pragma pack(push, 1)
typedef struct PatternEntry
{
	PatternEntry()
	{ _note=0;_inst=0;_mach=0;_cmd=0;_parameter=0;_volcmd=0;_volume=0;
	}
	UCHAR _note;
	UCHAR _inst;
	UCHAR _mach;
	UCHAR _cmd;
	UCHAR _parameter;
	UCHAR _volcmd;	// Unimplemented by now.
	UCHAR _volume;	// Unimplemented by now.
}
PatternEntry;

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
	MACH_UNDEFINED = -1,
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
	MACHMODE_UNDEFINED = -1,
	MACHMODE_GENERATOR = 0,
	MACHMODE_FX = 1,
	MACHMODE_MASTER = 2,
}
MachineMode;

struct PatternCmd {
	static const UCHAR PCMD_VELOCITY = 0xc;
	static const UCHAR PCMD_PATTERNBREAK = 0xd;
	static const UCHAR PCMD_OLD_MIDI = 0x10;
	static const UCHAR PCMD_NOTECUT = 0xe;
	static const UCHAR PCMD_PAN = 0xf8; 
	static const UCHAR PCMD_RETRIG_CONTINUE = 0xfa; 
	static const UCHAR PCMD_RETRIG = 0xfb; 
	static const UCHAR PCMD_VOLCHG = 0xfc;
	static const UCHAR PCMD_DELAY = 0xfd;
	static const UCHAR PCMD_CHGTPB = 0xfe;
	static const UCHAR PCMD_CHGBPM = 0xff;
};

#pragma warning(default : 4200)
#pragma pack(pop)

#endif
