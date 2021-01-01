// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_ITMODULE2_H
#define psy_audio_ITMODULE2_H

#include "../../detail/psydef.h"

// local
#include "machine.h"
#include "songio.h"

#ifdef __cplusplus
extern "C" {
#endif

// ITModule2
// mfc-psycle: ITModule2.h/.cpp
//
// Imports it2 files using sampulse

// little-endian values.
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
	#define IMPM_ID 0x4D504D49
	#define IMPI_ID 0x49504D49
	#define IMPS_ID 0x53504D49
#else
	#define IMPM_ID 0x494D504D
	#define IMPI_ID 0x494D5049
	#define IMPS_ID 0x494D5053
#endif

enum {
	IT2_FLAGS_STEREO = 0x1,
	IT2_FLAGS_VOLOPT = 0x2,
	IT2_FLAGS_USEINSTR = 0x4,
	IT2_FLAGS_LINEARSLIDES = 0x8,
	IT2_FLAGS_OLDEFFECTS = 0x10,
	IT2_FLAGS_LINKGXXMEM = 0x20,
	IT2_FLAGS_USEMIDIPITCH = 0x40,
	IT2_FLAGS_REQUESTMIDI = 0x80
};

enum {
	IT2_SPECIAL_FLAGS_HASMESSAGE = 0X01,
	IT2_SPECIAL_FLAGS_MIDIEMBEDDED = 0x08
	// Other values are unused.
};

enum {	
	IT2_CHAN_FLAGS_IS_SURROUND = 0x64,
	IT2_CHAN_FLAGS_IS_DISABLED = 0x80
};

typedef struct ITHeader
{
	uint32_t tag;
	char songName[26];
	uint16_t pHiligt, ordNum, insNum, sampNum, patNum, trackerV, ffv, flags, special;
	uint8_t gVol, mVol, iSpeed, iTempo, panSep, PWD;
	uint16_t msgLen;
	uint32_t msgOffset, reserved;
	uint8_t chanPan[64], chanVol[64];
} ITHeader;

typedef struct EmbeddedMIDIData {
	char Start[32];
	char Stop[32];
	char Tick[32];
	char NoteOn[32];
	char NoteOff[32];
	char Volume[32];
	char Pan[32];
	char BankChange[32];
	char ProgramChange[32];
	char SFx[16][32];
	char Zxx[128][32];
} EmbeddedMIDIData;

struct ITNotePair { uint8_t first; uint8_t second; };
struct ITNodePair1x { uint8_t first; uint8_t second; };
struct ITNodePair { int8_t first; uint8_t secondlo; uint8_t secondhi; };

typedef struct itInsHeader1x {
	uint32_t tag;
	char fileName[13];
	uint8_t flg, loopS, loopE, sustainS, sustainE;
	uint8_t unused1[2];
	uint16_t fadeout;
	uint8_t NNA, DNC;
	uint16_t trackerV;
	uint8_t noS;
	uint8_t unused2;
	char sName[26];
	uint8_t unused[6];
	struct ITNotePair notes[120];
	/// value of the volume for each tick. (Seems it was probably an internal IT thing to represent the envelope)
	uint8_t volEnv[200];
	/// Each of the defined nodes (tick index, value). value ranges from 0 to 64 (decimal). 0xFF is end of envelope.
	struct ITNodePair1x nodepair[25];
} itInsHeader1x;

struct ITEnvStruct {
	uint8_t flg, numP, loopS, loopE, sustainS, sustainE;
	struct ITNodePair nodes[25];
	uint8_t unused;
};

typedef struct itInsHeader2x {
	uint32_t tag;
	char fileName[13];
	uint8_t NNA, DCT, DCA;
	uint16_t fadeout;
	uint8_t pPanSep, pPanCenter, gVol, defPan, randVol, randPan;
	uint16_t trackerV;
	uint8_t noS, unused;
	char sName[26];
	uint8_t inFC, inFR, mChn, mPrg;
	uint16_t mBnk;
	struct ITNotePair notes[120];
	struct ITEnvStruct volEnv;
	struct ITEnvStruct panEnv;
	struct ITEnvStruct pitchEnv;
} itInsHeader2x;

typedef struct itSampleHeader {
	uint32_t tag;
	char fileName[13];
	uint8_t gVol, flg, vol;
	char sName[26];
	uint8_t cvt, dfp;
	uint32_t length, loopB, loopE, c5Speed, sustainB, sustainE, smpData;
	uint8_t vibS, vibD, vibR, vibT;
} itSampleHeader;

typedef struct s3mHeader {
	char songName[28];
	uint8_t end, type;
	uint8_t unused1[2];
	uint16_t ordNum, insNum, patNum, flags, trackerV, trackerInf;
	uint32_t tag; // SCRM
	uint8_t gVol, iSpeed, iTempo, mVol, uClick, defPan;
	uint8_t unused2[8];
	uint16_t pSpecial;
	uint8_t chanSet[32];
} s3mHeader;

typedef struct s3mSampleHeader {
	uint8_t type;
	char filename[12];
	uint8_t hiMemSeg;
	uint16_t lomemSeg;
	uint32_t length, loopb, loope;
	uint8_t vol;
	uint8_t unused;
	uint8_t packed, flags; //[P]ack   0=unpacked, 1=DP30ADPCM packing (not used by ST3.01)
	uint32_t c2speed;
	uint32_t unused2, internal1, internal2;
	char sName[28];
	/// SCRS
	uint32_t tag;
} s3mSampleHeader;

enum {
	IT2_SAMPLE_FLAGS_HAS_SAMPLE = 0x01,
	IT2_SAMPLE_FLAGS_IS16BIT = 0x02,
	IT2_SAMPLE_FLAGS_ISSTEREO = 0x04,
	IT2_SAMPLE_FLAGS_ISCOMPRESSED = 0x08,
	IT2_SAMPLE_FLAGS_USELOOP = 0x10,
	IT2_SAMPLE_FLAGS_USESUSTAIN = 0x20,
	IT2_SAMPLE_FLAGS_ISLOOPPINPONG = 0x40,
	IT2_SAMPLE_FLAGS_ISSUSTAINPINPONG = 0x80
};

enum {
	IT2_SAMPLE_CONVERT_IS_SIGNED = 0x01, ///< Only this one is used by Impulse Tracker. The others were for its sample importer.
	IT2_SAMPLE_CONVERTIS_MOTOROLA = 0x02,
	IT2_SAMPLE_CONVERTIS_DELTA = 0x04,
	IT2_SAMPLE_CONVERTIS_BYTEDELTA = 0x08,
	IT2_SAMPLE_CONVERTIS_12bit = 0x10,
	IT2_SAMPLE_CONVERTIS_PROMPT = 0x20
};

/*Generic header. casted to one of the other two depending on "type" (or "tag") value*/
typedef struct s3mInstHeader {
	uint8_t type;
	char fileName[12];
	uint8_t data[35];
	char sName[28];
	uint32_t tag;
} s3mInstHeader;

enum {
	S3M_SAMPLE_FLAGS_LOOP = 0x01,
	S3M_SAMPLE_FLAGS_STEREO = 0x02,
	S3M_SAMPLE_FLAGS_IS16BIT = 0x04
};

// Extra class for Reading of IT compressed samples.
typedef struct BitsBlock {
	/// pointer to data
	uint8_t* pdata;
	/// read position
	uint8_t* rpos;
	/// read end
	uint8_t* rend;
	/// remaining bits in current pos
	uint8_t rembits;
} BitsBlock;

INLINE void bitsblock_init(BitsBlock* self)
{
	self->pdata = 0;
	self->rpos = 0;
	self->rend = 0;
	self->rembits = 0;
}

INLINE void bitsblock_dispose(BitsBlock* self)
{
	free(self->pdata);
}

bool bitsblock_readblock(BitsBlock*, PsyFile*);
uint32_t bitsblock_readbits(BitsBlock*, unsigned char bitwidth);
	

enum {
	IT2_CMD_SET_SPEED = 1,
	IT2_CMD_JUMP_TO_ORDER = 2,
	IT2_CMD_BREAK_TO_ROW = 3,
	IT2_CMD_VOLUME_SLIDE = 4,
	IT2_CMD_PORTAMENTO_DOWN = 5,
	IT2_CMD_PORTAMENTO_UP = 6,
	IT2_CMD_TONE_PORTAMENTO = 7,
	IT2_CMD_VIBRATO = 8,
	IT2_CMD_TREMOR = 9,
	IT2_CMD_ARPEGGIO = 10,
	IT2_CMD_VOLSLIDE_VIBRATO = 11, ///< Dual command: H00 and Dxy 
	IT2_CMD_VOLSLIDE_TONEPORTA = 12, ///< Dual command: G00 and Dxy
	IT2_CMD_SET_CHANNEL_VOLUME = 13,
	IT2_CMD_CHANNEL_VOLUME_SLIDE = 14,
	IT2_CMD_SET_SAMPLE_OFFSET = 15,
	IT2_CMD_PANNING_SLIDE = 16,
	IT2_CMD_RETRIGGER_NOTE = 17,
	IT2_CMD_TREMOLO = 18,
	IT2_CMD_S = 19,
	IT2_CMD_SET_SONG_TEMPO = 20, ///< T0x Slide tempo down . T1x slide tempo up
	IT2_CMD_FINE_VIBRATO = 21,
	IT2_CMD_SET_GLOBAL_VOLUME = 22,
	IT2_CMD_GLOBAL_VOLUME_SLIDE = 23,
	IT2_CMD_SET_PANNING = 24,
	IT2_CMD_PANBRELLO = 25,
	IT2_CMD_MIDI_MACRO = 26  ///< see MIDI.TXT ([bohan] uhu? what file?)
};

enum {
	IT2_CMD_S_SET_FILTER = 0x00, ///< Greyed out in IT...
	IT2_CMD_S_SET_GLISSANDO_CONTROL = 0x10, ///< Greyed out in IT...
	IT2_CMD_S_FINETUNE = 0x20, ///< Greyed out in IT...
	IT2_CMD_S_SET_VIBRATO_WAVEFORM = 0x30, ///< Check XMInstrument::WaveData::WaveForms! IT is sine, square, sawdown and random
	IT2_CMD_S_SET_TREMOLO_WAVEFORM = 0x40, ///< Check XMInstrument::WaveData::WaveForms! IT is sine, square, sawdown and random
	IT2_CMD_S_SET_PANBRELLO_WAVEFORM = 0x50, ///< Check XMInstrument::WaveData::WaveForms! IT is sine, square, sawdown and random
	IT2_CMD_S_FINE_PATTERN_DELAY = 0x60, ///< causes a "pause" of x ticks ( i.e. the current row becomes x ticks longer)
	IT2_CMD_S7 = 0x70,
	IT2_CMD_S_SET_PAN = 0x80,
	IT2_CMD_S9 = 0x90,
	IT2_CMD_S_SET_HIGH_OFFSET = 0xA0,
	IT2_CMD_S_PATTERN_LOOP = 0xB0,
	IT2_CMD_S_DELAYED_NOTE_CUT = 0xC0,
	IT2_CMD_S_NOTE_DELAY = 0xD0,
	IT2_CMD_S_PATTERN_DELAY = 0xE0, ///< causes a "pause" of x rows ( i.e. the current row becomes x rows longer)
	IT2_CMD_S_SET_MIDI_MACRO = 0xF0
};

// ITModule2
typedef struct ITModule2 {
	// internal data
	ITHeader fileheader;
	int16_t extracolumn;
	int16_t maxextracolumn;
	psy_audio_Machine* sampler;
	EmbeddedMIDIData* embeddeddata;
	uint8_t highoffset[64];
	psy_Table xmtovirtual;
	s3mHeader  s3mFileH;
	// references
	psy_audio_SongFile* songfile;
} ITModule2;

void itmodule2_init(ITModule2*, psy_audio_SongFile*);
void itmodule2_dispose(ITModule2*);

bool itmodule2_load(ITModule2*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_ITMODULE2_H */
