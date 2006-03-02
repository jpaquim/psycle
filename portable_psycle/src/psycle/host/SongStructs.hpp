#pragma once
#include <project.hpp>
namespace psycle
{
	namespace host
	{
		#pragma pack(push, 1)

		class PatternEntry
		{
			public:
				inline PatternEntry()
				:
					_note(255),
					_inst(255),
#if !defined PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN
	#error PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN isn't defined! Check the code where this error is triggered.
#else
	#if PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN
					_volume(255),
	#endif
#endif
					_mach(255),
					_cmd(0),
					_parameter(0)
				{
				}
				unsigned char _note;
				unsigned char _inst;
#if !defined PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN
	#error PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN isn't defined! Check the code where this error is triggered.
#else
	#if PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN
				unsigned char _volume;
				unsigned char _cmd;
				unsigned char _parameter;
				unsigned char _mach;
	#else
				unsigned char _mach;
				unsigned char _cmd;
				unsigned char _parameter;
	#endif
#endif
		};

		// Patterns are organized in rows.
		// i.e. pattern[rows][tracks], being a row = NUMTRACKS*sizeof(PatternEntry) bytes
		// belong to the first line.
		#pragma warning(push)
		#pragma warning(disable:4200) // nonstandard extension used : zero-sized array in struct/union; Cannot generate copy-ctor or copy-assignment operator when UDT contains a zero-sized array
		class Pattern
		{
			public:
				PatternEntry _data[];
		};
		#pragma warning(pop)

		enum MachineType
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
			MACH_DUPLICATOR = 13,
			MACH_MIXER = 14,
			MACH_DUMMY = 255
		};

		enum MachineMode
		{
			MACHMODE_UNDEFINED = -1,
			MACHMODE_GENERATOR = 0,
			MACHMODE_FX = 1,
			MACHMODE_MASTER = 2,
		};

		struct PatternCmd
		{
			enum{
				EXTENDED	= 0xFE, //(see below)
				SET_TEMPO	= 0xFF,
				NOTE_DELAY	= 0xFD,
				RETRIGGER   = 0xFB,
				RETR_CONT	= 0xFA,
				SET_VOLUME	= 0x0FC,
				SET_PANNING = 0x0F8,
				BREAK_TO_LINE = 0xF2,
				JUMP_TO_ORDER = 0xF3,
				ARPEGGIO	  = 0xF0,

				// Extended Commands from 0xFE
				SET_LINESPERBEAT0 = 0x00,  // 
				SET_LINESPERBEAT1 = 0x10, // Range from FE00 to FE1F is reserved for changing lines per beat.
				SET_BYPASS = 0x20,
				SET_MUTE = 0x30,
				PATTERN_LOOP  = 0xB0, // Loops the current pattern x times. 0xFEB0 sets the loop start point.
				PATTERN_DELAY =	0xD0, // causes a "pause" of x rows ( i.e. the current row becomes x rows longer)
				FINE_PATTERN_DELAY=	0xF0 // causes a "pause" of x ticks ( i.e. the current row becomes x ticks longer)
			};
		};

		#pragma pack(pop)
	}
}
