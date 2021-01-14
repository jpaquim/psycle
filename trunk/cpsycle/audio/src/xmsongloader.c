// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "xmsongloader.h"
// local
#include "constants.h"
#include "psyconvert.h"
#include "instrument.h"
#include "instruments.h"
#include "machinefactory.h"
#include "sample.h"
#include "song.h"
#include "xmsampler.h"
// dsp
#include <operations.h>
// std
#include <assert.h>
#include <stdlib.h>
#include <string.h>
// platform
#include "../../detail/portable.h"

typedef bool BOOL;

// default sampulse group
#define INSTRUMENTGROUP 1 

#define ASSERT assert

static unsigned char ReadUInt1Start(PsyFile* fp, const int32_t start)
{
	if (start >= 0) psyfile_seek(fp, start);
	return psyfile_read_uint8(fp);	
}

static unsigned short ReadUInt2Start(PsyFile* fp, const int32_t start)
{
	if (start >= 0) psyfile_seek(fp, start);
	return psyfile_read_uint16(fp);	
}

static unsigned int ReadUInt4Start(PsyFile* fp, const int32_t start)
{
	if (start >= 0) psyfile_seek(fp, start);
	return psyfile_read_uint32(fp);	
}

static unsigned char ReadInt1Start(PsyFile* fp, const int32_t start)
{
	if (start >= 0) psyfile_seek(fp, start);
	return psyfile_read_int8(fp);
}

static unsigned short ReadInt2Start(PsyFile* fp, const int32_t start)
{
	if (start >= 0) psyfile_seek(fp, start);
	return psyfile_read_int16(fp);
}

static unsigned int ReadInt4Start(PsyFile* fp, const int32_t start)
{
	if (start >= 0) psyfile_seek(fp, start);
	return psyfile_read_int32(fp);
}

static char* AllocReadStr(PsyFile* fp, int size)
{	
	int start = -1;
	// allocate space
	char* pData = (char*)malloc(size + 1);
	if (pData == NULL)
		return NULL;

	// null terminate
	pData[size] = 0;

	// go to offset
	if (start >= 0)
		psyfile_seek(fp, start);

	// read data
	if (psyfile_read(fp, pData, size))
		return pData;

	free(pData);
	return NULL;
}

static char* AllocReadStrStart(PsyFile* fp, int size, signed int start)
{	
	// allocate space
	char* pData = (char*)malloc(size + 1);
	if (pData == NULL)
		return NULL;

	// null terminate
	pData[size] = 0;

	// go to offset
	if (start >= 0)
		psyfile_seek(fp, start);

	// read data
	if (psyfile_read(fp, pData, size))
		return pData;

	free(pData);
	return NULL;
}

	
static int16_t const BIGMODPERIODTABLE[37*8] = //((12note*3oct)+1note)*8fine
{
//-8fine(also note minus one).......Note........................+7fine
	907,900,894,887,881,875,868,862,856,850,844,838,832,826,820,814,
	808,802,796,791,785,779,774,768,762,757,752,746,741,736,730,725,
	720,715,709,704,699,694,689,684,678,675,670,665,660,655,651,646,
	640,636,632,628,623,619,614,610,604,601,597,592,588,584,580,575,
	570,567,563,559,555,551,547,543,538,535,532,528,524,520,516,513,
	508,505,502,498,494,491,487,484,480,477,474,470,467,463,460,457,
	453,450,447,444,441,437,434,431,428,425,422,419,416,413,410,407,
	404,401,398,395,392,390,387,384,381,379,376,373,370,368,365,363,
	360,357,355,352,350,347,345,342,339,337,335,332,330,328,325,323,
	320,318,316,314,312,309,307,305,302,300,298,296,294,292,290,288,
	285,284,282,280,278,276,274,272,269,268,266,264,262,260,258,256,
	254,253,251,249,247,245,244,242,240,239,237,235,233,232,230,228,
	226,225,224,222,220,219,217,216,214,212,211,209,208,206,205,203,
	202,200,199,198,196,195,193,192,190,189,188,187,185,184,183,181,
	180,179,177,176,175,174,172,171,170,169,167,166,165,164,163,161,
	160,159,158,157,156,155,154,152,151,150,149,148,147,146,145,144,
	143,142,141,140,139,138,137,136,135,134,133,132,131,130,129,128,
	127,126,125,125,123,123,122,121,120,119,118,118,117,116,115,114,
	113,113,112,111,110,109,109,108
};

static int calclpbfromspeed(int trackerspeed, int* outextraticks)
{
	int lpb;

	if (trackerspeed == 0) {
		trackerspeed = 6;
		lpb = 4;
	} else if (trackerspeed == 5) {
		lpb = 6;
	} else {
		lpb = (int)ceil(24.f / trackerspeed);
	}
	*outextraticks = trackerspeed - 24 / lpb;
	return lpb;
}

// XMSongLoader
// prototypes
static void xmsongloader_reset(XMSongLoader*);
static bool xmsongloader_isvalid(XMSongLoader*);
static bool xmsongloader_makexmsampler(XMSongLoader*);
static int xmsongloader_readtitle(XMSongLoader*);
static void xmsongloader_setsongcomments(XMSongLoader*);
static int xmsongloader_loadinstruments(XMSongLoader*, uintptr_t instr_start);
static int xmsongloader_loadinstrument(XMSongLoader*, uintptr_t slot,
	uintptr_t instr_start, uintptr_t* rv_nextstart);
static int xmsongloader_loadsampledata(XMSongLoader*, psy_audio_Sample*,
	int32_t sampleidx, uintptr_t start, uintptr_t* rv_nextstart);
static void xmsongloader_setenvelopes(psy_audio_Instrument*,
	const XMSAMPLEHEADER*);
static int xmsongloader_loadsampleheader(XMSongLoader*, psy_audio_Sample*,
	uintptr_t start, int32_t sampleidx, uintptr_t* rv_nextstart);
static int xmsongloader_loadpatterns(XMSongLoader*,
	uintptr_t* rv_nextstart);
static void xmsongloader_setsamplerslidemode(XMSongLoader*);
static void xmsongloader_setsamplerpanningmode(XMSongLoader*);
static void xmsongloader_makesequence(XMSongLoader*, struct XMFILEHEADER*);
static int xmsongloader_loadsinglepattern(XMSongLoader*, uintptr_t start,
	int patidx, int tracks, uintptr_t* rv_nextstart);
static bool xmsongloader_writepatternentry(XMSongLoader*,
	psy_audio_LegacyPattern, int row, int col, psy_audio_LegacyPatternEntry*);
// implementation
void xmsongloader_init(XMSongLoader* self, psy_audio_SongFile* songfile)
{
	assert(self);

	assert(songfile);
	assert(songfile->file);
	assert(songfile->song);

	self->songfile = songfile;
	self->fp = songfile->file;
	self->song = songfile->song;
	psy_table_init(&self->xmtovirtual);
	xmsongloader_reset(self);
}

void xmsongloader_dispose(XMSongLoader* self)
{
	assert(self);

	psy_table_dispose(&self->xmtovirtual);
}

void xmsongloader_reset(XMSongLoader* self)
{
	assert(self->songfile);
	assert(self->songfile->file);
	assert(self->songfile->song);

	self->fp = self->songfile->file;
	self->song = self->songfile->song;
	memset(self->highoffset, 0, sizeof(self->highoffset));
	memset(self->memportaup, 0, sizeof(self->memportaup));
	memset(self->memportadown, 0, sizeof(self->memportadown));
	memset(self->memportanote, 0, sizeof(self->memportanote));
	memset(self->memportapos, 0, sizeof(self->memportapos));
	memset(self->smplen, 0, sizeof(self->smplen));
	memset(self->smpflags, 0, sizeof(self->smpflags));
	psy_table_clear(&self->xmtovirtual);	
}

int xmsongloader_load(XMSongLoader* self)
{	
	int status;
	uintptr_t instr_start;
	
	status = PSY_OK;	
	if(!xmsongloader_isvalid(self)) {
		return PSY_ERRFILE;
	}
	xmsongloader_reset(self);
	if (!xmsongloader_makexmsampler(self)) {
		return PSY_ERRFILE;
	}
	if (status = xmsongloader_readtitle(self)) {
		return status;
	}
	xmsongloader_setsongcomments(self);		
	if (status = xmsongloader_loadpatterns(self, &instr_start)) {
		return status;
	}
	if (status = xmsongloader_loadinstruments(self, instr_start)) {
		return status;
	}
	return status;
}

bool xmsongloader_isvalid(XMSongLoader* self)
{
	char id[18];

	// get header
	memset(id, 0, 18);
	if (psyfile_seek(self->fp, 0) == -1) {
		return FALSE;
	}
	if (!psyfile_read(self->fp, id, 17)) {
		return FALSE;
	}
	// check header
	if (strncmp(id, XM_HEADER, sizeof(XM_HEADER) - 1) != 0) {
		return FALSE;
	}
	return TRUE;
}

bool xmsongloader_makexmsampler(XMSongLoader* self)
{
	self->sampler = psy_audio_machinefactory_makemachine(
		self->songfile->song->machinefactory, MACH_XMSAMPLER, "",
		psy_INDEX_INVALID);
	if (self->sampler) {
		psy_audio_Wire wire;

		psy_audio_machine_setposition(self->sampler, rand() / 64, rand() / 80);
		psy_audio_machines_insert(&self->song->machines, 0, self->sampler);
		wire = psy_audio_wire_make(0, psy_audio_MASTER_INDEX);
		psy_audio_machines_connect(&self->song->machines, wire);
		psy_audio_connections_setwirevolume(&self->song->machines.connections,
			wire, 0.355f); //-9dB
	}
	return self->sampler != NULL;
}

int xmsongloader_readtitle(XMSongLoader* self)
{
	char songname[21];

	// get song name
	memset(songname, 0, 21);
	if (psyfile_seek(self->fp, 17) == -1) {
		return PSY_ERRFILE;
	}
	if (!psyfile_read(self->fp, songname, 20)) {
		return PSY_ERRFILE;
	}
	psy_audio_song_settitle(self->song, songname);
	return PSY_OK;
}

void xmsongloader_setsongcomments(XMSongLoader* self)
{
	char_dyn_t* comments;
	
	comments = strdup("Imported from FastTracker II Module: ");
	comments = psy_strcat_realloc(comments, self->songfile->path);
	psy_audio_song_setcomments(self->song, comments);
	free(comments);
	comments = NULL;	
}

int xmsongloader_loadpatterns(XMSongLoader* self, uintptr_t* rv_nextstart)
{	
	int xmidx;
	int i;
	int extraticks;
	uintptr_t nextpatstart;
	int j;

	// get data
	if (psyfile_seek(self->fp, 60) == -1) {
		return PSY_ERRFILE;
	}
	if (!psyfile_read(self->fp, &self->header, sizeof(XMFILEHEADER))) {
		return PSY_ERRFILE;
	}	
	xmsongloader_setsamplerslidemode(self);	
	xmsongloader_setsamplerpanningmode(self);
	psy_audio_song_setnumsongtracks(self->song,
		psy_max(self->header.channels, 4));	
	self->instrcount = self->header.instruments;
	psy_audio_song_setbpm(self->song, self->header.tempo);
	psy_audio_song_settpb(self->song, 24);
	extraticks = 0;
	psy_audio_song_setlpb(self->song,
		calclpbfromspeed(self->header.speed, &extraticks));
	if (extraticks != 0) {
		psy_audio_song_setextraticksperbeat(self->song, extraticks);		
	}		
	self->maxextracolumn = (int16_t)psy_audio_song_numsongtracks(self->song);
	// Since in XM we load first the patterns, we initialize the map linearly.
	// Later, on the instruments, we will only create the virtual instruments
	// of existing sampled instruments
	xmidx = 1;
	for (i = MAX_MACHINES; i < MAX_VIRTUALINSTS &&
			xmidx <= self->header.instruments; ++i, ++xmidx) {
		psy_table_insert(&self->xmtovirtual, xmidx, (void*)(uintptr_t)i);
	}
	// get pattern data
	nextpatstart = ((uintptr_t)self->header.size + 60);
	for (j = 0; j < self->header.patterns && nextpatstart > 0; ++j) {
		int status;

		if (status = xmsongloader_loadsinglepattern(self, nextpatstart, j,
				self->header.channels, &nextpatstart)) {
			return status;
		}
	}
	psy_audio_song_setnumsongtracks(self->song, self->maxextracolumn);	
	xmsongloader_makesequence(self, &self->header);
	*rv_nextstart = nextpatstart;
	return PSY_OK;
}

void xmsongloader_setsamplerslidemode(XMSongLoader* self)
{
	psy_audio_MachineParam* param;

	param = psy_audio_machine_tweakparameter(self->sampler,
		XM_SAMPLER_TWK_AMIGASLIDES);
	if (param) {
		psy_audio_machine_parameter_tweak_scaled(self->sampler, param,
			(self->header.flags & 0x01)
			? FALSE
			: TRUE);
	}
}

void xmsongloader_setsamplerpanningmode(XMSongLoader* self)
{
	psy_audio_MachineParam* param;

	param = psy_audio_machine_tweakparameter(self->sampler,
		XM_SAMPLER_TWK_PANNINGMODE);
	if (param) {
		// XM_SAMPLER_PanningMode::TwoWay
		psy_audio_machine_parameter_tweak_scaled(self->sampler, param, 1);
	}
}

void xmsongloader_makesequence(XMSongLoader* self, struct XMFILEHEADER* xmheader)
{
	uint16_t i;	
	psy_audio_Song* song;

	song = self->songfile->song;	
	psy_audio_sequence_appendtrack(&song->sequence, psy_audio_sequencetrack_allocinit());
	for (i = 0; i < xmheader->norder; ++i) {		
		psy_audio_sequence_insert(&self->songfile->song->sequence,
			psy_audio_orderindex_make(0, i), xmheader->order[i]);
	}
}

// rv_nextstart: returns address of next pattern, 0 for invalid
int xmsongloader_loadsinglepattern(XMSongLoader* self, uintptr_t start,
	int patidx, int tracks, uintptr_t* rv_nextstart)
{
	int32_t headerlen;
	int16_t numrows;
	int16_t packedsize;
	uint16_t lastmach[64];
	int temp;
	psy_audio_LegacyPattern ppattern;
	psy_audio_Pattern* pattern;
	psy_audio_LegacyPatternEntry e;
	
	if (start >= 0) {
		if (psyfile_seek(self->fp, (uint32_t)start) == -1) {
			return PSY_ERRFILE;
		}
	}
	if (!psyfile_read(self->fp, &headerlen, sizeof(int32_t))) {
		return PSY_ERRFILE;
	}
	//char iPackingType = psyfile_read_int8();
	if (psyfile_skip(self->fp, 1) == -1) {
		return PSY_ERRFILE;
	}
	if (!psyfile_read(self->fp, &numrows, sizeof(int16_t))) {
		return PSY_ERRFILE;
	}
	if (!psyfile_read(self->fp, &packedsize, sizeof(int16_t))) {
		return PSY_ERRFILE;
	}	
	memset(lastmach, 255, sizeof(char) * 64);

	// create new psycle version pattern
	pattern = psy_audio_pattern_allocinit();
	psy_audio_pattern_setname(pattern, "unnamed");
	psy_audio_patterns_insert(&self->song->patterns, patidx, pattern);
	psy_audio_pattern_setlength(pattern,
		numrows * 1.0 / psy_audio_song_lpb(self->song));
	// create old mfc-psycle pattern for read
	ppattern = psy_audio_allocoldpattern(pattern,
		psy_audio_song_lpb(self->song), &temp);
	numrows = (int16_t)temp;

	if (packedsize == 0) {
		//Pattern is emtpy.
	} else {		
		int col;
		int row;
		psy_audio_PatternNode* node;

		// get next values
		for (row = 0; row < numrows; ++row) {
			self->extracolumn = tracks;
			for (col = 0; col < tracks; ++col) {
				// reset				
				uint8_t note;
				uint8_t instr;
				uint8_t vol;
				uint8_t type;
				uint8_t param;
				uint8_t volume;
				int exchwave[3] = {
					psy_audio_WAVEFORMS_SINUS,
					psy_audio_WAVEFORMS_SAWDOWN,
					psy_audio_WAVEFORMS_SQUARE
				};

				note = PSY2_NOTECOMMANDS_EMPTY;
				instr = 255;
				vol = 0;
				type = 0;
				param = 0;
				volume = 0;

				// read note
				if (!psyfile_read(self->fp, &note, sizeof(uint8_t))) {
					free(ppattern);
					return PSY_ERRFILE;
				}				

				// is compression bit set?
				if (note & 0x80)
				{
					unsigned char bReadNote;
					unsigned char bReadInstr;
					unsigned char bReadVol;
					unsigned char bReadType;
					unsigned char bReadParam;

					bReadNote = note & 0x01;
					bReadInstr = note & 0x02;
					bReadVol = note & 0x04;
					bReadType = note & 0x08;
					bReadParam = note & 0x10;

					note = 0;
					if (bReadNote) {						
						if (!psyfile_read(self->fp, &note, sizeof(uint8_t))) {
							free(ppattern);
							return PSY_ERRFILE;
						}
					}
					if (bReadInstr) {
						if (!psyfile_read(self->fp, &instr, sizeof(uint8_t))) {
							free(ppattern);
							return PSY_ERRFILE;
						}						
					}
					if (bReadVol) {
						if (!psyfile_read(self->fp, &vol, sizeof(uint8_t))) {
							free(ppattern);
							return PSY_ERRFILE;
						}
					}
					if (bReadType) {
						if (!psyfile_read(self->fp, &type, sizeof(uint8_t))) {
							free(ppattern);
							return PSY_ERRFILE;
						}
					}
					if (bReadParam) {
						if (!psyfile_read(self->fp, &param, sizeof(uint8_t))) {
							free(ppattern);
							return PSY_ERRFILE;
						}
					}
				} else
				{
					// read all values
					if (!psyfile_read(self->fp, &instr, sizeof(uint8_t))) {
						free(ppattern);
						return PSY_ERRFILE;
					}
					if (!psyfile_read(self->fp, &vol, sizeof(uint8_t))) {
						free(ppattern);
						return PSY_ERRFILE;
					}
					if (!psyfile_read(self->fp, &type, sizeof(uint8_t))) {
						free(ppattern);
						return PSY_ERRFILE;
					}
					if (!psyfile_read(self->fp, &param, sizeof(uint8_t))) {
						free(ppattern);
						return PSY_ERRFILE;
					}					
				}

				volume = 255;

				// volume/command
				if (vol >= 0x10 && vol <= 0x50)
				{
					volume = (vol == 0x50) ? 0x3F : (vol - 0x10);
				} else if (vol >= 0x60) {
					switch (vol & 0xF0)
					{
					case XMV_VOLUMESLIDEDOWN:
						volume = XM_SAMPLER_CMD_VOL_VOLSLIDEDOWN | (vol & 0x0F);
						break;
					case XMV_VOLUMESLIDEUP:
						volume = XM_SAMPLER_CMD_VOL_VOLSLIDEUP | (vol & 0x0F);
						break;
					case XMV_FINEVOLUMESLIDEDOWN:
						volume = XM_SAMPLER_CMD_VOL_FINEVOLSLIDEDOWN | (vol & 0x0F);
						break;
					case XMV_FINEVOLUMESLIDEUP:
						volume = XM_SAMPLER_CMD_VOL_FINEVOLSLIDEUP | (vol & 0x0F);
						break;
					case XMV_PANNING:
						volume = XM_SAMPLER_CMD_VOL_PANNING | (vol & 0x0F);
						break;
					case XMV_PANNINGSLIDELEFT:
						// Panning in FT2 has 256 values, so we convert to the 64values used in Sampulse.
						volume = XM_SAMPLER_CMD_VOL_PANSLIDELEFT | (((vol & 0x0F) < 4) ? 1 : (vol & 0x0F) >> 2);
						break;
					case XMV_PANNINGSLIDERIGHT:
						// Panning in FT2 has 256 values, so we convert to the 64values used in Sampulse.
						volume = XM_SAMPLER_CMD_VOL_PANSLIDERIGHT | (((vol & 0x0F) < 4) ? 1 : (vol & 0x0F) >> 2);
						break;
						// Ignoring this command for now.
						//					case XMVOL_CMD_XMV_VIBRATOSPEED:
						//						volume = XM_SAMPLER_CMD_VOL_VIBRATO_SPEED|(vol&0x0F);
						//						break;
					case XMV_VIBRATO:
						volume = XM_SAMPLER_CMD_VOL_VIBRATO | (vol & 0x0F);
						break;
					case XMV_PORTA2NOTE:
						volume = XM_SAMPLER_CMD_VOL_TONEPORTAMENTO | (vol & 0x0F);
						break;
					default:
						break;
					}
				}
				e._parameter = param;
				switch (type) {
				case XMCMD_ARPEGGIO:
					if (param != 0) {
						e._cmd = XM_SAMPLER_CMD_ARPEGGIO;
					} else {
						e._cmd = XM_SAMPLER_CMD_NONE;
					}
					break;
				case XMCMD_PORTAUP:
					e._cmd = XM_SAMPLER_CMD_PORTAMENTO_UP;
					if (e._parameter == 0) {
						if (self->memportapos[col] != 1)
						{
							e._parameter = self->memportaup[col];
						}
					} else {
						if (e._parameter > 0xDF) { e._parameter = 0xDF; }
						self->memportaup[col] = e._parameter;
					}
					self->memportapos[col] = 1;
					break;
				case XMCMD_PORTADOWN:
					e._cmd = XM_SAMPLER_CMD_PORTAMENTO_DOWN;
					if (e._parameter == 0) {
						if (self->memportapos[col] != 2)
						{
							e._parameter = self->memportadown[col];
						}
					} else {
						if (e._parameter > 0xDF) { e._parameter = 0xDF; }
						self->memportadown[col] = e._parameter;
					}
					self->memportapos[col] = 2;
					break;
				case XMCMD_PORTA2NOTE:
					e._cmd = XM_SAMPLER_CMD_PORTA2NOTE;
					if (e._parameter == 0) {
						if (self->memportapos[col] != 3)
						{
							e._parameter = self->memportanote[col];
						}
					} else {
						self->memportanote[col] = e._parameter;
					}
					self->memportapos[col] = 3;
					break;
				case XMCMD_VIBRATO:
					e._cmd = XM_SAMPLER_CMD_VIBRATO;
					break;
				case XMCMD_TONEPORTAVOL:
					e._cmd = XM_SAMPLER_CMD_TONEPORTAVOL;
					break;
				case XMCMD_VIBRATOVOL:
					e._cmd = XM_SAMPLER_CMD_VIBRATOVOL;
					break;
				case XMCMD_TREMOLO:
					e._cmd = XM_SAMPLER_CMD_TREMOLO;
					break;
				case XMCMD_PANNING:
					e._cmd = XM_SAMPLER_CMD_PANNING;
					break;
				case XMCMD_OFFSET:
					e._cmd = XM_SAMPLER_CMD_OFFSET | self->highoffset[col];
					break;
				case XMCMD_VOLUMESLIDE:
					e._cmd = XM_SAMPLER_CMD_VOLUMESLIDE;
					break;
				case XMCMD_POSITION_JUMP:
					e._cmd = psy_audio_PATTERNCMD_JUMP_TO_ORDER;
					break;
				case XMCMD_VOLUME:
					e._cmd = XM_SAMPLER_CMD_VOLUME;
					e._parameter = param * 2;
					break;
				case XMCMD_PATTERN_BREAK:
					e._cmd = psy_audio_PATTERNCMD_BREAK_TO_LINE;
					e._parameter = ((param & 0xF0) >> 4) * 10 + (param & 0x0F);
					break;
				case XMCMD_EXTENDED:
					switch (param & 0xf0) {
					case XMCMD_E_FINE_PORTA_UP:
						e._cmd = XM_SAMPLER_CMD_PORTAMENTO_UP;
						e._parameter = 0xF0 + (param & 0x0F);
						break;
					case XMCMD_E_FINE_PORTA_DOWN:
						e._cmd = XM_SAMPLER_CMD_PORTAMENTO_DOWN;
						e._parameter = 0xF0 + (param & 0x0F);
						break;
					case XMCMD_E_GLISSANDO_STATUS:
						e._cmd = XM_SAMPLER_CMD_EXTENDED;
						e._parameter = XM_SAMPLER_CMD_E_GLISSANDO_TYPE | ((param == 0) ? 0 : 1);
						break;
					case XMCMD_E_VIBRATO_WAVE:
						e._cmd = XM_SAMPLER_CMD_EXTENDED;
						e._parameter = XM_SAMPLER_CMD_E_VIBRATO_WAVE | exchwave[param & 0x3];
						break;
					case XMCMD_E_FINETUNE:
						e._cmd = XM_SAMPLER_CMD_NONE;
						e._parameter = 0;
						break;
					case XMCMD_E_PATTERN_LOOP:
						e._cmd = psy_audio_PATTERNCMD_EXTENDED;
						e._parameter = psy_audio_PATTERNCMD_PATTERN_LOOP + (param & 0xf);
						break;
					case XMCMD_E_TREMOLO_WAVE:
						e._cmd = XM_SAMPLER_CMD_EXTENDED;
						e._parameter = XM_SAMPLER_CMD_E_TREMOLO_WAVE | exchwave[param & 0x3];
						break;
					case XMCMD_E_MOD_RETRIG:
						e._cmd = XM_SAMPLER_CMD_RETRIG;
						e._parameter = param & 0xf;
						break;
					case XMCMD_E_FINE_VOLUME_UP:
						e._cmd = XM_SAMPLER_CMD_VOLUMESLIDE;
						e._parameter = 0x0f + ((param & 0xf) << 4);
						break;
					case XMCMD_E_FINE_VOLUME_DOWN:
						e._cmd = XM_SAMPLER_CMD_VOLUMESLIDE;
						e._parameter = 0xf0 + (param & 0xf);
						break;
					case XMCMD_E_DELAYED_NOTECUT:
						e._cmd = XM_SAMPLER_CMD_EXTENDED;
						e._parameter = XM_SAMPLER_CMD_E_DELAYED_NOTECUT | (param & 0xf);
						break;
					case XMCMD_E_NOTE_DELAY:
						e._cmd = XM_SAMPLER_CMD_EXTENDED;
						e._parameter = XM_SAMPLER_CMD_E_NOTE_DELAY | (param & 0xf);
						break;
					case XMCMD_E_PATTERN_DELAY:
						e._cmd = psy_audio_PATTERNCMD_EXTENDED;
						e._parameter = psy_audio_PATTERNCMD_PATTERN_DELAY | (param & 0xf);
						break;
					case XMCMD_E_SET_MIDI_MACRO:
						e._cmd = XM_SAMPLER_CMD_EXTENDED;
						e._parameter = XMCMD_MIDI_MACRO | (param & 0x0f);
						break;
					default:
						e._cmd = XM_SAMPLER_CMD_NONE;
						break;
					}
					break;
				case XMCMD_SETSPEED:
					if (param < 32)
					{
						int extraticks;

						e._cmd = psy_audio_PATTERNCMD_EXTENDED;
						extraticks = 0;
						e._parameter = calclpbfromspeed(param, &extraticks);
						if (extraticks != 0) {
							psy_audio_LegacyPatternEntry entry;

							psy_audio_legacypatternentry_init_all(&entry,
								PSY2_NOTECOMMANDS_EMPTY, 255, 255,								
								psy_audio_PATTERNCMD_EXTENDED,
								psy_audio_PATTERNCMD_ROW_EXTRATICKS | extraticks);
							if (!xmsongloader_writepatternentry(self, ppattern,
									row, self->extracolumn, &entry)) {
								free(ppattern);
								return PSY_ERRFILE;
							}
							self->extracolumn++;
						}
					} else
					{
						e._cmd = psy_audio_PATTERNCMD_SET_TEMPO;
					}
					break;
				case XMCMD_SET_GLOBAL_VOLUME:
					e._cmd = XM_SAMPLER_CMD_SET_GLOBAL_VOLUME;
					if (param >= 0x80) e._parameter = 0xFF;
					else e._parameter = param * 2;
					break;
				case XMCMD_GLOBAL_VOLUME_SLIDE:
					e._cmd = XM_SAMPLER_CMD_GLOBAL_VOLUME_SLIDE;
					//Double the parameter, since FT2's range is 0-0x40.
					if ((param & 0x0F) == 0 || (param & 0x0F) == 0xF) { // Slide up
						e._parameter = (param & 0xF0) >> 4;
						e._parameter = e._parameter > 7 ? 15 : e._parameter * 2;
						e._parameter <<= 4;
						e._parameter |= (param & 0x0F);
					} else if ((param & 0xF0) == 0 || (param & 0xF0) == 0xF0) { // Slide down
						e._parameter = (param & 0x0F);
						e._parameter = e._parameter > 7 ? 15 : e._parameter * 2;
						e._parameter |= (param & 0xF0);
					}
					break;
				case XMCMD_NOTE_OFF:
					e._cmd = XM_SAMPLER_CMD_VOLUME;
					e._parameter = 0;
					break;
				case XMCMD_SET_ENV_POSITION:
					e._cmd = XM_SAMPLER_CMD_SET_ENV_POSITION;
					break;
				case XMCMD_PANNINGSLIDE:
					e._cmd = XM_SAMPLER_CMD_PANNINGSLIDE;
					break;
				case XMCMD_RETRIG:
					e._cmd = XM_SAMPLER_CMD_RETRIG;
					break;
				case XMCMD_TREMOR:
					e._cmd = XM_SAMPLER_CMD_TREMOR;
					break;
				case XMCMD_EXTEND_XM_EFFECTS:
					switch (param & 0xf0) {
					case XMCMD_X_EXTRA_FINE_PORTA_DOWN:
						e._cmd = XM_SAMPLER_CMD_PORTAMENTO_DOWN;
						e._parameter = 0xE0 | (param & +0xf);
						break;
					case XMCMD_X_EXTRA_FINE_PORTA_UP:
						e._cmd = XM_SAMPLER_CMD_PORTAMENTO_UP;
						e._parameter = 0xE0 | (param & +0xf);
						break;
					case XMCMD_X9:
						switch (param & 0xf) {
						case XMCMD_X9_SURROUND_OFF:
							e._cmd = XM_SAMPLER_CMD_EXTENDED;
							e._parameter = XM_SAMPLER_CMD_E9 | XM_SAMPLER_CMD_E9_SURROUND_OFF;
							break;
						case XMCMD_X9_SURROUND_ON:
							e._cmd = XM_SAMPLER_CMD_EXTENDED;
							e._parameter = XM_SAMPLER_CMD_E9 | XM_SAMPLER_CMD_E9_SURROUND_ON;
							break;
						case XMCMD_X9_REVERB_OFF:
							e._cmd = XM_SAMPLER_CMD_EXTENDED;
							e._parameter = XM_SAMPLER_CMD_E9 | XM_SAMPLER_CMD_E9_REVERB_OFF;
							break;
						case XMCMD_X9_REVERB_FORCE:
							e._cmd = XM_SAMPLER_CMD_EXTENDED;
							e._parameter = XM_SAMPLER_CMD_E9 | XM_SAMPLER_CMD_E9_REVERB_FORCE;
							break;
						case XMCMD_X9_STANDARD_SURROUND:
							e._cmd = XM_SAMPLER_CMD_EXTENDED;
							e._parameter = XM_SAMPLER_CMD_E9 | XM_SAMPLER_CMD_E9_STANDARD_SURROUND;
							break;
						case XMCMD_X9_QUAD_SURROUND:
							e._cmd = XM_SAMPLER_CMD_EXTENDED;
							e._parameter = XM_SAMPLER_CMD_E9 | XM_SAMPLER_CMD_E9_QUAD_SURROUND;
							break;
						case XMCMD_X9_GLOBAL_FILTER:
							e._cmd = XM_SAMPLER_CMD_EXTENDED;
							e._parameter = XM_SAMPLER_CMD_E9 | XM_SAMPLER_CMD_E9_GLOBAL_FILTER;
							break;
						case XMCMD_X9_LOCAL_FILTER:
							e._cmd = XM_SAMPLER_CMD_EXTENDED;
							e._parameter = XM_SAMPLER_CMD_E9 | XM_SAMPLER_CMD_E9_LOCAL_FILTER;
							break;
						case XMCMD_X9_PLAY_FORWARD:
							e._cmd = XM_SAMPLER_CMD_EXTENDED;
							e._parameter = XM_SAMPLER_CMD_E9 | XM_SAMPLER_CMD_E9_PLAY_FORWARD;
							break;
						case XMCMD_X9_PLAY_BACKWARD:
							e._cmd = XM_SAMPLER_CMD_EXTENDED;
							e._parameter = XM_SAMPLER_CMD_E9 | XM_SAMPLER_CMD_E9_PLAY_BACKWARD;
							break;
						default:
							e._cmd = XM_SAMPLER_CMD_NONE;
							break;
						}
						break;
					case XMCMD_X_HIGH_OFFSET:
						self->highoffset[col] = param & 0x0F;
						break;
					default:
						e._cmd = XM_SAMPLER_CMD_NONE;
						break;
					}
					break;
				case XMCMD_PANBRELLO:
					e._cmd = XM_SAMPLER_CMD_PANBRELLO;
					break;
				case XMCMD_MIDI_MACRO:
					e._cmd = XM_SAMPLER_CMD_MIDI_MACRO;
					break;
				default:
					e._cmd = XM_SAMPLER_CMD_NONE;
					break;
				}
				// instrument/note
				note = note & 0x7f;
				switch (note)
				{
				case 0x00:
					e._note = PSY2_NOTECOMMANDS_EMPTY;
					break;// no note

				case 0x61:
					e._note = PSY2_NOTECOMMANDS_RELEASE;
					e._inst = 255;
					e._mach = 0;
					break;// noteoff		

				default:
					//if(note > 96 || note <= 0)
					// TRACE(_T("invalid note: %d\n"), note);
					e._note = note + 11; // +11 -> +12
					                     // ( FT2 C-0 is Psycle's C-1) -1
					                     // ( Ft2 C-0 is value 1)
					break;	// transpose
				}

				// If empty, do not inform machine
				if (e._note == PSY2_NOTECOMMANDS_EMPTY && instr == 255 &&
						volume == 255 && e._cmd == 00 && e._parameter == 00) {
					e._mach = 255;
					e._inst = 255;
				}
				// if instrument without note, or note without instrument,
				// cannot use virtual instrument, so use sampulse directly
				else if ((e._note == PSY2_NOTECOMMANDS_EMPTY && instr != 255) ||
						(e._note < psy_audio_NOTECOMMANDS_RELEASE &&
						 instr == 255)) {
					e._mach = 0;
					e._inst = instr;
					if (e._inst != 255) {
						// We cannot use the virtual instrument, but we should
						// remember which it is.
						if (psy_table_exists(&self->xmtovirtual, e._inst)) {
							lastmach[col] = (uint8_t)(uintptr_t)
								psy_table_at(&self->xmtovirtual, e._inst);
						}
					}
				}
				//default behaviour, let's find the virtual instrument.
				else {
					e._inst = instr;
					if (!psy_table_exists(&self->xmtovirtual, e._inst)) {
						if (e._inst != 255) {
							e._mach = 0;
							lastmach[col] = e._mach;
						} else if (lastmach[col] != 255) {
							e._mach = (uint8_t)lastmach[col];
						} else if (volume != 255) {
							e._mach = 0;
						} else {
							e._mach = 255;
						}
					} else {
						e._mach = (uint8_t)(uintptr_t)psy_table_at(
							&self->xmtovirtual, e._inst);
						e._inst = 255;
						lastmach[col] = e._mach;
					}
				}
				if (e._mach == 0) { // fallback to the old behaviour. This will
					                // happen only if an unused instrument is
					                // present in the pattern.
					if (e._cmd != 0 || e._parameter != 0) {
						if (volume != 255) {
							psy_audio_LegacyPatternEntry entry;

							psy_audio_legacypatternentry_init_all(&entry,
								PSY2_NOTECOMMANDS_MIDICC, col, e._mach, 0, 0);
							entry._cmd = XM_SAMPLER_CMD_SENDTOVOLUME;
							entry._parameter = volume;
							if (!xmsongloader_writepatternentry(self, ppattern,
									row, self->extracolumn, &entry)) {
								free(ppattern);
								return PSY_ERRFILE;
							}
							self->extracolumn++;
						}
					} else if (volume < 0x40) {
						e._cmd = XM_SAMPLER_CMD_VOLUME;
						e._parameter = volume * 2;
					} else if (volume != 255) {
						e._cmd = XM_SAMPLER_CMD_SENDTOVOLUME;
						e._parameter = volume;
					}
				} else {
					e._inst = volume;
				}
				if (!xmsongloader_writepatternentry(self, ppattern, row, col,
						&e)) {
					free(ppattern);
					return PSY_ERRFILE;
				}
			}
			self->maxextracolumn = psy_max(self->maxextracolumn,
				self->extracolumn);
		}

		// convert ppattern to pattern
		node = NULL;
		for (row = 0; row < numrows; ++row) {			
			uint32_t track;
			psy_dsp_big_beat_t offset;
			psy_dsp_big_beat_t bpl;

			bpl = 1.0 / psy_audio_song_lpb(self->song);
			offset = bpl * row;
			for (track = 0; track < MAX_TRACKS; ++track) {				
				psy_audio_LegacyPatternEntry* psy2ev;
				bool empty;

				psy2ev = psy_audio_ptrackline(ppattern, track, row);
				empty = psy2ev->_note == PSY2_NOTECOMMANDS_EMPTY &&
					psy2ev->_inst == 255 &&
					psy2ev->_mach == 255 &&
					psy2ev->_cmd == 0 &&
					psy2ev->_parameter == 0;
				if (!empty) {
					psy_audio_PatternEvent event;

					psy_audio_patternevent_clear(&event);
					event.note = (psy2ev->_note == 255) ? psy_audio_NOTECOMMANDS_EMPTY : psy2ev->_note;
					event.inst = (psy2ev->_inst == 255) ? psy_audio_NOTECOMMANDS_INST_EMPTY : psy2ev->_inst;
					event.mach = (psy2ev->_mach == 255) ? psy_audio_NOTECOMMANDS_MACH_EMPTY : psy2ev->_mach;
					event.cmd = psy2ev->_cmd;
					event.parameter = psy2ev->_parameter;								
					node = psy_audio_pattern_insert(pattern, node, track, offset,
						&event);
				}				
			}			
		}				
	}
	free(ppattern);
	//int z = ftell(_file);
	*rv_nextstart = start + packedsize + headerlen;
	return PSY_OK;
}

bool xmsongloader_writepatternentry(XMSongLoader* self,
	psy_audio_LegacyPattern pattern, int row, int col,
	psy_audio_LegacyPatternEntry* e)
{
	psy_audio_LegacyPatternEntry* pData;

	assert(self);	
	assert(pattern && e);
		
	pData = psy_audio_ptrackline(pattern, col, row);

	if (!pData) {
		return FALSE;
	}
		
	*pData = *e;

	return TRUE;
}	

int xmsongloader_loadinstruments(XMSongLoader* self, uintptr_t instr_start)
{
	int32_t i;	
	
	for (i = 1; i <= self->instrcount; ++i) {
		int status;
		
		if (status = xmsongloader_loadinstrument(self, i, instr_start,
				&instr_start)) {
			return status;
		}
		if (psy_table_exists(&self->xmtovirtual, i)) {
			// sampulse: 0
			psy_audio_song_insertvirtualgenerator(self->songfile->song,
				(uintptr_t)psy_table_at(&self->xmtovirtual, i), 0, i);
		}
	}
	return PSY_OK;
}

int xmsongloader_loadinstrument(XMSongLoader* self, uintptr_t slot,
	uintptr_t start, uintptr_t* rv_nextstart)
{	
	psy_audio_Instrument* instrument;	
	int32_t instrsize;
	char instrname[24];
	uint16_t samplecount;	
	XMSAMPLEHEADER samph;
	int exchwave[4] = {
		psy_audio_WAVEFORMS_SINUS,
		psy_audio_WAVEFORMS_SQUARE,
		psy_audio_WAVEFORMS_SAWDOWN,
		psy_audio_WAVEFORMS_SAWUP
	};
	int32_t i;
	int note;
	psy_audio_InstrumentEntry instentry;
			
	if (psyfile_seek(self->fp, (uint32_t)start) == -1) {
		return PSY_ERRFILE;
	}
	// create and insert instrument
	instrument = psy_audio_instrument_allocinit();			
	psy_audio_instrument_setindex(instrument, slot);
	psy_audio_instruments_insert(&self->song->instruments, instrument,
		psy_audio_instrumentindex_make(INSTRUMENTGROUP, slot));
	psy_dsp_envelope_clear(&instrument->volumeenvelope);
	psy_dsp_envelope_setmode(&instrument->volumeenvelope, psy_dsp_ENVELOPETIME_TICK);
	psy_dsp_envelope_clear(&instrument->panenvelope);
	psy_dsp_envelope_setmode(&instrument->panenvelope, psy_dsp_ENVELOPETIME_TICK);	
	psy_dsp_envelope_clear(&instrument->pitchenvelope);
	psy_dsp_envelope_setmode(&instrument->pitchenvelope, psy_dsp_ENVELOPETIME_TICK);
	psy_dsp_envelope_clear(&instrument->filterenvelope);
	psy_dsp_envelope_setmode(&instrument->filterenvelope, psy_dsp_ENVELOPETIME_TICK);
	// read header
	if (!psyfile_read(self->fp, &instrsize, sizeof(int32_t))) {
		return PSY_ERRFILE;
	}
	start += instrsize;
	memset(instrname, 0, sizeof(instrname));
	if (!psyfile_read(self->fp, instrname, 22)) {
		return PSY_ERRFILE;
	}
	psy_audio_instrument_setname(instrument, instrname);
	// //int iInstrType = ReadInt1();
	if (psyfile_skip(self->fp, 1) == -1) {
		return PSY_ERRFILE;
	}
	if (!psyfile_read(self->fp, &samplecount, sizeof(uint16_t))) {
		return PSY_ERRFILE;
	}	
	if (samplecount > 1) {
		//TRACE(_T("ssmple count = %d\n"), iSampleCount);
	}
	if (samplecount == 0) {
		*rv_nextstart = start;
		return PSY_OK;
	}

	memset(&samph, 0, sizeof samph);
	if (!psyfile_read(self->fp, &samph, sizeof(XMSAMPLEHEADER))) {
		return PSY_ERRFILE;
	}
	xmsongloader_setenvelopes(instrument, &samph);
	// load individual samples	
	for (i = 0; i < samplecount; i++) {
		int status;
		psy_audio_Sample* wave;

		wave = psy_audio_sample_allocinit(0);		
		if (status = xmsongloader_loadsampleheader(self, wave, start, i, &start)) {
			psy_audio_sample_dispose(wave);
			return PSY_ERRFILE;
		}
		// Only get REAL samples.
		// if (self->smplen[*cursample] > 0) {
			psy_audio_samples_insert(&self->song->samples, wave,
				sampleindex_make(slot, i));
		// }
	}
	// load individual samples
	for (i = 0; i < samplecount; i++) {
		int status;
		psy_audio_Sample* wave;

		wave = psy_audio_samples_at(&self->song->samples,
			sampleindex_make(slot, i));
		assert(wave);
		if (!wave) {
			return PSY_ERRFILE;
		}
		if (status = xmsongloader_loadsampledata(self, wave, i, start, &start)) {
			return status;
		}
		wave->vibrato.attack = samph.vibsweep == 0 ? 0 : 256 - samph.vibsweep;
		wave->vibrato.depth = samph.vibdepth;
		wave->vibrato.speed = samph.vibrate;
		wave->vibrato.type = exchwave[samph.vibtype & 3];
	}
	// create instrument entries
	psy_audio_instrument_clearentries(instrument);
	psy_audio_instrumententry_init(&instentry);
	instentry.sampleindex =
		sampleindex_make(slot, samph.snum[0]);
	for (note = 1; note < 96; ++note) {
		if (samph.snum[note] != instentry.sampleindex.subslot) {
			instentry.keyrange.high = note - 1;
			psy_audio_instrument_addentry(instrument, &instentry);
			instentry.keyrange.low = note;
			instentry.sampleindex.subslot = samph.snum[note];
		}
	}
	instentry.keyrange.high = 119;
	psy_audio_instrument_addentry(instrument, &instentry);	
	*rv_nextstart = start;
	return PSY_OK;
}

int xmsongloader_loadsampledata(XMSongLoader* self, psy_audio_Sample* wave,
	int32_t sampleidx, uintptr_t start, uintptr_t* rv_nextstart)
{
	// parse

	BOOL b16Bit;
	char* smpbuf;
	int16_t wNew;	
	int32_t samplecnt;

	b16Bit = self->smpflags[sampleidx] & 0x10;
	wNew = 0;

	// cache sample data
	if (psyfile_seek(self->fp, (uint32_t)start) == -1) {
		return PSY_ERRFILE;
	}

	if (self->smplen[sampleidx] == 0) {
		return PSY_OK;
	}	
	smpbuf = (char*)malloc(self->smplen[sampleidx]);
	if (!smpbuf) {
		return PSY_ERRFILE;
	}
	memset(smpbuf, 0, self->smplen[sampleidx]);
	if (!psyfile_read(self->fp, smpbuf, self->smplen[sampleidx])) {
		free(smpbuf);
		return PSY_ERRFILE;
	}	
	samplecnt = self->smplen[sampleidx];	
	psy_audio_sample_resize(wave, 1);
	wave->numframes = self->smplen[sampleidx];
	psy_audio_sample_allocwavedata(wave);

	// unpack sample data
	if (b16Bit)
	{
		// 16 bit mono sample, delta
		int out = 0;
		for (int j = 0; j <= samplecnt - 2; j += 2)
		{
			wNew += (smpbuf[j] & 0xFF) | (smpbuf[j + 1] << 8);
			wave->channels.samples[0][out] = (psy_dsp_amp_t)wNew;			
			out++;
		}
	} else
	{
		// 8 bit mono sample
		for (int j = 0; j < samplecnt; j++)
		{
			wNew += (int16_t)(smpbuf[j] << 8);
			wave->channels.samples[0][j] = (psy_dsp_amp_t)wNew;			
		}
	}

	// cleanup
	free(smpbuf);

	// complete
	start += self->smplen[sampleidx];
	*rv_nextstart = start;	
	return PSY_OK;
}
	
	
void xmsongloader_setenvelopes(psy_audio_Instrument* inst, const XMSAMPLEHEADER* sampleHeader)
{
	// volume envelope	
	
	psy_dsp_envelope_clear(&inst->volumeenvelope);
	psy_dsp_envelope_setmode(&inst->volumeenvelope, psy_dsp_ENVELOPETIME_TICK);
	if (sampleHeader->vtype & 1) {// enable volume envelope		
		int i;
		int envelope_point_num = sampleHeader->vnum;
		// In FastTracker, the volume fade only works if the envelope is activated, so we only calculate
		// volumefadespeed in this case, so that a check during playback time is not needed.		
		psy_audio_instrument_setvolumefadespeed(inst,
			psy_dsp_map_32768_1(sampleHeader->volfade));				

		psy_dsp_envelope_setenabled(&inst->volumeenvelope, TRUE);
		if (envelope_point_num > 12) { // Max number of envelope points in Fasttracker format is 12.
			envelope_point_num = 12;
		}
		// Format of FastTracker points is :
		// Point : frame number. ( 1 frame= line*(24/TPB), samplepos= frame*(samplesperrow*TPB/24))
		// Value : 0..64. , divide by 64 to use it as a multiplier.
		psy_dsp_envelope_append(&inst->volumeenvelope, psy_dsp_envelopepoint_make(
			sampleHeader->venv[0], (float)sampleHeader->venv[1] / 64.0f));		
		for (i = 1; i < envelope_point_num; i++) {
			if (sampleHeader->venv[i * 2] > sampleHeader->venv[(i - 1) * 2])// Some rare modules have erroneous points. This tries to solve that.
				psy_dsp_envelope_append(&inst->volumeenvelope, psy_dsp_envelopepoint_make(
					sampleHeader->venv[i * 2], (float)sampleHeader->venv[i * 2 + 1] / 64.0f));
		}

		if (sampleHeader->vtype & 2) {
			psy_dsp_envelope_setsustainbegin(&inst->volumeenvelope, sampleHeader->vsustain);
			psy_dsp_envelope_setsustainend(&inst->volumeenvelope, sampleHeader->vsustain);
		} else
		{
			// We can't ignore the loop because IT Envelopes do a fadeout when the envelope end is reached and FT does not.
			// IT also sets the Sustain points to the end of the envelope, but i can't see a reason for this to be needed.
//				inst.AmpEnvelope().SustainBegin(inst.AmpEnvelope().NumOfPoints()-1);
//				inst.AmpEnvelope().SustainEnd(inst.AmpEnvelope().NumOfPoints()-1);
		}
	
		if(sampleHeader->vtype & 4){
			if(sampleHeader->vloops < sampleHeader->vloope){
				psy_dsp_envelope_setloopstart(&inst->volumeenvelope,
					sampleHeader->vloops);
				psy_dsp_envelope_setloopend(&inst->volumeenvelope,
					sampleHeader->vloope);				
			}
			// if loopstart >= loopend, Fasttracker ignores the loop!.
			// We can't ignore them because IT Envelopes do a fadeout when the envelope end is reached and FT does not.
			else {
//					inst.AmpEnvelope().LoopStart(XMInstrument::Envelope::INVALID);
//					inst.AmpEnvelope().LoopEnd(XMInstrument::Envelope::INVALID);
				psy_dsp_envelope_setloopstart(&inst->volumeenvelope,
					psy_dsp_envelope_numofpoints(&inst->volumeenvelope) - 1);
				psy_dsp_envelope_setloopend(&inst->volumeenvelope,
					psy_dsp_envelope_numofpoints(&inst->volumeenvelope) - 1);				
			}
		}
		else
		{
			// We can't ignore the loop because IT Envelopes do a fadeout when the envelope end is reached and FT does not.
			psy_dsp_envelope_setloopstart(&inst->volumeenvelope,
				psy_dsp_envelope_numofpoints(&inst->volumeenvelope) - 1);
			psy_dsp_envelope_setloopend(&inst->volumeenvelope,
				psy_dsp_envelope_numofpoints(&inst->volumeenvelope) - 1);
		}
	} else {
		psy_dsp_envelope_setenabled(&inst->volumeenvelope, FALSE);
	}
	
	// pan envelope	
	psy_dsp_envelope_clear(&inst->panenvelope);
	psy_dsp_envelope_setmode(&inst->panenvelope, psy_dsp_ENVELOPETIME_TICK);
	if(sampleHeader->ptype & 1){// enable volume envelope
		int envelope_point_num;

		psy_dsp_envelope_setenabled(&inst->panenvelope, TRUE);
			
		if(sampleHeader->ptype & 2){
			psy_dsp_envelope_setsustainbegin(&inst->panenvelope, sampleHeader->psustain);
			psy_dsp_envelope_setsustainend(&inst->panenvelope, sampleHeader->psustain);			
		}			
		if(sampleHeader->ptype & 4){
			if (sampleHeader->ploops < sampleHeader->ploope) {
				psy_dsp_envelope_setloopstart(&inst->panenvelope,
					sampleHeader->ploops);
				psy_dsp_envelope_setloopend(&inst->panenvelope,
					sampleHeader->ploope);
			} else {
				psy_dsp_envelope_setloopstart(&inst->panenvelope,
					sampleHeader->ploope);
				psy_dsp_envelope_setloopend(&inst->panenvelope,
					sampleHeader->ploops);
			}			
		}
		envelope_point_num = sampleHeader->pnum;
		if(envelope_point_num > 12){
			envelope_point_num = 12;
		}

		for(int i = 0; i < envelope_point_num;i++){
			psy_dsp_envelope_append(&inst->panenvelope, psy_dsp_envelopepoint_make(
				sampleHeader->penv[i * 2], (float)(sampleHeader->penv[i * 2 + 1] -32.0f )/ 32.0f));
		}

	} else {
		psy_dsp_envelope_setenabled(&inst->panenvelope, FALSE);
	}
}

int xmsongloader_loadsampleheader(XMSongLoader* self, psy_audio_Sample* wave,
	uintptr_t start, int32_t sampleidx, uintptr_t* rv_nextstart)
{	
	int32_t len;
	int32_t loopstart;
	int32_t looplength;
	// params
	int8_t vol;
	int8_t finetune;
	int8_t flags;
	uint8_t panning;
	int8_t relativenote;
	char cname[23];
	bool bLoop;
	bool bPingPong;
	bool b16Bit;

	// get sample header
	if (psyfile_seek(self->fp, (uint32_t)start) == -1) {
		return PSY_ERRFILE;
	}
	if (!psyfile_read(self->fp, &len, sizeof(int32_t))) {
		return PSY_ERRFILE;
	}	

	// loop data
	if (!psyfile_read(self->fp, &loopstart, sizeof(int32_t))) {
		return PSY_ERRFILE;
	}
	if (!psyfile_read(self->fp, &looplength, sizeof(int32_t))) {
		return PSY_ERRFILE;
	}	

	// params
	if (!psyfile_read(self->fp, &vol, sizeof(int8_t))) {
		return PSY_ERRFILE;
	}
	if (!psyfile_read(self->fp, &finetune, sizeof(int8_t))) {
		return PSY_ERRFILE;
	}
	if (!psyfile_read(self->fp, &flags, sizeof(int8_t))) {
		return PSY_ERRFILE;
	}
	if (!psyfile_read(self->fp, &panning, sizeof(uint8_t))) {
		return PSY_ERRFILE;
	}
	if (!psyfile_read(self->fp, &relativenote, sizeof(int8_t))) {
		return PSY_ERRFILE;
	}
	//char iReserved = ReadInt1();
	if (psyfile_skip(self->fp, 1) == -1) {
		return PSY_ERRFILE;
	}	

	// sample name
	memset(cname, 0, sizeof(cname));
	if (!psyfile_read(self->fp, &cname, 22)) {
		return PSY_ERRFILE;
	}	
	// parse
	bLoop = (flags & 0x01 || flags & 0x02) && (looplength > 0);
	bPingPong = flags & 0x02;
	b16Bit = flags & 0x10;

	// alloc wave memory

	ASSERT(len < (1 << 30)); // Since in some places, signed values are used, we cannot use the whole range.
	
	//_wave.Init();
	if (len > 0) // Sounds Stupid, but it isn't. Some modules save sample header when there is no sample.
	{
		// allocwave data in loadsampledata		
	}
	wave->panenabled = TRUE;
	wave->panfactor = panning / 255.0f;	

	if (bLoop)
	{
		if (bPingPong) {
			wave->loop.type = wave->loop.type = psy_audio_SAMPLE_LOOP_BIDI;
		} else {
			wave->loop.type = wave->loop.type = psy_audio_SAMPLE_LOOP_NORMAL;			
		}

		if (b16Bit)
		{
			wave->loop.start = loopstart / 2;
			wave->loop.end = (looplength + loopstart) / 2;
		} else
		{
			wave->loop.start = loopstart;
			wave->loop.end = looplength + loopstart;
		}

		//			TRACE2("l:%x s:%x e:%x \n",_wave.WaveLength(),_wave.WaveLoopStart(),_wave.WaveLoopEnd()); 

	} else {
		wave->loop.type = psy_audio_SAMPLE_LOOP_DO_NOT;		
	}

	wave->samplerate = 8363.0;
	wave->defaultvolume = vol * 2;
	wave->tune = relativenote;
	wave->finetune = (int16_t)(finetune / 1.28); // WaveFineTune has +-100 range in Psycle.
	psy_audio_sample_setname(wave, cname);	

	self->smplen[sampleidx] = len;
	self->smpflags[sampleidx] = flags;

	start += 40;
	*rv_nextstart = start;
	return PSY_OK;
}

// MODSongLoader
//
// prototypes
static void modsongloader_reset(MODSongLoader*);
static bool modsongloader_isvalid(MODSongLoader*);
static void modsongloader_loadpatterns(MODSongLoader*);
static void modsongloader_loadsinglepattern(MODSongLoader*, int patidx, int tracks);
static unsigned char modsongloader_convertperiodtonote(unsigned short period);
static bool modsongloader_writepatternentry(MODSongLoader*,
	psy_audio_PatternNode** node, psy_audio_Pattern*,
	uintptr_t row, uintptr_t col, const psy_audio_PatternEvent*);
static void modsongloader_makesequence(MODSongLoader*, struct MODHEADER*);
static void modsongloader_loadinstrument(MODSongLoader*, int idx);
static void modsongloader_loadsampleheader(MODSongLoader*, psy_audio_Sample*, int instridx);
static void modsongloader_loadsampledata(MODSongLoader*, psy_audio_Sample*, int instridx);
// implementation
void modsongloader_init(MODSongLoader* self, psy_audio_SongFile* songfile)
{
	assert(self);

	self->songfile = songfile;	
	psy_table_init(&self->xmtovirtual);
	modsongloader_reset(self);
}

void modsongloader_dispose(MODSongLoader* self)
{
	assert(self);

	psy_table_dispose(&self->xmtovirtual);
}

void modsongloader_reset(MODSongLoader* self)
{	
	memset(self->smplen, 0, sizeof(self->smplen));	
	psy_table_clear(&self->xmtovirtual);
}

bool modsongloader_load(MODSongLoader* self)
{
	char* pSongName;
	char* comments;
	psy_audio_SongProperties songproperties;
	PsyFile* fp;
	psy_audio_Song* song;
	int i;
	int virtidx;
	char pID[5];

	assert(self);

	modsongloader_reset(self);
	// file pointer	
	fp = self->songfile->file;
	song = self->songfile->song;

	// check validity
	if(!modsongloader_isvalid(self)){
		return FALSE;
	}
	
	// build sampler
	self->sampler = psy_audio_machinefactory_makemachine(
		self->songfile->song->machinefactory, MACH_XMSAMPLER, "", psy_INDEX_INVALID);
	if (self->sampler) {
		psy_audio_machine_setposition(self->sampler, rand() / 64, rand() / 80);	
		psy_audio_machines_insert(&self->songfile->song->machines, 0,
			self->sampler);
		psy_audio_machines_connect(&self->songfile->song->machines,
			psy_audio_wire_make(0, psy_audio_MASTER_INDEX));
		psy_audio_connections_setwirevolume(&self->songfile->song->machines.connections,
			psy_audio_wire_make(0, psy_audio_MASTER_INDEX), 0.5f);
		{ // XM_SAMPLER_PanningMode
			psy_audio_MachineParam* param;

			param = psy_audio_machine_tweakparameter(self->sampler, 40);
			if (param) {
				// XM_SAMPLER_PanningMode::TwoWay
				psy_audio_machine_parameter_tweak_scaled(self->sampler, param, 1);
			}
		}
	}	
	// get song name
	pSongName = AllocReadStrStart(fp, 20,0);
	if(pSongName==NULL)
		return FALSE;

	comments = strdup("Imported from MOD Module: ");
	comments = psy_strcat_realloc(comments, self->songfile->path);
	psy_audio_songproperties_init(&songproperties, pSongName, "", comments);
	free(comments);
	comments = NULL;
	psy_audio_song_setproperties(self->songfile->song, &songproperties);
	free(pSongName);
	pSongName = NULL;	

	// get data
	psyfile_seek(fp, 20);
	for (i=0;i<31;i++) {
		psy_audio_Sample* wave;

		wave = psy_audio_sample_allocinit(1);	
		modsongloader_loadsampleheader(self, wave, i);		
		psy_audio_samples_insert(&song->samples, wave,
			sampleindex_make(i, 0));
	}
	psyfile_seek(fp, 950);
	psyfile_read(fp, &self->header,sizeof(self->header));
			
	pID[0]=self->header.pID[0];pID[1]=self->header.pID[1];pID[2]= self->header.pID[2];pID[3]= self->header.pID[3];pID[4]=0;
	{ // AmigaSlides
		psy_audio_MachineParam* param;

		param = psy_audio_machine_tweakparameter(self->sampler, 24);
		if (param) {
			psy_audio_machine_parameter_tweak_scaled(self->sampler, param,
				TRUE);
		}
	}
	if ( !strncmp(pID,"M.K.",4)) {
		song->properties.tracks = 4; }
	else if ( !strncmp(pID,"M!K!",4)) { song->properties.tracks = 4;  }
	else if ( !strncmp(pID+1,"CHN",4)) { char tmp[2]; tmp[0] = pID[0]; tmp[1]=0; song->properties.tracks = atoi(tmp); }
	else if ( !strncmp(pID+2,"CH",4)) { char tmp[3]; tmp[0] = pID[0]; tmp[1]=pID[1]; tmp[2]=0; song->properties.tracks = atoi(tmp); }
	song->properties.bpm = 125;
	song->properties.lpb = 4;
	song->properties.tpb = 24;
	song->properties.extraticksperbeat = 0;	

	if (song->properties.tracks<=8) {
		uintptr_t i;

		for (i = 0; i< song->properties.tracks ; i++ )
		{
			//if (i%4 == 0 || i%4 == 3) m_pSampler->rChannel(i).DefaultPanFactorFloat(0.25f,true);
			//else m_pSampler->rChannel(i).DefaultPanFactorFloat(0.75,true);
		}
	}
	else {
		//for (int i = 0; i< song.SONGTRACKS ; i++ )
		//{
			//m_pSampler->rChannel(i).DefaultPanFactorFloat(0.5f,true);
		//}
	}	
	virtidx = MAX_MACHINES;
	for (i=0; i < 31;i++) {
		if (self->samples[i].sampleLength > 0 ) {
			psy_table_insert(&self->xmtovirtual, i, (void*)(uintptr_t)virtidx);			
			virtidx++;
		}
	}
	modsongloader_loadpatterns(self);
	for(i = 0;i < 31;i++){
		modsongloader_loadinstrument(self, i);
		// sampulse: 0
		psy_audio_song_insertvirtualgenerator(self->songfile->song,
			(uintptr_t)psy_table_at(&self->xmtovirtual, i), 0, i);
	}
	modsongloader_makesequence(self, &self->header);
	return TRUE;
}

bool modsongloader_isvalid(MODSongLoader* self)
{
	bool bIsValid = FALSE;
	char *pID = AllocReadStrStart(self->songfile->file, 4,1080);

	bIsValid = !strncmp(pID,"M.K.",4);
	if ( !bIsValid ) bIsValid = !strncmp(pID,"M!K!",4);
	if ( !bIsValid ) bIsValid = !strncmp(pID+1,"CHN",4);
	if ( !bIsValid ) bIsValid = !strncmp(pID+2,"CH",4);

	free(pID);
	return bIsValid;
}

void modsongloader_loadpatterns(MODSongLoader* self)
{
	int npatterns=0;
	int j;
	
	// get pattern data
	psyfile_seek(self->songfile->file, 1084);
	for(j = 0;j < self->header.songlength ;j++){
		modsongloader_loadsinglepattern(self, j,
			(int)self->songfile->song->properties.tracks);
	}
	if(self->speedpatch) {
		self->songfile->song->properties.tracks++;
	}
}

void modsongloader_makesequence(MODSongLoader* self, struct MODHEADER* modheader)
{
	uint8_t i;
	
	psy_audio_sequence_appendtrack(&self->songfile->song->sequence,
		psy_audio_sequencetrack_allocinit());
	for (i = 0; i < modheader->songlength; ++i) {		
		psy_audio_sequence_insert(&self->songfile->song->sequence,
			psy_audio_orderindex_make(0, i), modheader->order[i]);		
	}
}

// return address of next pattern, 0 for invalid
void modsongloader_loadsinglepattern(MODSongLoader* self, int patidx,int tracks) //, std::map<int,int>& modtovirtual)
{
	short iNumRows = 64;
	psy_audio_Pattern* pattern;
	PsyFile* fp;
	psy_audio_Song* song;
	unsigned char lastmach[64];
	psy_audio_PatternEvent e;
	unsigned char mentry[4];
	int row;
	int col;
	psy_audio_PatternNode* node;

	fp = self->songfile->file;
	song = self->songfile->song;
	pattern = psy_audio_pattern_allocinit();
	psy_audio_pattern_setname(pattern, "unnamed");
	psy_audio_patterns_insert(&song->patterns, patidx, pattern);
	psy_audio_pattern_setlength(pattern, iNumRows * 1.0 / self->songfile->song->properties.lpb);
	
	memset(lastmach,255,sizeof(char)*64);
	psy_audio_patternevent_clear(&e);
		
	// get next values
	node = NULL;
	for(row = 0; row < iNumRows; row++)
	{
		for(col=0; col< tracks; col++)
		{	
			// reset
			unsigned char note= psy_audio_NOTECOMMANDS_EMPTY;
			unsigned char instr=255;
			unsigned char type=0;
			unsigned char param=0;
			unsigned short period=428;
			int exchwave[3] = {
				psy_audio_WAVEFORMS_SINUS,
				psy_audio_WAVEFORMS_SAWDOWN,
				psy_audio_WAVEFORMS_SQUARE
			};


			// read note
			mentry[0] = psyfile_read_uint8(fp); mentry[1] = psyfile_read_uint8(fp); mentry[2] = psyfile_read_uint8(fp); mentry[3] = psyfile_read_uint8(fp);
			instr = ((mentry[0] & 0xF0) + (mentry[2] >> 4));
			period = ((mentry[0]& 0x0F) << 8) + mentry[1];
			type = (mentry[2] & 0x0F);
			param = mentry[3];
			note = modsongloader_convertperiodtonote(period);

			// translate
			e.parameter = param;

			switch(type){
				case XMCMD_ARPEGGIO:
					if(param != 0){
						e.cmd = XM_SAMPLER_CMD_ARPEGGIO;
					} else {
						e.cmd = XM_SAMPLER_CMD_NONE;
					}
					break;
				case XMCMD_PORTAUP:
					e.cmd = XM_SAMPLER_CMD_PORTAMENTO_UP;
					break;
				case XMCMD_PORTADOWN:
					e.cmd = XM_SAMPLER_CMD_PORTAMENTO_DOWN;
					break;
				case XMCMD_PORTA2NOTE:
					e.cmd = XM_SAMPLER_CMD_PORTA2NOTE;
					break;
				case XMCMD_VIBRATO:
					e.cmd = XM_SAMPLER_CMD_VIBRATO;
					break;
				case XMCMD_TONEPORTAVOL:
					e.cmd = XM_SAMPLER_CMD_TONEPORTAVOL;
					break;
				case XMCMD_VIBRATOVOL:
					e.cmd = XM_SAMPLER_CMD_VIBRATOVOL;
					break;
				case XMCMD_TREMOLO:
					e.cmd = XM_SAMPLER_CMD_TREMOLO;
					break;
				case XMCMD_PANNING:
					e.cmd = XM_SAMPLER_CMD_PANNING;
					break;
				case XMCMD_OFFSET:
					e.cmd = XM_SAMPLER_CMD_OFFSET;
					break;
				case XMCMD_VOLUMESLIDE:
					e.cmd = XM_SAMPLER_CMD_VOLUMESLIDE;
					break;
				case XMCMD_POSITION_JUMP:
					e.cmd = psy_audio_PATTERNCMD_JUMP_TO_ORDER;
					break;
				case XMCMD_VOLUME:
					e.cmd = XM_SAMPLER_CMD_VOLUME;
					e.parameter = param<=0x40?param*2:0x80;
					break;
				case XMCMD_PATTERN_BREAK:
					e.cmd =psy_audio_PATTERNCMD_BREAK_TO_LINE;
					e.parameter = ((param&0xF0)>>4)*10 + (param&0x0F);
					break;
				case XMCMD_EXTENDED:
					switch(param & 0xf0){
					case XMCMD_E_FINE_PORTA_UP:
						e.cmd = XM_SAMPLER_CMD_PORTAMENTO_UP;
						e.parameter= 0xF0+(param&0x0F);
						break;
					case XMCMD_E_FINE_PORTA_DOWN:
						e.cmd = XM_SAMPLER_CMD_PORTAMENTO_DOWN;
						e.parameter= 0xF0+(param&0x0F);
						break;
					case XMCMD_E_GLISSANDO_STATUS:
						e.cmd = XM_SAMPLER_CMD_EXTENDED;
						e.parameter = XM_SAMPLER_CMD_E_GLISSANDO_TYPE | ((param==0)?0:1);
						break;
					case XMCMD_E_VIBRATO_WAVE:
						e.cmd = XM_SAMPLER_CMD_EXTENDED;
						e.parameter =XM_SAMPLER_CMD_E_VIBRATO_WAVE | exchwave[param & 0x3];
						break;
					case XMCMD_E_FINETUNE:
						e.cmd = XM_SAMPLER_CMD_NONE;
						e.parameter = 0;
						break;
					case XMCMD_E_PATTERN_LOOP:
						e.cmd =psy_audio_PATTERNCMD_EXTENDED;
						e.parameter =psy_audio_PATTERNCMD_PATTERN_LOOP | (param & 0xf);
						break;
					case XMCMD_E_TREMOLO_WAVE:
						e.cmd = XM_SAMPLER_CMD_EXTENDED;
						e.parameter = XM_SAMPLER_CMD_E_TREMOLO_WAVE | exchwave[param & 0x3];
						break;
					case XMCMD_E_MOD_RETRIG:
						e.cmd = XM_SAMPLER_CMD_RETRIG;
						e.parameter = param & 0xf;
						break;
					case XMCMD_E_FINE_VOLUME_UP:
						e.cmd = XM_SAMPLER_CMD_VOLUMESLIDE;
						e.parameter = 0x0f + ((param & 0xf)<<4);
						break;
					case XMCMD_E_FINE_VOLUME_DOWN:
						e.cmd = XM_SAMPLER_CMD_VOLUMESLIDE;
						e.parameter = 0xf0 + (param & 0xf);
						break;
					case XMCMD_E_DELAYED_NOTECUT:
						e.cmd = XM_SAMPLER_CMD_EXTENDED;
						e.parameter = XM_SAMPLER_CMD_E_DELAYED_NOTECUT | (param & 0xf);
						break;
					case XMCMD_E_NOTE_DELAY:
						e.cmd = XM_SAMPLER_CMD_EXTENDED;
						e.parameter = XM_SAMPLER_CMD_E_NOTE_DELAY | ( param & 0xf);
						break;
					case XMCMD_E_PATTERN_DELAY:
						e.cmd =psy_audio_PATTERNCMD_EXTENDED;
						e.parameter = psy_audio_PATTERNCMD_PATTERN_DELAY | (param & 0xf);
						break;
					default:
						e.cmd = XM_SAMPLER_CMD_NONE;
						e.parameter = 0;
						break;
					}
					break;
				case XMCMD_SETSPEED:
					if ( param < 32)
					{
						int extraticks;

						e.cmd=psy_audio_PATTERNCMD_EXTENDED;
						extraticks=0;
						e.parameter = calclpbfromspeed(param,&extraticks);
						if (extraticks != 0) {
							psy_audio_PatternEvent entry;

							self->speedpatch=TRUE;
							psy_audio_patternevent_clear(&entry);								
							entry.cmd =psy_audio_PATTERNCMD_EXTENDED;
							entry.parameter =psy_audio_PATTERNCMD_ROW_EXTRATICKS | extraticks;
							modsongloader_writepatternentry(self, &node,
								pattern, row, song->properties.tracks, &entry);
						}
					}
					else
					{
						e.cmd =psy_audio_PATTERNCMD_SET_TEMPO;
					}
					break;
				default:
					e.cmd = XM_SAMPLER_CMD_NONE;
					break;
			}
			// instrument/note
			if ( note != 255 ) e.note  = note+12;
			else e.note  = note;
			if ( instr != 0 ) e.inst = instr-1;
			else e.inst = 255;

			// If empty, do not inform machine
			if (e.note == psy_audio_NOTECOMMANDS_EMPTY && e.inst == 255 && e.cmd == 00 && e.parameter == 00) {
				e.mach = 255;
			}
			// if instrument without note, or note without instrument, cannot use virtual instrument, so use sampulse directly
			else if (( e.note == psy_audio_NOTECOMMANDS_EMPTY && e.inst != 255 ) || ( e.note < psy_audio_NOTECOMMANDS_RELEASE && e.inst == 255)) {
				e.mach = 0;
				if (e.inst != 255) {
					//We cannot use the virtual instrument, but we should remember which it is.
					if (psy_table_exists(&self->xmtovirtual, e.inst)) {
						lastmach[col] = (uint8_t)(uintptr_t)psy_table_at(&self->xmtovirtual, e.inst);
					}
				}
			}
			//default behaviour, let's find the virtual instrument.
			else {				
				e.inst = instr;
				if (!psy_table_exists(&self->xmtovirtual, e.inst)) {
					if (e.inst != 255) {
						e.mach = 0;
						lastmach[col] = e.mach;
					} else if (lastmach[col] != 255) {
						e.mach = (uint8_t)lastmach[col];
					} else {
						e.mach = 255;
					}
				} else {
					e.mach = (uint8_t)(uintptr_t)psy_table_at(
						&self->xmtovirtual, e.inst);
					e.inst = 255;					
				}
			}
			modsongloader_writepatternentry(self, &node, pattern, row, col, &e);
		}
	}
}
	
unsigned char modsongloader_convertperiodtonote(unsigned short period)
{
	if (period==0) {
		return 255;
	}
	else if (BIGMODPERIODTABLE[295] <= period && period <= BIGMODPERIODTABLE[0]) {
		int count2=0;
		for (;count2<37; count2++)
		{
			if (period == BIGMODPERIODTABLE[count2*8]) {
				break;
			}
			else if (period > BIGMODPERIODTABLE[count2*8]) {
				if (period < BIGMODPERIODTABLE[(count2*8)-4]) {
					break;
				}
				count2--;
				break;
				//TODO: Supposedly, we should add the command XMCMD_E::E_FINETUNE
			}
		}
		return count2-1+36;
	}
	else {
		int note = (int)((152.89760383681376337437517761588 /*48 + 12*log2(1.0/428.0)*/
			- log10((double)(period)) * 39.863137138648348174443833153873) + 0.5); /*12/log10(2)*/
		return (unsigned char)(note);
	}
}
	
bool modsongloader_writepatternentry(MODSongLoader* self,
	psy_audio_PatternNode** node, psy_audio_Pattern* pattern,
	uintptr_t row, uintptr_t col, const psy_audio_PatternEvent* e)
{
	assert(self);
	assert(pattern && node && e);

	if (!psy_audio_patternevent_empty(e)) {
		psy_audio_PatternEvent ev;

		ev = *e;
		if (e->inst == psy_audio_NOTECOMMANDS_EMPTY) {
			// until now empty instruments has been marked with 255,
			// correct it to psy_audio_NOTECOMMANDS_INST_EMPTY (16bit)
			ev.inst = psy_audio_NOTECOMMANDS_INST_EMPTY;
		}
		*node = psy_audio_pattern_insert(pattern, *node, col,
			(psy_dsp_beat_t)(row * 1.0 / self->songfile->song->properties.lpb),
			&ev);
	}
	return TRUE;
}	

void modsongloader_loadinstrument(MODSongLoader* self, int idx)
{	
	psy_audio_Instrument* instrument;
	PsyFile* fp;
	psy_audio_Song* song;

	fp = self->songfile->file;
	song = self->songfile->song;	

	instrument = psy_audio_instrument_allocinit();
	psy_audio_instrument_setindex(instrument, idx);
	psy_audio_instruments_insert(&song->instruments, instrument,
		psy_audio_instrumentindex_make(INSTRUMENTGROUP, idx));
	psy_audio_instrument_setname(instrument, self->samples[idx].sampleName);	
	if (self->samples[idx].sampleLength > 0 ) 
	{
		psy_audio_InstrumentEntry instentry;		

		modsongloader_loadsampledata(self, psy_audio_samples_at(
			&song->samples, sampleindex_make(idx, 0)), idx);		
		// create instrument entries
		psy_audio_instrument_clearentries(instrument);
		psy_audio_instrumententry_init(&instentry);
		instentry.sampleindex =
			sampleindex_make(idx, 0);
		psy_audio_instrument_addentry(instrument, &instentry);
	}

	//instr.ValidateEnabled();	
}

void modsongloader_loadsampleheader(MODSongLoader* self, psy_audio_Sample* wave, int instridx)
{
	PsyFile* fp;
	BOOL bLoop;
	char tmpfine;

	fp = self->songfile->file;
	psyfile_read(fp, self->samples[instridx].sampleName,22);
	self->samples[instridx].sampleName[21]='\0';
	self->smplen[instridx] = (psyfile_read_uint8(fp)*0x100+psyfile_read_uint8(fp))*2; 
	self->samples[instridx].sampleLength = self->smplen[instridx];
	self->samples[instridx].finetune = psyfile_read_uint8(fp);
	self->samples[instridx].volume = psyfile_read_uint8(fp);
	self->samples[instridx].loopStart =((psyfile_read_uint8(fp)*256+psyfile_read_uint8(fp))*2);
	self->samples[instridx].loopLength = (psyfile_read_uint8(fp)*256+psyfile_read_uint8(fp))*2;

	// parse
	bLoop = (self->samples[instridx].loopLength > 3);
	
	if (self->smplen[instridx] > 0 )
	{
		wave->numframes = self->smplen[instridx];
		psy_audio_sample_allocwavedata(wave);		
	}

	if(bLoop)
	{		
		wave->loop.type = psy_audio_SAMPLE_LOOP_NORMAL;
		wave->loop.start = self->samples[instridx].loopStart;
		if (self->samples[instridx].loopStart+self->samples[instridx].loopLength > self->smplen[instridx])
		{
			wave->loop.end = self->smplen[instridx];
		} else wave->loop.end = self->samples[instridx].loopStart+self->samples[instridx].loopLength;
	} else {
		wave->loop.type = psy_audio_SAMPLE_LOOP_DO_NOT;
	}

	wave->defaultvolume = self->samples[instridx].volume * 2;
	wave->samplerate = (psy_dsp_big_hz_t)8363.0;
	tmpfine = (char)self->samples[instridx].finetune;
	if (tmpfine > 7 ) tmpfine -= 16;
	// finetune has +-100 range in Psycle
	wave->finetune = (int16_t)(tmpfine * 12.5);
	psy_audio_sample_setname(wave, self->samples[instridx].sampleName);
}

void modsongloader_loadsampledata(MODSongLoader* self, psy_audio_Sample* _wave, int iinstridx)
{
	// parse	
	int16_t oldvalue;
	uintptr_t j;
	unsigned char* smpbuf;

	// cache sample data
	smpbuf = (unsigned char*)malloc(self->smplen[iinstridx]);
	if (smpbuf) {
		uint16_t sampleCnt;

		psyfile_read(self->songfile->file, smpbuf, self->smplen[iinstridx]);

		sampleCnt = self->smplen[iinstridx];
		// 8 bit mono sample
		oldvalue = 0;
		for (j = 0; j < sampleCnt && j < _wave->numframes; j++)
		{
			int16_t value;
			//In mods, samples are signed integer, so we can simply left shift
			value = (int16_t)(smpbuf[j] << 8);
			value += oldvalue;
			_wave->channels.samples[0][j] = (psy_dsp_amp_t)value;
			oldvalue = value;
		}

		// cleanup
		free(smpbuf);
	}
}
