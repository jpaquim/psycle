#pragma once
#include "SongStructs.hpp"
#include "FileIO.hpp"

class Song;

namespace psycle
	{
	namespace host
		{
		class XMSampler;

		class ITModule2 : public OldPsyFile
		{
		public:
			// Extra class for Reading of IT compressed samples.
			class BitsBlock {
			public:
				BitsBlock(){ pdata=NULL;rpos=NULL;rend=NULL;rembits=0; };
				~BitsBlock(){ zapArray(pdata); };
				bool ReadBlock(OldPsyFile* pFile);
				unsigned long ReadBits(unsigned char bitwidth);
			private:
				unsigned char* pdata; // pointer to data
				unsigned char* rpos; //read position
				unsigned char* rend; //read end.
				unsigned char rembits; // remaining bits in current pos.
			};

			ITModule2(void);
			~ITModule2(void);

//////////////////////////////////////////////////////////////////////////
///    IT Fileformat
		struct CMD
		{
			enum{
			 SET_SPEED=				1,
			 JUMP_TO_ORDER=			2,
			 BREAK_TO_ROW=			3,
			 VOLUME_SLIDE=			4,
			 PORTAMENTO_DOWN=		5,
			 PORTAMENTO_UP=			6,
			 TONE_PORTAMENTO=		7,
			 VIBRATO=				8,
			 TREMOR=					9,
			 ARPEGGIO=				10,
			 VOLSLIDE_VIBRATO=		11, // Dual command: H00 and Dxy 
			 VOLSLIDE_TONEPORTA=		12, // Dual command: G00 and Dxy
			 SET_CHANNEL_VOLUME=		13,
			 CHANNEL_VOLUME_SLIDE=	14,
			 SET_SAMPLE_OFFSET=		15,
			 PANNING_SLIDE=			16,
			 RETRIGGER_NOTE=			17,
			 TREMOLO=				18,
			 S=						19,
			 SET_SONG_TEMPO=			20, // T0x Slide tempo down . T1x slide tempo up
			 FINE_VIBRATO=			21,
			 SET_GLOBAL_VOLUME=		22,
			 GLOBAL_VOLUME_SLIDE=	23,
			 SET_PANNING=			24,
			 PANBRELLO=				25,
			 MIDI_MACRO=				26 //see MIDI.TXT
			};
		};
		struct CMD_S
		{
			enum{
			 S_SET_FILTER=				0, // Greyed out in IT...
			 S_SET_GLISSANDO_CONTROL=	1, // Greyed out in IT...
			 S_FINETUNE=				2, // Greyed out in IT...
			 S_SET_VIBRATO_WAVEFORM=	3,
			 S_SET_TREMOLO_WAVEFORM=	4,
			 S_SET_PANBRELLO_WAVEFORM=	5,
			 S_FINE_PATTERN_DELAY=		6,
			 S7=						7,
			 S_SET_PAN=					8,
			 S9=						9,
			 S_SET_HIGH_OFFSET=			10,
			 S_PATTERN_LOOP=			11,
			 S_DELAYED_NOTE_CUT=		12,
			 S_NOTE_DELAY=				13,
			 S_PATTERN_DELAY=			14,
			 S_SET_MIDI_MACRO=			15
			};
		};
		struct CMD_S7
		{
			enum{
			S7_BACKGROUNDNOTECUT=	0,
			S7_BACKGROUNDNOTEOFF=	1,
			S7_BACKGROUNDNOTEFADE=	2,
			S7_SETNOTECUT=			3,
			S7_SETNOTECONTINUE=		4,
			S7_SETNOTEOFF=			5,
			S7_SETNOTEFADE=			6,
			S7_VOLENVOFF=			7,
			S7_VOLENVON=			8,
			S7_PANENVOFF=			9,
			S7_PANENVON=			10,
			S7_PITCHENVON=			11,
			S7_PITCHENVOFF=			12
			};
		};
		struct CMD_S9
		{
			enum{
			S9_SURROUND_OFF=		0,
			S9_SURROUND_ON=			1
			};
		};

/*			//Some effects needed for XM compatibility
			#define IT_XM_PORTAMENTO_DOWN    27
			#define IT_XM_PORTAMENTO_UP      28
			#define IT_XM_FINE_VOLSLIDE_DOWN 29
			#define IT_XM_FINE_VOLSLIDE_UP   30
			#define IT_XM_RETRIGGER_NOTE     31
*/

		// little-endian values.
		static const unsigned long IMPM_ID=0x4D504D49;
		static const unsigned long IMPI_ID=0x49504D49;
		static const unsigned long IMPS_ID=0x49534D49;

		typedef struct {
			unsigned long tag;
			char songName[26];
			unsigned short pHiligt,ordNum,insNum,sampNum,patNum,trackerV,ffv,flags,special;
			unsigned char gVol,mVol,iSpeed,iTempo,panSep,PWD;
			unsigned short msgLen;
			unsigned long msgOffset,reserved;
			unsigned char chanPan[64],chanVol[64];
		}itHeader;
		struct Flags
		{
			enum {
				STEREO=			0x1,
				VOLOPT=			0x2,
				USEINSTR=		0x4,
				LINEARSLIDES=	0x8,
				OLDEFFECTS=		0x10,
				LINKGXXMEM=		0x20,
				USEMIDIPITCH=	0x40,
				REQUESTMIDI=	0x80
			};
		};
		struct SpecialFlags
		{
			enum
			{
				HASMESSAGE=		0x01,
				MIDIEMBEDDED=	0x08
				// Other values are unused.
			};
		};
		struct ChanFlags
		{
			enum
			{
				IS_SURROUND=	0x64,
				IS_DISABLED=	0x80
			};
		};
		typedef std::pair<char,char> ITNotePair;
		typedef std::pair<char,char> ITNodePair1x;
//		typedef std::pair<short,char> ITNodePair;
		class ITNodePair { public: __int8 first; __int8 secondlo; _int8 secondhi; };

		typedef struct {
			unsigned long tag;
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
		}itInsHeader1x;

		typedef struct {
			unsigned char flg,numP,loopS,loopE,sustainS,sustainE;
			ITNodePair nodes[25];
		}ITEnvStruct;

		typedef struct {
			unsigned long tag;
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
		}itInsHeader2x;
		struct EnvFlags // Same flags used for itInsHeader1x
		{
			enum
			{	
				USE_ENVELOPE=	0x01,
				USE_LOOP=		0x02,
				USE_SUSTAIN=	0x04,
				ISFILTER=		0x80, // Only meaningful in a pitch envelope
			};
		};
		typedef struct {
			unsigned long tag;
			char fileName[13];
			unsigned char gVol,flg,vol;
			char sName[26];
			unsigned char cvt,dfp;
			unsigned long length,loopB,loopE,c5Speed,sustainB,sustainE,smpData;
			unsigned char vibS,vibD,vibR,vibT;
		}itSampleHeader;
		struct SampleFlags
		{
			enum
			{
				HAS_SAMPLE=		0x01,
				IS16BIT=		0x02,
				ISSTEREO=		0x04,
				ISCOMPRESSED=	0x08,
				USELOOP=		0x10,
				USESUSTAIN=		0x20,
				ISLOOPPINPONG=	0x40,
				ISSUSTAINPINPONG=0x80
			};
		};
		struct SampleConvert
		{
			enum
			{
				IS_SIGNED	= 0x01,	// Only this one is used by Impulse Tracker.
				IS_MOTOROLA = 0x02,
				IS_DELTA	= 0x04,
				IS_BYTEDELTA= 0x08,
				IS_12bit	= 0x10,
				IS_PROMPT	= 0x20,
			};
		};
		public:
			bool LoadITModule(Song *song);
			bool LoadOldITInst(XMSampler *sampler,int iInstIdx);
			bool LoadITInst(XMSampler *sampler,int iInstIdx);
			bool LoadITSample(XMSampler *sampler,int iSampleIdx);
			bool LoadITSampleData(XMSampler *sampler,int iSampleIdx,unsigned int iLen,bool bstereo,bool b16Bit, unsigned char convert);
			bool LoadITCompressedData(XMSampler *sampler,int iSampleIdx,unsigned int iLen,bool b16Bit);
			bool LoadITPattern(int patIdx);
			
//////////////////////////////////////////////////////////////////////////
///    S3M Fileformat 
			
			
		// little-endian
		static const unsigned long SCRM_ID=0x4D524353;
		static const unsigned long SCRS_ID=0x53524353;
		static const unsigned long SCRI_ID=0x49524353;
		// big-endian
		//static unsigned long SCRM_ID=0x5343524D;
		//static unsigned long SCRS_ID=0x53435253;
		//static unsigned long SCRI_ID=0x53435249;
		typedef struct {
			char songName[28];
			unsigned char end,type;
			short unused1;
			unsigned short ordNum,insNum,patNum,flags,trackerV,trackerInf;
			unsigned long tag; // SCRM
			unsigned char gVol,iSpeed,iTempo,mVol,uClick,defPan;
			char unused2[8];
			unsigned short pSpecial;
			unsigned char chanSet[32];
		}s3mHeader;
		struct S3MFlags
		{
			enum
			{
				ST2VIBRATO=		0x01,// Unused in ST3
				ST2TEMPO=		0x02,// Unused in ST3
				AMIGASLIDES=	0x04,// Unused in ST3
				VOLOPT=			0x08,
				AMIGALIMITS=	0x10,
				ENABLE_S0=		0x20,// Unused in ST3
				SLIDEALWAYS=	0x40,
				CUSTOMDATA=		0x80 // Unused in ST3?
			};
		};
		struct S3MChanType
		{
			enum
			{
				ISRIGHTCHAN=	0x08, // Else, is Left Chan.
				ISADLIBCHAN=	0x10,
				HASCUSTOMPOS=	0x20,
				ISENABLED=		0x80
			};
		};
		typedef struct {
			unsigned char type;
			char fileName[12],data[35],sName[28];
			unsigned long tag;
		}s3mInstHeader;

		typedef struct {
			unsigned char type;
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
			unsigned long tag; // SCRS
		}s3mSampleHeader;
		struct S3MSampleFlags
		{
			enum
			{
				LOOP=		0x01,
				STEREO=		0x02,
				IS16BIT=	0x04
			};
		};
		typedef struct {
			unsigned char type;
			char filename[12],unused[3];
			unsigned char D00,D01,D02,D03,D04,D05,D06,D07,D08,D09,D0A,D0B,vol;
			char unused2[3];
			unsigned long c2speed;
			char unused3[12],sName[28];
			unsigned long tag; // SCRI
		}s3madlibheader;

		public:
			bool LoadS3MModuleX(Song *song);
			bool LoadS3MInstX(XMSampler *sampler,int iInstIdx);
			bool LoadS3MSampleX(XMSampler *sampler,s3mSampleHeader *currHeader,int iInstIdx,int iSampleIdx);
			bool LoadS3MSampleDataX(XMSampler *sampler,int iInstIdx,int iSampleIdx,unsigned int iLen,bool bstereo,bool b16Bit,bool packed);
			bool LoadS3MPatternX(int patIdx);
		private:
			s3mHeader  s3mFileH;
			itHeader itFileH;
			Song* s;
		};
	}
}