/* -*- mode:c++, indent-tabs-mode:t -*- */
#pragma once
#include "SongStructs.hpp"
#include "FileIO.hpp"
#include <cstdint>

namespace psycle {
	namespace host {
		class Song;
		class XMSampler;

		class ITModule2 : public OldPsyFile
		{
		private:
			Song * s;

		public:
			ITModule2();
			virtual ~ITModule2() throw();

			// Extra class for Reading of IT compressed samples.
			class BitsBlock
			{
			public:
				BitsBlock() : pdata(0), rpos(0), rend(0), rembits(0) {}
				~BitsBlock() throw() { delete[] pdata; }
				bool ReadBlock(OldPsyFile* pFile);
				unsigned long ReadBits(unsigned char bitwidth);
			private:
				/// pointer to data
				unsigned char * pdata;
				/// read position
				unsigned char * rpos;
				/// read end
				unsigned char * rend;
				/// remaining bits in current pos
				unsigned char rembits;
			};

			//////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////////////////////
			// IT Fileformat

		public:
			struct CMD
			{
				enum
				{
					SET_SPEED            =  1,
					JUMP_TO_ORDER        =  2,
					BREAK_TO_ROW         =  3,
					VOLUME_SLIDE         =  4,
					PORTAMENTO_DOWN      =  5,
					PORTAMENTO_UP        =  6,
					TONE_PORTAMENTO      =  7,
					VIBRATO              =  8,
					TREMOR               =  9,
					ARPEGGIO             = 10,
					VOLSLIDE_VIBRATO     = 11, ///< Dual command: H00 and Dxy 
					VOLSLIDE_TONEPORTA   = 12, ///< Dual command: G00 and Dxy
					SET_CHANNEL_VOLUME   = 13,
					CHANNEL_VOLUME_SLIDE = 14,
					SET_SAMPLE_OFFSET    = 15,
					PANNING_SLIDE        = 16,
					RETRIGGER_NOTE       = 17,
					TREMOLO              = 18,
					S                    = 19,
					SET_SONG_TEMPO       = 20, ///< T0x Slide tempo down . T1x slide tempo up
					FINE_VIBRATO         = 21,
					SET_GLOBAL_VOLUME    = 22,
					GLOBAL_VOLUME_SLIDE  = 23,
					SET_PANNING          = 24,
					PANBRELLO            = 25,
					MIDI_MACRO           = 26  ///< see MIDI.TXT ([bohan] uhu? what file?)
				};
			};

			struct CMD_S
			{
				enum
				{
					S_SET_FILTER             = 0x00, ///< Greyed out in IT...
					S_SET_GLISSANDO_CONTROL  = 0x10, ///< Greyed out in IT...
					S_FINETUNE               = 0x20, ///< Greyed out in IT...
					S_SET_VIBRATO_WAVEFORM   = 0x30, ///< Check XMInstrument::WaveData::WaveForms! IT is sine, square, sawdown and random
					S_SET_TREMOLO_WAVEFORM   = 0x40, ///< Check XMInstrument::WaveData::WaveForms! IT is sine, square, sawdown and random
					S_SET_PANBRELLO_WAVEFORM = 0x50, ///< Check XMInstrument::WaveData::WaveForms! IT is sine, square, sawdown and random
					S_FINE_PATTERN_DELAY     = 0x60, ///< causes a "pause" of x ticks ( i.e. the current row becomes x ticks longer)
					S7                       = 0x70,
					S_SET_PAN                = 0x80,
					S9                       = 0x90,
					S_SET_HIGH_OFFSET        = 0xA0,
					S_PATTERN_LOOP           = 0xB0,
					S_DELAYED_NOTE_CUT       = 0xC0,
					S_NOTE_DELAY             = 0xD0,
					S_PATTERN_DELAY          = 0xE0, ///< causes a "pause" of x rows ( i.e. the current row becomes x rows longer)
					S_SET_MIDI_MACRO         = 0xF0
				};
			};

			struct CMD_S7
			{
				enum
				{
					S7_BACKGROUNDNOTECUT  =  0,
					S7_BACKGROUNDNOTEOFF  =  1,
					S7_BACKGROUNDNOTEFADE =  2,
					S7_SETNOTECUT         =  3,
					S7_SETNOTECONTINUE    =  4,
					S7_SETNOTEOFF         =  5,
					S7_SETNOTEFADE        =  6,
					S7_VOLENVOFF          =  7,
					S7_VOLENVON           =  8,
					S7_PANENVOFF          =  9,
					S7_PANENVON           = 10,
					S7_PITCHENVON         = 11,
					S7_PITCHENVOFF        = 12
				};
			};

			struct CMD_S9
			{
				enum
				{
					S9_SURROUND_OFF	=		0,
					S9_SURROUND_ON	=		1,
					S9_REVERB_OFF	=		8,
					S9_REVERB_FORCE	=		9,
					S9_STANDARD_SURROUND = 10,
					S9_QUAD_SURROUND     = 11, ///< Select quad surround mode: this allows you to pan in the rear channels, especially useful for 4-speakers playback. Note that S9A and S9B do not activate the surround for the current channel, it is a global setting that will affect the behavior of the surround for all channels. You can enable or disable the surround for individual channels by using the S90 and S91 effects. In quad surround mode, the channel surround will stay active until explicitely disabled by a S90 effect
					S9_GLOBAL_FILTER     = 12, ///< Select global filter mode (IT compatibility). This is the default, when resonant filters are enabled with a Zxx effect, they will stay active until explicitely disabled by setting the cutoff frequency to the maximum (Z7F), and the resonance to the minimum (Z80).
					S9_LOCAL_FILTER      = 13, ///< Select local filter mode (MPT beta compatibility): when this mode is selected, the resonant filter will only affect the current note. It will be deactivated when a new note is being played.
					S9_PLAY_FORWARD      = 14, ///< Play forward. You may use this to temporarily force the direction of a bidirectional loop to go forward.
					S9_PLAY_BACKWARD     = 15  ///< Play backward. The current instrument will be played backwards, or it will temporarily set the direction of a loop to go backward. 
				};
			};

			struct ITVOLCMD
			{
				enum
				{
					VOL_VOL_POSITION		=   0x0,
					VOL_FINE_VOLSLIDE_UP	=	0x41,
					VOL_FINE_VOL_SLIDE_DOWN	=	0x4B,
					VOL_VOL_SLIDE_UP		=	0x55,
					VOL_VOL_SLIDE_DOWN		=	0x5F,
					VOL_PITCH_SLIDE_DOWN	=	0x69,
					VOL_PITCH_SLIDE_UP		=	0x73,
					VOL_PAN_POSITION		=	0x80,
					VOL_PORTANOTE			=	0xC1,
					VOL_VIBRATO				=	0xCB 
				};
			};

			/*
			The following effects 'memorise' their previous values:
			(D/K/L), (E/F/G), (HU), I, J, N, O, S, T, W

			Commands H and U are linked even more closely.
			If you use H00 or U00, then the previous vibrato, no matter
			whether it was set with Hxx or Uxx will be used.


			Volume column effects are selected by pressing A-H in the first
			column of the effects.

			Ax = fine volume slide up by x
			Bx = fine volume slide down by x
			Cx = volume slide up by x
			Dx = volume slide down by x
			Ex = pitch slide down by x
			Fx = pitch slide up by x
			Gx = portamento to note with speed x
			Hx = vibrato with depth x

			// Volume ranges from 0->64
			// Panning ranges from 0->64, mapped onto 128->192
			// Prepare for the following also:
			//  65->74 = Fine volume up
			//  75->84 = Fine volume down
			//  85->94 = Volume slide up
			//  95->104 = Volume slide down
			//  105->114 = Pitch Slide down
			//  115->124 = Pitch Slide up
			//  193->202 = Portamento to
			//  203->212 = Vibrato
			In all cases, if x is 0, then the effect memory is used (as
			explained in (4)

			The memory for Ax/Bx/Cx/Dx are shared, as is the memory for
			Ex/Fx.
			*/


			/*
			//Some effects needed for XM compatibility
			int const static IT_XM_PORTAMENTO_DOWN    = 27;
			int const static IT_XM_PORTAMENTO_UP      = 28;
			int const static IT_XM_FINE_VOLSLIDE_DOWN = 29;
			int const static IT_XM_FINE_VOLSLIDE_UP   = 30;
			int const static IT_XM_RETRIGGER_NOTE     = 31;
			*/

			// little-endian values.
			static const unsigned long IMPM_ID=0x4D504D49;
			static const unsigned long IMPI_ID=0x49504D49;
			static const unsigned long IMPS_ID=0x49534D49;

			struct itHeader
			{
				std::uint32_t tag;
				char songName[26];
				unsigned short pHiligt,ordNum,insNum,sampNum,patNum,trackerV,ffv,flags,special;
				unsigned char gVol,mVol,iSpeed,iTempo,panSep,PWD;
				unsigned short msgLen;
				unsigned long msgOffset,reserved;
				unsigned char chanPan[64],chanVol[64];
			};
			struct Flags
			{
				enum
				{
					STEREO       = 0x1,
					VOLOPT       = 0x2,
					USEINSTR     = 0x4,
					LINEARSLIDES = 0x8,
					OLDEFFECTS   = 0x10,
					LINKGXXMEM   = 0x20,
					USEMIDIPITCH = 0x40,
					REQUESTMIDI  = 0x80
				};
			};
			struct SpecialFlags
			{
				enum
				{
					HASMESSAGE   = 0x01,
					MIDIEMBEDDED = 0x08
					// Other values are unused.
				};
			};
			struct ChanFlags
			{
				enum
				{
					IS_SURROUND = 0x64,
					IS_DISABLED = 0x80
				};
			};

			struct EmbeddedMIDIData
			{
				std::int8_t Start[32];
				std::int8_t Stop[32];
				std::int8_t Tick[32];
				std::int8_t NoteOn[32];
				std::int8_t NoteOff[32];
				std::int8_t Volume[32];
				std::int8_t Pan[32];
				std::int8_t BankChange[32];
				std::int8_t ProgramChange[32];
				std::int8_t SFx[16][32];
				char Zxx[128][32];
			};

			typedef std::pair<char,char> ITNotePair;
			typedef std::pair<char,char> ITNodePair1x;
			//		typedef std::pair<short,char> ITNodePair;
			class ITNodePair { public: std::int8_t first; std::uint8_t secondlo; std::uint8_t secondhi; };

			struct itInsHeader1x
			{
				std::uint32_t tag;
				char fileName[13];
				unsigned char flg,loopS,loopE,sustainS,sustainE;
				unsigned short fadeout;
				unsigned char NNA,DNC;
				unsigned short trackerV;
				unsigned char noS;
				char sName[26];
				char unused[6];
				ITNotePair notes[120];
				/// There is no clear docs about what this is for.
				char volEnv[200];
				ITNodePair1x nodepair[25];
			};

			struct ITEnvStruct
			{
				std::uint8_t flg,numP,loopS,loopE,sustainS,sustainE;
				ITNodePair nodes[25];
				std::uint8_t unused;
			};

			struct itInsHeader2x
			{
				std::uint32_t tag;
				char fileName[13];
				unsigned char NNA,DCT,DCA;
				unsigned short fadeout;
				unsigned char pPanSep,pPanCenter,gVol,defPan,randVol,randPan;
				unsigned short trackerV;
				unsigned char noS,unused;
				char sName[26];
				unsigned char inFC,inFR,mChn,mPrg;
				unsigned short mBnk;
				ITNotePair notes[120];
				ITEnvStruct volEnv;
				ITEnvStruct panEnv;
				ITEnvStruct pitchEnv;
			};

			struct EnvFlags // Same flags used for itInsHeader1x
			{
				enum
				{	
					USE_ENVELOPE = 0x01,
					USE_LOOP     = 0x02,
					USE_SUSTAIN  = 0x04,
					ENABLE_CARRY = 0x08,
					ISFILTER     = 0x80, // Only meaningful in a pitch envelope
				};
			};

			struct itSampleHeader
			{
				std::uint32_t tag;
				char fileName[13];
				unsigned char gVol,flg,vol;
				char sName[26];
				std::uint8_t cvt,dfp;
				std::uint32_t length,loopB,loopE,c5Speed,sustainB,sustainE,smpData;
				std::uint8_t vibS,vibD,vibR,vibT;
			};

			struct SampleFlags
			{
				enum
				{
					HAS_SAMPLE       = 0x01,
					IS16BIT          = 0x02,
					ISSTEREO         = 0x04,
					ISCOMPRESSED     = 0x08,
					USELOOP          = 0x10,
					USESUSTAIN       = 0x20,
					ISLOOPPINPONG    = 0x40,
					ISSUSTAINPINPONG = 0x80
				};
			};

			struct SampleConvert
			{
				enum
				{
					IS_SIGNED    = 0x01, ///< Only this one is used by Impulse Tracker. The others were for its sample importer.
					IS_MOTOROLA = 0x02,
					IS_DELTA	= 0x04,
					IS_BYTEDELTA = 0x08,
					IS_12bit	= 0x10,
					IS_PROMPT    = 0x20
				};
			};
		public:
			bool LoadITModule(Song *song);
			bool LoadOldITInst(XMSampler *sampler,int iInstIdx);
			bool LoadITInst(XMSampler *sampler,int iInstIdx);
			void LoadInstrumentFromFile(XMSampler & sampler, const int idx);
			bool LoadITSample(XMSampler *sampler,int iSampleIdx);
			bool LoadITSampleData(XMSampler *sampler,int iSampleIdx,unsigned int iLen,bool bstereo,bool b16Bit, unsigned char convert);
			bool LoadITCompressedData(XMSampler *sampler,int iSampleIdx,unsigned int iLen,bool b16Bit,unsigned char convert);
			bool LoadITPattern(int patIdx,int &numchans);
			void ParseEffect(PatternEntry&pent, int command,int param,int channel);
		private:
			unsigned char highOffset[64];
			EmbeddedMIDIData* embeddedData;
			itHeader itFileH;


			//////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////////////////////
			// S3M Fileformat 


		public:
			// little-endian
			static const std::uint32_t SCRM_ID = 0x4D524353;
			static const std::uint32_t SCRS_ID = 0x53524353;
			static const std::uint32_t SCRI_ID = 0x49524353;
			// big-endian
			//static const std::uint32_t SCRM_ID = 0x5343524D;
			//static const std::uint32_t SCRS_ID = 0x53435253;
			//static const std::uint32_t SCRI_ID = 0x53435249;

			struct s3mHeader
			{
				char songName[28];
				std::uint8_t end,type;
				std::uint16_t unused1;
				std::uint16_t ordNum,insNum,patNum,flags,trackerV,trackerInf;
				std::uint32_t tag; // SCRM
				std::uint8_t gVol,iSpeed,iTempo,mVol,uClick,defPan;
				std::uint8_t unused2[8];
				std::uint16_t pSpecial;
				std::uint8_t chanSet[32];
			};

			struct S3MFlags
			{
				enum
				{
					ST2VIBRATO  = 0x01, ///< Unused in ST3
					ST2TEMPO    = 0x02, ///< Unused in ST3
					AMIGASLIDES = 0x04, ///< Unused in ST3
					VOLOPT      = 0x08,
					AMIGALIMITS = 0x10,
					ENABLE_S0   = 0x20, ///< Unused in ST3
					SLIDEALWAYS = 0x40,
					CUSTOMDATA  = 0x80  ///< Unused in ST3?
				};
			};

			struct S3MChanType
			{
				enum
				{
					ISRIGHTCHAN  = 0x08, ///< Else, is Left Chan.
					ISADLIBCHAN  = 0x10,
					HASCUSTOMPOS = 0x20,
					ISDISABLED   = 0x80,
					ISUNUSED     = 0xFF
				};
			};

			struct s3mInstHeader
			{
				std::uint8_t type;
				char fileName[12],data[35],sName[28];
				std::uint32_t tag;
			};

			struct s3mSampleHeader
			{
				std::uint8_t type;
				char filename[12];
				unsigned char hiMemSeg;
				unsigned short lomemSeg;
				unsigned long length,loopb,loope;
				unsigned char vol;
				char unused;
				unsigned char packed,flags;
				unsigned long c2speed;
				long unused2,internal,internal2;
				char sName[28];
				/// SCRS
				std::uint32_t tag;
			};

			struct S3MSampleFlags
			{
				enum
				{
					LOOP    = 0x01,
					STEREO  = 0x02,
					IS16BIT = 0x04
				};
			};

			struct s3madlibheader
			{
				std::uint8_t type;
				char filename[12];
				std::uint8_t unused[3];
				std::uint8_t D00,D01,D02,D03,D04,D05,D06,D07,D08,D09,D0A,D0B,vol;
				std::uint8_t unused2[3];
				std::uint32_t c2speed;
				std::uint8_t unused3[12];
				char sName[28];
				/// SCRI
				std::uint32_t tag;
			};

		public:
			bool LoadS3MModuleX(Song *song);
			bool LoadS3MInstX(XMSampler *sampler,int iInstIdx);
			bool LoadS3MSampleX(XMSampler *sampler,s3mSampleHeader *currHeader,int iInstIdx,int iSampleIdx);
			bool LoadS3MSampleDataX(XMSampler *sampler,int iInstIdx,int iSampleIdx,unsigned int iLen,bool bstereo,bool b16Bit,bool packed);
			bool LoadS3MPatternX(int patIdx);
		private:
			s3mHeader  s3mFileH;
		};
	}
}
