#pragma once
#include "version.hpp"
#include <universalis/compiler.hpp>
#include <universalis/compiler/numeric.hpp>
namespace psycle
{
	namespace host
	{
		/// number of samples per tweak slide update
		#define TWEAK_SLIDE_SAMPLES		64
		/// number of tws commands that can be active on one machine
		#define MAX_TWS					16
		/// Slots avaiable to load machines of each class (gen and FX). Power of 2! Important!
		#define MAX_BUSES				64
		/// Max number of machines+1 (master)
		#define MAX_MACHINES			129
		/// Index of MasterMachine
		#define MASTER_INDEX			128
		/// Max number of instruments.
		#define MAX_INSTRUMENTS			256
		/// Instrument index of the wave preview.
		#define PREV_WAV_INS			255
		/// Number of tracks of the sequence (psycle just support one sequence now). Modify this, CURRENT_FILE_VERSION_SEQD and add the appropiated load and save code.
		#define MAX_SEQUENCES			1
		/// harcoded maximal number different patterns.
		#define MAX_PATTERNS			256
		/// Max number of pattern tracks
		#define MAX_TRACKS				64
		/// harcoded maximal number of lines per pattern
		#define MAX_LINES				256
		/// Size in bytes of an event (note-aux-mac-effect). Increment if you add columns to a track. (like panning). Modify this, CURRENT_FILE_VERSION_PATD and add the apropiated load and save code.
#if !defined PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN
	#error PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN isn't defined! Check the code where this error is triggered.
#else
	#if PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN
		#define EVENT_SIZE				6
	#else
		#define EVENT_SIZE				5
	#endif
#endif
		/// PSY2-fileformat Constants
		#define OLD_MAX_TRACKS			32
		#define OLD_MAX_WAVES			16
		#define OLD_MAX_INSTRUMENTS		255
		#define OLD_MAX_PLUGINS				256
		/// \todo Lock latency acts like a semaphore (Sleep(LOCK_LATENCY)). Should we do a real semaphore instead?
		#define LOCK_LATENCY			256
		/// \todo changing this breaks file format.
		#define MAX_SONG_POSITIONS		128
		/// Max input connections and output connections a machine can have. (\todo: should be replaced by a dynamic array)
		#define MAX_CONNECTIONS		12

		/// Miscellaneous offset data.
		#define MULTIPLY				MAX_TRACKS * EVENT_SIZE
		#define MULTIPLY2				MULTIPLY * MAX_LINES		
		#define MAX_PATTERN_BUFFER_LEN	MULTIPLY2 * MAX_PATTERNS	

		/// Temporary buffer to get all the audio from Master (which work in small chunks), and send it to the soundcard after converting it to float.
		#define MAX_DELAY_BUFFER		65536
		/// Sampler
		#define OVERLAPTIME				128
		/// \todo Size of audio blocks which are work'd together (max value). If changed, change "MAX_BUFFER_LENGTH" in machineinterface.h, appropiatedly.
		#define STREAM_SIZE				256

		/// Current version of the Song file chunks. 0xAABB  A= Major version (can't be loaded, skip the whole chunk), B=minor version. It can be loaded with the existing loader, but not all information will be avaiable.
		#define CURRENT_FILE_VERSION_INFO	0x0000
		#define CURRENT_FILE_VERSION_SNGI	0x0000
		#define CURRENT_FILE_VERSION_SEQD	0x0000
		#define CURRENT_FILE_VERSION_PATD	0x0000
		#define CURRENT_FILE_VERSION_MACD	0x0000
		#define CURRENT_FILE_VERSION_INSD	0x0000
		#define CURRENT_FILE_VERSION_WAVE	0x0000

		#define CURRENT_CACHE_MAP_VERSION	0x0001

		#define CURRENT_FILE_VERSION CURRENT_FILE_VERSION_INFO+CURRENT_FILE_VERSION_SNGI+CURRENT_FILE_VERSION_SEQD+CURRENT_FILE_VERSION_PATD+CURRENT_FILE_VERSION_MACD+CURRENT_FILE_VERSION_INSD+CURRENT_FILE_VERSION_WAVE

		/// \todo
		typedef UNIVERSALIS__COMPILER__DEPRECATED("same as std::uint8_t" ) universalis::compiler::numeric< 8>::unsigned_int byte;
		/// \todo
		typedef UNIVERSALIS__COMPILER__DEPRECATED("same as std::uint16_t") universalis::compiler::numeric<16>::unsigned_int word;
		/// \todo
		typedef UNIVERSALIS__COMPILER__DEPRECATED("same as std::uint32_t") universalis::compiler::numeric<32>::unsigned_int dword;
	}
}
