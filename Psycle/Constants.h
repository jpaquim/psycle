#ifndef __CONSTANTS_H
#define __CONSTANTS_H

#define VERSION_NUMBER "1.7.4"
#define CONVERT_INTERNAL_MACHINES

#define TWEAK_SLIDE_SAMPLES		64 // number of samples per tweak slide update
#define MAX_TWS					16 // # of tws commands that can be active on one machine

#define MAX_PLUGINS				256 // Legacy! It is used in File loading/saving
#define MAX_BUSES				64	// Power of 2! Important!
#define MAX_MACHINES			129
#define MASTER_INDEX			128
#define MAX_TRACKS				64	
#define OLD_MAX_TRACKS			32	// Note: changing this breaks file format.. but not for long
#define OLD_MAX_WAVES			16
#define MAX_WAVES				32
#define MAX_LINES				256
#define MAX_INSTRUMENTS			256
#define OLD_MAX_INSTRUMENTS		255
#define MAX_SEQUENCES			1 // for multipattern
#define PREV_WAV_INS			255
#define LOCK_LATENCY			256
#define MAX_PATTERNS			255		// MAX NUM OF PATTERNS - NOTE: this was used incorrectly in many places instead of MAX_SONG_POSITIONS
#define EVENT_SIZE				5
#define MULTIPLY				MAX_TRACKS * EVENT_SIZE		// you don't need to calc these by hand, 
#define MULTIPLY2				MULTIPLY * MAX_LINES		// precompiler will do that for you
#define MAX_PATTERN_BUFFER_LEN	MULTIPLY2 * MAX_PATTERNS	// without any affect on the generated code
#define MAX_SONG_POSITIONS		128 // Note: changing this breaks file format
#define MAX_CONNECTIONS		12

#define MAX_DELAY_BUFFER		65536 // Dalay Delay ,Flanger and Player
#define OVERLAPTIME				128  // Sampler
#define STREAM_SIZE				256	// If changed, change "MAX_BUFFER_LENGTH" in machineinterface.h, if needed.

// we need to make a high/low version system that breaks the UINT into 2 numbers
#define CURRENT_FILE_VERSION_INFO	0
#define CURRENT_FILE_VERSION_SNGI	0
#define CURRENT_FILE_VERSION_SEQD	0
#define CURRENT_FILE_VERSION_PATD	0
#define CURRENT_FILE_VERSION_MACD	0
#define CURRENT_FILE_VERSION_INSD	0
#define CURRENT_FILE_VERSION_WAVE	0

#define CURRENT_CACHE_MAP_VERSION	0

#define CURRENT_FILE_VERSION		CURRENT_FILE_VERSION_INFO+CURRENT_FILE_VERSION_SNGI+CURRENT_FILE_VERSION_SEQD+CURRENT_FILE_VERSION_PATD+CURRENT_FILE_VERSION_MACD+CURRENT_FILE_VERSION_INSD+CURRENT_FILE_VERSION_WAVE

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;


#endif
