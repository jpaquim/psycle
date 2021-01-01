// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "xmsongloader.h"
// local
#include "constants.h"
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
static bool xmsongloader_loadinstruments(XMSongLoader*, uintptr_t instr_start);
static uintptr_t xmsongloader_loadinstrument(XMSongLoader*, int slot,
	uintptr_t instr_start);
static void xmsongloader_setenvelopes(psy_audio_Instrument*,
	const XMSAMPLEHEADER*);
static uintptr_t xmsongloader_loadpatterns(XMSongLoader*);
static void xmsongloader_makesequence(XMSongLoader*, struct XMFILEHEADER*);
static uintptr_t xmsongloader_loadsinglepattern(XMSongLoader*, uintptr_t start,
	int patidx, int tracks);
static bool xmsongloader_writepatternentry(XMSongLoader*,
	psy_audio_PatternNode**, psy_audio_Pattern* pattern,
	int row, int col, psy_audio_PatternEvent* e);
// implementation
void xmsongloader_init(XMSongLoader* self, psy_audio_SongFile* songfile)
{
	assert(self);

	self->songfile = songfile;	
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
	memset(self->highoffset, 0, sizeof(self->highoffset));
	memset(self->memportaup, 0, sizeof(self->memportaup));
	memset(self->memportadown, 0, sizeof(self->memportadown));
	memset(self->memportanote, 0, sizeof(self->memportanote));
	memset(self->memportapos, 0, sizeof(self->memportapos));
	memset(self->smplen, 0, sizeof(self->smplen));
	memset(self->smpflags, 0, sizeof(self->smpflags));
	psy_table_clear(&self->xmtovirtual);
}

bool xmsongloader_load(XMSongLoader* self)
{
	PsyFile* fp;
	psy_audio_SongProperties songproperties;
	char* comments;
	char* songname;
	uintptr_t instr_start;

	//CExclusiveLock lock(&song.semaphore, 2, true);
	// check validity
	if(!xmsongloader_isvalid(self)){
		return FALSE;
	}
	xmsongloader_reset(self);
	// build sampler
	self->sampler = psy_audio_machinefactory_makemachine(
		self->songfile->song->machinefactory, MACH_XMSAMPLER, "", UINTPTR_MAX);
	if (self->sampler) {
		psy_audio_machine_setposition(self->sampler, rand() / 64, rand() / 80);	
		psy_audio_machines_insert(&self->songfile->song->machines, 0,
			self->sampler);
		psy_audio_machines_connect(&self->songfile->song->machines,
			psy_audio_wire_make(0, psy_audio_MASTER_INDEX));
		psy_audio_connections_setwirevolume(&self->songfile->song->machines.connections,
			psy_audio_wire_make(0, psy_audio_MASTER_INDEX), 0.355f); //-9dB
	}	
	// get song name
	fp = self->songfile->file;
	songname = AllocReadStrStart(fp, 20,17);

	if (songname == NULL)
		return FALSE;

	comments = strdup("Imported from FastTracker II Module: ");
	comments = psy_strcat_realloc(comments, self->songfile->path);
	psy_audio_songproperties_init(&songproperties, songname, "", comments);
	free(comments);
	comments = NULL;
	psy_audio_song_setproperties(self->songfile->song, &songproperties);
	free(songname);
	songname = NULL;
				
	instr_start = xmsongloader_loadpatterns(self);
	xmsongloader_loadinstruments(self, instr_start);
	return TRUE;
}

bool xmsongloader_isvalid(XMSongLoader* self)
{			
	PsyFile* fp;
	bool bIsValid = FALSE;
		
	fp = self->songfile->file;
	// get header
	char * pID = AllocReadStrStart(fp, 17, 0);

	// tracker name	
	char * pTrackerName = AllocReadStrStart(fp, 20,38);
		
	// get tracker version
	char * pTrackerVer = AllocReadStrStart(fp, 2,58);	

	// process
	// TRACE(_T("Header: %s\n"),pID);
	// TRACE(_T("Tracker: %s\n"),pTrackerName);
	// TRACE(_T("Fileformat Version %i.%i\n"),int(pTrackerVer[1]),int(pTrackerVer[0]));

	// check header
	bIsValid = (!strncmp(pID,XM_HEADER,sizeof(XM_HEADER)-1));
		

	// cleanup
	free(pID);
	free(pTrackerName);
	free(pTrackerVer);

	return bIsValid;
}

uintptr_t xmsongloader_loadpatterns(XMSongLoader* self) // , std::map<int, int>& xmtovirtual)
{
	PsyFile* fp;
	int xmidx;
	int i;

	fp = self->songfile->file;
	// get data
	psyfile_seek(fp, 60);
	psyfile_read(fp, &self->header,sizeof(XMFILEHEADER));

	{ // IsAmigaSlides
		psy_audio_MachineParam* param;

		param = psy_audio_machine_tweakparameter(self->sampler, 24);
		if (param) {
			psy_audio_machine_parameter_tweak_scaled(self->sampler, param,
				(self->header.flags & 0x01) ? FALSE : TRUE);
		}
	}
	{ // XM_SAMPLER_PanningMode
		psy_audio_MachineParam* param;

		param = psy_audio_machine_tweakparameter(self->sampler, 40);
		if (param) {
			// XM_SAMPLER_PanningMode::TwoWay
			psy_audio_machine_parameter_tweak_scaled(self->sampler, param, 1);
		}
	}	
	self->songfile->song->properties.tracks = psy_max((int)self->header.channels, 4);
	//song.SONGTRACKS = std::max((int)header.channels, 4);
	self->m_iInstrCnt = self->header.instruments;
	self->songfile->song->properties.bpm = self->header.tempo;
	//song.BeatsPerMin(header.tempo);
	self->songfile->song->properties.tpb = 24;
	//song.TicksPerBeat(24);
	int extraticks=0;
	self->songfile->song->properties.lpb = calclpbfromspeed(
		self->header.speed, &extraticks);
	if (extraticks != 0) {
		self->songfile->song->properties.extraticksperbeat = extraticks;
	}
	
	// for(int i = 0;i < MAX_SONG_POSITIONS && i < header.norder;i++)
	// {
	//	if ( header.order[i] < MAX_PATTERNS ){
	//		song.playOrder[i]=header.order[i];
	//	} else { 
	//		song.playOrder[i]=0;
	//	}
	//}
	//
	//if ( header.norder > MAX_SONG_POSITIONS ){
		//song.playLength=MAX_SONG_POSITIONS;
	//} else {
		//song.playLength=header.norder;
	//}

	self->maxextracolumn = self->songfile->song->properties.tracks; // song.SONGTRACKS;

	// Since in XM we load first the patterns, we initialize the map linearly. Later, on the instruments,
	// we will only create the virtual instruments of existing sampled instruments
	xmidx = 1;
	for (i = MAX_MACHINES; i < MAX_VIRTUALINSTS && xmidx <= self->header.instruments; i++, xmidx++) {
		psy_table_insert(&self->xmtovirtual, xmidx, (void*)(uintptr_t)i);		
	}

	// get pattern data
	uintptr_t nextPatStart = self->header.size + 60;
	for(int j = 0;j < self->header.patterns && nextPatStart > 0;j++){
		nextPatStart = xmsongloader_loadsinglepattern(self, nextPatStart, j, self->header.channels); // , xmtovirtual);
	}
	self->songfile->song->properties.tracks = self->maxextracolumn;
	//song.SONGTRACKS = maxextracolumn;
	xmsongloader_makesequence(self, &self->header);
	return nextPatStart;
}

void xmsongloader_makesequence(XMSongLoader* self, struct XMFILEHEADER* xmheader)
{
	uintptr_t i;
	psy_audio_SequencePosition sequenceposition;
	psy_audio_Song* song;

	song = self->songfile->song;
	sequenceposition.tracknode =
		psy_audio_sequence_appendtrack(&song->sequence, psy_audio_sequencetrack_allocinit());
	for (i = 0; i < xmheader->norder; ++i) {
		sequenceposition.trackposition =
			psy_audio_sequence_last(&song->sequence, sequenceposition.tracknode);
		psy_audio_sequence_insert(&song->sequence, sequenceposition,
			xmheader->order[i]);
	}
}

// return address of next pattern, 0 for invalid
uintptr_t xmsongloader_loadsinglepattern(XMSongLoader* self, uintptr_t start,
	int patidx, int tracks)
{
	PsyFile* fp;
	psy_audio_Song* song;
	int32_t headerlen;
	int16_t numrows;
	int16_t packedsize;
	uint16_t lastmach[64];
	psy_audio_Pattern* pattern;
	psy_audio_PatternEvent e;

	fp = self->songfile->file;
	song = self->songfile->song;
	headerlen = ReadInt4Start(fp, start);
	psyfile_skip(fp, 1); //char iPackingType = psyfile_read_int8();
	numrows = psyfile_read_int16(fp);
	packedsize = psyfile_read_int16(fp);
	memset(lastmach, 255, sizeof(uint16_t)*64);

	pattern = psy_audio_pattern_allocinit();
	psy_audio_pattern_setname(pattern, "unnamed");
	psy_audio_patterns_insert(&song->patterns, patidx, pattern);
	psy_audio_pattern_setlength(pattern,
		numrows * 1.0 / self->songfile->song->properties.lpb);

	if (packedsize == 0) {
		//Pattern is emtpy.
	} else {
		psy_audio_PatternNode* node;
		int col;
		int row;

		node = NULL;
		// get next values
		for (row = 0; row < numrows; ++row) {
			self->extracolumn = tracks;
			for(col = 0; col < tracks; ++col) {	
				// reset
				// during event translation 255 is used for instruments until
				// writepatternentry will set the empty(255) instruments to
				// psy_audio_NOTECOMMANDS_INST_EMPTY
				uint8_t note;
				uint8_t instr;
				uint8_t vol;
				uint8_t type;
				uint8_t param;
				uint8_t volume;

				note = 255;
				instr = 255;
				vol = 0;
				type = 0;
				param = 0;
				volume = 0;

				psy_audio_patternevent_init(&e);
				// read note
				note = psyfile_read_int8(fp);

				// is compression bit set?
				if(note & 0x80)
				{
					unsigned char bReadNote = note&0x01;
					unsigned char bReadInstr = note&0x02;
					unsigned char bReadVol = note&0x04;
					unsigned char bReadType = note&0x08;
					unsigned char bReadParam  = note&0x10;

					note = 0;
					if(bReadNote) note = psyfile_read_int8(fp); 
					if(bReadInstr) instr = psyfile_read_int8(fp);
					if(bReadVol) vol = psyfile_read_int8(fp);
					if(bReadType) type = psyfile_read_int8(fp);
					if(bReadParam) param = psyfile_read_int8(fp);
				}
				else
				{
					// read all values
					instr = psyfile_read_int8(fp);
					vol = psyfile_read_int8(fp);
					type = psyfile_read_int8(fp);
					param = psyfile_read_int8(fp);
				}								

				volume = 255;

				// volume/command
				if(vol >= 0x10 && vol <= 0x50)
				{
					volume=(vol == 0x50)?0x3F:(vol-0x10);
				}
				else if(vol >= 0x60){
					switch(vol&0xF0)
					{						
					case XMV_VOLUMESLIDEDOWN:						
						volume = XM_SAMPLER_CMD_VOL_VOLSLIDEDOWN |(vol&0x0F);
						break;
					case XMV_VOLUMESLIDEUP:
						volume = XM_SAMPLER_CMD_VOL_VOLSLIDEUP|(vol&0x0F);
						break;
					case XMV_FINEVOLUMESLIDEDOWN:
						volume = XM_SAMPLER_CMD_VOL_FINEVOLSLIDEDOWN|(vol&0x0F);
						break;
					case XMV_FINEVOLUMESLIDEUP:
						volume = XM_SAMPLER_CMD_VOL_FINEVOLSLIDEUP|(vol&0x0F);
						break;
					case XMV_PANNING:
						volume = XM_SAMPLER_CMD_VOL_PANNING|(vol&0x0F);
						break;
					case XMV_PANNINGSLIDELEFT:
						// Panning in FT2 has 256 values, so we convert to the 64values used in Sampulse.
						volume = XM_SAMPLER_CMD_VOL_PANSLIDELEFT|(((vol&0x0F) < 4)?1:(vol&0x0F)>>2);
						break;
					case XMV_PANNINGSLIDERIGHT:
						// Panning in FT2 has 256 values, so we convert to the 64values used in Sampulse.
						volume = XM_SAMPLER_CMD_VOL_PANSLIDERIGHT|(((vol&0x0F) < 4)?1:(vol&0x0F)>>2);
						break;
// Ignoring this command for now.
//						case XMVOL_CMD_XMV_VIBRATOSPEED:
//							volume = XM_SAMPLER_CMD_VOL::VOL_VIBRATO_SPEED|(vol&0x0F);
//							break;
					case XMV_VIBRATO:
						volume = XM_SAMPLER_CMD_VOL_VIBRATO|(vol&0x0F);
						break;
					case XMV_PORTA2NOTE:
						volume = XM_SAMPLER_CMD_VOL_TONEPORTAMENTO|(vol&0x0F);
						break;
					default:
						break;
					}
				}
				e.parameter = param;				
				int exchwave[3] = {
					psy_audio_WAVEFORMS_SINUS,		
					psy_audio_WAVEFORMS_SAWDOWN,
					psy_audio_WAVEFORMS_SQUARE
				};
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
						if ( e.parameter == 0) {
							if (self->memportapos[col] != 1 )
							{
								e.parameter = self->memportaup[col];
							}
						}
						else {
							if ( e.parameter > 0xDF ) { e.parameter = 0xDF; }
							self->memportaup[col] = e.parameter;
						}
						self->memportapos[col] = 1;
						break;
					case XMCMD_PORTADOWN:
						e.cmd = XM_SAMPLER_CMD_PORTAMENTO_DOWN;
						if ( e.parameter == 0) {
							if ( self->memportapos[col] != 2 )
							{
								e.parameter = self->memportadown[col];
							}
						}
						else {
							if ( e.parameter > 0xDF ) { e.parameter = 0xDF; }
							self->memportadown[col] = e.parameter;
						}
						self->memportapos[col] = 2;
						break;
					case XMCMD_PORTA2NOTE:
						e.cmd = XM_SAMPLER_CMD_PORTA2NOTE;
						if ( e.parameter == 0) {
							if (self->memportapos[col] != 3 )
							{
								e.parameter = self->memportanote[col];
							}
						}
						else {
							self->memportanote[col] = e.parameter;
						}
						self->memportapos[col] = 3;
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
						e.cmd = XM_SAMPLER_CMD_OFFSET | self->highoffset[col]; 
						break;
					case XMCMD_VOLUMESLIDE:
						e.cmd = XM_SAMPLER_CMD_VOLUMESLIDE;
						break;
					case XMCMD_POSITION_JUMP:
						e.cmd = psy_audio_PATTERNCMD_JUMP_TO_ORDER;
						break;
					case XMCMD_VOLUME:
						e.cmd = XM_SAMPLER_CMD_VOLUME;
						e.parameter = param * 2;
						break;
					case XMCMD_PATTERN_BREAK:
						e.cmd = psy_audio_PATTERNCMD_BREAK_TO_LINE;
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
							e.cmd = psy_audio_PATTERNCMD_EXTENDED;
							e.parameter = psy_audio_PATTERNCMD_PATTERN_LOOP + (param & 0xf);
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
							e.cmd = psy_audio_PATTERNCMD_EXTENDED;
							e.parameter = psy_audio_PATTERNCMD_PATTERN_DELAY | (param & 0xf);
							break;
						case XMCMD_E_SET_MIDI_MACRO:
							e.cmd = XM_SAMPLER_CMD_EXTENDED;
							e.parameter = XMCMD_MIDI_MACRO | (param & 0x0f);
							break;
						default:
							e.cmd = XM_SAMPLER_CMD_NONE;
							break;
						}
						break;
					case XMCMD_SETSPEED:
						if ( param < 32)
						{
							e.cmd= psy_audio_PATTERNCMD_EXTENDED;
							int extraticks=0;
							e.parameter = calclpbfromspeed(param, &extraticks);
							if (extraticks != 0) {
								psy_audio_PatternEvent entry;

								psy_audio_patternevent_init_all(&entry,
									psy_audio_NOTECOMMANDS_EMPTY,
									psy_audio_NOTECOMMANDS_EMPTY,
									psy_audio_NOTECOMMANDS_MACH_EMPTY,
									0,
									psy_audio_PATTERNCMD_EXTENDED,
									psy_audio_PATTERNCMD_ROW_EXTRATICKS | extraticks);
								xmsongloader_writepatternentry(self, &node,
									pattern, row, self->extracolumn, &entry);
								self->extracolumn++;
							}
						}
						else
						{
							e.cmd = psy_audio_PATTERNCMD_SET_TEMPO;
						}
						break;
					case XMCMD_SET_GLOBAL_VOLUME:						
						e.cmd = XM_SAMPLER_CMD_SET_GLOBAL_VOLUME;
						if (param >= 0x80) e.parameter = 0xFF;
						else e.parameter = param*2;
						break;
					case XMCMD_GLOBAL_VOLUME_SLIDE:
						e.cmd = XM_SAMPLER_CMD_GLOBAL_VOLUME_SLIDE;
						//Double the parameter, since FT2's range is 0-0x40.
						if ( (param & 0x0F) == 0 || (param & 0x0F) == 0xF){ // Slide up
							e.parameter = (param & 0xF0)>>4;
							e.parameter = e.parameter>7?15:e.parameter*2;
							e.parameter<<=4;
							e.parameter |= (param & 0x0F);
						}
						else if ( (param & 0xF0) == 0 || (param & 0xF0) == 0xF0)  { // Slide down
							e.parameter = (param & 0x0F);
							e.parameter = e.parameter>7?15:e.parameter*2;
							e.parameter |= (param & 0xF0);
						}
						break;
					case XMCMD_NOTE_OFF:
						e.cmd = XM_SAMPLER_CMD_VOLUME;
						e.parameter = 0;
						break;
					case XMCMD_SET_ENV_POSITION:
						e.cmd = XM_SAMPLER_CMD_SET_ENV_POSITION;
						break;
					case XMCMD_PANNINGSLIDE:
						e.cmd = XM_SAMPLER_CMD_PANNINGSLIDE;
						break;
					case XMCMD_RETRIG:
						e.cmd = XM_SAMPLER_CMD_RETRIG;
						break;
					case XMCMD_TREMOR:
						e.cmd =  XM_SAMPLER_CMD_TREMOR;
						break;
					case XMCMD_EXTEND_XM_EFFECTS:
						switch(param & 0xf0){
						case XMCMD_X_EXTRA_FINE_PORTA_DOWN:
							e.cmd = XM_SAMPLER_CMD_PORTAMENTO_DOWN;
							e.parameter = 0xE0 | (param & +0xf);
							break;
						case XMCMD_X_EXTRA_FINE_PORTA_UP:
							e.cmd = XM_SAMPLER_CMD_PORTAMENTO_UP;
							e.parameter = 0xE0 | (param & +0xf);
							break;
						case XMCMD_X9:
							switch ( param & 0xf){
							case XMCMD_X9_SURROUND_OFF:
								e.cmd = XM_SAMPLER_CMD_EXTENDED;
								e.parameter = XM_SAMPLER_CMD_E9 | XM_SAMPLER_CMD_E9_SURROUND_OFF;
								break;
							case XMCMD_X9_SURROUND_ON:
								e.cmd = XM_SAMPLER_CMD_EXTENDED;
								e.parameter = XM_SAMPLER_CMD_E9 | XM_SAMPLER_CMD_E9_SURROUND_ON;
								break;
							case XMCMD_X9_REVERB_OFF:
								e.cmd = XM_SAMPLER_CMD_EXTENDED;
								e.parameter = XM_SAMPLER_CMD_E9 | XM_SAMPLER_CMD_E9_REVERB_OFF;
								break;
							case XMCMD_X9_REVERB_FORCE:
								e.cmd = XM_SAMPLER_CMD_EXTENDED;
								e.parameter = XM_SAMPLER_CMD_E9 | XM_SAMPLER_CMD_E9_REVERB_FORCE;
								break;
							case XMCMD_X9_STANDARD_SURROUND:
								e.cmd = XM_SAMPLER_CMD_EXTENDED;
								e.parameter = XM_SAMPLER_CMD_E9 | XM_SAMPLER_CMD_E9_STANDARD_SURROUND;
								break;
							case XMCMD_X9_QUAD_SURROUND:
								e.cmd = XM_SAMPLER_CMD_EXTENDED;
								e.parameter = XM_SAMPLER_CMD_E9 | XM_SAMPLER_CMD_E9_QUAD_SURROUND;
								break;
							case XMCMD_X9_GLOBAL_FILTER:
								e.cmd = XM_SAMPLER_CMD_EXTENDED;
								e.parameter = XM_SAMPLER_CMD_E9 | XM_SAMPLER_CMD_E9_GLOBAL_FILTER;
								break;
							case XMCMD_X9_LOCAL_FILTER:
								e.cmd = XM_SAMPLER_CMD_EXTENDED;
								e.parameter = XM_SAMPLER_CMD_E9 | XM_SAMPLER_CMD_E9_LOCAL_FILTER;
								break;
							case XMCMD_X9_PLAY_FORWARD:
								e.cmd = XM_SAMPLER_CMD_EXTENDED;
								e.parameter = XM_SAMPLER_CMD_E9 | XM_SAMPLER_CMD_E9_PLAY_FORWARD;
								break;
							case XMCMD_X9_PLAY_BACKWARD:
								e.cmd = XM_SAMPLER_CMD_EXTENDED;
								e.parameter = XM_SAMPLER_CMD_E9 | XM_SAMPLER_CMD_E9_PLAY_BACKWARD;
								break;
							default:
								e.cmd = XM_SAMPLER_CMD_NONE;
								break;
							}
							break;
						case XMCMD_X_HIGH_OFFSET:
							self->highoffset[col] = param & 0x0F;
							break;
						default:
							e.cmd = XM_SAMPLER_CMD_NONE;
							break;
						}
						break;
					case XMCMD_PANBRELLO:
						e.cmd = XM_SAMPLER_CMD_PANBRELLO;
						break;
					case XMCMD_MIDI_MACRO:
						e.cmd = XM_SAMPLER_CMD_MIDI_MACRO;
						break;
					default:
						e.cmd = XM_SAMPLER_CMD_NONE;
						break;
				}
				// instrument/note
				note = note & 0x7f;
				switch(note)
				{
					case 0x00: 
						e.note = psy_audio_NOTECOMMANDS_EMPTY;
						break;// no note

					case 0x61:
						e.note = psy_audio_NOTECOMMANDS_RELEASE;
						e.inst = psy_audio_NOTECOMMANDS_EMPTY;
						e.mach = 0;
						break;// noteoff		
						
					default: 
						//if(note > 96 || note <= 0)
						// TRACE(_T("invalid note: %d\n"), note);
						e.note  = note+11; // +11 -> +12 ( FT2 C-0 is Psycle's C-1) -1 ( Ft2 C-0 is value 1)

						break;	// transpose
				}

				// If empty, do not inform machine
				if (e.note == psy_audio_NOTECOMMANDS_EMPTY && instr == psy_audio_NOTECOMMANDS_EMPTY && volume == 255 && e.cmd == 00 && e.parameter == 00) {
					e.mach = psy_audio_NOTECOMMANDS_MACH_EMPTY;
					e.inst = psy_audio_NOTECOMMANDS_EMPTY;
				}
				// if instrument without note, or note without instrument, cannot use virtual instrument, so use sampulse directly
				else if (( e.note == psy_audio_NOTECOMMANDS_EMPTY && instr != psy_audio_NOTECOMMANDS_EMPTY) || ( e.note < psy_audio_NOTECOMMANDS_RELEASE && instr == psy_audio_NOTECOMMANDS_EMPTY)) {
					e.mach = 0;
					e.inst = instr;
					if (e.inst != psy_audio_NOTECOMMANDS_EMPTY) {
						// We cannot use the virtual instrument, but we should remember which it is.
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
						}
						else if (lastmach[col] != 255) {
							e.mach = (uint8_t)lastmach[col];
						}
						else if (volume != 255) {
							e.mach = 0;
						}
						else {
							e.mach = 255;
						}
					}
					else {
						e.mach = (uint8_t)(uintptr_t)psy_table_at(
							&self->xmtovirtual, e.inst);
						e.inst= 255;
						lastmach[col]=e.mach;
					}
				}
				if (e.mach == 0) { // fallback to the old behaviour. This will happen only if an unused instrument is present in the pattern.
					if(e.cmd != 0 || e.parameter != 0) {
						if(volume!= psy_audio_NOTECOMMANDS_EMPTY) {
							psy_audio_PatternEvent entry;

							psy_audio_patternevent_init_all(&entry,
								psy_audio_NOTECOMMANDS_MIDICC,
								col,
								e.mach,
								0,
								0,
								0);

							entry.cmd = XM_SAMPLER_CMD_SENDTOVOLUME;
							entry.parameter = volume;
							xmsongloader_writepatternentry(self, &node,
								pattern, row,self->extracolumn,&entry);
							self->extracolumn++;
						}
					}
					else if(volume < 0x40) {
						e.cmd = XM_SAMPLER_CMD_VOLUME;
						e.parameter = volume*2;
					}
					else if(volume!=255) {
						e.cmd = XM_SAMPLER_CMD_SENDTOVOLUME;
						e.parameter = volume;
					}
				}
				else {
					e.inst = volume;
				}
				xmsongloader_writepatternentry(self, &node, pattern, row, col,
					&e);
			}
			self->maxextracolumn = psy_max(self->maxextracolumn,
				self->extracolumn);
		}
	}

	//int z = ftell(_file);
	return start + packedsize + headerlen;

}

bool xmsongloader_writepatternentry(XMSongLoader* self,
	psy_audio_PatternNode** node, psy_audio_Pattern* pattern,
	int row, int col, psy_audio_PatternEvent* e)
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

// Load instruments
bool xmsongloader_loadinstruments(XMSongLoader* self, uintptr_t instr_start)
{
	uintptr_t i;	
	
	for (i = 1; i <= self->header.instruments; ++i) {
		instr_start = xmsongloader_loadinstrument(self, i, instr_start);				
		if (psy_table_exists(&self->xmtovirtual, i)) {
			// sampulse: 0
			psy_audio_song_insertvirtualgenerator(self->songfile->song,
				(uintptr_t)psy_table_at(&self->xmtovirtual, i), 0, i);
		}
	}
	return TRUE;
}

uintptr_t xmsongloader_loadinstrument(XMSongLoader* self, int slot, uintptr_t start)
{
// void xm_readinstrument(psy_audio_SongFile* self, uint32_t slot, uint32_t start, bool xi)
//{
	struct XMINSTRUMENTHEADER instrumentheader;
	psy_audio_Instrument* instrument;
	PsyFile* fp;
	psy_audio_Song* song;	

	fp = self->songfile->file;
	song = self->songfile->song;
	psyfile_seek(fp, start);
	
	instrument = psy_audio_instrument_allocinit();			
	psy_audio_instrument_setindex(instrument, slot + 1);
	psy_audio_instruments_insert(&song->instruments, instrument,
		psy_audio_instrumentindex_make(0, slot));
	//if (!xi) {
		instrumentheader.size = psyfile_read_uint32(fp);
	//} else {
		//instrumentheader.size = 0;
	//}
	
	psyfile_read(fp, &instrumentheader.name,
		sizeof(instrumentheader.name));
	instrumentheader.name[sizeof(instrumentheader.name) - 1] = '\0';
	psy_audio_instrument_setname(instrument, instrumentheader.name);
	start += instrumentheader.size;	

	instrumentheader.type = psyfile_read_uint8(fp);
	instrumentheader.samples = psyfile_read_uint16(fp);
	//if (!xi) {	
	
	//} else {
	//	uint16_t version;
	//
	//	psyfile_skip(fp, 20);
	//	version = psyfile_read_uint16(fp);
	//	if (version != 0x102) {
	//		return;
	//	}
	//	instrumentheader.samples = 1;
	//}	
	if (instrumentheader.samples > 0) {
		struct XMSAMPLEHEADER sampleheader;
		XMSAMPLESTRUCT* xmsamples = 0;
		int s;
		int note;
		psy_audio_InstrumentEntry instentry;
		
		//if (!xi) {
			// psy_audio_Sample header size
			psyfile_read(fp, &sampleheader.shsize, 4);
		//}
		// psy_audio_Sample number for all notes
		psyfile_read(fp, &sampleheader.snum, 96);
		// Points for volume envelope
		psyfile_read(fp, &sampleheader.venv, 48);
		// Points for panning envelope
		psyfile_read(fp, &sampleheader.penv, 48);
		// Number of volume points
		psyfile_read(fp, &sampleheader.vnum, 1);
		// Number of panning points
		psyfile_read(fp, &sampleheader.pnum, 1);
		// Volume sustain point
		psyfile_read(fp, &sampleheader.psustain, 1);
		// Volume loop start point
		psyfile_read(fp, &sampleheader.vloops, 1);
		// Volume loop end point
		psyfile_read(fp, &sampleheader.vloope, 1);
		// Panning sustain point
		psyfile_read(fp, &sampleheader.psustain, 1);
		// Panning loop start point
		psyfile_read(fp, &sampleheader.ploops, 1);
		// Panning loop end point
		psyfile_read(fp, &sampleheader.ploope, 1);
		// Volume type: bit 0: On; 1: Sustain; 2: Loop
		psyfile_read(fp, &sampleheader.vtype, 1);
		// Panning type: bit 0: On; 1: Sustain; 2: Loop
		psyfile_read(fp, &sampleheader.ptype, 1);
		psyfile_read(fp, &sampleheader.vibtype, 1);
		psyfile_read(fp, &sampleheader.vibsweep, 1);
		psyfile_read(fp, &sampleheader.vibdepth, 1);
		psyfile_read(fp, &sampleheader.vibrate, 1);
		psyfile_read(fp, &sampleheader.volfade, 2);
		psyfile_read(fp, &sampleheader.reserved, 2);
		//if (xi) {
		//	psyfile_skip(fp, 20);
		//	psyfile_read(fp, &instrumentheader.samples, 2);
		//}
		xmsamples = (XMSAMPLESTRUCT*)malloc(sizeof(
			XMSAMPLESTRUCT) * (int)instrumentheader.samples);
		// create instrument entries
		psy_audio_instrument_clearentries(instrument);
		psy_audio_instrumententry_init(&instentry);
		instentry.sampleindex =
			sampleindex_make(slot, sampleheader.snum[0]);
		for (note = 1; note < 96; ++note) {
			if (sampleheader.snum[note] != instentry.sampleindex.subslot) {
				instentry.keyrange.high = note - 1;
				psy_audio_instrument_addentry(instrument, &instentry);
				instentry.keyrange.low = note;
				instentry.sampleindex.subslot = sampleheader.snum[note];
			}
		}
		instentry.keyrange.high = 119;
		psy_audio_instrument_addentry(instrument, &instentry);
		//if (!xi) {
			psyfile_seek(fp, start);
		//}
		for (s = 0; s < instrumentheader.samples; ++s) {
			//if (!xi) {
				psyfile_seek(fp, start);
				//}
			psyfile_read(fp, &xmsamples[s].samplen, 4);
			psyfile_read(fp, &xmsamples[s].loopstart, 4);
			psyfile_read(fp, &xmsamples[s].looplen, 4);
			psyfile_read(fp, &xmsamples[s].vol, 1);
			// Finetune (signed byte -16..+15)
			psyfile_read(fp, &xmsamples[s].finetune, 1);
			// Type: Bit 0-1: 0 = No loop, 1 = Forward loop,
			//		 2 = Ping-pong loop;
			//		 4: 16-bit sampledata				
			psyfile_read(fp, &xmsamples[s].type, 1);
			// Panning (0-255)
			psyfile_read(fp, &xmsamples[s].pan, 1);
			// Relative note number (signed byte)
			psyfile_read(fp, &xmsamples[s].relnote, 1);
			psyfile_read(fp, &xmsamples[s].res, 1);
			psyfile_read(fp, &xmsamples[s].name, 22);
			xmsamples[s].name[21] = '\0';
			start += 40;
		}
		xmsongloader_setenvelopes(instrument, &sampleheader);
		for (s = 0; s < instrumentheader.samples; ++s) {
			psy_audio_Sample* sample;
			int is16bit;

			sample = psy_audio_sample_allocinit(1);
			psy_audio_sample_setname(sample, xmsamples[s].name);

			is16bit = (xmsamples[s].type & 0x10) == 0x10;
			sample->panfactor = xmsamples[s].pan / (psy_dsp_amp_t)255.f;
			// Type: Bit 0-1: 0 = No loop, 1 = Forward loop,
			//		 2 = Ping-pong loop;
			//		 4: 16-bit sampledata
			if (xmsamples[s].looplen > 0) {
				if (xmsamples[s].type & 0x01) {
					sample->loop.type = psy_audio_SAMPLE_LOOP_NORMAL;
				} else
					if (xmsamples[s].type & 0x02) {
						sample->loop.type = psy_audio_SAMPLE_LOOP_BIDI;
					} else {
						sample->loop.type = psy_audio_SAMPLE_LOOP_DO_NOT;
					}
			} else {
				sample->loop.type = psy_audio_SAMPLE_LOOP_DO_NOT;
			}
			if (sample->loop.type != psy_audio_SAMPLE_LOOP_DO_NOT) {
				sample->loop.start = xmsamples[s].loopstart / (is16bit ? 2 : 1);
				sample->loop.end = (xmsamples[s].loopstart + xmsamples[s].looplen) /
					(is16bit ? 2 : 1);
			}
			sample->numframes = xmsamples[s].samplen / (is16bit ? 2 : 1);
			sample->samplerate = 8363;
			sample->defaultvolume = xmsamples[s].vol * 2;
			sample->tune = xmsamples[s].relnote - 24;
			// WaveFineTune has +-100 range in Psycle.
			sample->finetune = (int16_t)(xmsamples[s].finetune / 1.28f);
			// Sounds Stupid, but it isn't. Some modules save sample
			// header when there is no sample.
			if (xmsamples[s].samplen > 0) {
				unsigned char* smpbuf;
				int16_t oldvalue;
				uintptr_t i;
				uintptr_t j;

				smpbuf = malloc(xmsamples[s].samplen);
				if (smpbuf) {
					psyfile_read(fp, smpbuf, xmsamples[s].samplen);
					psy_audio_sample_allocwavedata(sample);
					oldvalue = 0;
					for (i = 0, j = 0; i < sample->numframes; ++i) {
						int16_t value;

						if (is16bit) {
							value = (smpbuf[j] & 0xFF) | (smpbuf[j + 1] << 8);
							j += 2;
						} else {
							value = (int16_t)(smpbuf[j] << 8);
							++j;
						}
						value += oldvalue;
						sample->channels.samples[0][i] = (psy_dsp_amp_t)value;
						oldvalue = value;
					}
					free(smpbuf);
				}
			}
			psy_audio_samples_insert(&song->samples, sample,
				sampleindex_make(slot, s));
			start += xmsamples[s].samplen;
		}
		free(xmsamples);
	}
	return start;
}
	
	
void xmsongloader_setenvelopes(psy_audio_Instrument* inst, const XMSAMPLEHEADER* sampleHeader)
{
	// volume envelope
	psy_dsp_EnvelopeSettings ampenvelope;

	psy_dsp_envelopesettings_init(&ampenvelope);
	//inst.AmpEnvelope().Init();
	//inst.AmpEnvelope().Mode(XMInstrument::Envelope::Mode::TICK);
	if (sampleHeader->vtype & 1) {// enable volume envelope
		// inst.AmpEnvelope().IsEnabled(true);
		// In FastTracker, the volume fade only works if the envelope is activated, so we only calculate
		// volumefadespeed in this case, so that a check during playback time is not needed.		
		// inst.VolumeFadeSpeed(value_mapper::map_32768_1(sampleHeader.volfade));

		int envelope_point_num = sampleHeader->vnum;
		if (envelope_point_num > 12) { // Max number of envelope points in Fasttracker format is 12.
			envelope_point_num = 12;
		}
		// Format of FastTracker points is :
		// Point : frame number. ( 1 frame= line*(24/TPB), samplepos= frame*(samplesperrow*TPB/24))
		// Value : 0..64. , divide by 64 to use it as a multiplier.
		psy_dsp_envelopesettings_append(&ampenvelope, psy_dsp_envelopepoint_make(
			(int)sampleHeader->venv[0] * 512 / 8363.f, (float)sampleHeader->venv[1] / 64.0f));
		//inst.AmpEnvelope().Append((int)sampleHeader.venv[0] ,(float)sampleHeader.venv[1] / 64.0f);
		for (int i = 1; i < envelope_point_num; i++) {
			if (sampleHeader->venv[i * 2] > sampleHeader->venv[(i - 1) * 2])// Some rare modules have erroneous points. This tries to solve that.
				psy_dsp_envelopesettings_append(&ampenvelope, psy_dsp_envelopepoint_make(
					(int)sampleHeader->venv[i * 2] * 512 / 8363.f, (float)sampleHeader->venv[i * 2 + 1] / 64.0f));
		}
		psy_dsp_envelopesettings_copy(&inst->volumeenvelope, &ampenvelope);
		psy_dsp_envelopesettings_dispose(&ampenvelope);
	}
	
		
		if(sampleHeader->vtype & 2) {
			psy_dsp_envelopesettings_setsustainbegin(&inst->volumeenvelope, sampleHeader->vsustain);
			psy_dsp_envelopesettings_setsustainend(&inst->volumeenvelope, sampleHeader->vsustain);			
		}
		else
		{
			// We can't ignore the loop because IT Envelopes do a fadeout when the envelope end is reached and FT does not.
			// IT also sets the Sustain points to the end of the envelope, but i can't see a reason for this to be needed.
//				inst.AmpEnvelope().SustainBegin(inst.AmpEnvelope().NumOfPoints()-1);
//				inst.AmpEnvelope().SustainEnd(inst.AmpEnvelope().NumOfPoints()-1);
		}

/*
		if(sampleHeader.vtype & 4){
			if(sampleHeader.vloops < sampleHeader.vloope){
				inst.AmpEnvelope().LoopStart(sampleHeader.vloops);
				inst.AmpEnvelope().LoopEnd(sampleHeader.vloope);
			}
			// if loopstart >= loopend, Fasttracker ignores the loop!.
			// We can't ignore them because IT Envelopes do a fadeout when the envelope end is reached and FT does not.
			else {
//					inst.AmpEnvelope().LoopStart(XMInstrument::Envelope::INVALID);
//					inst.AmpEnvelope().LoopEnd(XMInstrument::Envelope::INVALID);
				inst.AmpEnvelope().LoopStart(inst.AmpEnvelope().NumOfPoints()-1);
				inst.AmpEnvelope().LoopEnd(inst.AmpEnvelope().NumOfPoints()-1);
			}
		}
		else
		{
			// We can't ignore the loop because IT Envelopes do a fadeout when the envelope end is reached and FT does not.
			inst.AmpEnvelope().LoopStart(inst.AmpEnvelope().NumOfPoints()-1);
			inst.AmpEnvelope().LoopEnd(inst.AmpEnvelope().NumOfPoints()-1);
		}*/

	//} else {
		//inst.AmpEnvelope().IsEnabled(false);
	//}

	/*
	// pan envelope
	inst.PanEnvelope().Init();
	inst.PanEnvelope().Mode(XMInstrument::Envelope::Mode::TICK);
	if(sampleHeader.ptype & 1){// enable volume envelope
			
		inst.PanEnvelope().IsEnabled(true);
			
		if(sampleHeader.ptype & 2){
			inst.PanEnvelope().SustainBegin(sampleHeader.psustain);
			inst.PanEnvelope().SustainEnd(sampleHeader.psustain);
		}

			
		if(sampleHeader.ptype & 4){
			if(sampleHeader.ploops < sampleHeader.ploope){
				inst.PanEnvelope().LoopStart(sampleHeader.ploops);
				inst.PanEnvelope().LoopEnd(sampleHeader.ploope);
			} else {
				inst.PanEnvelope().LoopStart(sampleHeader.ploope);
				inst.PanEnvelope().LoopEnd(sampleHeader.ploops);
			}
		}
		int envelope_point_num = sampleHeader.pnum;
		if(envelope_point_num > 12){
			envelope_point_num = 12;
		}

		for(int i = 0; i < envelope_point_num;i++){
			inst.PanEnvelope().Append((int)sampleHeader.penv[i * 2] ,(float)(sampleHeader.penv[i * 2 + 1]-32.0f) / 32.0f);
		}

	} else {
		inst.PanEnvelope().IsEnabled(false);
	}*/
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
	psy_audio_PatternNode** node, psy_audio_Pattern* pattern,
	const int row, const int col, const psy_audio_PatternEvent*);
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
		self->songfile->song->machinefactory, MACH_XMSAMPLER, "", UINTPTR_MAX);
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
	for (int i=0;i<31;i++) {
		psy_audio_Sample* wave;

		wave = psy_audio_sample_allocinit(1);	
		modsongloader_loadsampleheader(self, wave, i);		
		psy_audio_samples_insert(&song->samples, wave,
			sampleindex_make(i, 0));
	}
	psyfile_seek(fp, 950);
	psyfile_read(fp, &self->header,sizeof(self->header));
		
	char pID[5];
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
	for (int i=0; i < 31;i++) {
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
	
	// get pattern data
	psyfile_seek(self->songfile->file, 1084);
	for(int j = 0;j < self->header.songlength ;j++){
		modsongloader_loadsinglepattern(self, j, self->songfile->song->properties.tracks);
	}
	if(self->speedpatch) {
		self->songfile->song->properties.tracks++;
	}
}

void modsongloader_makesequence(MODSongLoader* self, struct MODHEADER* modheader)
{
	uintptr_t i;
	psy_audio_SequencePosition sequenceposition;
	psy_audio_Song* song;

	song = self->songfile->song;
	sequenceposition.tracknode =
		psy_audio_sequence_appendtrack(&song->sequence, psy_audio_sequencetrack_allocinit());
	for (i = 0; i < modheader->songlength; ++i) {
		sequenceposition.trackposition =
			psy_audio_sequence_last(&song->sequence, sequenceposition.tracknode);
		psy_audio_sequence_insert(&song->sequence, sequenceposition,
			modheader->order[i]);
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

			// read note
			mentry[0] = psyfile_read_uint8(fp); mentry[1] = psyfile_read_uint8(fp); mentry[2] = psyfile_read_uint8(fp); mentry[3] = psyfile_read_uint8(fp);
			instr = ((mentry[0] & 0xF0) + (mentry[2] >> 4));
			period = ((mentry[0]& 0x0F) << 8) + mentry[1];
			type = (mentry[2] & 0x0F);
			param = mentry[3];
			note = modsongloader_convertperiodtonote(period);

			// translate
			e.parameter = param;

			int exchwave[3] = {
					psy_audio_WAVEFORMS_SINUS,
					psy_audio_WAVEFORMS_SAWDOWN,
					psy_audio_WAVEFORMS_SQUARE
			};

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
						e.cmd=psy_audio_PATTERNCMD_EXTENDED;
						int extraticks=0;
						e.parameter = calclpbfromspeed(param,&extraticks);
						if (extraticks != 0) {
							self->speedpatch=TRUE;
							psy_audio_PatternEvent entry;
							
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
	const int row, const int col, const psy_audio_PatternEvent* e)
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
		psy_audio_instrumentindex_make(0, idx));
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
	wave->samplerate = 8363;
	char tmpfine = (char)self->samples[instridx].finetune;
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
