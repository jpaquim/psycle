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
					_note(0),
					_inst(0),
					_mach(0),
					_cmd(0),
					_parameter(0)
					//_volcmd(0),
					//_volume(0)
				{
				}
				compiler::uint8 _note;
				compiler::uint8 _inst;
				compiler::uint8 _mach;
				compiler::uint8 _cmd;
				compiler::uint8 _parameter;
				//compiler::uint8 _volcmd;	// Unimplemented for now. Used by XMSampler
				//compiler::uint8 _volume;	// Unimplemented for now. Used by XMSampler
		};

		// Patterns are organized in lines by rows,
		// i.e. the first TRACK*sizeof(Entry) bytes
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
				MACH_DUMMY = 255
		};

		enum MachineMode
		{
			MACHMODE_UNDEFINED = -1,
			MACHMODE_GENERATOR = 0,
			MACHMODE_FX = 1,
			MACHMODE_MASTER = 2,
		};

		class PatternCmd
		{
			public:
				static const compiler::uint8 PCMD_VELOCITY = 0xc;
				static const compiler::uint8 PCMD_PATTERNBREAK = 0xd;
				static const compiler::uint8 PCMD_OLD_MIDI = 0x10;
				static const compiler::uint8 PCMD_NOTECUT = 0xe;
				static const compiler::uint8 PCMD_PAN = 0xf8; 
				static const compiler::uint8 PCMD_RETRIG_CONTINUE = 0xfa; 
				static const compiler::uint8 PCMD_RETRIG = 0xfb; 
				static const compiler::uint8 PCMD_VOLCHG = 0xfc;
				static const compiler::uint8 PCMD_DELAY = 0xfd;
				static const compiler::uint8 PCMD_CHGTPB = 0xfe;
				static const compiler::uint8 PCMD_CHGBPM = 0xff;
		};

		#pragma pack(pop)
	}
}
