#pragma once
namespace psycle
{
	namespace host
	{
		/// The release number of the psycle host application.
		/// Releases are named this way:
		/// M.m.r.p , where:
		/// - M = major version number.
		/// - m = minor version number.
		/// - r = revision number, if even, release. if odd, beta.
		/// - p = patch number.
		/// It is cubersome to change because we need to update:
		/// - include/psycle/host/Constants.h: #define VERSION_NUMBER "x.x.x.x"
		/// - src/psycle/host/resources/Psycle.rc: FILEVERSION x,x,x,x
		/// - src/psycle/host/resources/Psycle.rc: PRODUCTVERSION x,x,x,x
		/// - src/psycle/host/resources/Psycle.rc: VALUE "FileVersion", "x.x.x.x"
		/// - src/psycle/host/resources/Psycle.rc: VALUE "ProductVersion", "x.x.x.x"
		/// - src/psycle/host/resources/Psycle.rc: VALUE "SpecialBuild", "msvc 7.1"
		/// - make/doxygen/doxygen.configuration: PROJECT_NUMBER
		/// - doc/for-end-users/readme.txt
		#define VERSION_NUMBER "Beta 1.7.7.3" /* "$Revision$ $Date$" */
			
		/// number of samples per tweak slide update
		#define TWEAK_SLIDE_SAMPLES		64
		/// number of tws commands that can be active on one machine
		#define MAX_TWS					16

		/// Legacy! It is used in File loading/saving
		#define MAX_PLUGINS				256
		/// Power of 2! Important!
		#define MAX_BUSES				64
		#define MAX_MACHINES			129
		#define MASTER_INDEX			128
		#define MAX_TRACKS				64
		/// \todo changing this breaks file format.. but not for long
		#define OLD_MAX_TRACKS			32
		#define OLD_MAX_WAVES			16
		#define MAX_WAVES				32
		#define MAX_LINES				256
		#define MAX_INSTRUMENTS			256
		#define OLD_MAX_INSTRUMENTS		255
		/// for multipattern
		#define MAX_SEQUENCES			1
		#define PREV_WAV_INS			255
		#define LOCK_LATENCY			256
		/// harcoded maximal number of patterns
		#define MAX_PATTERNS			255
		#define EVENT_SIZE				5

		// you don't need to calc the following by hand, 
		// precompiler will do that for you
		// without any affect on the generated code

		#define MULTIPLY				MAX_TRACKS * EVENT_SIZE
		#define MULTIPLY2				MULTIPLY * MAX_LINES		
		#define MAX_PATTERN_BUFFER_LEN	MULTIPLY2 * MAX_PATTERNS	
		/// \todo changing this breaks file format
		#define MAX_SONG_POSITIONS		128
		#define MAX_CONNECTIONS		12

		/// Player \todo <bohan> a delay buffer in the player?
		#define MAX_DELAY_BUFFER		65536
		/// Sampler
		#define OVERLAPTIME				128
		/// \todo If changed, change "MAX_BUFFER_LENGTH" in machineinterface.h, if needed.
		#define STREAM_SIZE				256

		/// \todo we need to make a high/low version system that breaks the UINT into 2 numbers
		#define CURRENT_FILE_VERSION_INFO	0
		#define CURRENT_FILE_VERSION_SNGI	0
		#define CURRENT_FILE_VERSION_SEQD	0
		#define CURRENT_FILE_VERSION_PATD	0
		#define CURRENT_FILE_VERSION_MACD	0
		#define CURRENT_FILE_VERSION_INSD	0
		#define CURRENT_FILE_VERSION_WAVE	0

		#define CURRENT_CACHE_MAP_VERSION	0

		#define CURRENT_FILE_VERSION CURRENT_FILE_VERSION_INFO+CURRENT_FILE_VERSION_SNGI+CURRENT_FILE_VERSION_SEQD+CURRENT_FILE_VERSION_PATD+CURRENT_FILE_VERSION_MACD+CURRENT_FILE_VERSION_INSD+CURRENT_FILE_VERSION_WAVE

		/// \todo add real detection of type size
		typedef unsigned char byte;
		/// \todo add real detection of type size
		typedef unsigned short word;
		/// \todo add real detection of type size
		typedef unsigned long dword;
	}
}
