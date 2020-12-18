// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "xmsongloader.h"
// local
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

static unsigned char ReadUInt1(PsyFile* fp)
{
	return psyfile_read_uint8(fp);	
}

static unsigned short ReadUInt2(PsyFile* fp)
{
	return psyfile_read_uint16(fp);	
}

static unsigned int ReadUInt4(PsyFile* fp)
{
	return psyfile_read_uint32(fp);	
}

static unsigned char ReadInt1(PsyFile* fp)
{
	return psyfile_read_int8(fp);
}

static unsigned short ReadInt2(PsyFile* fp)
{
	return psyfile_read_int16(fp);
}

static unsigned int ReadInt4(PsyFile* fp)
{
	return psyfile_read_int32(fp);
}

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

// prototypes
static bool xmsongloader_isvalid(XMSongLoader* self);
static size_t xmsongloader_loadsampleheader(XMSongLoader*, psy_audio_Sample* _wave, size_t iStart, int iInstrIdx, int iSampleIdx);
static size_t xmsongloader_loadsampledata(XMSongLoader*, psy_audio_Sample* _wave, size_t iStart, int iInstrIdx, int iSampleIdx);
static bool xmsongloader_loadinstruments(XMSongLoader*, size_t iInstrStart);
static size_t xmsongloader_loadinstrument(XMSongLoader*, int slot, size_t iStart);
static void xmsongloader_setenvelopes(psy_audio_Instrument* inst, const XMSAMPLEHEADER* sampleHeader);
static size_t xmsongloader_loadpatterns(XMSongLoader*);
static void xmsongloader_makesequence(XMSongLoader*, struct XMFILEHEADER*);
static size_t xmsongloader_loadsinglepattern(XMSongLoader* self, size_t start, int patIdx, int iTracks);
static bool xmsongloader_writepatternentry(XMSongLoader* self,
	psy_audio_PatternNode** node,
	int patIdx, int row, int col, psy_audio_PatternEvent* e);
// implementation
void xmsongloader_init(XMSongLoader* self, psy_audio_SongFile* songfile)
{
	int i;

	assert(self);

	self->songfile = songfile;
	for (i = 0; i < 32; i++)
	{
		self->highOffset[i] = 0;
		self->memPortaUp[i] = 0;
		self->memPortaDown[i] = 0;
		self->memPortaNote[i] = 0;
		self->memPortaPos[i] = 0;
	}
	for (i = 0; i < 256; i++)
	{
		self->smpLen[i] = 0;
		self->smpFlags[i] = 0;
	}
}

void xmsongloader_dispose(XMSongLoader* self)
{
	assert(self);
}

bool xmsongloader_load(XMSongLoader* self)
{
	PsyFile* fp;
	psy_audio_SongProperties songproperties;
	char* comments;

	//CExclusiveLock lock(&song.semaphore, 2, true);
	// check validity
	if(!xmsongloader_isvalid(self)){
		return FALSE;
	}
	// song.CreateMachine(MACH_XMSAMPLER, rand()/64, rand()/80, "sampulse",0);
	// song.InsertConnectionNonBlocking(0,MASTER_INDEX,0,0,0.355f); //-9dB
	//song.seqBus=0;
	// build sampler
	// m_pSampler = static_cast<XMSampler *>(song._pMachine[0]);
	// get song name

	{
		psy_audio_Machine* machine;

		machine = psy_audio_machinefactory_makemachine(self->songfile->song->machinefactory,
			MACH_XMSAMPLER, "", UINTPTR_MAX);
		if (machine) {
			psy_audio_machines_insert(&self->songfile->song->machines, 0, machine);
			psy_audio_machines_connect(&self->songfile->song->machines,
				psy_audio_wire_make(0, psy_audio_MASTER_INDEX));
			psy_audio_connections_setwirevolume(&self->songfile->song->machines.connections,
				psy_audio_wire_make(0, psy_audio_MASTER_INDEX), 0.355f); //-9dB
		}
	}

	fp = self->songfile->file;
	char * pSongName = AllocReadStrStart(fp, 20,17);

	if (pSongName == NULL)
		return FALSE;

	comments = strdup("Imported from FastTracker II Module: ");
	comments = psy_strcat_realloc(comments, self->songfile->path);
	psy_audio_songproperties_init(&songproperties, pSongName, "", comments);
	free(comments);
	comments = NULL;
	psy_audio_song_setproperties(self->songfile->song, &songproperties);
	free(pSongName);
	pSongName = NULL;	
		
	// std::map<int, int> xmtovirtual;
	size_t iInstrStart = xmsongloader_loadpatterns(self); // LoadPatterns(self);//, xmtovirtual);
	xmsongloader_loadinstruments(self, iInstrStart);
	//LoadInstruments(song,iInstrStart, xmtovirtual);
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

size_t xmsongloader_loadpatterns(XMSongLoader* self) // , std::map<int, int>& xmtovirtual)
{
	PsyFile* fp;

	fp = self->songfile->file;
	// get data
	psyfile_seek(fp, 60);
	psyfile_read(fp, &self->m_Header,sizeof(XMFILEHEADER));

	// m_pSampler->IsAmigaSlides((m_Header.flags & 0x01)?false:true);
	// m_pSampler->XM_SAMPLER_PanningMode(XM_SAMPLER_PanningMode::TwoWay);
	self->songfile->song->properties.tracks = psy_max((int)self->m_Header.channels, 4);
	//song.SONGTRACKS = std::max((int)m_Header.channels, 4);
	self->m_iInstrCnt = self->m_Header.instruments;
	self->songfile->song->properties.bpm = self->m_Header.tempo;
	//song.BeatsPerMin(m_Header.tempo);
	self->songfile->song->properties.tpb = 24;
	//song.TicksPerBeat(24);
	int extraticks=0;
	self->songfile->song->properties.lpb = 4; // todo
	//song.LinesPerBeat(XM_SAMPLER_CalcLPBFromSpeed(m_Header.speed,extraticks));
	if (extraticks != 0) {
		//song.ExtraTicksPerLine(extraticks);
	}
	
	// for(int i = 0;i < MAX_SONG_POSITIONS && i < m_Header.norder;i++)
	// {
	//	if ( m_Header.order[i] < MAX_PATTERNS ){
	//		song.playOrder[i]=m_Header.order[i];
	//	} else { 
	//		song.playOrder[i]=0;
	//	}
	//}
	//
	//if ( m_Header.norder > MAX_SONG_POSITIONS ){
		//song.playLength=MAX_SONG_POSITIONS;
	//} else {
		//song.playLength=m_Header.norder;
	//}

	self->m_maxextracolumn = self->songfile->song->properties.tracks; // song.SONGTRACKS;

	//Since in XM we load first the patterns, we initialize the map linearly. Later, on the instruments,
	//we will only create the virtual instruments of existing sampled instruments
	//int xmidx=1;
	//for (int i=MAX_MACHINES;i<MAX_VIRTUALINSTS && xmidx <= m_iInstrCnt;i++, xmidx++) {
		//xmtovirtual[xmidx]=i;
	//}

	// get pattern data
	size_t nextPatStart = self->m_Header.size + 60;
	for(int j = 0;j < self->m_Header.patterns && nextPatStart > 0;j++){
		nextPatStart = xmsongloader_loadsinglepattern(self, nextPatStart, j, self->m_Header.channels); // , xmtovirtual);
	}
	self->songfile->song->properties.tracks = self->m_maxextracolumn;
	//song.SONGTRACKS = m_maxextracolumn;
	xmsongloader_makesequence(self, &self->m_Header);
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
size_t xmsongloader_loadsinglepattern(XMSongLoader* self, size_t start, int patIdx, int iTracks) // , std::map<int, int>& xmtovirtual)
{
	PsyFile* fp;
	psy_audio_Song* song;

	fp = self->songfile->file;
	song = self->songfile->song;
	int iHeaderLen = ReadInt4Start(fp, start);
	psyfile_skip(fp, 1); //char iPackingType = ReadInt1();
	short iNumRows = ReadInt2(fp);
	short iPackedSize = ReadInt2(fp);
	unsigned char lastmach[64];
	memset(lastmach,255,sizeof(char)*64);	
	psy_audio_Pattern* pattern;

	pattern = psy_audio_pattern_allocinit();
	psy_audio_pattern_setname(pattern, "unnamed");
	psy_audio_patterns_insert(&song->patterns, patIdx, pattern);
	psy_audio_pattern_setlength(pattern, iNumRows * 0.25);
	//song.AllocNewPattern(patIdx,"unnamed",iNumRows,false);

	//PatternEntry e;
	psy_audio_PatternEvent e;

	if(iPackedSize == 0)
	{
		//Pattern is emtpy.
	}	
	else
	{
		psy_audio_PatternNode* node;

		node = NULL;
		// get next values
		for(int row = 0;row < iNumRows;row++)
		{
			self->m_extracolumn = iTracks;
			for(int col=0;col<iTracks;col++)
			{	
				// reset
				unsigned char note=psy_audio_NOTECOMMANDS_EMPTY;
				uint16_t instr= psy_audio_NOTECOMMANDS_INST_EMPTY;
				unsigned char vol=0;
				unsigned char type=0;
				unsigned char param=0;

				psy_audio_patternevent_clear(&e);

				// read note
				note = ReadInt1(fp);

				// is compression bit set?
				if(note & 0x80)
				{
					unsigned char bReadNote = note&0x01;
					unsigned char bReadInstr = note&0x02;
					unsigned char bReadVol = note&0x04;
					unsigned char bReadType = note&0x08;
					unsigned char bReadParam  = note&0x10;

					note = 0;
					if(bReadNote) note = ReadInt1(fp); 
					if(bReadInstr) instr = ReadInt1(fp);
					if(bReadVol) vol = ReadInt1(fp);
					if(bReadType) type = ReadInt1(fp);
					if(bReadParam) param = ReadInt1(fp);
				}
				else
				{
					// read all values
					instr = ReadInt1(fp);
					vol = ReadInt1(fp);
					type = ReadInt1(fp);
					param = ReadInt1(fp);
				}								

				uint16_t volume = psy_audio_NOTECOMMANDS_INST_EMPTY;

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
							if (self->memPortaPos[col] != 1 )
							{
								e.parameter = self->memPortaUp[col];
							}
						}
						else {
							if ( e.parameter > 0xDF ) { e.parameter = 0xDF; }
							self->memPortaUp[col] = e.parameter;
						}
						self->memPortaPos[col] = 1;
						break;
					case XMCMD_PORTADOWN:
						e.cmd = XM_SAMPLER_CMD_PORTAMENTO_DOWN;
						if ( e.parameter == 0) {
							if ( self->memPortaPos[col] != 2 )
							{
								e.parameter = self->memPortaDown[col];
							}
						}
						else {
							if ( e.parameter > 0xDF ) { e.parameter = 0xDF; }
							self->memPortaDown[col] = e.parameter;
						}
						self->memPortaPos[col] = 2;
						break;
					case XMCMD_PORTA2NOTE:
						e.cmd = XM_SAMPLER_CMD_PORTA2NOTE;
						if ( e.parameter == 0) {
							if (self->memPortaPos[col] != 3 )
							{
								e.parameter = self->memPortaNote[col];
							}
						}
						else {
							self->memPortaNote[col] = e.parameter;
						}
						self->memPortaPos[col] = 3;
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
						e.cmd = XM_SAMPLER_CMD_OFFSET | self->highOffset[col]; 
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
							e.parameter = 0; // XM_SAMPLER_CalcLPBFromSpeed(param, extraticks);
							if (extraticks != 0) {
								psy_audio_PatternEvent entry;

								psy_audio_patternevent_init_all(&entry,
									psy_audio_NOTECOMMANDS_EMPTY,
									psy_audio_NOTECOMMANDS_INST_EMPTY,
									psy_audio_NOTECOMMANDS_MACH_EMPTY,
									0,
									psy_audio_PATTERNCMD_EXTENDED,
									psy_audio_PATTERNCMD_ROW_EXTRATICKS | extraticks);
								xmsongloader_writepatternentry(self, &node, patIdx,row,self->m_extracolumn, &entry);
								self->m_extracolumn++;
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
							self->highOffset[col] = param & 0x0F;
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
						e.inst = psy_audio_NOTECOMMANDS_INST_EMPTY;
						e.mach = 0;
						break;// noteoff		
						
					default: 
						//if(note > 96 || note <= 0)
						// TRACE(_T("invalid note: %d\n"), note);
						e.note  = note+11; // +11 -> +12 ( FT2 C-0 is Psycle's C-1) -1 ( Ft2 C-0 is value 1)

						break;	// transpose
				}

				// If empty, do not inform machine
				if (e.note == psy_audio_NOTECOMMANDS_EMPTY && instr == psy_audio_NOTECOMMANDS_INST_EMPTY && volume == 255 && e.cmd == 00 && e.parameter == 00) {
					e.mach = psy_audio_NOTECOMMANDS_MACH_EMPTY;
					e.inst = psy_audio_NOTECOMMANDS_INST_EMPTY;
				}
				// if instrument without note, or note without instrument, cannot use virtual instrument, so use sampulse directly
				else if (( e.note == psy_audio_NOTECOMMANDS_EMPTY && instr != psy_audio_NOTECOMMANDS_INST_EMPTY) || ( e.note < psy_audio_NOTECOMMANDS_RELEASE && instr == psy_audio_NOTECOMMANDS_INST_EMPTY)) {
					e.mach = 0;
					//e.inst = instr;
					//if (e.inst != psy_audio_NOTECOMMANDS_INST_EMPTY) {
						//We cannot use the virtual instrument, but we should remember which it is.
						// std::map<int,int>::const_iterator it = xmtovirtual.find(e._inst);
						//if (it != xmtovirtual.end()) {
						//lastmach[col] = e.inst; // it->second;
						//}
					//}
				}
				//default behaviour, let's find the virtual instrument.
				else {
					e.inst = instr;
					//e.mach = 0;
					//std::map<int,int>::const_iterator it = xmtovirtual.find(e._inst);
					//if (it == xmtovirtual.end()) {
						//if (e.inst != psy_audio_NOTECOMMANDS_INST_EMPTY) {
							//e.mach = 0;
							//lastmach[col] = e.mach;
						//}
						//else if (lastmach[col] != 255) {
							//e.mach = lastmach[col];
						//}
						//else if (volume != 255) {
							
						//}
						//else {
							//e.mach = 255;
						//}
					//}
					//else {
						//e.mach=it->second;
						//e.inst= psy_audio_NOTECOMMANDS_INST_EMPTY;
						//lastmach[col]=e.mach;
					//}
				}
				/*if (e.mach == 0) { // fallback to the old behaviour. This will happen only if an unused instrument is present in the pattern.
					if(e.cmd != 0 || e.parameter != 0) {
						if(volume!=255) {
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
							xmsongloader_writepatternentry(self, &node, patIdx,row,self->m_extracolumn,&entry);
							self->m_extracolumn++;
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
				}*/
				//else {
				//	e.inst = volume;
				//}

				xmsongloader_writepatternentry(self, &node, patIdx,row,col,&e);
			}
			self->m_maxextracolumn = psy_max(self->m_maxextracolumn,self->m_extracolumn);
		}
	}

	//int z = ftell(_file);
	return start + iPackedSize + iHeaderLen;

}

bool xmsongloader_writepatternentry(XMSongLoader* self,
	psy_audio_PatternNode** node,
	int patIdx, int row, int col, psy_audio_PatternEvent* e)
{
	if (!psy_audio_patternevent_empty(e)) {		
		psy_audio_Pattern* pattern;
		psy_audio_PatternEvent ev;

		psy_audio_patternevent_clear(&ev);
		ev.note = e->note;
		ev.inst = e->inst;
		ev.mach = 0;
		ev.parameter = e->parameter;
		ev.cmd = e->cmd;
		pattern = psy_audio_patterns_at(&self->songfile->song->patterns, patIdx);
		if (pattern) {
			// don't overflow song buffer 
			//if (patIdx >= MAX_PATTERNS) return false;
			*node = psy_audio_pattern_insert(pattern, *node, col, (psy_dsp_beat_t)(row * 0.25), &ev);

			//PatternEntry* pData = reinterpret_cast<PatternEntry*>(song._ptrackline(patIdx, col, row));

			//*pData = e;
		}
	}
	return TRUE;
}	

/*size_t xmsongloader_loadinstrument(XMSongLoader* self, psy_audio_Instrument* instr, size_t iStart,  int idx, int* curSample)
{
	PsyFile* fp;
	psy_audio_Song* song;

	fp = self->songfile->file;
	song = self->songfile->song;
	psyfile_seek(fp, iStart);

	// read header
	int iInstrSize = ReadInt4();
	//assert(iInstrSize==0x107||iInstrSize==0x21); // Skale Tracker (or MadTracker or who knows which more) don't have the "reserved[20]" parameter in the XMSAMPLEHEADER
	char sInstrName[23] = {0};
	psyfile_read(fp, sInstrName,22);
	sInstrName[22]= 0;

	psyfile_skip(fp, 1); //int iInstrType = ReadInt1();
	int iSampleCount = ReadInt2();

	if(iSampleCount>1)
 		TRACE(_T("ssmple count = %d\n"),iSampleCount);

	psy_audio_instrument_setname(instr, sInstrName);
	// instr.Name(sInstrName);
	iStart += iInstrSize;

	if(iSampleCount==0)
		return iStart;

        
	XMSAMPLEHEADER _samph;
	memset(&_samph, 0, sizeof _samph);
	psyfile_read(fp, &_samph,sizeof(XMSAMPLEHEADER));
		
	int exchwave[4] = {
		psy_audio_WAVEFORMS_SINUS,
		psy_audio_WAVEFORMS_SQUARE,
		psy_audio_WAVEFORMS_SAWDOWN,
		psy_audio_WAVEFORMS_SAWUP
	};	

	SetEnvelopes(instr,_samph);

	unsigned char* sRemap = (char*)malloc(iSampleCount);
	int i;
	// read instrument data	
	for(i=0;i<iSampleCount;i++)
	{
		psy_audio_Sample* wave;

		wave = psy_audio_sample_alloc();
		iStart = xmsongloader_loadsampleheader(self, wave,iStart,idx,curSample);
			// Only get REAL samples.
		if ( self->smpLen[*curSample] > 0) { // && curSample < MAX_INSTRUMENTS-2 ) {
			psy_audio_SampleIndex index;

			index = sampleindex_make(*curSample, _samph.snum[0]);
			psy_audio_samples_insert(self->songfile->song, wave, index);
			//song.samples.SetSample(wave,curSample);
			//sRemap[i] = curSample;
			*curSample++;
		}
		//else { sRemap[i] = MAX_INSTRUMENTS-1; }
	}
	// load individual samples
	for(i=0;i<iSampleCount;i++)
	{
		//if ( sRemap[i] < MAX_INSTRUMENTS-1)
		//{
			XMInstrument::WaveData<>& _wave = song.samples.get(sRemap[i]);
			iStart = LoadSampleData(_wave,iStart,idx,sRemap[i]);
			_wave.VibratoAttack(_samph.vibsweep==0?0:256-_samph.vibsweep);
			_wave.VibratoDepth(_samph.vibdepth);
			_wave.VibratoSpeed(_samph.vibrate);
			_wave.VibratoType(exchwave[_samph.vibtype&3]);
		}
	}

	XMInstrument::NotePair npair;
	if ( _samph.snum[0] < iSampleCount) npair.second=sRemap[_samph.snum[0]];
	else npair.second=255;
	for(int i = 0;i < XMInstrument::NOTE_MAP_SIZE;i++){
		npair.first=i;
		if (i< 12){
			//npair.second=_samph.snum[0]; implicit.
			instr.NoteToSample(i,npair);
		} else if(i < 108){
			if ( _samph.snum[i-12] < iSampleCount) npair.second=sRemap[_samph.snum[i-12]];
			else npair.second=255;
			instr.NoteToSample(i,npair);
		} else {
			//npair.second=_samph.snum[95]; implicit.
			instr.NoteToSample(i,npair);
		}
	}
	instr.ValidateEnabled();
	delete[] sRemap;
	return iStart;
}
*/

// Load instruments
bool xmsongloader_loadinstruments(XMSongLoader* self, size_t iInstrStart) // , std::map<int, int>& xmtovirtual)
{
	uintptr_t slot;
	uint32_t start;

	start = psyfile_getpos(self->songfile->file);
	for (slot = 0; slot < self->m_Header.instruments; ++slot) {
		start = xmsongloader_loadinstrument(self, slot + 1, start);
	}
	return TRUE;
}


size_t xmsongloader_loadinstrument(XMSongLoader* self, int slot, size_t start)
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
			sample->defaultvolume = xmsamples[s].vol * 2 / (psy_dsp_amp_t)128.f;
			sample->tune = xmsamples[s].relnote;
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
				psyfile_read(fp, smpbuf, xmsamples[s].samplen);
				sample->channels.samples[0] = dsp.memory_alloc(sample->numframes,
					sizeof(psy_dsp_amp_t));
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
			psy_audio_samples_insert(&song->samples, sample,
				sampleindex_make(slot, s));
			start += xmsamples[s].samplen;
		}
		free(xmsamples);
	}
	return start;
}
	
size_t xmsongloader_loadsampleheader(XMSongLoader* self, psy_audio_Sample* _wave, size_t iStart, int iInstrIdx, int iSampleIdx)
{
	PsyFile* fp;

	fp = self->songfile->file;
	// get sample header
	psyfile_seek(fp, iStart);
	int iLen = ReadInt4(fp);

	// loop data
	int iLoopStart = ReadInt4(fp);
	int iLoopLength = ReadInt4(fp);

	// params
	char iVol = ReadInt1(fp);
	char iFineTune = ReadInt1(fp);
	char iFlags = ReadInt1(fp);
	unsigned char iPanning = ReadInt1(fp);
	char iRelativeNote = ReadInt1(fp);
	psyfile_skip(fp, 1);//char iReserved = ReadInt1();

	// sample name
	char * cName = AllocReadStr(fp, 22);

	// parse
	BOOL bLoop = (iFlags & 0x01 || iFlags & 0x02) && (iLoopLength>0);
	BOOL bPingPong = iFlags & 0x02;
	BOOL b16Bit = iFlags & 0x10;
	
	// alloc wave memory

	ASSERT(iLen < (1 << 30)); // Since in some places, signed values are used, we cannot use the whole range.

	psy_audio_sample_init(_wave, 1);	
	if ( iLen > 0 ) // Sounds Stupid, but it isn't. Some modules save sample header when there is no sample.
	{
		_wave->channels.samples[0] = dsp.memory_alloc(_wave->numframes, iLen);
			//sizeof(psy_dsp_amp_t));
		//_wave.AllocWaveData(b16Bit?iLen / 2:iLen,FALSE);
	}
	_wave->panenabled = TRUE; // PanEnabled(true);
	_wave->panfactor = iPanning / 255.0f; // .PanFactor(iPanning/255.0f);
		
	if(bLoop)
	{
		if(bPingPong){
			_wave->loop.type = psy_audio_SAMPLE_LOOP_BIDI;
			// _wave.WaveLoopType(XMInstrument::WaveData<>::LoopType::BIDI);
		}else {
			_wave->loop.type = psy_audio_SAMPLE_LOOP_NORMAL;
			//_wave.WaveLoopType(XMInstrument::WaveData<>::LoopType::NORMAL);
		}
		
		if(b16Bit)
		{
			_wave->loop.start = iLoopStart / 2;
			//_wave.WaveLoopStart(iLoopStart / 2);
			_wave->loop.end = (iLoopLength + iLoopStart) / 2;
			//_wave.WaveLoopEnd((iLoopLength  + iLoopStart )/ 2);
		}
		else
		{
			_wave->loop.start = iLoopStart;
			// _wave.WaveLoopStart(iLoopStart);
			_wave->loop.end = (iLoopLength + iLoopStart) / 2;
			//_wave.WaveLoopEnd(iLoopLength + iLoopStart);
		}
			
//			TRACE2("l:%x s:%x e:%x \n",_wave.WaveLength(),_wave.WaveLoopStart(),_wave.WaveLoopEnd()); 

	} else {
		_wave->loop.type = psy_audio_SAMPLE_LOOP_DO_NOT;
		// _wave.WaveLoopType(XMInstrument::WaveData<>::LoopType::DO_NOT);
	}

	_wave->samplerate = 8363; //.WaveSampleRate(8363);
	// todo check scale
	_wave->defaultvolume = (iVol * 2) / (psy_dsp_amp_t)0x80; // .WaveVolume(iVol * 2);
	_wave->tune = iRelativeNote; // .WaveTune(iRelativeNote);
	_wave->finetune = (short)(iFineTune / 1.28); // WaveFineTune(iFineTune / 1.28); // WaveFineTune has +-100 range in Psycle.
	//std::string sName = cName;
	psy_audio_sample_setname(_wave, cName);
	//_wave.WaveName(sName);
	//delete[] cName;

	self->smpLen[iSampleIdx] = iLen;
	self->smpFlags[iSampleIdx] = iFlags;

	return iStart + 40;

}
	
size_t xmsongloader_loadsampledata(XMSongLoader* self, psy_audio_Sample* _wave, size_t iStart, int iInstrIdx, int iSampleIdx)
{
	PsyFile* fp;

	fp = self->songfile->file;
	// parse
		
	BOOL b16Bit = self->smpFlags[iSampleIdx] & 0x10;
	short wNew=0;

	// cache sample data
	psyfile_seek(fp, iStart);
	char* smpbuf = (char*)malloc(self->smpLen[iSampleIdx]);
	if (!smpbuf) {
		// todo check return value
		return iStart;
	}
	memset(smpbuf,0,self->smpLen[iSampleIdx]);
	psyfile_read(fp, smpbuf, self->smpLen[iSampleIdx]);

	int sampleCnt = self->smpLen[iSampleIdx];

	// unpack sample data
	if(b16Bit)
	{				
		// 16 bit mono sample, delta
		int out=0;
		for(int j=0;j<sampleCnt;j+=2)
		{						
			wNew += (smpbuf[j]&0xFF) | (smpbuf[j+1]<<8);
			_wave->channels.samples[0][j / 2] = (psy_dsp_amp_t)wNew;
			//*(const_cast<signed short*>(_wave.pWaveDataL()) + out) = wNew;
			out++;
		}   
	}
	else
	{
		// 8 bit mono sample
		for(int j=0;j<sampleCnt;j++)
		{			
			wNew += (signed short)(smpbuf[j]<<8);
			_wave->channels.samples[0][j] = (psy_dsp_amp_t)wNew;
			////*(const_cast<signed short*>(_wave.pWaveDataL()) + j) = wNew;
		}
	}

	// cleanup
	free(smpbuf);

	// complete			
	iStart += self->smpLen[iSampleIdx];
	return iStart;
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
	
		/*
		if(sampleHeader.vtype & 2){
			inst.AmpEnvelope().SustainBegin(sampleHeader.vsustain);
			inst.AmpEnvelope().SustainEnd(sampleHeader.vsustain);
		}
		else
		{
			// We can't ignore the loop because IT Envelopes do a fadeout when the envelope end is reached and FT does not.
			// IT also sets the Sustain points to the end of the envelope, but i can't see a reason for this to be needed.
//				inst.AmpEnvelope().SustainBegin(inst.AmpEnvelope().NumOfPoints()-1);
//				inst.AmpEnvelope().SustainEnd(inst.AmpEnvelope().NumOfPoints()-1);
		}

			
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

void modsongloader_init(MODSongLoader* self, psy_audio_SongFile* songfile)
{
	assert(self);

	self->songfile = songfile;
}

void modsongloader_dispose(MODSongLoader* self)
{
	assert(self);
}

/*


	
	MODSongLoader::MODSongLoader(void)
	{
		for (int i=0; i<32; i++)
		{
			smpLen[i]=0;
		}
		speedpatch = false;
	}

	MODSongLoader::~MODSongLoader(void)
	{

	}

	bool MODSongLoader::Load(Song& song,CProgressDialog& progress, bool fullopen)
	{
		CExclusiveLock lock(&song.semaphore, 2, true);
		// check validity
		if(!IsValid()){
			return false;
		}
		song.CreateMachine(MACH_XMSAMPLER, rand()/64, rand()/80, "sampulse",0);
//		song.InsertConnectionNonBlocking(0,MASTER_INDEX,0,0,0.5f); // This is done later, when determining the number of channels.
		song.seqBus=0;
		m_pSampler = static_cast<XMSampler *>(song._pMachine[0]);
		m_pSampler->XM_SAMPLER_PanningMode(XM_SAMPLER_PanningMode::TwoWay);
		// get song name

		char * pSongName = AllocReadStr(20,0);
		if(pSongName==NULL)
			return false;

		song.name = pSongName;
		song.author = "";
		song.comments = "Imported from MOD Module: ";
		song.comments.append(szName);
		delete[] pSongName; pSongName = 0;

		// get data
		Seek(20);
		for (int i=0;i<31;i++) {
			XMInstrument::WaveData<> wave;
			LoadSampleHeader(wave,i);
			song.samples.SetSample(wave,i);
		}
		Seek(950);
		Read(&m_Header,sizeof(m_Header));
		
		char pID[5];
		pID[0]=m_Header.pID[0];pID[1]=m_Header.pID[1];pID[2]=m_Header.pID[2];pID[3]=m_Header.pID[3];pID[4]=0;
		
		
		m_pSampler->IsAmigaSlides(true);
		if ( !strncmp(pID,"M.K.",4)) { song.SONGTRACKS = 4; song.InsertConnectionNonBlocking(0,MASTER_INDEX,0,0,0.355f); }//-9dB
		else if ( !strncmp(pID,"M!K!",4)) { song.SONGTRACKS = 4; song.InsertConnectionNonBlocking(0,MASTER_INDEX,0,0,0.355f); }//-9dB
		else if ( !strncmp(pID+1,"CHN",4)) { char tmp[2]; tmp[0] = pID[0]; tmp[1]=0; song.SONGTRACKS = atoi(tmp);  song.InsertConnectionNonBlocking(0,MASTER_INDEX,0,0,0.355f); } //-9dB
		else if ( !strncmp(pID+2,"CH",4)) { char tmp[3]; tmp[0] = pID[0]; tmp[1]=pID[1]; tmp[2]=0; song.SONGTRACKS = atoi(tmp); song.InsertConnectionNonBlocking(0,MASTER_INDEX,0,0,0.355f);}//-9dB
		song.BeatsPerMin(125);
		song.LinesPerBeat(4);
		song.TicksPerBeat(24);
		song.ExtraTicksPerLine(0);

		if (song.SONGTRACKS<=8) {
			for (int i = 0; i< song.SONGTRACKS ; i++ )
			{
				if (i%4 == 0 || i%4 == 3) m_pSampler->rChannel(i).DefaultPanFactorFloat(0.25f,true);
				else m_pSampler->rChannel(i).DefaultPanFactorFloat(0.75,true);
			}
		}
		else {
			for (int i = 0; i< song.SONGTRACKS ; i++ )
			{
				m_pSampler->rChannel(i).DefaultPanFactorFloat(0.5f,true);
			}
		}
		std::map<int,int> modtovirtual;
		int virtidx=MAX_MACHINES;
		for (int i=0; i < 31;i++) {
			if (m_Samples[i].sampleLength > 0 ) {
				modtovirtual[i]=virtidx;
				virtidx++;
			}
		}
		LoadPatterns(song, modtovirtual);
		for(int i = 0;i < 31;i++){
			LoadInstrument(song,i);
			if (song.xminstruments.IsEnabled(i)) {
				song.SetVirtualInstrument(modtovirtual[i],0,i);
			}
		}
		return true;
	}

	bool MODSongLoader::IsValid()
	{
		bool bIsValid = false;
		char *pID = AllocReadStr(4,1080);

		bIsValid = !strncmp(pID,"M.K.",4);
		if ( !bIsValid ) bIsValid = !strncmp(pID,"M!K!",4);
		if ( !bIsValid ) bIsValid = !strncmp(pID+1,"CHN",4);
		if ( !bIsValid ) bIsValid = !strncmp(pID+2,"CH",4);

		delete[] pID;
		return bIsValid;
	}

	void MODSongLoader::LoadPatterns(Song & song, std::map<int,int>& modtovirtual)
	{
		int npatterns=0;
		for(int i = 0;i < MAX_SONG_POSITIONS && i < m_Header.songlength;i++)
		{
			if ( m_Header.order[i] < MAX_PATTERNS ){
				song.playOrder[i]=m_Header.order[i];
				if ( m_Header.order[i] > npatterns) npatterns=m_Header.order[i];
			} else { 
				song.playOrder[i]=0;
				if ( m_Header.order[i] > npatterns) npatterns=m_Header.order[i];
			}
		}
		npatterns++;
		if ( m_Header.songlength > MAX_SONG_POSITIONS ){
			song.playLength=MAX_SONG_POSITIONS;
		} else {
			song.playLength=m_Header.songlength;
		}

		// get pattern data
		Seek(1084);
		for(int j = 0;j < npatterns ;j++){
			LoadSinglePattern(song,j,song.SONGTRACKS, modtovirtual );
		}
		if(speedpatch) {
			song.SONGTRACKS++;
		}
	}

	char * MODSongLoader::AllocReadStr(int32_t size, signed int start)
	{
		// allocate space
		char *pData = new char[size + 1];
		if(pData==NULL)
			return NULL;

		// null terminate
		pData[size]=0;

		// go to offset
		if(start>=0)
			Seek(start);

		// read data
		if(Read(pData,size))
			return pData;

		delete[] pData;
		return NULL;
	}





	// return address of next pattern, 0 for invalid
	void MODSongLoader::LoadSinglePattern(Song & song,int patIdx,int iTracks, std::map<int,int>& modtovirtual)
	{

		short iNumRows = 64;

		song.AllocNewPattern(patIdx,"unnamed",iNumRows,false);

		unsigned char lastmach[64];
		std::memset(lastmach,255,sizeof(char)*64);

		PatternEntry e;
		unsigned char mentry[4];

			// get next values
			for(int row = 0;row < iNumRows;row++)
			{
				for(int col=0;col<iTracks;col++)
				{	
					// reset
					unsigned char note=notecommands::empty;
					unsigned char instr=255;
					unsigned char type=0;
					unsigned char param=0;
					unsigned short period=428;

					// read note
					mentry[0] = ReadUInt1(); mentry[1] = ReadUInt1(); mentry[2] = ReadUInt1(); mentry[3] = ReadUInt1();
					instr = ((mentry[0] & 0xF0) + (mentry[2] >> 4));
					period = ((mentry[0]& 0x0F) << 8) + mentry[1];
					type = (mentry[2] & 0x0F);
					param = mentry[3];
					note = ConvertPeriodtoNote(period);

					// translate
					e._parameter = param;

					int exchwave[3]={XMInstrument::WaveData<>::WaveForms::SINUS,
						XMInstrument::WaveData<>::WaveForms::SAWDOWN,
						XMInstrument::WaveData<>::WaveForms::SQUARE
					};

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
							e._cmd = PatternCMD_JUMP_TO_ORDER;
							break;
						case XMCMD_VOLUME:
							e._cmd = XM_SAMPLER_CMD_VOLUME;
							e._parameter = param<=0x40?param*2:0x80;
							break;
						case XMCMD_PATTERN_BREAK:
							e._cmd = PatternCMD_BREAK_TO_LINE;
							e._parameter = ((param&0xF0)>>4)*10 + (param&0x0F);
							break;
						case XMCMD_EXTENDED:
							switch(param & 0xf0){
							case XMCMD_E::E_FINE_PORTA_UP:
								e._cmd = XM_SAMPLER_CMD_PORTAMENTO_UP;
								e._parameter= 0xF0+(param&0x0F);
								break;
							case XMCMD_E::E_FINE_PORTA_DOWN:
								e._cmd = XM_SAMPLER_CMD_PORTAMENTO_DOWN;
								e._parameter= 0xF0+(param&0x0F);
								break;
							case XMCMD_E::E_GLISSANDO_STATUS:
								e._cmd = XM_SAMPLER_CMD_EXTENDED;
								e._parameter = XM_SAMPLER_CMD_E::E_GLISSANDO_TYPE | ((param==0)?0:1);
								break;
							case XMCMD_E::E_VIBRATO_WAVE:
								e._cmd = XM_SAMPLER_CMD_EXTENDED;
								e._parameter =XM_SAMPLER_CMD_E::E_VIBRATO_WAVE | exchwave[param & 0x3];
								break;
							case XMCMD_E::E_FINETUNE:
								e._cmd = XM_SAMPLER_CMD_NONE;
								e._parameter = 0;
								break;
							case XMCMD_E::E_PATTERN_LOOP:
								e._cmd = PatternCMD_EXTENDED;
								e._parameter = PatternCMD_PATTERN_LOOP | (param & 0xf);
								break;
							case XMCMD_E::E_TREMOLO_WAVE:
								e._cmd = XM_SAMPLER_CMD_EXTENDED;
								e._parameter = XM_SAMPLER_CMD_E::E_TREMOLO_WAVE | exchwave[param & 0x3];
								break;
							case XMCMD_E::E_MOD_RETRIG:
								e._cmd = XM_SAMPLER_CMD_RETRIG;
								e._parameter = param & 0xf;
								break;
							case XMCMD_E::E_FINE_VOLUME_UP:
								e._cmd = XM_SAMPLER_CMD_VOLUMESLIDE;
								e._parameter = 0x0f + ((param & 0xf)<<4);
								break;
							case XMCMD_E::E_FINE_VOLUME_DOWN:
								e._cmd = XM_SAMPLER_CMD_VOLUMESLIDE;
								e._parameter = 0xf0 + (param & 0xf);
								break;
							case XMCMD_E::E_DELAYED_NOTECUT:
								e._cmd = XM_SAMPLER_CMD_EXTENDED;
								e._parameter = XM_SAMPLER_CMD_E::E_DELAYED_NOTECUT | (param & 0xf);
								break;
							case XMCMD_E::E_NOTE_DELAY:
								e._cmd = XM_SAMPLER_CMD_EXTENDED;
								e._parameter = XM_SAMPLER_CMD_E::E_NOTE_DELAY | ( param & 0xf);
								break;
							case XMCMD_E::E_PATTERN_DELAY:
								e._cmd = PatternCMD_EXTENDED;
								e._parameter =  PatternCMD_PATTERN_DELAY | (param & 0xf);
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
								e._cmd=PatternCMD_EXTENDED;
								int extraticks=0;
								e._parameter = XM_SAMPLER_CalcLPBFromSpeed(param,extraticks);
								if (extraticks != 0) {
									speedpatch=true;
									PatternEntry entry(notecommands::empty,0xFF,0xFF,0,0);
									entry._cmd = PatternCMD_EXTENDED;
									entry._parameter = PatternCMD_ROW_EXTRATICKS | extraticks;
									WritePatternEntry(song,patIdx,row,song.SONGTRACKS,entry);	
								}
							}
							else
							{
								e._cmd = PatternCMD_SET_TEMPO;
							}
							break;
						default:
							e._cmd = XM_SAMPLER_CMD_NONE;
							break;
					}
					// instrument/note
					if ( note != 255 ) e._note  = note+12;
					else e._note  = note;
					if ( instr != 0 ) e._inst = instr-1;
					else e._inst = 255;

					// If empty, do not inform machine
					if (e._note == notecommands::empty && e._inst == 255 && e._cmd == 00 && e._parameter == 00) {
						e._mach = 255;
					}
					// if instrument without note, or note without instrument, cannot use virtual instrument, so use sampulse directly
					else if (( e._note == notecommands::empty && e._inst != 255 ) || ( e._note < notecommands::release && e._inst == 255)) {
						e._mach = 0;
						if (e._inst != 255) {
							//We cannot use the virtual instrument, but we should remember which it is.
							std::map<int,int>::const_iterator it = modtovirtual.find(e._inst);
							if (it != modtovirtual.end()) {
								lastmach[col]=it->second;
							}
						}
					}
					//default behaviour, let's find the virtual instrument.
					else {
						std::map<int,int>::const_iterator it = modtovirtual.find(e._inst);
						if (it == modtovirtual.end()) {
							if (e._inst != 255) {
								e._mach = 0;
								lastmach[col] = e._mach;
							}
							else if (lastmach[col] != 255) {
								e._mach = lastmach[col];
							}
							else {
								e._mach = 255;
							}
						}
						else {
							e._mach=it->second;
							e._inst=255;
						}
					}

					WritePatternEntry(song,patIdx,row,col,e);	
				}
			}
	}
	unsigned char MODSongLoader::ConvertPeriodtoNote(unsigned short period)
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
			int note = round<int,double>(152.89760383681376337437517761588 /*48 + 12*log2(1.0/428.0)*/
				//-log10(static_cast<double>(period)) * 39.863137138648348174443833153873) /*12/log10(2)*/ //;
				/*
			return static_cast<unsigned char>(note);
		}
	}

	BOOL MODSongLoader::WritePatternEntry(Song & song,
		const int patIdx, const int row, const int col,PatternEntry &e)
	{
		// don't overflow song buffer 
		if(patIdx>=MAX_PATTERNS) return false;

		PatternEntry* pData = reinterpret_cast<PatternEntry*>(song._ptrackline(patIdx,col,row));

		*pData = e;

		return true;
	}	

	void MODSongLoader::LoadInstrument(Song & song, int idx)
	{
		XMInstrument instr;
		instr.Init();
		instr.Name(m_Samples[idx].sampleName);

		if (m_Samples[idx].sampleLength > 0 ) 
		{
			LoadSampleData(song.samples.get(idx),idx);

			int i;
			XMInstrument::NotePair npair;
			npair.second=idx;
			for(i = 0;i < XMInstrument::NOTE_MAP_SIZE;i++){
				npair.first=i;
				instr.NoteToSample(i,npair);
			}
		}

		instr.ValidateEnabled();
		song.xminstruments.SetInst(instr,idx);
	}

	void MODSongLoader::LoadSampleHeader(XMInstrument::WaveData<>& _wave, int iInstrIdx)
	{
		Read(m_Samples[iInstrIdx].sampleName,22);	m_Samples[iInstrIdx].sampleName[21]='\0';

		smpLen[iInstrIdx] = (ReadUInt1()*0x100+ReadUInt1())*2; 
		m_Samples[iInstrIdx].sampleLength = smpLen[iInstrIdx];
		m_Samples[iInstrIdx].finetune = ReadUInt1();
		m_Samples[iInstrIdx].volume = ReadUInt1();
		m_Samples[iInstrIdx].loopStart =((ReadUInt1()*256+ReadUInt1())*2); 
		m_Samples[iInstrIdx].loopLength = (ReadUInt1()*256+ReadUInt1())*2; 

		// parse
		BOOL bLoop = (m_Samples[iInstrIdx].loopLength > 3);

		_wave.Init();
		if ( smpLen[iInstrIdx] > 0 )
		{
			_wave.AllocWaveData(smpLen[iInstrIdx],false);
		}

		if(bLoop)
		{
			_wave.WaveLoopType(XMInstrument::WaveData<>::LoopType::NORMAL);
			_wave.WaveLoopStart(m_Samples[iInstrIdx].loopStart);
			if ( m_Samples[iInstrIdx].loopStart+m_Samples[iInstrIdx].loopLength > smpLen[iInstrIdx]) 
			{
					_wave.WaveLoopEnd(smpLen[iInstrIdx]);
			} else 	_wave.WaveLoopEnd(m_Samples[iInstrIdx].loopStart+m_Samples[iInstrIdx].loopLength);
		} else {
			_wave.WaveLoopType(XMInstrument::WaveData<>::LoopType::DO_NOT);
		}

		_wave.WaveVolume(m_Samples[iInstrIdx].volume * 2);
		_wave.WaveSampleRate(8363);
		char tmpfine = (char)m_Samples[iInstrIdx].finetune;
		if (tmpfine > 7 ) tmpfine -= 16;
		_wave.WaveFineTune(tmpfine*12.5);// finetune has +-100 range in Psycle
		std::string sName = m_Samples[iInstrIdx].sampleName;
		_wave.WaveName(sName);

	}

	void MODSongLoader::LoadSampleData(XMInstrument::WaveData<>& _wave, int iInstrIdx)
	{
		// parse
		short wNew=0;

		// cache sample data
		unsigned char * smpbuf = new unsigned char[smpLen[iInstrIdx]];
		Read(smpbuf,smpLen[iInstrIdx]);

		int sampleCnt = smpLen[iInstrIdx];

		// 8 bit mono sample
		for(int j=0;j<sampleCnt;j++)
		{
			//In mods, samples are signed integer, so we can simply left shift
			wNew = (smpbuf[j]<<8);
			*(const_cast<signed short*>(_wave.pWaveDataL()) + j) = wNew;
		}

		// cleanup
		delete[] smpbuf;
	}
*/

