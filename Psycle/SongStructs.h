#ifndef _SONGSTRUCTS_H
#define _SONGSTRUCTS_H

#pragma pack(push, 1)
typedef struct PatternEntry
{
	UCHAR _note;
	UCHAR _inst;
	UCHAR _mach;
	UCHAR _cmd;
	UCHAR _parameter;
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
		MACH_DUMMY = 255
}
MachineType;

typedef enum
{
	MACHMODE_GENERATOR = 0,
		MACHMODE_FX = 1,
		MACHMODE_MASTER = 2,
		MACHMODE_PLUGIN = 3,
}
MachineMode;

#pragma warning(default : 4200)
#pragma pack(pop)

#endif
