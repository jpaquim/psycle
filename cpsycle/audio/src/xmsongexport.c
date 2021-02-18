// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "xmsongexport.h"
// local
#include "psyconvert.h"
#include "samplerdefs.h"
#include "song.h"
#include "songio.h"
#include "xmdefs.h"
#include "sampler.h"
// std
#include <math.h>
// platform
#include "../../detail/portable.h"

// prototypes
static int xmsongexport_writesongheader(XMSongExport*);
static int xmsongexport_savepatterns(XMSongExport*);
static int xmsongexport_savesinglepattern(XMSongExport*, int patIdx);
static void xmsongexport_getcommand(XMSongExport*,int i,
	const psy_audio_LegacyPatternEntry*, unsigned char* vol, unsigned char* type,
	unsigned char* param);
static int xmsongexport_saveinstruments(XMSongExport*);
static int xmsongexport_saveemptyinstrument(XMSongExport*, const char* name);
static int xmsongexport_savesampulseinstrument(XMSongExport*, int instIdx);
static int xmsongexport_savesamplerinstrument(XMSongExport*, int instIdx);
static int xmsongexport_savesampleheader(XMSongExport* self, int instIdx, int sampleIdx);
static int xmsongexport_savesampledata(XMSongExport*, int instrIdx, int sampleindex);
static void xmsongexport_setsampulseenvelopes(XMSongExport*, int instrIdx,
	struct XMSAMPLEHEADER*);
static void xmsongexport_setsamplerenvelopes(XMSongExport*, int instrIdx,
	struct XMSAMPLEHEADER*);

//implementation
void xmsongexport_init(XMSongExport* self, psy_audio_SongFile* songfile)
{
	assert(self);
	assert(songfile);
	assert(songfile->song);
	assert(songfile->file);

	self->songfile = songfile;
	self->fp = songfile->file;
	self->song = songfile->song;
}

void xmsongexport_dispose(XMSongExport* self)
{
}

int xmsongexport_exportsong(XMSongExport* self)
{
	int status;

	assert(self);
	assert(self->songfile);
	assert(self->songfile->song);
	assert(self->songfile->file);

	if (status = xmsongexport_writesongheader(self)) {
		return PSY_ERRFILE;
	}
	if (status = xmsongexport_savepatterns(self)) {
		return PSY_ERRFILE;
	}
	if (status = xmsongexport_saveinstruments(self)) {
		return PSY_ERRFILE;
	}
	return PSY_OK;
}

int xmsongexport_writesongheader(XMSongExport* self)
{
	char modulename[20];
	uintptr_t i;	
	bool hasSampler;
	int samInstruments;
	uint16_t temp;
	int status;	

	self->macInstruments = 0;
	hasSampler = FALSE;
	for (i = 0; i < 256; i++) {
		self->isBlitzorVst[i] = FALSE;
	}
	for (i = 0; i < MAX_BUSES; i++) {
		if (psy_audio_machines_at(&self->song->machines, i)  != NULL) {
			self->isSampler[i] = hasSampler = (psy_audio_machine_type(psy_audio_machines_at(&self->song->machines, i)) == MACH_SAMPLER);
			self->isSampulse[i] = (psy_audio_machine_type(psy_audio_machines_at(&self->song->machines, i)) == MACH_XMSAMPLER);
			if (!(self->isSampler[i] || self->isSampulse[i])) {
				self->macInstruments++;
				if (psy_audio_machine_type(psy_audio_machines_at(&self->song->machines, i)) == MACH_PLUGIN) {
					const psy_audio_MachineInfo* info;

					info = psy_audio_machine_info(psy_audio_machines_at(&self->song->machines, i));
					if (info && info->Name) {
						const char* str = info->Name;
						if (strstr(str, "blitz")) {						
							self->isBlitzorVst[i + 1] = TRUE;
						}
					}
				} else if (psy_audio_machine_type(psy_audio_machines_at(&self->song->machines, i)) == MACH_VST) {
					self->isBlitzorVst[i + 1] = TRUE;
				}
			}
		} else {
			self->isSampler[i] = FALSE;
			self->isSampulse[i] = FALSE;
		}
	}
	//If the xminstrument 0 is not used, do not increase the instrument number (the loader does this so exporting a loaded song adds a new empty slot)
	self->correctionIndex = 1; // (song.xminstruments.IsEnabled(0)) ? 1 : 0;
	self->xmInstruments = (int32_t)psy_audio_instruments_size(&self->song->instruments, 1); // song.GetHighestXMInstrumentIndex() + 1;
	if (self->xmInstruments > 1 && self->correctionIndex == 0) self->xmInstruments--;
	//If there is a sampler machine, we have to take the samples into account.
	samInstruments = (int32_t)psy_audio_instruments_size(&self->song->instruments, 0); //0; // (hasSampler) ? song.GetHighestInstrumentIndex() + 1 : 0;

	if (status = psyfile_write(self->fp, XM_HEADER, 17)) { //ID text
		return status;
	}
	// Module name
	// modulename: PE + songtitle 0..17 (incl.) + spaces
	// no null termination
	memset(modulename, 0x20, 20);
	modulename[0] = 'P';
	modulename[1] = 'E';
	modulename[2] = ':';
	for (i = 3; i < 20 && i < strlen(psy_audio_song_title(self->song)); ++i) {
		modulename[i] = psy_audio_song_title(self->song)[i];
	}	
	if (status = psyfile_write(self->fp, modulename, sizeof(modulename))) {
		return status;
	}
	// The hex  value  0x1A  in  a  normal  XM  file or 0x00  in  a  Stripped  on	
	temp = 0x1A;
	if (status = psyfile_write(self->fp, &temp, 1)) {
		return status;
	}
	// Tracker name
	if (status = psyfile_write(self->fp, "FastTracker v2.00   ", 20)) {
		return status;
	}
	// Version number	
	temp = 0x0104;
	if (status = psyfile_write(self->fp, &temp, 2)) {
		return status;
	}

	memset(&self->m_Header, 0, sizeof(self->m_Header));
	self->m_Header.size = sizeof(self->m_Header);
	self->m_Header.norder = (uint16_t)psy_audio_sequence_maxtracksize(&self->song->sequence);
	self->m_Header.restartpos = 0;	
	self->m_Header.channels = (uint16_t)psy_min(psy_audio_song_numsongtracks(self->song), 32);
	// Number of patterns stored in file. There should be at least 1 pattern
	// if you expect to hear anything out of the speakers. The maximum value is
	// 256. Don’t confuse this with[Song length]!
	self->m_Header.patterns = (uint16_t)psy_min(psy_audio_patterns_size(&self->song->patterns), 256);
	self->m_Header.instruments = psy_min(128, self->macInstruments + self->xmInstruments + samInstruments);
	self->m_Header.flags = 0x0001; //Linear frequency.	
	self->m_Header.speed = (uint16_t)(floor(24.f / psy_audio_song_lpb(self->song))) +
		(uint16_t)psy_audio_song_extraticksperbeat(self->song);
	self->m_Header.tempo = (uint16_t)psy_audio_song_bpm(self->song);

	// Pattern order table	
	for (i = 0; i < psy_audio_sequence_track_size(&self->song->sequence, 0);
			++i) {
		self->m_Header.order[i] = (uint8_t)psy_min(0xFF,
				psy_audio_sequence_patternindex(&self->song->sequence,
			psy_audio_orderindex_make(0, i)));		
	}	
	if (psyfile_write(self->fp, &self->m_Header, sizeof(self->m_Header))) {
		return PSY_ERRFILE;
	}
	return PSY_OK;
}

int xmsongexport_savepatterns(XMSongExport* self)
{
	uint16_t i;
	int status;

	for (i = 0; i < self->m_Header.patterns; ++i) {
		if (status = xmsongexport_savesinglepattern(self, i)) {
			return status;
		}
	}
	return PSY_OK;
}

// return address of next pattern, 0 for invalid
int xmsongexport_savesinglepattern(XMSongExport* self, int patIdx)
{
	//Temp entries for volume on virtual generators.
	psy_audio_LegacyPatternEntry volumeEntries[32];
	struct XMPATTERNHEADER ptHeader;
	size_t currentpos;
	int32_t maxtracks;
	int32_t i;
	int32_t j;
	psy_audio_Pattern* pattern;
	psy_audio_LegacyPattern ppattern;
	int32_t lines;
	int status;
	
	memset(&ptHeader, 0, sizeof(ptHeader));
	ptHeader.size = sizeof(ptHeader);
	//ptHeader.packingtype = 0; implicit from memset.
	pattern = psy_audio_patterns_at(&self->song->patterns, patIdx);
	if (!pattern) {
		return PSY_ERRFILE;
	}	
	ppattern = psy_audio_allocoldpattern(pattern, psy_audio_song_lpb(self->song),
		&lines);
	ptHeader.rows = psy_min(256, lines);
	//ptHeader.packedsize = 0; implicit from memset.

	if (status = psyfile_write(self->fp, &ptHeader, sizeof(ptHeader))) {
		return status;
	}
	currentpos = psyfile_getpos(self->fp);

	maxtracks = (int32_t)psy_min(psy_audio_song_numsongtracks(self->song), 32);
	// check every pattern for validity
	if (psy_audio_sequence_patternused(&self->song->sequence, patIdx))
	{		
		for (i = 0; i < maxtracks; i++) {
			self->lastInstr[i] = -1;
		}
		self->addTicks = 0;
		for (j = 0; j < ptHeader.rows; j++) {
			for (i = 0; i < maxtracks; i++) {
				self->extraEntry[i] = NULL;
			}
			for (i = 0; i < (int32_t)psy_audio_song_numsongtracks(self->song); i++) {
				const psy_audio_LegacyPatternEntry* pData;

				pData = psy_audio_ptrackline_const(ppattern, i, j);
				if (pData->_note == PSY2_NOTECOMMANDS_MIDICC) {
					if (pData->_inst < maxtracks) {
						self->extraEntry[pData->_inst] = pData;
					}
				} else if (pData->_note != PSY2_NOTECOMMANDS_TWEAK && pData->_note != PSY2_NOTECOMMANDS_TWEAKSLIDE) {
					if (pData->_cmd == psy_audio_PATTERNCMD_EXTENDED && (pData->_parameter & 0xF0) == psy_audio_PATTERNCMD_ROW_EXTRATICKS) {
						self->addTicks = pData->_parameter & 0x0F;
					}
				}
			}
			for (i = 0; i < maxtracks; ++i) {
				unsigned char note;
				psy_audio_Machine* mac = NULL;
				unsigned char instr = 0;
				int instrint = 0xFF;
				unsigned char vol;
				unsigned char type;
				unsigned char param;
				unsigned char bWriteNote;
				unsigned char bWriteInstr;
				unsigned char bWriteVol;
				unsigned char bWriteType;
				unsigned char bWriteParam;
				char compressed;
				const psy_audio_LegacyPatternEntry* pData;

				pData = psy_audio_ptrackline_const(ppattern, i, j);
				if (pData->_note == psy_audio_NOTECOMMANDS_TWEAK ||
						pData->_note == psy_audio_NOTECOMMANDS_TWEAKSLIDE) {
					unsigned char compressed = 0x80;

					if (status = psyfile_write(self->fp, &compressed, 1)) {
						free(ppattern);
						return status;
					}
					continue;
				} else if (pData->_note == psy_audio_NOTECOMMANDS_MIDICC) {
					if (pData->_inst > 0x7F) {
						unsigned char compressed = 0x80 + 2 + 8 + 16;
						unsigned char val = XMCMD_MIDI_MACRO;

						if (status = psyfile_write(self->fp, &compressed, 1)) {
							free(ppattern);
							return status;						
						}
						if (status = psyfile_write(self->fp, &pData->_inst, 1)) {
							free(ppattern);
							return status;
						}
						if (status = psyfile_write(self->fp, &val, 1)) {
							free(ppattern);
							return status;
						}
						if (status = psyfile_write(self->fp, &pData->_cmd, 1)) {
							free(ppattern);
							return status;
						}
					} else {
					unsigned char compressed = 0x80;

					if (status = psyfile_write(self->fp, &compressed, 1)) {
						free(ppattern);
						return status;
					}
					}
					continue;
				}
				if (pData->_mach < MAX_BUSES) {
					mac = psy_audio_machines_at(&self->song->machines, pData->_mach);
					instrint = pData->_inst;
				} else if (pData->_mach >= MAX_MACHINES && pData->_mach < MAX_VIRTUALINSTS) {
					// todo virtual generators
					mac = psy_audio_machines_at(&self->song->machines, pData->_mach);
					/// song.GetMachineOfBus(pData->_mach, instrint);
					if (instrint == -1) instrint = 0xFF;
					if (mac != NULL && pData->_inst != 255) {
						volumeEntries[i]._cmd = (psy_audio_machine_type(mac) == MACH_SAMPLER)
							? PS1_SAMPLER_CMD_VOLUME :
							XM_SAMPLER_CMD_SENDTOVOLUME;
						volumeEntries[i]._parameter = pData->_inst;
						self->extraEntry[i] = &volumeEntries[i];
					}
				}
				if (mac != NULL) {
					if (psy_audio_machine_type(mac) == MACH_SAMPLER) {
						if (instrint != 0xFF) instr = (uint8_t)(self->macInstruments + self->xmInstruments + instrint + 1);
					} else if (psy_audio_machine_type(mac) == MACH_XMSAMPLER) {
						if (instrint != 0xFF &&
							(pData->_note != psy_audio_NOTECOMMANDS_EMPTY || pData->_mach < MAX_BUSES)) {
							instr = (uint8_t)(self->macInstruments + instrint + self->correctionIndex);
						}
					} else {
						instr = (uint8_t)(pData->_mach + 1);
					}
					if (instr != 0) self->lastInstr[i, instr];
				}

				if (pData->_note >= 12 && pData->_note < 108) {
					bool isdefaultc4;

					// todo
					isdefaultc4 = FALSE; // (Sampler*)mac)->isDefaultC4() == false)
					if (mac != NULL && psy_audio_machine_type(mac) == MACH_SAMPLER && isdefaultc4)
					{
						note = pData->_note + 1;
					} else {
						note = pData->_note - 11;
					}
				} else if (pData->_note == psy_audio_NOTECOMMANDS_RELEASE) {
					note = 0x61;
				} else {
					note = 0x00;
				}

				vol = 0;
				type = 0;
				param = 0;

				xmsongexport_getcommand(self, i, pData, &vol, &type, &param);
				if (self->extraEntry[i] != NULL) {
					xmsongexport_getcommand(self, i, self->extraEntry[i], &vol,
						&type, &param);
				}

				bWriteNote = note != 0;
				bWriteInstr = instr != 0;
				bWriteVol = vol != 0;
				bWriteType = type != 0;
				bWriteParam = param != 0;

				compressed = 0x80 + bWriteNote + (bWriteInstr << 1) + (bWriteVol << 2)
					+ (bWriteType << 3) + (bWriteParam << 4);

				if (compressed != 0x9F) {
					if (status = psyfile_write(self->fp, &compressed, 1)) { // 0x9F means to write everything.
						free(ppattern);
						return status;
					}
				}
				if (bWriteNote) {
					if (status = psyfile_write(self->fp, &note, 1)) {
						free(ppattern);
						return status;
					}
				}
				if (bWriteInstr) {
					if (status = psyfile_write(self->fp, &instr, 1)) {
						free(ppattern);
						return status;
					}
				}
				if (bWriteVol) {
					if (status = psyfile_write(self->fp, &vol, 1)) {
						free(ppattern);
						return status;
					}					
				}
				if (bWriteType) {
					if (status = psyfile_write(self->fp, &type, 1)) {
						free(ppattern);
						return status;
					}
				}
				if (bWriteParam) {
					if (status = psyfile_write(self->fp, &param, 1)) {
						free(ppattern);
						return status;
					}
				}
			}
		}
		ptHeader.packedsize = (uint16_t)((psyfile_getpos(self->fp) - currentpos) & 0xFFFF);
		if (psyfile_seek(self->fp, (uint32_t)(currentpos - sizeof(ptHeader))) == -1) {
			free(ppattern);
			return status;
		}
		if (status = psyfile_write(self->fp, &ptHeader, sizeof(ptHeader))) {
			free(ppattern);
			return status;
		}
		psyfile_skip(self->fp, ptHeader.packedsize);		
	}	
	else {
		if (status = psyfile_write(self->fp, &ptHeader, sizeof(ptHeader))) {
			free(ppattern);
			ppattern = NULL;
			return status;
		}
	}
	free(ppattern);
	ppattern = NULL;
	return PSY_OK;
}

void xmsongexport_getcommand(XMSongExport* self, int i, const psy_audio_LegacyPatternEntry* pData, unsigned char* vol, unsigned char* type, unsigned char* param)
{	
	int singleEffectCharacter = (pData->_cmd & 0xF0);
	if (singleEffectCharacter == 0xF0) { //Global commands
		switch (pData->_cmd) {
		case psy_audio_PATTERNCMD_SET_TEMPO:
			if (pData->_parameter >= 0x20) {
				*type = XMCMD_SETSPEED;
				*param = pData->_parameter;
			}
			break;
		case psy_audio_PATTERNCMD_EXTENDED:
			switch (pData->_parameter & 0xF0) {
			case psy_audio_PATTERNCMD_SET_LINESPERBEAT0:
			case psy_audio_PATTERNCMD_SET_LINESPERBEAT1:
				*type = XMCMD_SETSPEED;
				*param = (uint8_t)(floor(24.f / pData->_parameter) + self->addTicks);
				self->addTicks = 0;
				break;
			case psy_audio_PATTERNCMD_PATTERN_LOOP:
				*type = XMCMD_EXTENDED;
				*param = XMCMD_E_PATTERN_LOOP + (pData->_parameter & 0x0F);
				break;
			case psy_audio_PATTERNCMD_PATTERN_DELAY:
				*type = XMCMD_EXTENDED;
				*param = XMCMD_E_PATTERN_DELAY + (pData->_parameter & 0x0F);
				break;
			default:
				break;
			}
			break;
		case psy_audio_PATTERNCMD_JUMP_TO_ORDER:
			*type = XMCMD_POSITION_JUMP;
			*param = pData->_parameter;
			break;
		case psy_audio_PATTERNCMD_BREAK_TO_LINE:
			*type = XMCMD_PATTERN_BREAK;
			*param = ((pData->_parameter / 10) << 4) + (pData->_parameter % 10);
			break;
		case psy_audio_PATTERNCMD_SET_VOLUME:
			if (pData->_inst == 255) {
				*type = XMCMD_SET_GLOBAL_VOLUME;
				*param = pData->_parameter >> 1;
			}
			break;
		case psy_audio_PATTERNCMD_NOTE_DELAY:
			*type = XMCMD_EXTENDED;
			*param = XMCMD_E_NOTE_DELAY | ((uint8_t)(pData->_parameter * self->song->properties.tpb / 256));
			break;
		case psy_audio_PATTERNCMD_ARPEGGIO:
			*type = XMCMD_ARPEGGIO;
			*param = pData->_parameter;
			break;
		default:
			break;
		}
	} else if (self->lastInstr[i] != -1 && self->isBlitzorVst[self->lastInstr[i]]) {
		if (singleEffectCharacter == 0xE0) { //Blitz and VST "special" slide up
			//int toneDest = (pData->_cmd & 0x0F); This has no mapping to module commands
			*type = XMCMD_PORTAUP;
			*param = pData->_parameter;
		} else if (singleEffectCharacter == 0xD0) { //Blitz and VST "special" slide down
			//int toneDest = (pData->_cmd & 0x0F); This has no mapping to module commands
			*type = XMCMD_PORTADOWN;
			*param = pData->_parameter;
		} else if (singleEffectCharacter == 0xC0) { //Blitz
			switch (pData->_cmd) {
			case 0xC1: // Blitz slide up
				*type = XMCMD_PORTAUP;
				*param = pData->_parameter;
				break;
			case 0xC2: // Blitz slide donw
				*type = XMCMD_PORTADOWN;
				*param = pData->_parameter;
				break;
			case 0xC3: // blitz tone portamento and VST porta
				*type = XMCMD_PORTA2NOTE;
				*param = pData->_parameter;
				break;
			case 0xCC: // blitz old volume
				*vol = 0x10 + (pData->_parameter >> 2);
				break;
			default:
				break;
			}
		} else if (pData->_cmd == 0x0C) {
			*vol = 0x10 + (pData->_parameter >> 2);
		} else if (pData->_cmd > 0) {
			*type = XMCMD_ARPEGGIO;
			*param = pData->_cmd;
		}
	} else if (self->lastInstr[i] != -1 && self->lastInstr[i] > self->macInstruments && 
		self->lastInstr[i] <= self->macInstruments + self->xmInstruments)
	{ //Sampulse
		switch (pData->_cmd) {
		case XM_SAMPLER_CMD_ARPEGGIO:
			*type = XMCMD_ARPEGGIO;
			*param = pData->_parameter;
			break;
		case XM_SAMPLER_CMD_PORTAMENTO_UP:
			if (pData->_parameter < 0xF0) {
				*type = XMCMD_PORTAUP;
				*param = pData->_parameter;
			} else {
				*type = XMCMD_EXTENDED;
				*param = XMCMD_E_FINE_PORTA_UP | (pData->_parameter & 0xF);
			}
			break;
		case XM_SAMPLER_CMD_PORTAMENTO_DOWN:
			if (pData->_parameter < 0xF0) {
				*type = XMCMD_PORTADOWN;
				*param = pData->_parameter;
			} else {
				*type = XMCMD_EXTENDED;
				*param = XMCMD_E_FINE_PORTA_DOWN | (pData->_parameter & 0xF);
			}
			break;
		case XM_SAMPLER_CMD_PORTA2NOTE:
			*type = XMCMD_PORTA2NOTE;
			*param = pData->_parameter;
			break;
		case XM_SAMPLER_CMD_VIBRATO:
			*type = XMCMD_VIBRATO;
			*param = pData->_parameter;
			break;
		case XM_SAMPLER_CMD_TONEPORTAVOL:
			*type = XMCMD_ARPEGGIO;
			*param = pData->_parameter;
			break;
		case XM_SAMPLER_CMD_VIBRATOVOL:
			*type = XMCMD_VIBRATOVOL;
			*param = pData->_parameter;
			break;
		case XM_SAMPLER_CMD_TREMOLO:
			*type = XMCMD_TREMOLO;
			*param = pData->_parameter;
			break;
		case XM_SAMPLER_CMD_PANNING:
			*type = XMCMD_PANNING;
			*param = pData->_parameter;
			break;
		case XM_SAMPLER_CMD_OFFSET:
			*type = XMCMD_OFFSET;
			*param = pData->_parameter;
			break;
		case XM_SAMPLER_CMD_VOLUMESLIDE:
			if ((pData->_parameter & 0xF) == 0xF) {
				*type = XMCMD_EXTENDED;
				*param = XMCMD_E_FINE_VOLUME_UP | ((pData->_parameter & 0xF0) >> 4);
			} else if ((pData->_parameter & 0xF0) == 0xF0) {
				*type = XMCMD_EXTENDED;
				*param = XMCMD_E_FINE_VOLUME_DOWN | (pData->_parameter & 0xF);
			} else {
				*type = XMCMD_VOLUMESLIDE;
				*param = pData->_parameter;
			}
			break;
		case XM_SAMPLER_CMD_VOLUME:
			*vol = 0x10 + ((pData->_parameter <= 0x80) ? (pData->_parameter >> 1) : 0x40);
			break;
		case XM_SAMPLER_CMD_EXTENDED:
			switch (pData->_parameter & 0xF0) {
			case XM_SAMPLER_CMD_E_GLISSANDO_TYPE:
				*type = XMCMD_EXTENDED;
				*param = XMCMD_E_GLISSANDO_STATUS | ((pData->_parameter == 0) ? 0 : 1);
				break;
			case XM_SAMPLER_CMD_E_DELAYED_NOTECUT:
				*type = XMCMD_EXTENDED;
				*param = XMCMD_E_DELAYED_NOTECUT | (pData->_parameter & 0xF);
				break;
			case XM_SAMPLER_CMD_E_NOTE_DELAY:
				*type = XMCMD_EXTENDED;
				*param = XMCMD_E_NOTE_DELAY | (pData->_parameter & 0xf);
				break;
			default:
				break;
			}
			break;
		case XM_SAMPLER_CMD_RETRIG:
			*type = XMCMD_EXTENDED;
			*param = XMCMD_E_MOD_RETRIG | (pData->_parameter & 0xF);
			break;
		case XM_SAMPLER_CMD_SENDTOVOLUME:
			if (pData->_parameter < 0x40) {
				*vol = 0x10 + ((pData->_parameter < 0x40) ? pData->_parameter : 0x40);
			} else if ((pData->_parameter & 0xF0) == XM_SAMPLER_CMD_VOL_VOLSLIDEUP) {
				*vol = XMV_VOLUMESLIDEUP | (pData->_parameter & 0x0F);
			} else if ((pData->_parameter & 0xF0) == XM_SAMPLER_CMD_VOL_VOLSLIDEDOWN) {
				*vol = XMV_VOLUMESLIDEDOWN | (pData->_parameter & 0x0F);
			} else if ((pData->_parameter & 0xF0) == XM_SAMPLER_CMD_VOL_FINEVOLSLIDEUP) {
				*vol = XMV_FINEVOLUMESLIDEUP | (pData->_parameter & 0x0F);
			} else if ((pData->_parameter & 0xF0) == XM_SAMPLER_CMD_VOL_FINEVOLSLIDEDOWN) {
				*vol = XMV_FINEVOLUMESLIDEDOWN | (pData->_parameter & 0x0F);
			} else if ((pData->_parameter & 0xF0) == XM_SAMPLER_CMD_VOL_PANNING) {
				*vol = XMV_PANNING | (pData->_parameter & 0x0F);
			} else if ((pData->_parameter & 0xF0) == XM_SAMPLER_CMD_VOL_PANSLIDELEFT) {
				*vol = XMV_PANNINGSLIDELEFT | (((pData->_parameter & 0x0F) > 3) ? 0xF : (pData->_parameter & 0x0F) << 2);
			} else if ((pData->_parameter & 0xF0) == XM_SAMPLER_CMD_VOL_PANSLIDERIGHT) {
				*vol = XMV_PANNINGSLIDERIGHT | (((pData->_parameter & 0x0F) > 3) ? 0xF : (pData->_parameter & 0x0F) << 2);
			} else if ((pData->_parameter & 0xF0) == XM_SAMPLER_CMD_VOL_VIBRATO) {
				*vol = XMV_VIBRATO | (pData->_parameter & 0x0F);
			} else if ((pData->_parameter & 0xF0) == XM_SAMPLER_CMD_VOL_TONEPORTAMENTO) {
				*vol = XMV_PORTA2NOTE | (pData->_parameter & 0x0F);
			}
			break;
		default:
			break;
		}
	} else if (self->lastInstr[i] != -1 && self->lastInstr[i] > self->macInstruments + self->xmInstruments)
	{ // Sampler
		switch (pData->_cmd) {
		case 0x01:
			*type = XMCMD_PORTAUP;
			*param = pData->_parameter >> 1; // There isn't a correct conversion. It is non-linear, but acting on speed, not period, and based on sample rate.
			break;
		case 0x02:
			*type = XMCMD_PORTADOWN;
			*param = pData->_parameter >> 1; // There isn't a correct conversion. It is non-linear, but acting on speed, not period, and based on sample rate.
			break;
		case 0x08:
			*type = XM_SAMPLER_CMD_PANNING;
			*param = pData->_parameter;
			break;
		case 0x09:
			*type = XM_SAMPLER_CMD_OFFSET;
			*param = pData->_parameter;  //The offset in sampler is of the whole sample size, not a fixed amount.
			break;
		case 0x0C:
			*vol = 0x10 + (pData->_parameter >> 2);
			break;
		case 0x0E: {
			switch (pData->_parameter & 0xF0) {
			case 0xC0:
				*type = XMCMD_E_DELAYED_NOTECUT;
				*param = (uint8_t)((pData->_parameter * self->song->properties.lpb) >> 2);
				break;
			case 0xD0:
				*type = XMCMD_E_NOTE_DELAY;
				*param = (uint8_t)((pData->_parameter * self->song->properties.lpb) >> 2);
				break;
			default:
				break;
			}
			break;
		}
		case 0x15:
			*type = XMCMD_RETRIG;
			*param = pData->_parameter;
			break;
		default:
			break;
		}
	} else if (pData->_cmd == 0x0C) {
		*vol = 0x10 + (pData->_parameter >> 2);
	}
}

int xmsongexport_saveinstruments(XMSongExport* self)
{
	int i;
	int j;
	int remaining;

	for (i = 0; i < MAX_BUSES; i++) {
		if (psy_audio_machines_at(&self->song->machines, i) != NULL && 
			psy_audio_machine_type(psy_audio_machines_at(&self->song->machines, i)) != MACH_SAMPLER &&
			psy_audio_machine_type(psy_audio_machines_at(&self->song->machines, i)) != MACH_XMSAMPLER) {
			xmsongexport_saveemptyinstrument(self,
				psy_audio_machine_editname(psy_audio_machines_at(&self->song->machines, i)));
		}
	}
	remaining = self->m_Header.instruments - self->macInstruments;
	for (j = 0, i = (self->correctionIndex == 0) ? 1 : 0; j < self->xmInstruments && j < remaining; j++, i++) {
		if (psy_audio_samples_at(&self->song->samples, psy_audio_sampleindex_make(i, 0))) {
			xmsongexport_savesampulseinstrument(self, i);
		} else {
			psy_audio_Instrument* inst;

			inst = psy_audio_instruments_at(&self->song->instruments, psy_audio_instrumentindex_make(1, i));
			xmsongexport_saveemptyinstrument(self, inst ? psy_audio_instrument_name(inst) : "");
		}
	}
	remaining = self->m_Header.instruments - self->macInstruments - self->xmInstruments;
	for (i = 0; i < remaining; i++) {
		if (psy_audio_samples_at(&self->song->samples, psy_audio_sampleindex_make(i, 0))) {
			xmsongexport_savesamplerinstrument(self, i);
		} else {
			psy_audio_Instrument* inst;

			inst = psy_audio_instruments_at(&self->song->instruments, psy_audio_instrumentindex_make(0, i));
			xmsongexport_saveemptyinstrument(self, inst ? psy_audio_instrument_name(inst) : "");
		}
	}
	return PSY_OK;
}

int xmsongexport_saveemptyinstrument(XMSongExport* self, const char* name)
{
	struct XMINSTRUMENTHEADER insHeader;
	int status;
	
	memset(&insHeader, 0, sizeof(insHeader));
	//insHeader.type = 0; Implicit by memset
	insHeader.size = sizeof(insHeader);
	strncpy(insHeader.name, name, 22); //Names are not null terminated
	//insHeader.samples = 0; Implicit by memset
	if (status = psyfile_write(self->fp, &insHeader, sizeof(insHeader))) {
		return status;
	}
	return PSY_OK;
}

int xmsongexport_savesampulseinstrument(XMSongExport* self, int instIdx)
{
	psy_audio_Instrument* inst;
	psy_audio_InstrumentIndex index;
	struct XMINSTRUMENTHEADER insHeader;
	struct XMSAMPLEHEADER samphead;
	
	index = psy_audio_instrumentindex_make(1, instIdx);
	inst = psy_audio_instruments_at(&self->song->instruments,
		index);
	if (!inst) {
		return PSY_ERRFILE;
	}		
	memset(&insHeader, 0, sizeof(insHeader));
	strncpy(insHeader.name, psy_audio_instrument_name(inst), 22); //Names are not null terminated
	//insHeader.type = 0; Implicit by memset	
	memset(&samphead, 0, sizeof(samphead));	
	
	//If no samples for this instrument, write it and exit.
	if (psy_audio_samples_size(&self->song->samples, instIdx)) {
		int status;

		insHeader.size = sizeof(insHeader);
		//insHeader.samples = 0; Implicit by memset
		if (status = psyfile_write(self->fp, &insHeader, sizeof(insHeader))) {
			return PSY_ERRFILE;
		}
	} else {
		int status;
		psy_TableIterator ite;

		insHeader.size = sizeof(insHeader) + sizeof(XMSAMPLEHEADER);
		insHeader.samples = (uint16_t)psy_audio_samples_size(&self->song->samples, instIdx);
		if (status = psyfile_write(self->fp, &insHeader, sizeof(insHeader))) {
			return PSY_ERRFILE;
		}
		xmsongexport_setsampulseenvelopes(self, instIdx, &samphead);
		samphead.volfade = (uint16_t)
			(floor(psy_audio_instrument_volumefadespeed(inst) * 32768.0f));
		samphead.shsize = sizeof(XMSAMPLESTRUCT);
		if (status = psyfile_write(self->fp, &samphead, sizeof(samphead))) {
			return PSY_ERRFILE;
		}
		for (ite = psy_audio_samples_groupbegin(&self->song->samples, instIdx);
				!psy_tableiterator_equal(&ite, psy_table_end());
				psy_tableiterator_inc(&ite)) {
			if (status = xmsongexport_savesampleheader(self, instIdx,
					(int)psy_tableiterator_key(&ite))) {
				return status;
			}
		}
		for (ite = psy_audio_samples_groupbegin(&self->song->samples, instIdx);
				!psy_tableiterator_equal(&ite, psy_table_end());
				psy_tableiterator_inc(&ite)) {
			if (status = xmsongexport_savesampledata(self, instIdx,
					(int)psy_tableiterator_key(&ite))) {
				return status;
			}
		}		
	}
	return PSY_OK;
}

int xmsongexport_savesamplerinstrument(XMSongExport* self, int instIdx)
{
	psy_audio_Sample* sample;
	struct XMINSTRUMENTHEADER insHeader;
	char temp[22];

	memset(&insHeader, 0, sizeof(insHeader));		
	memset(temp, 0, sizeof(temp));
	sample = psy_audio_samples_at(&self->song->samples, psy_audio_sampleindex_make(instIdx, 0));
	if (sample) {
		strncpy(temp, psy_audio_sample_name(sample), 22); //Names are not null terminated
	}	
	strncpy(insHeader.name, temp, 22); //Names are not null terminated
	//insHeader.type = 0; Implicit by memset
	//If it has samples, add the whole header.
	if (sample) {
		int status;
		XMSAMPLEHEADER _samph;

		insHeader.size = sizeof(insHeader) + sizeof(XMSAMPLEHEADER);
		insHeader.samples = 1;		
		if (status = psyfile_write(self->fp, &insHeader, sizeof(insHeader))) {
			return PSY_ERRFILE;
		}		
		memset(&_samph, 0, sizeof(_samph));
		xmsongexport_setsamplerenvelopes(self, instIdx, &_samph);
		_samph.volfade = 0x400;
		_samph.shsize = sizeof(XMSAMPLESTRUCT);
		if (status = psyfile_write(self->fp, &_samph, sizeof(_samph))) {
			return PSY_ERRFILE;
		}
		xmsongexport_savesampleheader(self, instIdx, 0);
		xmsongexport_savesampledata(self, instIdx, 0);
	} else {
		int status;

		insHeader.size = sizeof(insHeader);
		//insHeader.samples = 0; Implicit by memset
		if (status = psyfile_write(self->fp, &insHeader, sizeof(insHeader))) {
			return PSY_ERRFILE;
		}
	}
	return PSY_OK;
}

int xmsongexport_savesampleheader(XMSongExport* self, int instIdx, int sampleIdx)
{
	psy_audio_Sample* wave;
	psy_audio_SampleIndex index;
	XMSAMPLESTRUCT stheader;
	int tune;
	int finetune;
	uint8_t type;
	int status;

	index = psy_audio_sampleindex_make(instIdx, sampleIdx);
	wave = psy_audio_samples_at(&self->song->samples,
		index);
	memset(&stheader, 0, sizeof(stheader));
	strncpy(stheader.name, psy_audio_sample_name(wave), 22); //Names are not null terminated
	// stheader.res Implicitely set at zero by memset

	tune = wave->zone.tune;
	finetune = (int)((float)wave->zone.finetune * 1.28);
	if (wave->samplerate != 8363) {
		//correct the tuning
		double newtune = log10((double)(wave->samplerate) / 8363.0) / log10(2.0);
		double floortune = floor(newtune * 12.0);
		tune += (int)(floortune);
		finetune += (int)(floor(((newtune * 12.0) - floortune) * 128.0));
		if (finetune > 127) { tune++; finetune -= 127; }
	}

	//All samples are 16bits in Psycle.
	stheader.samplen = (uint32_t)(wave->numframes * 2);
	stheader.loopstart = (uint32_t)(wave->loop.start * 2);
	stheader.looplen = (uint32_t)((wave->loop.end - wave->loop.start) * 2);
	stheader.vol = (int)(wave->globalvolume * 255) >> 1;
	stheader.relnote = tune;
	stheader.finetune = finetune;

	type = 0;
	if (wave->loop.type == psy_audio_SAMPLE_LOOP_NORMAL) type = 1;
	else if (wave->loop.type == psy_audio_SAMPLE_LOOP_BIDI) type = 2;
	type |= 0x10; // 0x10 -> 16bits
	stheader.type = type;
	stheader.pan = (uint8_t)(wave->panfactor * 255.f);

	if (status = psyfile_write(self->fp, &stheader, sizeof(stheader))) {
		return PSY_ERRFILE;
	}
	return PSY_OK;
}

int xmsongexport_savesampledata(XMSongExport* self, int instrIdx, int sampleindex)
{
	psy_audio_Sample* wave;	
	psy_dsp_amp_t* samples;
	int32_t length;
	int16_t prev;
	int j;
	
	wave = psy_audio_samples_at(&self->song->samples,
		psy_audio_sampleindex_make(instrIdx, sampleindex));
	// pack sample data
	samples = wave->channels.samples[0];
	length = (int32_t)wave->numframes;
	prev = 0;
	for (j = 0; j < length; ++j) {
		int status;
		short delta = (int16_t)samples[j] - prev;

		//This is expected to be in little endian.
		if (status = psyfile_write(self->fp, &delta, sizeof(int16_t))) {
			return PSY_ERRFILE;
		}
		prev = (short)samples[j];
	}
	return PSY_OK;
}

void xmsongexport_setsampulseenvelopes(XMSongExport* self, int instrIdx,
	struct XMSAMPLEHEADER* sampleHeader)
{
	psy_audio_Instrument* inst;
	psy_audio_InstrumentIndex index;	

	index = psy_audio_instrumentindex_make(0, instrIdx);
	inst = psy_audio_instruments_at(&self->song->instruments,
		index);
	if (!inst) {
		return;
	}
	sampleHeader->vtype = 0;

	if (psy_dsp_envelope_isenabled(&inst->volumeenvelope)) {
		const psy_dsp_Envelope* env = &inst->volumeenvelope;
		float convert = 1.f;
		int idx;
		unsigned int i;

		sampleHeader->vtype = 1;
		// Max number of envelope points in Fasttracker format is 12.
		sampleHeader->vnum = (uint8_t)(psy_min(12u, psy_dsp_envelope_numofpoints(env)));
		if (psy_dsp_envelope_mode(env) == psy_dsp_ENVELOPETIME_SECONDS) {
			convert = (24.f * (float)(self->song->properties.bpm)) / 60000.f * 1000.f;
		}
		// Format of FastTracker points is :
		// Point : frame number. ( 1 frame= line*(24/TPB), samplepos= frame*(samplesperrow*TPB/24))
		// Value : 0..64. , divide by 64 to use it as a multiplier.
		idx = 0;
		for (i = 0; i < psy_dsp_envelope_numofpoints(env) && i < 12; i++) {
			sampleHeader->venv[idx] = (uint16_t)(psy_dsp_envelope_time(env, i) * convert); idx++;
			sampleHeader->venv[idx] = (uint16_t)(psy_dsp_envelope_value(env, i) * 64.f); idx++;
		}

		if (psy_dsp_envelope_sustainbegin(env) != psy_INDEX_INVALID) {
			sampleHeader->vtype |= 2;
			sampleHeader->vsustain = (uint8_t)(psy_dsp_envelope_sustainbegin(env));
		}
		if (psy_dsp_envelope_loopstart(env) != psy_INDEX_INVALID) {
			sampleHeader->vtype |= 4;
			sampleHeader->vloops = (uint8_t)(psy_dsp_envelope_loopstart(env));
			sampleHeader->vloope = (uint8_t)(psy_dsp_envelope_loopend(env));
		}
	}
	if (psy_dsp_envelope_isenabled(&inst->panenvelope)) {
		float convert;		
		const psy_dsp_Envelope* env = &inst->panenvelope;
		int idx;
		unsigned int i;

		sampleHeader->ptype = 1;
		// Max number of envelope points in Fasttracker format is 12.
		sampleHeader->pnum = (uint8_t)(psy_min(12u, psy_dsp_envelope_numofpoints(env)));
		convert = 1.f;
		if (psy_dsp_envelope_mode(env) == psy_dsp_ENVELOPETIME_SECONDS) {
			convert = (24.f * (float)(self->song->properties.bpm)) / 60000.f * 1000.f;
		}
		// Format of FastTracker points is :
		// Point : frame number. ( 1 frame= line*(24/TPB), samplepos= frame*(samplesperrow*TPB/24))
		// Value : 0..64. , divide by 64 to use it as a multiplier.
		idx = 0;
		for (i = 0; i < psy_dsp_envelope_numofpoints(env) && i < 12; i++) {
			sampleHeader->penv[idx] = (uint16_t)(psy_dsp_envelope_time(env, i) * convert); idx++;
			sampleHeader->penv[idx] = (uint16_t)(32 + (psy_dsp_envelope_value(env, i) * 32.f)); idx++;
		}

		if (psy_dsp_envelope_sustainbegin(env) != psy_INDEX_INVALID) {
			sampleHeader->ptype |= 2;
			sampleHeader->psustain = (uint8_t)(psy_dsp_envelope_sustainbegin(env));
		}
		if (psy_dsp_envelope_loopstart(env) != psy_INDEX_INVALID) {
			sampleHeader->ptype |= 4;
			sampleHeader->ploops = (uint8_t)(psy_dsp_envelope_loopstart(env));
			sampleHeader->ploope = (uint8_t)(psy_dsp_envelope_loopend(env));
		}
	}	
}

void xmsongexport_setsamplerenvelopes(XMSongExport* self, int instrIdx,
	struct XMSAMPLEHEADER* sampleHeader)
{
	psy_audio_Instrument* instrument;
	psy_audio_InstrumentIndex index;
	psy_audio_LegacyInstrument inst;

	index = psy_audio_instrumentindex_make(0, instrIdx);
	instrument = psy_audio_instruments_at(&self->song->instruments,
		index);
	if (!instrument) {
		return;
	}
	inst = psy_audio_legacyinstrument(instrument);
	sampleHeader->vtype = 0;

	if (inst.ENV_AT != 1 || inst.ENV_DT != 1 || inst.ENV_SL != 100 || inst.ENV_RT != 220)
	{
		float convert = 0.f;
		int32_t idx;

		sampleHeader->vtype = 3;
		sampleHeader->vsustain = 1;

		sampleHeader->vnum = 4;
		convert = (float)(self->song->properties.bpm) * 24.f / (44100.f * 60.f);
		// Format of FastTracker points is :
		// Point : frame number. ( 1 frame= line*(24/TPB), samplepos= frame*(samplesperrow*TPB/24))
		// Value : 0..64. , divide by 64 to use it as a multiplier.
		idx = 0;
		sampleHeader->venv[idx] = 0; idx++;
		sampleHeader->venv[idx] = 0; idx++;
		sampleHeader->venv[idx] = (uint16_t)(inst.ENV_AT * convert); idx++;
		sampleHeader->venv[idx] = 64; idx++;
		sampleHeader->venv[idx] = (uint16_t)((inst.ENV_DT + inst.ENV_DT) * convert); idx++;
		sampleHeader->venv[idx] = (uint16_t)(inst.ENV_SL * 0.64f); idx++;
		sampleHeader->venv[idx] = (uint16_t)((inst.ENV_RT + inst.ENV_DT + inst.ENV_RT) * convert); idx++;
		sampleHeader->venv[idx] = 0; idx++;
	}
}
