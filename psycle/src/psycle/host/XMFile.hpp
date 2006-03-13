#include "detail/project.hpp"
#include <cstdint>
#pragma once
namespace psycle
{
	namespace host
	{
		static const char * XM_HEADER = "extended module: ";

		struct XMCMD
		{
			// (*)  = If the command std::uint8_t is zero, the last nonzero std::uint8_t for the command should be used.
			// (0)  = It is executed (also) on tick 0, else, it does nothing on tick 0.
			// (0!) = It is executed *only* on tick 0.
			enum
			{
			  ARPEGGIO				=	0x00,// Arpeggio. If arpeggio value is zero, then command= NONE.
			  PORTAUP				=	0x01,// Portamento Up		 (*)
			  PORTADOWN				=	0x02,// Portamento Down	 (*)
			  PORTA2NOTE			=	0x03,//  Tone Portamento	 (*)
			  VIBRATO				=	0x04,// Do Vibrato		 (*)
			  TONEPORTAVOL			=	0x05,// Tone Portament & Volume Slide (*)
			  VIBRATOVOL			=	0x06,// Vibrato & Volume Slide (*)
			  TREMOLO				=	0x07,// Tremolo			 (*)
			  PANNING				=	0x08,// Set Panning Position
			  OFFSET				=	0x09,// Set Sample Offset
			  VOLUMESLIDE			=	0x0a,// Volume Slide		 (*)
			  POSITION_JUMP			=	0x0b,// Position Jump
			  VOLUME				=	0x0c,// Set Volume
			  PATTERN_BREAK			=	0x0d,// Pattern Break
			  EXTENDED				=	0x0e,// Extend Command
			  SETSPEED				=	0x0f,// Set Speed or BPM
			  SET_GLOBAL_VOLUME		=	0x10,// Set Global Volume
			  GLOBAL_VOLUME_SLIDE 	=	0x11,// Global Volume Slide (*)
			  NOTE_OFF				=	0x14,// Note Off
			  SET_ENV_POSITION 		=	0x15,// Set Envelope Position
			  PANNINGSLIDE			=	0x19,// PANNING SLIDE		 (*)
			  RETRIG				=	0x1B,// Retrigger Note	 (*)
			  TREMOR				=	0x1D,// Tremor
			  EXTEND_XM_EFFECTS 	=	0x21,// Extend XM Effects	
			  PANBRELLO				=	0x22,// Panbrello
			  MIDI_MACRO			=	0x23// Panbrello
			};
		};

		struct XMCMD_E
		{
			enum
			{
			  E_FINE_PORTA_UP	=	0x10, //E1 (*) Fine porta up
			  E_FINE_PORTA_DOWN	=	0x20, //E2 (*) Fine porta down
			  E_GLISSANDO_STATUS=	0x30, //E3     Set gliss control 
			  E_VIBRATO_WAVE	=	0x40, //E4     Set vibrato control
			  E_FINETUNE		=	0x50, //E5     Set finetune
			  E_PATTERN_LOOP	=	0x60, //E6     Set loop begin/loop
			  E_TREMOLO_WAVE	=	0x70, //E7     Set tremolo control
			  E_MOD_RETRIG		=	0x90, //E9     Retrig note
			  E_FINE_VOLUME_UP	=	0xA0, //EA (*) Fine volume slide up
			  E_FINE_VOLUME_DOWN=	0xB0, //EB (*) Fine volume slide down
			  E_DELAYED_NOTECUT	=	0xC0, //EC     Note cut
			  E_NOTE_DELAY		=	0xD0, //ED     Note delay
			  E_PATTERN_DELAY	=	0xE0, //EE     Pattern delay
			  E_SET_MIDI_MACRO	=	0xF0
			};
		};

		struct XMCMD_X
		{
			enum
			{
			  X_EXTRA_FINE_PORTA_UP		= 0x10, //X1 (*) Extra fine porta up
			  X_EXTRA_FINE_PORTA_DOWN	= 0x20, //X2 (*) Extra fine porta downp
			  X9						= 0x90, //X9 (*) Modplug's Extension
			  X_HIGH_OFFSET				= 0xA0
			};
		};

		struct XMCMD_X9
		{
			enum
			{
			  X9_SURROUND_OFF		= 0x0,
			  X9_SURROUND_ON		= 0x1,
			  X9_REVERB_OFF			= 0x8,
			  X9_REVERB_FORCE		= 0x9,
			  X9_STANDARD_SURROUND	= 0xA,
			  X9_QUAD_SURROUND		= 0xB,// Select quad surround mode: this allows you to pan in the rear channels, especially useful for 4-speakers playback. Note that S9A and S9B do not activate the surround for the current channel, it is a global setting that will affect the behavior of the surround for all channels. You can enable or disable the surround for individual channels by using the S90 and S91 effects. In quad surround mode, the channel surround will stay active until explicitely disabled by a S90 effect
			  X9_GLOBAL_FILTER		= 0xC,// Select global filter mode (IT compatibility). This is the default, when resonant filters are enabled with a Zxx effect, they will stay active until explicitely disabled by setting the cutoff frequency to the maximum (Z7F), and the resonance to the minimum (Z80).
			  X9_LOCAL_FILTER		= 0xD,// Select local filter mode (MPT beta compatibility): when this mode is selected, the resonant filter will only affect the current note. It will be deactivated when a new note is being played.
			  X9_PLAY_FORWARD		= 0xE,// Play forward. You may use this to temporarily force the direction of a bidirectional loop to go forward.
			  X9_PLAY_BACKWARD		= 0xF // Play backward. The current instrument will be played backwards, or it will temporarily set the direction of a loop to go backward. 
			};
		};

		struct XMVOL_CMD
		{
			enum
			{
				XMV_VOLUME0             = 0x10,
				XMV_VOLUME1             = 0x20,
				XMV_VOLUME2             = 0x30,
				XMV_VOLUME3             = 0x40,
				XMV_VOLUME4             = 0x50,
				XMV_VOLUMESLIDEDOWN     = 0x60,
				XMV_VOLUMESLIDEUP       = 0x70,
				XMV_FINEVOLUMESLIDEDOWN = 0x80,
				XMV_FINEVOLUMESLIDEUP   = 0x90,
				XMV_VIBRATOSPEED        = 0xA0,
				XMV_VIBRATO				= 0xB0, // depth  ( vibrato trigger )
				XMV_PANNING             = 0xC0,
				XMV_PANNINGSLIDELEFT    = 0xD0,
				XMV_PANNINGSLIDERIGHT   = 0xE0,
				XMV_PORTA2NOTE          = 0xF0
			};
		};

		#if defined DIVERSALIS__COMPILER__MICROSOFT
			#pragma pack(push, 1)
		#else
			#error todo
		#endif

		struct XMFILEHEADER // UNIVERSALIS__COMPILER__ALIGN(1)
		{
			std::uint32_t size;
			std::uint16_t norder;
			std::uint16_t restartpos;
			std::uint16_t channels;
			std::uint16_t patterns;
			std::uint16_t instruments;
			std::uint16_t flags;
			std::uint16_t speed;
			std::uint16_t tempo;
			std::uint8_t  order[256];
		};


		struct XMINSTRUMENTHEADER // UNIVERSALIS__COMPILER__ALIGN(1)
		{
			std::uint32_t size;
			char          name[22];
			std::uint8_t  type;
			std::uint8_t  samples;
			std::uint8_t  samplesh;
		};

		struct XMSAMPLEHEADER // UNIVERSALIS__COMPILER__ALIGN(1)
		{
			std::uint32_t shsize;
			std::uint8_t  snum[96];
			std::uint16_t venv[24];
			std::uint16_t penv[24];
			std::uint8_t  vnum, pnum;
			std::uint8_t  vsustain, vloops, vloope, psustain, ploops, ploope;
			std::uint8_t  vtype, ptype;
			std::uint8_t  vibtype, vibsweep, vibdepth, vibrate;
			std::uint16_t volfade;
			std::uint16_t res;
			std::uint8_t  reserved1[20];
		};

		struct XMSAMPLESTRUCT // UNIVERSALIS__COMPILER__ALIGN(1)
		{
			std::uint32_t samplen;
			std::uint32_t loopstart;
			std::uint32_t looplen;
			std::uint8_t  vol;
			std::int8_t   finetune;
			std::uint8_t  type;
			std::uint8_t  pan;
			std::int8_t   relnote;
			std::uint8_t  res;
			char          name[22];
		};

		#if defined DIVERSALIS__COMPILER__MICROSOFT
			#pragma pack(pop)
		#else
			#error todo
		#endif
	}
}
