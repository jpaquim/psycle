// Constants used by psycle
#pragma once
#include "version.hpp"
#include <psycle/plugin_interface.hpp>
namespace psy
{
	namespace core
	{

/// number of samples per tweak slide update
const int TWEAK_SLIDE_SAMPLES = 64;
/// number of tws commands that can be active on one machine
const int MAX_TWS = 16;
/// Slots avaiable to load machines of each class (gen and FX). Power of 2! Important!
const int MAX_BUSES = 64;
/// Max number of machines+1 (master)
const int MAX_MACHINES = 129;
/// Index of MasterMachine
const int MASTER_INDEX = 128;
/// Max number of instruments.
const int MAX_INSTRUMENTS = 256;
/// Instrument index of the wave preview.
//const int PREV_WAV_INS = 255;
/// Number of tracks of the sequence (psycle just support one sequence now). Modify this, CURRENT_FILE_VERSION_SEQD and add the appropiated load and save code.
const int MAX_SEQUENCES = 1;
/// maximum number of different patterns. PSY3 Fileformat supports up to 2^32. UI is limited to 256 for now.
const int MAX_PATTERNS = 256;
/// Max number of pattern tracks
		/*unsigned*/ int const MAX_TRACKS = plugin_interface::MAX_TRACKS;
/// harcoded maximal number of lines per pattern
const int MAX_LINES = 1024;
/// maximum number of positions in the sequence. PSY3 Fileformat supports up to 2^32. UI is limited to 256 for now.
const int MAX_SONG_POSITIONS = 256;
/// Max input connections and output connections a machine can have. (\todo: should be replaced by a dynamic array)
const int MAX_CONNECTIONS = 12;
/// Size in bytes of an event (note-aux-mac-effect). Increment if you add columns to a track. (like panning). Modify this, CURRENT_FILE_VERSION_PATD and add the apropiated load and save code.
/*#if !defined PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN
	#error PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN isn't defined! Check the code where this error is triggered.
#else
	#if PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN
		#define EVENT_SIZE 6
	#else
		#define EVENT_SIZE 5
	#endif
#endif*/
const int EVENT_SIZE = 5;

/// Miscellaneous offset data.
const int MULTIPLY = MAX_TRACKS * EVENT_SIZE;
const int MULTIPLY2 = MULTIPLY * MAX_LINES;
const int MAX_PATTERN_BUFFER_LEN = MULTIPLY2 * MAX_PATTERNS;

/// \todo Lock latency acts like a semaphore (Sleep(LOCK_LATENCY)). Should we do a real semaphore instead?
const int LOCK_LATENCY = 256;
/// Temporary buffer to get all the audio from Master (which work in small chunks), and send it to the soundcard after converting it to float.
const int MAX_DELAY_BUFFER = 65536;
/// Sampler
const int OVERLAPTIME = 128;
/// Maximum size of the audio block to be passed to the Work() function.
const int STREAM_SIZE = plugin_interface::MAX_BUFFER_LENGTH;

/// Current version of the Song file chunks. 0xAABB  A= Major version (can't be loaded, skip the whole chunk), B=minor version. It can be loaded with the existing loader, but not all information will be avaiable.
const int CURRENT_FILE_VERSION_INFO = 0x0000;
const int CURRENT_FILE_VERSION_SNGI = 0x0000;
const int CURRENT_FILE_VERSION_SEQD = 0x0000;
const int CURRENT_FILE_VERSION_PATD = 0x0000;
const int CURRENT_FILE_VERSION_MACD = 0x0000;
const int CURRENT_FILE_VERSION_INSD = 0x0000;
const int CURRENT_FILE_VERSION_WAVE = 0x0000;
const int CURRENT_FILE_VERSION = CURRENT_FILE_VERSION_INFO+CURRENT_FILE_VERSION_SNGI+CURRENT_FILE_VERSION_SEQD+CURRENT_FILE_VERSION_PATD+CURRENT_FILE_VERSION_MACD+CURRENT_FILE_VERSION_INSD+CURRENT_FILE_VERSION_WAVE;

/// \todo add real detection of type size
typedef unsigned char byte;
/// \todo add real detection of type size
typedef unsigned short word;
/// \todo add real detection of type size
typedef unsigned long dword;

const int CURRENT_CACHE_MAP_VERSION = 0x0002;
	}
}
