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

#pragma warning(default : 4200)
#pragma pack(pop)

#endif
