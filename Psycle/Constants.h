#ifndef __CONSTANTS_H
#define __CONSTANTS_H

#define VERSION_NUMBER "1.7RC1"


#define MAX_PLUGINS				256 // Legacy! It is used in File loading/saving
#define MAX_BUSES				64	// Power of 2! Important!
#define MAX_MACHINES			128
#define MAX_TRACKS				32	// Note: changing this breaks file format
#define MAX_WAVES				16
#define MAX_LINES				256
#define MAX_INSTRUMENTS			255
#define PREV_WAV_INS			254
#define LOCK_LATENCY			256
#define MAX_PATTERNS			128		// MAX NUM OF PATTERNS - NOTE: this was used incorrectly in many places instead of MAX_SONG_POSITIONS
#define MULTIPLY				MAX_TRACKS * 5				// you don't need to calc these by hand, 
#define MULTIPLY2				MULTIPLY * MAX_LINES		// precompiler will do that for you
#define MAX_PATTERN_BUFFER_LEN	MULTIPLY2 * MAX_PATTERNS	// without any affect on the generated code
#define MAX_SONG_POSITIONS		128 // Note: changing this breaks file format
#define MAX_CONNECTIONS		12

#define MAX_DELAY_BUFFER		65536 // Dalay Delay ,Flanger and Player
#define OVERLAPTIME				128  // Sampler
#define STREAM_SIZE				256	// If changed, change "MAX_BUFFER_LENGTH" in machineinterface.h, if needed.

#endif