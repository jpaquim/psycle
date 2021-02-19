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
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

// typedef bool BOOL;

// default sampulse group
#define INSTRUMENTGROUP 1 

#define ASSERT assert
	
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
static void xmsongloader_tweaksampler(XMSongLoader*, int twk, int val);
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
	int status;

	// get header
	memset(id, 0, 18);
	if (psyfile_seek(self->fp, 0) == -1) {
		return FALSE;
	}
	if (status = psyfile_read(self->fp, id, 17)) {
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
		self->songfile->song->machinefactory, psy_audio_XMSAMPLER, "",
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
	int status;

	// get song name
	memset(songname, 0, 21);
	if (psyfile_seek(self->fp, 17) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, songname, 20)) {
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
	int status;

	// get data
	if (psyfile_seek(self->fp, 60) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &self->header, sizeof(XMFILEHEADER))) {
		return status;
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
	xmsongloader_tweaksampler(self, XM_SAMPLER_TWK_AMIGASLIDES,
		(self->header.flags & 0x01)
		? FALSE
		: TRUE);	
}

void xmsongloader_setsamplerpanningmode(XMSongLoader* self)
{
	xmsongloader_tweaksampler(self, XM_SAMPLER_TWK_PANNINGMODE, 1);
}

void xmsongloader_tweaksampler(XMSongLoader* self, int twk, int val)
{
	psy_audio_MachineParam* param;

	param = psy_audio_machine_tweakparameter(self->sampler, twk);
	if (param) {
		psy_audio_machine_parameter_tweak_scaled(self->sampler, param, val);
	}
}

void xmsongloader_makesequence(XMSongLoader* self, struct XMFILEHEADER* xmheader)
{
	uint16_t i;		
	
	psy_audio_sequence_appendtrack(&self->song->sequence,
		psy_audio_sequencetrack_allocinit());
	for (i = 0; i < xmheader->norder; ++i) {		
		psy_audio_sequence_insert(&self->song->sequence,
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
	int status;
	
	if (start >= 0) {
		if (psyfile_seek(self->fp, (uint32_t)start) == -1) {
			return PSY_ERRFILE;
		}
	}
	if (status = psyfile_read(self->fp, &headerlen, sizeof(int32_t))) {
		return status;
	}
	//char iPackingType = psyfile_read_int8();
	if (psyfile_skip(self->fp, 1) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &numrows, sizeof(int16_t))) {
		return status;
	}
	if (status = psyfile_read(self->fp, &packedsize, sizeof(int16_t))) {
		return status;
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
				if (status = psyfile_read(self->fp, &note, sizeof(uint8_t))) {
					free(ppattern);
					return status;
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
						if (status = psyfile_read(self->fp, &note, sizeof(uint8_t))) {
							free(ppattern);
							return status;
						}
					}
					if (bReadInstr) {
						if (status = psyfile_read(self->fp, &instr, sizeof(uint8_t))) {
							free(ppattern);
							return status;
						}						
					}
					if (bReadVol) {
						if (status = psyfile_read(self->fp, &vol, sizeof(uint8_t))) {
							free(ppattern);
							return status;
						}
					}
					if (bReadType) {
						if (status = psyfile_read(self->fp, &type, sizeof(uint8_t))) {
							free(ppattern);
							return status;
						}
					}
					if (bReadParam) {
						if (status = psyfile_read(self->fp, &param, sizeof(uint8_t))) {
							free(ppattern);
							return status;
						}
					}
				} else
				{
					// read all values
					if (status = psyfile_read(self->fp, &instr, sizeof(uint8_t))) {
						free(ppattern);
						return status;
					}
					if (status = psyfile_read(self->fp, &vol, sizeof(uint8_t))) {
						free(ppattern);
						return status;
					}
					if (status = psyfile_read(self->fp, &type, sizeof(uint8_t))) {
						free(ppattern);
						return status;
					}
					if (status = psyfile_read(self->fp, &param, sizeof(uint8_t))) {
						free(ppattern);
						return status;
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
		psy_audio_convert_legacypattern(pattern, ppattern, MAX_TRACKS, numrows,
			(uint32_t)psy_audio_song_lpb(self->song));
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
	int status;
			
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
	if (status = psyfile_read(self->fp, &instrsize, sizeof(int32_t))) {
		return status;
	}
	start += instrsize;
	memset(instrname, 0, sizeof(instrname));
	if (status = psyfile_read(self->fp, instrname, 22)) {
		return status;
	}
	psy_audio_instrument_setname(instrument, instrname);
	// //int iInstrType = ReadInt1();
	if (psyfile_skip(self->fp, 1) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &samplecount, sizeof(uint16_t))) {
		return status;
	}	
	if (samplecount > 1) {
		//TRACE(_T("ssmple count = %d\n"), samplecount);
	}
	if (samplecount == 0) {
		*rv_nextstart = start;
		return PSY_OK;
	}

	memset(&samph, 0, sizeof samph);
	if (status = psyfile_read(self->fp, &samph, sizeof(XMSAMPLEHEADER))) {
		return status;
	}
	xmsongloader_setenvelopes(instrument, &samph);
	// load individual samples	
	for (i = 0; i < samplecount; i++) {
		int status;
		psy_audio_Sample* wave;

		wave = psy_audio_sample_allocinit(0);		
		if (status = xmsongloader_loadsampleheader(self, wave, start, i, &start)) {
			psy_audio_sample_deallocate(wave);
			return PSY_ERRFILE;
		}
		// Only get REAL samples.
		// if (self->smplen[*cursample] > 0) {
			psy_audio_samples_insert(&self->song->samples, wave,
				psy_audio_sampleindex_make(slot, i));
		// }
	}
	// load individual samples
	for (i = 0; i < samplecount; i++) {
		int status;
		psy_audio_Sample* wave;

		wave = psy_audio_samples_at(&self->song->samples,
			psy_audio_sampleindex_make(slot, i));
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
		psy_audio_sampleindex_make(slot, samph.snum[0]);
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
	int status;

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
	if (status = psyfile_read(self->fp, smpbuf, self->smplen[sampleidx])) {
		free(smpbuf);
		return status;
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
		int j;

		for (j = 0; j <= samplecnt - 2; j += 2)
		{
			wNew += (smpbuf[j] & 0xFF) | (smpbuf[j + 1] << 8);
			wave->channels.samples[0][out] = (psy_dsp_amp_t)wNew;			
			out++;
		}
	} else
	{		
		// 8 bit mono sample
		int j;

		for (j = 0; j < samplecnt; j++)
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
		psy_dsp_envelope_append(&inst->volumeenvelope, psy_dsp_envelopepoint_make_all(
			sampleHeader->venv[0], (float)sampleHeader->venv[1] / 64.0f, 0.0, 65535.f, 0.0, 1.0));		
		for (i = 1; i < envelope_point_num; i++) {
			if (sampleHeader->venv[i * 2] > sampleHeader->venv[(i - 1) * 2])// Some rare modules have erroneous points. This tries to solve that.
				psy_dsp_envelope_append(&inst->volumeenvelope, psy_dsp_envelopepoint_make_all(
					sampleHeader->venv[i * 2], (float)sampleHeader->venv[i * 2 + 1] / 64.0f,
					0.0, 65535.f, 0.0, 1.0));
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
		int i;

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

		for(i = 0; i < envelope_point_num;i++){
			psy_dsp_envelope_append(&inst->panenvelope, psy_dsp_envelopepoint_make_all(
				sampleHeader->penv[i * 2], (float)(sampleHeader->penv[i * 2 + 1] -32.0f )/ 32.0f,
				0.0, 65535.f, 0.0, 1.0));
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
	int status;

	// get sample header
	if (psyfile_seek(self->fp, (uint32_t)start) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &len, sizeof(int32_t))) {
		return status;
	}	

	// loop data
	if (status = psyfile_read(self->fp, &loopstart, sizeof(int32_t))) {
		return status;
	}
	if (status = psyfile_read(self->fp, &looplength, sizeof(int32_t))) {
		return status;
	}	

	// params
	if (status = psyfile_read(self->fp, &vol, sizeof(int8_t))) {
		return status;
	}
	if (status = psyfile_read(self->fp, &finetune, sizeof(int8_t))) {
		return status;
	}
	if (status = psyfile_read(self->fp, &flags, sizeof(int8_t))) {
		return status;
	}
	if (status = psyfile_read(self->fp, &panning, sizeof(uint8_t))) {
		return status;
	}
	if (status = psyfile_read(self->fp, &relativenote, sizeof(int8_t))) {
		return status;
	}
	//char iReserved = ReadInt1();
	if (psyfile_skip(self->fp, 1) == -1) {
		return PSY_ERRFILE;
	}	

	// sample name
	memset(cname, 0, sizeof(cname));
	if (status = psyfile_read(self->fp, &cname, 22)) {
		return status;
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
	wave->zone.tune = relativenote;
	wave->zone.finetune = (int16_t)(finetune / 1.28); // WaveFineTune has +-100 range in Psycle.
	psy_audio_sample_setname(wave, cname);	

	self->smplen[sampleidx] = len;
	self->smpflags[sampleidx] = flags;

	start += 40;
	*rv_nextstart = start;
	return PSY_OK;
}

// XI Instrument load
int xmsongloader_loadxi(XMSongLoader* self, psy_audio_InstrumentIndex index)
{
	int exchwave[4] = {
		psy_audio_WAVEFORMS_SINUS,
		psy_audio_WAVEFORMS_SQUARE,
		psy_audio_WAVEFORMS_SAWDOWN,
		psy_audio_WAVEFORMS_SAWUP
	};
	int note;
	int instidx;
	XMINSTRUMENTFILEHEADER fileheader;
	XMSAMPLEFILEHEADER insheader;
	XMSAMPLEHEADER insheaderb;
	char* pinsheaderb;
	psy_audio_Instrument* instr;
	psy_audio_InstrumentEntry instentry;
	uint32_t samplecount;
	uint32_t i;
	int sample;
	int status;

	instidx = (int)index.subslot;
	if (status = psyfile_read(self->fp, &fileheader, sizeof(XMINSTRUMENTFILEHEADER))) {
		return status;
	}
	if (strncmp(fileheader.extxi, XI_HEADER, sizeof(fileheader.extxi)) != 0) {
		return PSY_ERRFILE;
	}
	fileheader.name[22] = 0;
	if (status = psyfile_read(self->fp, &insheader, sizeof(XMSAMPLEFILEHEADER))) {
		return status;
	}
	pinsheaderb = (char*)(&insheaderb) + 4;
	memcpy(pinsheaderb, &insheader, sizeof(XMSAMPLEFILEHEADER) - 2);

	// create and insert instrument
	instr = psy_audio_instrument_allocinit();
	psy_audio_instrument_setindex(instr, instidx);
	psy_audio_instruments_insert(&self->song->instruments, instr,
		psy_audio_instrumentindex_make(index.groupslot, instidx));
	psy_dsp_envelope_clear(&instr->volumeenvelope);
	psy_dsp_envelope_setmode(&instr->volumeenvelope, psy_dsp_ENVELOPETIME_TICK);
	psy_dsp_envelope_clear(&instr->panenvelope);
	psy_dsp_envelope_setmode(&instr->panenvelope, psy_dsp_ENVELOPETIME_TICK);
	psy_dsp_envelope_clear(&instr->pitchenvelope);
	psy_dsp_envelope_setmode(&instr->pitchenvelope, psy_dsp_ENVELOPETIME_TICK);
	psy_dsp_envelope_clear(&instr->filterenvelope);
	psy_dsp_envelope_setmode(&instr->filterenvelope, psy_dsp_ENVELOPETIME_TICK);

	psy_audio_instrument_setname(instr, fileheader.name);
	xmsongloader_setenvelopes(instr, &insheaderb);

	samplecount = 0;
	for (i = 0; i < 96; ++i)
	{
		if (insheader.snum[i] > samplecount) samplecount = insheader.snum[i];
	}
	samplecount++;
	if (samplecount > 32) samplecount = 32;

	sample = -1;
	/*if (loadhelp.IsPreview())
	{
		if (insheader.snum[48] >= samplecount)
		{
			for (int f = 0; f < 96; f++) if (insheader.snum[f] < samplecount) {
				sample = insheader.snum[f];
				break;
			}
		} else { sample = insheader.snum[48]; }
		std::vector<XMInstrument::WaveData<>*> waves(samplecount);
		for (i = 0; i < samplecount; i++)
		{
			waves[i] = new XMInstrument::WaveData<>();
			LoadSampleHeader(*waves[i], GetPos(), instidx, i);
		}
		// load individual samples
		for (i = 0; i < samplecount; i++)
		{
			if (smpLen[i] > 0)
			{
				LoadSampleData(*waves[i], GetPos(), instidx, i);
				if (i == sample) {
					int dummy = -1;
					XMInstrument::WaveData<>& _wave = loadhelp.GetNextSample(dummy);
					_wave = *waves[i];
					_wave.VibratoAttack(insheader.vibsweep == 0 ? 0 : 256 - insheader.vibsweep);
					_wave.VibratoDepth(insheader.vibdepth);
					_wave.VibratoSpeed(insheader.vibrate);
					_wave.VibratoType(exchwave[insheader.vibtype & 3]);
					break;
				}
			}
		}
		for (i = 0; i < samplecount; i++)
		{
			delete waves[i];
		}
	} else {*/
	{
		// read instrument data
		for (i = 0; i < samplecount; ++i) {
			int status;
			uintptr_t start;
			psy_audio_Sample* wave;

			wave = psy_audio_sample_allocinit(0);
			start = psyfile_getpos(self->fp);
			if (status = xmsongloader_loadsampleheader(self, wave, start, i, &start)) {
				psy_audio_sample_deallocate(wave);
				return PSY_ERRFILE;
			}
			// Only get REAL samples.
			// if (self->smplen[*cursample] > 0) {
			psy_audio_samples_insert(&self->song->samples, wave,
				psy_audio_sampleindex_make(instidx, i));
			// }
		}
		// load individual samples		
		for (i = 0; i < samplecount; ++i) {
			int status;
			psy_audio_Sample* wave;
			uintptr_t start;

			wave = psy_audio_samples_at(&self->song->samples,
				psy_audio_sampleindex_make(instidx, i));
			assert(wave);
			if (!wave) {
				return PSY_ERRFILE;
			}
			start = psyfile_getpos(self->fp);
			if (status = xmsongloader_loadsampledata(self, wave, i, start, &start)) {
				return status;
			}
			wave->vibrato.attack = insheader.vibsweep == 0 ? 0 : 256 - insheader.vibsweep;
			wave->vibrato.depth = insheader.vibdepth;
			wave->vibrato.speed = insheader.vibrate;
			wave->vibrato.type = exchwave[insheader.vibtype & 3];
		}
		// create instrument entries
		psy_audio_instrument_clearentries(instr);
		psy_audio_instrumententry_init(&instentry);
		instentry.sampleindex =
			psy_audio_sampleindex_make(instidx, insheader.snum[0]);
		for (note = 1; note < 96; ++note) {
			if (insheader.snum[note] != instentry.sampleindex.subslot) {
				instentry.keyrange.high = note - 1;
				psy_audio_instrument_addentry(instr, &instentry);
				instentry.keyrange.low = note;
				instentry.sampleindex.subslot = insheader.snum[note];
			}
		}
		instentry.keyrange.high = 119;
		psy_audio_instrument_addentry(instr, &instentry);		
		// instr.ValidateEnabled();		
	}
	return PSY_OK;
}

// MODSongLoader
//
// prototypes
static void modsongloader_reset(MODSongLoader*);
static bool modsongloader_isvalid(MODSongLoader*);
static bool modsongloader_makexmsampler(MODSongLoader*);
static void modsongloader_setsamplerpanningmode(MODSongLoader*);
static void modsongloader_setsamplerslidemode(MODSongLoader*);
static void modsongloader_tweaksampler(MODSongLoader*, int twk, int val);
static int modsongloader_readtitle(MODSongLoader*);
static void modsongloader_setsongcomments(MODSongLoader*);
static void modsongloader_setnumsongtracks(MODSongLoader*);
static void modsongloader_setchannelpanning(MODSongLoader*);
static int modsongloader_loadpatterns(MODSongLoader*);
static int modsongloader_loadsinglepattern(MODSongLoader*, int patidx, int tracks);
static unsigned char modsongloader_convertperiodtonote(unsigned short period);
static bool modsongloader_writepatternentry(MODSongLoader*,
	psy_audio_LegacyPattern, int row, int col, psy_audio_LegacyPatternEntry*);
static void modsongloader_makesequence(MODSongLoader*, struct MODHEADER*);
static int modsongloader_loadinstrument(MODSongLoader*, int idx);
static int modsongloader_loadsampleheader(MODSongLoader*, psy_audio_Sample*,
	int instridx);
static int modsongloader_loadsampledata(MODSongLoader*, psy_audio_Sample*,
	int instridx);
// implementation
void modsongloader_init(MODSongLoader* self, psy_audio_SongFile* songfile)
{
	assert(self);

	assert(songfile);
	assert(songfile->file);
	assert(songfile->song);

	self->songfile = songfile;
	self->fp = songfile->file;
	self->song = songfile->song;
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

int modsongloader_load(MODSongLoader* self)
{
	int status;		
	int i;
	int virtidx;	

	assert(self);

	status = PSY_OK;
	if (!modsongloader_isvalid(self)) {
		return PSY_ERRFILE;
	}
	modsongloader_reset(self);	
	if (!modsongloader_makexmsampler(self)) {
		return PSY_ERRFILE;
	}
	modsongloader_setsamplerpanningmode(self);
	modsongloader_setsamplerslidemode(self);
	if (status = modsongloader_readtitle(self)) {
		return status;
	}
	modsongloader_setsongcomments(self);
	// get data
	if (psyfile_seek(self->fp, 20) == -1) {
		return PSY_ERRFILE;
	}
	for (i=0; i < 31; i++) {
		psy_audio_Sample* wave;

		wave = psy_audio_sample_allocinit(1);	
		if (status = modsongloader_loadsampleheader(self, wave, i)) {
			psy_audio_sample_deallocate(wave);
			return status;
		}
		psy_audio_samples_insert(&self->song->samples, wave,
			psy_audio_sampleindex_make(i, 0));
	}
	if (psyfile_seek(self->fp, 950) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &self->header, sizeof(self->header))) {
		return status;
	}
	modsongloader_setnumsongtracks(self);
	psy_audio_song_setbpm(self->song, 125);
	psy_audio_song_setlpb(self->song, 4);
	psy_audio_song_settpb(self->song, 24);
	psy_audio_song_setextraticksperbeat(self->song, 0);	
	
	virtidx = MAX_MACHINES;
	for (i = 0; i < 31; ++i) {
		if (self->samples[i].sampleLength > 0 ) {
			psy_table_insert(&self->xmtovirtual, i, (void*)(uintptr_t)virtidx);			
			virtidx++;
		}
	}
	modsongloader_setchannelpanning(self);
	if (status = modsongloader_loadpatterns(self)) {
		return status;
	}
	for(i = 0; i < 31; ++i) {
		modsongloader_loadinstrument(self, i);
		// sampulse: 0
		psy_audio_song_insertvirtualgenerator(self->songfile->song,
			(uintptr_t)psy_table_at(&self->xmtovirtual, i), 0, i);
	}
	modsongloader_makesequence(self, &self->header);
	return PSY_OK;
}

bool modsongloader_isvalid(MODSongLoader* self)
{
	bool bIsValid;
	char id[5];
	int status;

	bIsValid = FALSE;	
	memset(id, 0, 5);
	if (psyfile_seek(self->fp, 1080) == -1) {
		return FALSE;
	}
	if (status = psyfile_read(self->fp, id, 4)) {
		return FALSE;
	}
	bIsValid = !strncmp(id,"M.K.",4);
	if ( !bIsValid ) bIsValid = !strncmp(id,"M!K!",4);
	if ( !bIsValid ) bIsValid = !strncmp(id+1,"CHN",4);
	if ( !bIsValid ) bIsValid = !strncmp(id+2,"CH",4);
	
	return bIsValid;
}

bool modsongloader_makexmsampler(MODSongLoader* self)
{
	self->sampler = psy_audio_machinefactory_makemachine(
		self->songfile->song->machinefactory, psy_audio_XMSAMPLER, "",
		psy_INDEX_INVALID);
	if (self->sampler) {
		psy_audio_Wire wire;

		psy_audio_machine_setposition(self->sampler, rand() / 64, rand() / 80);
		psy_audio_machines_insert(&self->song->machines, 0, self->sampler);
		wire = psy_audio_wire_make(0, psy_audio_MASTER_INDEX);
		psy_audio_machines_connect(&self->song->machines, wire);
		psy_audio_connections_setwirevolume(&self->song->machines.connections,
			wire, 0.5f);
	}
	return self->sampler != NULL;
}

void modsongloader_setsamplerpanningmode(MODSongLoader* self)
{
	// XM_SAMPLER_PanningMode::TwoWay
	modsongloader_tweaksampler(self, XM_SAMPLER_TWK_PANNINGMODE, 1);	
}

void modsongloader_setsamplerslidemode(MODSongLoader* self)
{
	modsongloader_tweaksampler(self, XM_SAMPLER_TWK_AMIGASLIDES, TRUE);	
}

void modsongloader_tweaksampler(MODSongLoader* self, int twk, int val)
{
	psy_audio_MachineParam* param;

	param = psy_audio_machine_tweakparameter(self->sampler, twk);
	if (param) {
		psy_audio_machine_parameter_tweak_scaled(self->sampler, param, val);
	}
}


int modsongloader_readtitle(MODSongLoader* self)
{
	char songname[21];
	int status;

	// get song name
	memset(songname, 0, 21);
	if (psyfile_seek(self->fp, 0) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, songname, 20)) {
		return status;
	}
	psy_audio_song_settitle(self->song, songname);
	return PSY_OK;
}

void modsongloader_setsongcomments(MODSongLoader* self)
{
	char_dyn_t* comments;

	comments = strdup("Imported from MOD Module: ");
	comments = psy_strcat_realloc(comments, self->songfile->path);
	psy_audio_song_setcomments(self->song, comments);
	free(comments);
	comments = NULL;
}

void modsongloader_setnumsongtracks(MODSongLoader* self)
{
	char pID[5];

	pID[0] = self->header.pID[0];
	pID[1] = self->header.pID[1];
	pID[2] = self->header.pID[2];
	pID[3] = self->header.pID[3];
	pID[4] = 0;
	if (!strncmp(pID, "M.K.", 4)) {
		psy_audio_song_setnumsongtracks(self->song, 4);
	} else if (!strncmp(pID, "M!K!", 4)) {
		psy_audio_song_setnumsongtracks(self->song, 4);
	} else if (!strncmp(pID + 1, "CHN", 4)) {
		char tmp[2]; tmp[0] = pID[0]; tmp[1] = 0;
		psy_audio_song_setnumsongtracks(self->song, atoi(tmp));
	} else if (!strncmp(pID + 2, "CH", 4)) {
		char tmp[3]; tmp[0] = pID[0]; tmp[1] = pID[1]; tmp[2] = 0;
		psy_audio_song_setnumsongtracks(self->song, atoi(tmp));
	}
}

void modsongloader_setchannelpanning(MODSongLoader* self)
{
	if (psy_audio_song_numsongtracks(self->song) <= 8) {
		uintptr_t i;
				
		for (i = 0; i < psy_audio_song_numsongtracks(self->song); ++i) {
			uintptr_t paramidx;
			psy_audio_MachineParam* param;

			paramidx = XM_SAMPLER_TWK_CHANNEL_START +
				i * 4 + XM_SAMPLER_TWK_CHANNEL_PANNING;
			param = psy_audio_machine_tweakparameter(self->sampler, paramidx);
			if (param) {
				if (i % 4 == 0 || i % 4 == 3) {
					psy_audio_machine_parameter_tweak(self->sampler, param, 0.25f);
				} else {
					psy_audio_machine_parameter_tweak(self->sampler, param, 0.75f);
				}
			}
		}
	} else {
		uintptr_t i;
		
		for (i = 0; i < psy_audio_song_numsongtracks(self->song); ++i) {
			uintptr_t paramidx;
			psy_audio_MachineParam* param;

			paramidx = i * 4 + XM_SAMPLER_TWK_CHANNEL_PANNING;
			param = psy_audio_machine_tweakparameter(self->sampler, paramidx);
			if (param) {
				psy_audio_machine_parameter_tweak(self->sampler, param, 0.5f);
			}
		}
	}
}

int modsongloader_loadpatterns(MODSongLoader* self)
{	
	uint8_t j;
	int i;
	int npatterns;	

	npatterns = 0;
	for (i = 0; i < self->header.songlength; ++i) {					
		if (self->header.order[i] > npatterns) {
			npatterns = self->header.order[i];
		}
	}
	++npatterns;
	if (psyfile_seek(self->songfile->file, 1084) == -1) {
		return PSY_ERRFILE;
	}
	for(j = 0; j < npatterns; ++j){
		int status;

		// get pattern data
		if (status = modsongloader_loadsinglepattern(self, j,
				(int)psy_audio_song_numsongtracks(self->song))) {
			return status;
		}
	}
	if (self->speedpatch) {
		psy_audio_song_setnumsongtracks(self->song,
			psy_audio_song_numsongtracks(self->song) + 1);		
	}
	return PSY_OK;
}

void modsongloader_makesequence(MODSongLoader* self,
	struct MODHEADER* modheader)
{
	uint8_t i;
	
	psy_audio_sequence_appendtrack(&self->song->sequence,
		psy_audio_sequencetrack_allocinit());
	for (i = 0; i < modheader->songlength; ++i) {		
		psy_audio_sequence_insert(&self->song->sequence,
			psy_audio_orderindex_make(0, i), modheader->order[i]);		
	}
}

int modsongloader_loadsinglepattern(MODSongLoader* self, int patidx, int tracks)
{
	short iNumRows = 64;
	psy_audio_Pattern* pattern;	
	unsigned char lastmach[64];
	psy_audio_LegacyPatternEntry e;
	unsigned char mentry[4];
	int row;
	int col;
	int temp;
	psy_audio_PatternNode* node;
	psy_audio_LegacyPattern ppattern;

	// create new psycle version pattern
	pattern = psy_audio_pattern_allocinit();
	psy_audio_pattern_setname(pattern, "unnamed");
	psy_audio_patterns_insert(&self->song->patterns, patidx, pattern);
	psy_audio_pattern_setlength(pattern,
		iNumRows * 1.0 / psy_audio_song_lpb(self->song));
	// create old mfc-psycle pattern for read
	ppattern = psy_audio_allocoldpattern(pattern,
		psy_audio_song_lpb(self->song), &temp);
	iNumRows = (int16_t)temp;	
	
	memset(lastmach,255,sizeof(char)*64);
		
	// get next values
	node = NULL;
	for(row = 0; row < iNumRows; row++)
	{
		for(col=0; col< tracks; col++)
		{	
			// reset
			uint8_t _note;
			uint8_t _instr;
			uint8_t type;
			uint8_t param;
			uint16_t period;
			int exchwave[3] = {
				psy_audio_WAVEFORMS_SINUS,
				psy_audio_WAVEFORMS_SAWDOWN,
				psy_audio_WAVEFORMS_SQUARE
			};
			int status;

			_note = PSY2_NOTECOMMANDS_EMPTY;
			_instr = 255;
			type = 0;
			param = 0;
			period = 428;			

			// read _note
			if (status = psyfile_read(self->fp, &mentry[0], sizeof(uint8_t))) {
				free(ppattern);
				return status;
			}
			if (status = psyfile_read(self->fp, &mentry[1], sizeof(uint8_t))) {
				free(ppattern);
				return status;
			}
			if (status = psyfile_read(self->fp, &mentry[2], sizeof(uint8_t))) {
				free(ppattern);
				return status;
			}
			if (status = psyfile_read(self->fp, &mentry[3], sizeof(uint8_t))) {
				free(ppattern);
				return status;
			}
			_instr = ((mentry[0] & 0xF0) + (mentry[2] >> 4));
			period = ((mentry[0]& 0x0F) << 8) + mentry[1];
			type = (mentry[2] & 0x0F);
			param = mentry[3];
			_note = modsongloader_convertperiodtonote(period);

			// translate
			e._parameter = param;			
			switch(type){
				case XMCMD_ARPEGGIO:
					if(param != 0){
						e._cmd = XM_SAMPLER_CMD_ARPEGGIO;
					} else {
						e._cmd = XM_SAMPLER_CMD_NONE;
					}
					break;
				case XMCMD_PORTAUP:
					e._cmd = XM_SAMPLER_CMD_PORTAMENTO_UP;
					break;
				case XMCMD_PORTADOWN:
					e._cmd = XM_SAMPLER_CMD_PORTAMENTO_DOWN;
					break;
				case XMCMD_PORTA2NOTE:
					e._cmd = XM_SAMPLER_CMD_PORTA2NOTE;
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
					e._cmd = XM_SAMPLER_CMD_OFFSET;
					break;
				case XMCMD_VOLUMESLIDE:
					e._cmd = XM_SAMPLER_CMD_VOLUMESLIDE;
					break;
				case XMCMD_POSITION_JUMP:
					e._cmd = psy_audio_PATTERNCMD_JUMP_TO_ORDER;
					break;
				case XMCMD_VOLUME:
					e._cmd = XM_SAMPLER_CMD_VOLUME;
					e._parameter = param<=0x40?param*2:0x80;
					break;
				case XMCMD_PATTERN_BREAK:
					e._cmd =psy_audio_PATTERNCMD_BREAK_TO_LINE;
					e._parameter = ((param&0xF0)>>4)*10 + (param&0x0F);
					break;
				case XMCMD_EXTENDED:
					switch(param & 0xf0){
					case XMCMD_E_FINE_PORTA_UP:
						e._cmd = XM_SAMPLER_CMD_PORTAMENTO_UP;
						e._parameter= 0xF0+(param&0x0F);
						break;
					case XMCMD_E_FINE_PORTA_DOWN:
						e._cmd = XM_SAMPLER_CMD_PORTAMENTO_DOWN;
						e._parameter= 0xF0+(param&0x0F);
						break;
					case XMCMD_E_GLISSANDO_STATUS:
						e._cmd = XM_SAMPLER_CMD_EXTENDED;
						e._parameter = XM_SAMPLER_CMD_E_GLISSANDO_TYPE | ((param==0)?0:1);
						break;
					case XMCMD_E_VIBRATO_WAVE:
						e._cmd = XM_SAMPLER_CMD_EXTENDED;
						e._parameter =XM_SAMPLER_CMD_E_VIBRATO_WAVE | exchwave[param & 0x3];
						break;
					case XMCMD_E_FINETUNE:
						e._cmd = XM_SAMPLER_CMD_NONE;
						e._parameter = 0;
						break;
					case XMCMD_E_PATTERN_LOOP:
						e._cmd =psy_audio_PATTERNCMD_EXTENDED;
						e._parameter =psy_audio_PATTERNCMD_PATTERN_LOOP | (param & 0xf);
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
						e._parameter = 0x0f + ((param & 0xf)<<4);
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
						e._parameter = XM_SAMPLER_CMD_E_NOTE_DELAY | ( param & 0xf);
						break;
					case XMCMD_E_PATTERN_DELAY:
						e._cmd =psy_audio_PATTERNCMD_EXTENDED;
						e._parameter = psy_audio_PATTERNCMD_PATTERN_DELAY | (param & 0xf);
						break;
					default:
						e._cmd = XM_SAMPLER_CMD_NONE;
						e._parameter = 0;
						break;
					}
					break;
				case XMCMD_SETSPEED:
					if ( param < 32)
					{
						int extraticks;

						e._cmd=psy_audio_PATTERNCMD_EXTENDED;
						extraticks=0;
						e._parameter = calclpbfromspeed(param,&extraticks);
						if (extraticks != 0) {
							psy_audio_LegacyPatternEntry entry;

							self->speedpatch=TRUE;
							psy_audio_legacypatternentry_init_all(&entry,
								PSY2_NOTECOMMANDS_EMPTY, 0xFF, 0xFF, 0, 0);														
							entry._cmd =psy_audio_PATTERNCMD_EXTENDED;
							entry._parameter =psy_audio_PATTERNCMD_ROW_EXTRATICKS | extraticks;
							modsongloader_writepatternentry(self, ppattern, row,
								(int)psy_audio_song_numsongtracks(self->song), &entry);
						}
					}
					else
					{
						e._cmd =psy_audio_PATTERNCMD_SET_TEMPO;
					}
					break;
				default:
					e._cmd = XM_SAMPLER_CMD_NONE;
					break;
			}
			// _instrument/_note
			if ( _note != 255 ) e._note  = _note+12;
			else e._note  = _note;
			if ( _instr != 0 ) e._inst = _instr-1;
			else e._inst = 255;

			// If empty, do not inform _machine
			if (e._note == PSY2_NOTECOMMANDS_EMPTY && e._inst == 255 && e._cmd == 00 && e._parameter == 00) {
				e._mach = 255;
			}
			// if _instrument without _note, or _note without _instrument, cannot use virtual _instrument, so use sampulse directly
			else if (( e._note == PSY2_NOTECOMMANDS_EMPTY && e._inst != 255 ) || ( e._note < PSY2_NOTECOMMANDS_RELEASE && e._inst == 255)) {
				e._mach = 0;
				if (e._inst != 255) {
					//We cannot use the virtual _instrument, but we should remember which it is.
					if (psy_table_exists(&self->xmtovirtual, e._inst)) {
						lastmach[col] = (uint8_t)(uintptr_t)
							psy_table_at(&self->xmtovirtual, e._inst);
					}
				}
			}
			//default behaviour, let's find the virtual _instrument.
			else {				
				if (!psy_table_exists(&self->xmtovirtual, e._inst)) {
					if (e._inst != 255) {
						e._mach = 0;
						lastmach[col] = e._mach;
					} else if (lastmach[col] != 255) {
						e._mach = (uint8_t)lastmach[col];
					} else {
						e._mach = 255;
					}
				} else {
					e._mach = (uint8_t)(uintptr_t)psy_table_at(
						&self->xmtovirtual, e._inst);
					e._inst = 255;					
				}
			}
			modsongloader_writepatternentry(self, ppattern, row, col, &e);
		}		
	}
	psy_audio_convert_legacypattern(pattern, ppattern, MAX_TRACKS, iNumRows,
		(uint32_t)psy_audio_song_lpb(self->song));
	free(ppattern);
	ppattern = NULL;
	return PSY_OK;
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

int modsongloader_loadinstrument(MODSongLoader* self, int idx)
{	
	psy_audio_Instrument* instrument;			

	instrument = psy_audio_instrument_allocinit();
	psy_audio_instrument_setindex(instrument, idx);
	psy_audio_instruments_insert(&self->song->instruments, instrument,
		psy_audio_instrumentindex_make(INSTRUMENTGROUP, idx));
	psy_audio_instrument_setname(instrument, self->samples[idx].sampleName);	
	if (self->samples[idx].sampleLength > 0) {
		int status;
		psy_audio_InstrumentEntry instentry;		

		if (status = modsongloader_loadsampledata(self,
			psy_audio_samples_at(&self->song->samples,
				psy_audio_sampleindex_make(idx, 0)), idx)) {
			return status;
		}
		// create instrument entries
		psy_audio_instrument_clearentries(instrument);
		psy_audio_instrumententry_init(&instentry);
		instentry.sampleindex = psy_audio_sampleindex_make(idx, 0);
		psy_audio_instrument_addentry(instrument, &instentry);
	}
	//instr.ValidateEnabled();
	return PSY_OK;
}

int modsongloader_loadsampleheader(MODSongLoader* self, psy_audio_Sample* wave,
	int instridx)
{	
	BOOL bLoop;
	char tmpfine;
	uint8_t byte1;
	uint8_t byte2;
	uint8_t finetune;
	uint8_t volume;
	int status;

	if (status = psyfile_read(self->fp, self->samples[instridx].sampleName, 22)) {
		return status;
	}
	self->samples[instridx].sampleName[21]='\0';

	if (status = psyfile_read(self->fp, &byte1, sizeof(uint8_t))) {
		return status;
	}
	if (status = psyfile_read(self->fp, &byte2, sizeof(uint8_t))) {
		return status;
	}
	self->smplen[instridx] = byte1 * 0x100 + byte2 * 2; 	
	self->samples[instridx].sampleLength = self->smplen[instridx];
	if (status = psyfile_read(self->fp, &finetune, sizeof(uint8_t))) {
		return status;
	}
	self->samples[instridx].finetune = finetune;
	if (status = psyfile_read(self->fp, &volume, sizeof(uint8_t))) {
		return status;
	}
	self->samples[instridx].volume = volume;
	if (status = psyfile_read(self->fp, &byte1, sizeof(uint8_t))) {
		return status;
	}
	if (status = psyfile_read(self->fp, &byte2, sizeof(uint8_t))) {
		return status;
	}
	self->samples[instridx].loopStart =(byte1 * 256 + byte2) *2;
	if (status = psyfile_read(self->fp, &byte1, sizeof(uint8_t))) {
		return status;
	}
	if (status = psyfile_read(self->fp, &byte2, sizeof(uint8_t))) {
		return status;
	}
	self->samples[instridx].loopLength = (byte1 * 256 + byte2)*2;
	// parse
	bLoop = (self->samples[instridx].loopLength > 3);	
	if (self->smplen[instridx] > 0) {
		wave->numframes = self->smplen[instridx];
		psy_audio_sample_allocwavedata(wave);		
	}

	if(bLoop) {
		wave->loop.type = psy_audio_SAMPLE_LOOP_NORMAL;
		wave->loop.start = self->samples[instridx].loopStart;
		if (self->samples[instridx].loopStart +
				self->samples[instridx].loopLength > self->smplen[instridx]) {
			wave->loop.end = self->smplen[instridx];
		} else {
			wave->loop.end = self->samples[instridx].loopStart
				+ self->samples[instridx].loopLength;
		}
	} else {
		wave->loop.type = psy_audio_SAMPLE_LOOP_DO_NOT;
	}
	wave->defaultvolume = self->samples[instridx].volume * 2;
	wave->samplerate = (psy_dsp_big_hz_t)8363.0;
	tmpfine = (char)self->samples[instridx].finetune;
	if (tmpfine > 7 ) tmpfine -= 16;
	// finetune has +-100 range in Psycle
	wave->zone.finetune = (int16_t)(tmpfine * 12.5);
	psy_audio_sample_setname(wave, self->samples[instridx].sampleName);
	return PSY_OK;
}

int modsongloader_loadsampledata(MODSongLoader* self, psy_audio_Sample* wave,
	int instidx)
{
	// parse	
	uintptr_t j;
	unsigned char* smpbuf;

	// cache sample data
	smpbuf = (unsigned char*)malloc(self->smplen[instidx]);
	if (smpbuf) {
		uint16_t sampleCnt;
		int status;

		if (status = psyfile_read(self->fp, smpbuf, self->smplen[instidx])) {
			// cleanup
			free(smpbuf);
			return status;
		}
		sampleCnt = self->smplen[instidx];
		// 8 bit mono sample		
		for (j = 0; j < sampleCnt; ++j)
		{			
			int16_t value;

			// In mods, samples are signed integer, so we can simply left shift
			value = smpbuf[j] << 8;			
			wave->channels.samples[0][j] = (psy_dsp_amp_t)value;
		}
		// cleanup
		free(smpbuf);
	}
	return PSY_OK;
}
