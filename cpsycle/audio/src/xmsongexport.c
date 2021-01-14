// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "xmsongexport.h"
#include "samplerdefs.h"
#include "song.h"
#include "songio.h"
#include "xmdefs.h"
#include <string.h>
#include <math.h>
#include "psy3saver.h"
#include "sampler.h"
// platform
#include "../../detail/portable.h"

// prototypes
static void xmsongexport_writesongheader(XMSongExport*, psy_audio_SongFile*);
static void xmsongexport_savepatterns(XMSongExport*, psy_audio_SongFile*);
static void xmsongexport_savesinglepattern(XMSongExport*, psy_audio_SongFile*,
	int patIdx);
static void xmsongexport_getcommand(XMSongExport*, psy_audio_SongFile*, int i,
	const psy_audio_LegacyPatternEntry*, unsigned char* vol, unsigned char* type,
	unsigned char* param);
static void xmsongexport_saveinstruments(XMSongExport*, psy_audio_SongFile*);
static void xmsongexport_saveemptyinstrument(XMSongExport*, psy_audio_SongFile*,
	const char* name);
static void xmsongexport_savesampulseinstrument(XMSongExport*, psy_audio_SongFile*,
	int instIdx);
static void xmsongexport_savesamplerinstrument(XMSongExport*, psy_audio_SongFile*,
	int instIdx);
static void xmsongexport_savesampleheader(XMSongExport*, psy_audio_SongFile*,
	int instIdx);
static void xmsongexport_savesampledata(XMSongExport*, psy_audio_SongFile*,
	int instrIdx);
static void xmsongexport_setsampulseenvelopes(XMSongExport*, psy_audio_SongFile*,
	int instrIdx, struct XMSAMPLEHEADER*);
static void xmsongexport_setsamplerenvelopes(XMSongExport*, psy_audio_SongFile*,
	int instrIdx, struct XMSAMPLEHEADER*);

//implementation
void xmsongexport_init(XMSongExport* self)
{
}

void xmsongexport_dispose(XMSongExport* self)
{
}

void xmsongexport_exportsong(XMSongExport* self, psy_audio_SongFile* songfile)
{
	xmsongexport_writesongheader(self, songfile);
	xmsongexport_savepatterns(self, songfile);
	xmsongexport_saveinstruments(self, songfile);
}

void xmsongexport_writesongheader(XMSongExport* self, psy_audio_SongFile* songfile)
{
	char modulename[20];
	uintptr_t i;
	psy_List* t;
	psy_audio_SequenceTrack* track;
	bool hasSampler;
	int samInstruments;
	uint16_t temp;

	self->macInstruments = 0;
	hasSampler = FALSE;
	for (i = 0; i < 256; i++) { self->isBlitzorVst[i] = FALSE; }
	for (i = 0; i < MAX_BUSES; i++) {
		if (psy_audio_machines_at(&songfile->song->machines, i)  != NULL) {
			self->isSampler[i] = hasSampler = (psy_audio_machine_type(psy_audio_machines_at(&songfile->song->machines, i)) == MACH_SAMPLER);
			self->isSampulse[i] = (psy_audio_machine_type(psy_audio_machines_at(&songfile->song->machines, i)) == MACH_XMSAMPLER);
			if (!(self->isSampler[i] || self->isSampulse[i])) {
				self->macInstruments++;
				if (psy_audio_machine_type(psy_audio_machines_at(&songfile->song->machines, i)) == MACH_PLUGIN) {
					const psy_audio_MachineInfo* info;

					info = psy_audio_machine_info(psy_audio_machines_at(&songfile->song->machines, i));
					if (info && info->Name) {
						const char* str = info->Name;
						if (strstr(str, "blitz")) {						
							self->isBlitzorVst[i + 1] = TRUE;
						}
					}
				} else if (psy_audio_machine_type(psy_audio_machines_at(&songfile->song->machines, i)) == MACH_VST) {
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
	self->xmInstruments = (int32_t)psy_audio_instruments_size(&songfile->song->instruments, 1); // song.GetHighestXMInstrumentIndex() + 1;
	if (self->xmInstruments > 1 && self->correctionIndex == 0) self->xmInstruments--;
	//If there is a sampler machine, we have to take the samples into account.
	samInstruments = (int32_t)psy_audio_instruments_size(&songfile->song->instruments, 0); //0; // (hasSampler) ? song.GetHighestInstrumentIndex() + 1 : 0;

	psyfile_write(songfile->file, XM_HEADER, 17);//ID text
	// Module name
	// modulename: PE + songtitle 0..17 (incl.) + spaces
	// no null termination
	memset(modulename, 0x20, 20);
	modulename[0] = 'P';
	modulename[1] = 'E';
	modulename[2] = ':';
	for (i = 3; i < 20 && i < strlen(psy_audio_song_title(songfile->song)); ++i) {
		modulename[i] = psy_audio_song_title(songfile->song)[i];
	}	
	psyfile_write(songfile->file, modulename, sizeof(modulename));
	// The hex  value  0x1A  in  a  normal  XM  file or 0x00  in  a  Stripped  on	
	temp = 0x1A;
	psyfile_write(songfile->file, &temp, 1);
	// Tracker name
	psyfile_write(songfile->file, "FastTracker v2.00   ", 20);
	// Version number	
	temp = 0x0104;
	psyfile_write(songfile->file, &temp, 2);

	memset(&self->m_Header, 0, sizeof(self->m_Header));
	self->m_Header.size = sizeof(self->m_Header);
	self->m_Header.norder = (uint16_t)psy_audio_sequence_maxtracksize(&songfile->song->sequence);
	self->m_Header.restartpos = 0;	
	self->m_Header.channels = (uint16_t)psy_min(psy_audio_song_numsongtracks(songfile->song), 32);
	// Number of patterns stored in file. There should be at least 1 pattern
	// if you expect to hear anything out of the speakers. The maximum value is
	// 256. Don’t confuse this with[Song length]!
	self->m_Header.patterns = (uint16_t)psy_min(psy_audio_patterns_size(&songfile->song->patterns), 256);
	self->m_Header.instruments = psy_min(128, self->macInstruments + self->xmInstruments + samInstruments);
	self->m_Header.flags = 0x0001; //Linear frequency.	
	self->m_Header.speed = (uint16_t)(floor(24.f / songfile->song->properties.lpb)) +
		(uint16_t)songfile->song->properties.extraticksperbeat;
	self->m_Header.tempo = (int)songfile->song->properties.bpm;
	// Pattern order table
	track = (psy_audio_SequenceTrack*)songfile->song->sequence.tracks->entry;
	for (t = track->entries; t != 0; t = t->next) {
		psy_audio_SequenceEntry* entry;

		entry = (psy_audio_SequenceEntry*)t->entry;
		self->m_Header.order[i] = (uint8_t)entry->patternslot;
	}	
	psyfile_write(songfile->file, &self->m_Header, sizeof(self->m_Header));
}

void xmsongexport_savepatterns(XMSongExport* self, psy_audio_SongFile* songfile)
{
	int i;

	for (i = 0; i < self->m_Header.patterns; ++i) {
		xmsongexport_savesinglepattern(self, songfile, i);
	}
}

// return address of next pattern, 0 for invalid
void xmsongexport_savesinglepattern(XMSongExport* self, psy_audio_SongFile* songfile, int patIdx)
{
	//Temp entries for volume on virtual generators.
	psy_audio_LegacyPatternEntry volumeEntries[32];
	struct XMPATTERNHEADER ptHeader;
	size_t currentpos;
	int32_t maxtracks;
	int32_t i;
	int32_t j;
	psy_audio_Pattern* pattern;
	int32_t lines;
	PsyFile* fp;

	fp = songfile->file;
	memset(&ptHeader, 0, sizeof(ptHeader));
	ptHeader.size = sizeof(ptHeader);
	//ptHeader.packingtype = 0; implicit from memset.
	pattern = psy_audio_patterns_at(&songfile->song->patterns, patIdx);
	lines = (uint16_t)(psy_audio_pattern_length(pattern) *
		songfile->song->properties.lpb);
	ptHeader.rows = psy_min(256, lines);
	//ptHeader.packedsize = 0; implicit from memset.

	psyfile_write(songfile->file, &ptHeader, sizeof(ptHeader));
	currentpos = psyfile_getpos(songfile->file);

	maxtracks = (int32_t)psy_min(psy_audio_song_numsongtracks(songfile->song), 32);
	// check every pattern for validity
	if (psy_audio_sequence_patternused(&songfile->song->sequence, patIdx))
	{
		// convert pattern to legacy pattern
		psy_audio_LegacyPattern ppattern;

		ppattern = psy_audio_allocoldpattern(pattern,
			psy_audio_song_lpb(songfile->song), &lines);
		for (i = 0; i < maxtracks; i++) {
			self->lastInstr[i] = -1;
		}
		self->addTicks = 0;
		for (j = 0; j < ptHeader.rows; j++) {
			for (i = 0; i < maxtracks; i++) {
				self->extraEntry[i] = NULL;
			}
			for (i = 0; i < (int32_t)psy_audio_song_numsongtracks(songfile->song); i++) {
				const psy_audio_LegacyPatternEntry* pData;

				pData = psy_audio_ptrackline_const(ppattern, i, j);
				if (pData->_note == psy_audio_NOTECOMMANDS_MIDICC) {
					if (pData->_inst < maxtracks) {
						self->extraEntry[pData->_inst] = pData;
					}
				} else if (pData->_note != psy_audio_NOTECOMMANDS_TWEAK && pData->_note != psy_audio_NOTECOMMANDS_TWEAKSLIDE) {
					if (pData->_cmd == psy_audio_PATTERNCMD_EXTENDED && (pData->_parameter & 0xF0) == psy_audio_PATTERNCMD_ROW_EXTRATICKS) {
						self->addTicks = pData->_parameter & 0x0F;
					}
				}
			}
			for (i = 0; i < maxtracks; i++) {
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
					psyfile_write(fp, &compressed, 1);
					continue;
				} else if (pData->_note == psy_audio_NOTECOMMANDS_MIDICC) {
					if (pData->_inst > 0x7F) {
						unsigned char compressed = 0x80 + 2 + 8 + 16;
						psyfile_write(fp, &compressed, 1);
						psyfile_write(fp, &pData->_inst, 1);
						unsigned char val = XMCMD_MIDI_MACRO;
						psyfile_write(fp, &val, 1);
						psyfile_write(fp, &pData->_cmd, 1);
					} else {
						unsigned char compressed = 0x80;
						psyfile_write(fp, &compressed, 1);
					}
					continue;
				}
				if (pData->_mach < MAX_BUSES) {
					mac = psy_audio_machines_at(&songfile->song->machines, pData->_mach);
					instrint = pData->_inst;
				} else if (pData->_mach >= MAX_MACHINES && pData->_mach < MAX_VIRTUALINSTS) {
					// todo virtual generators
					mac = psy_audio_machines_at(&songfile->song->machines, pData->_mach);
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

				xmsongexport_getcommand(self, songfile, i, pData, &vol, &type, &param);
				if (self->extraEntry[i] != NULL) {
					xmsongexport_getcommand(self, songfile, i,
						self->extraEntry[i], &vol, &type, &param);
				}

				bWriteNote = note != 0;
				bWriteInstr = instr != 0;
				bWriteVol = vol != 0;
				bWriteType = type != 0;
				bWriteParam = param != 0;

				compressed = 0x80 + bWriteNote + (bWriteInstr << 1) + (bWriteVol << 2)
					+ (bWriteType << 3) + (bWriteParam << 4);

				if (compressed != 0x9F) psyfile_write_int8(songfile->file, compressed); // 0x9F means to write everything.
				if (bWriteNote) psyfile_write(songfile->file, &note, 1);
				if (bWriteInstr) psyfile_write(songfile->file, &instr, 1);
				if (bWriteVol) psyfile_write(songfile->file, &vol, 1);
				if (bWriteType) psyfile_write(songfile->file, &type, 1);
				if (bWriteParam) psyfile_write(songfile->file, &param, 1);
			}
		}
		ptHeader.packedsize = (uint16_t)((psyfile_getpos(songfile->file) - currentpos) & 0xFFFF);
		psyfile_seek(songfile->file, (uint32_t)(currentpos - sizeof(ptHeader)));
		psyfile_write(songfile->file, &ptHeader, sizeof(ptHeader));
		psyfile_skip(songfile->file, ptHeader.packedsize);
		free(ppattern);
		ppattern = NULL;
	}	
	else {
		psyfile_write(fp, &ptHeader, sizeof(ptHeader));
	}
}

void xmsongexport_getcommand(XMSongExport* self, psy_audio_SongFile* songfile, int i, const psy_audio_LegacyPatternEntry* pData, unsigned char* vol, unsigned char* type, unsigned char* param)
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
			*param = XMCMD_E_NOTE_DELAY | ((uint8_t)(pData->_parameter * songfile->song->properties.tpb / 256));
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
				*param = (uint8_t)((pData->_parameter * songfile->song->properties.lpb) >> 2);
				break;
			case 0xD0:
				*type = XMCMD_E_NOTE_DELAY;
				*param = (uint8_t)((pData->_parameter * songfile->song->properties.lpb) >> 2);
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

void xmsongexport_saveinstruments(XMSongExport* self, psy_audio_SongFile* songfile)
{
	int i;
	int j;
	int remaining;

	for (i = 0; i < MAX_BUSES; i++) {
		if (psy_audio_machines_at(&songfile->song->machines, i) != NULL && 
			psy_audio_machine_type(psy_audio_machines_at(&songfile->song->machines, i)) != MACH_SAMPLER &&
			psy_audio_machine_type(psy_audio_machines_at(&songfile->song->machines, i)) != MACH_XMSAMPLER) {
			xmsongexport_saveemptyinstrument(self, songfile,
				psy_audio_machine_editname(psy_audio_machines_at(&songfile->song->machines, i)));
		}
	}
	remaining = self->m_Header.instruments - self->macInstruments;
	for (j = 0, i = (self->correctionIndex == 0) ? 1 : 0; j < self->xmInstruments && j < remaining; j++, i++) {
		if (psy_audio_samples_at(&songfile->song->samples, sampleindex_make(i, 0))) {
			xmsongexport_savesampulseinstrument(self, songfile, i);
		} else {
			psy_audio_Instrument* inst;

			inst = psy_audio_instruments_at(&songfile->song->instruments, psy_audio_instrumentindex_make(1, i));
			xmsongexport_saveemptyinstrument(self, songfile, inst ? psy_audio_instrument_name(inst) : "");
		}
	}
	remaining = self->m_Header.instruments - self->macInstruments - self->xmInstruments;
	for (i = 0; i < remaining; i++) {
		if (psy_audio_samples_at(&songfile->song->samples, sampleindex_make(i, 0))) {
			xmsongexport_savesamplerinstrument(self, songfile, i);
		} else {
			psy_audio_Instrument* inst;

			inst = psy_audio_instruments_at(&songfile->song->instruments, psy_audio_instrumentindex_make(0, i));
			xmsongexport_saveemptyinstrument(self, songfile, inst ? psy_audio_instrument_name(inst) : "");
		}
	}
}

void xmsongexport_saveemptyinstrument(XMSongExport* self, psy_audio_SongFile* songfile,
	const char* name)
{
	struct XMINSTRUMENTHEADER insHeader;
	
	memset(&insHeader, 0, sizeof(insHeader));
	//insHeader.type = 0; Implicit by memset
	insHeader.size = sizeof(insHeader);
	strncpy(insHeader.name, name, 22); //Names are not null terminated
	//insHeader.samples = 0; Implicit by memset
	psyfile_write(songfile->file, &insHeader, sizeof(insHeader));
}

void xmsongexport_savesampulseinstrument(XMSongExport* self, psy_audio_SongFile* songfile,
	int instIdx)
{
	psy_audio_Instrument* inst;
	psy_audio_InstrumentIndex index;
	PsyFile* fp;

	fp = songfile->file;
	index = psy_audio_instrumentindex_make(1, instIdx);
	inst = psy_audio_instruments_at(&songfile->song->instruments,
		index);
	if (!inst) {
		return;
	}	
	struct XMINSTRUMENTHEADER insHeader;
	memset(&insHeader, 0, sizeof(insHeader));
	strncpy(insHeader.name, psy_audio_instrument_name(inst), 22); //Names are not null terminated
	//insHeader.type = 0; Implicit by memset
	struct XMSAMPLEHEADER samphead;
	memset(&samphead, 0, sizeof(samphead));

	{
		/*psy_Table notemap;
		psy_List* p;
		static const int NOTE_MAP_SIZE = 120;
		uint8_t i;
		psy_List* sampidxs;
		psy_Table sRemap; // std::map<unsigned char, unsigned char> sRemap;
		unsigned char sample;
		int i;
				
		psy_table_init(&notemap);
		for (p = inst->entries; p != NULL; psy_list_next(&p)) {
			psy_audio_InstrumentEntry* entry;
			uint8_t key;

			entry = (psy_audio_InstrumentEntry*)psy_list_entry(p);
			for (key = entry->keyrange.low; key <= entry->keyrange.high; ++key) {
				psy_table_insert(&notemap, key, (void*)(uintptr_t)entry->sampleindex.slot);
			}
		}
		sampidxs = NULL;
		psy_table_init(&sRemap);
		sample = 0;
		unsigned char sample = 0;
		for (int i = 12; i < 108; i++) {
			const XMInstrument::NotePair& pair = inst.NoteToSample(i);
			uintptr_t sampleidx;
			if (psy_table_exists(&notemap, i)) {
				sampleidx = psy_table_at(&notemap, i);
				if (sampleidx != psy_INDEX_INVALID) {
					if (!psy_table_exists(&sRemap, sampleidx)) {
						psy_list_append(&sampidxs, (uintptr_t)sampleidx);
						psy_table_insert(&sRemap, (uintptr_t)sampleidx, sample);					
						samphead.snum[i - 12] = sample;
						sample++;
					} else {
						samphead.snum[i - 12] = sampleidx;
					}
			}
		}		
		psy_table_dispose(&notemap);
		psy_table_dispose(&sRemap);
		psy_list_free(&sampidxs);		

		//If no samples for this instrument, write it and exit.
		if (sampidxs.size() == 0) {*/
		insHeader.size = sizeof(insHeader);
		//insHeader.samples = 0; Implicit by memset
		psyfile_write(fp, &insHeader, sizeof(insHeader));
	/*} else {
		insHeader.size = sizeof(insHeader) + sizeof(XMSAMPLEHEADER);
		insHeader.samples = static_cast<uint16_t>(sampidxs.size());
		psyfile_write(fp, &insHeader, sizeof(insHeader));

		SetSampulseEnvelopes(song, instIdx, samphead);
		samphead.volfade = static_cast<uint16_t>(floor(inst.VolumeFadeSpeed() * 32768.0f));
		samphead.shsize = sizeof(XMSAMPLESTRUCT);
		psyfile_write(fp, &samphead, sizeof(samphead));

		for (std::list<unsigned char>::const_iterator ite = sampidxs.begin(); ite != sampidxs.end(); ++ite) {
			xmsongexport_savesampleheader(self, song, static_cast<int>(*ite));
		}
		for (std::list<unsigned char>::const_iterator ite = sampidxs.begin(); ite != sampidxs.end(); ++ite) {
			xmsongexport_savesampledata(self, song, static_cast<int>(*ite));
		}*/
	}
}

void xmsongexport_savesamplerinstrument(XMSongExport* self, psy_audio_SongFile* songfile, int instIdx)
{
	struct XMINSTRUMENTHEADER insHeader;
	memset(&insHeader, 0, sizeof(insHeader));
	psy_audio_Sample* sample;
	char temp[22];
	PsyFile* fp;

	fp = songfile->file;
	memset(temp, 0, sizeof(temp));
	sample = psy_audio_samples_at(&songfile->song->samples, sampleindex_make(instIdx, 0));
	if (sample) {
		strncpy(temp, psy_audio_sample_name(sample), 22); //Names are not null terminated
	}	
	strncpy(insHeader.name, temp, 22); //Names are not null terminated
	//insHeader.type = 0; Implicit by memset
	//If it has samples, add the whole header.
	if (sample) {		
		insHeader.size = sizeof(insHeader) + sizeof(XMSAMPLEHEADER);
		insHeader.samples = 1;
		psyfile_write(fp, &insHeader, sizeof(insHeader));

		XMSAMPLEHEADER _samph;
		memset(&_samph, 0, sizeof(_samph));
		xmsongexport_setsamplerenvelopes(self, songfile, instIdx, &_samph);
		_samph.volfade = 0x400;
		_samph.shsize = sizeof(XMSAMPLESTRUCT);
		psyfile_write(fp, &_samph, sizeof(_samph));

		xmsongexport_savesampleheader(self, songfile, instIdx);
		xmsongexport_savesampledata(self, songfile, instIdx);
	} else {
		insHeader.size = sizeof(insHeader);
		//insHeader.samples = 0; Implicit by memset
		psyfile_write(fp, &insHeader, sizeof(insHeader));
	}
}

void xmsongexport_savesampleheader(XMSongExport* self, psy_audio_SongFile* songfile, int instIdx)
{
	psy_audio_Sample* wave;
	psy_audio_SampleIndex index;
	XMSAMPLESTRUCT stheader;
	int tune;
	int finetune;
	uint8_t type;

	index = sampleindex_make(instIdx, 0);
	wave = psy_audio_samples_at(&songfile->song->samples,
		index);
	memset(&stheader, 0, sizeof(stheader));
	strncpy(stheader.name, psy_audio_sample_name(wave), 22); //Names are not null terminated
	// stheader.res Implicitely set at zero by memset

	tune = wave->tune;
	finetune = (int)((float)wave->finetune * 1.28);
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

	psyfile_write(songfile->file, &stheader, sizeof(stheader));
}

void xmsongexport_savesampledata(XMSongExport* self, psy_audio_SongFile* songfile,
	int instrIdx)
{
	psy_audio_Sample* wave;
	psy_audio_SampleIndex index;
	psy_dsp_amp_t* samples;
	int32_t length;
	int16_t prev;
	int j;

	index = sampleindex_make(instrIdx, 0);
	wave = psy_audio_samples_at(&songfile->song->samples,
		index);
	// pack sample data
	samples = wave->channels.samples[0];
	length = (int32_t)wave->numframes;
	prev = 0;
	for (j = 0; j < length; ++j) {
		short delta = (short)samples[j] - prev;
		//This is expected to be in little endian.
		psyfile_write(songfile->file, &delta, sizeof(short));
		prev = (short)samples[j];
	}
}

void xmsongexport_setsampulseenvelopes(XMSongExport* self, psy_audio_SongFile* songfile,
	int instrIdx, struct XMSAMPLEHEADER* sampleHeader)
{
	psy_audio_Instrument* inst;
	psy_audio_InstrumentIndex index;	

	index = psy_audio_instrumentindex_make(0, instrIdx);
	inst = psy_audio_instruments_at(&songfile->song->instruments,
		index);
	if (!inst) {
		return;
	}
	sampleHeader->vtype = 0;

	if (psy_dsp_envelope_isenabled(&inst->volumeenvelope)) {		
		sampleHeader->vtype = 1;
		const psy_dsp_Envelope* env = &inst->volumeenvelope;
		float convert = 1.f;

		// Max number of envelope points in Fasttracker format is 12.
		sampleHeader->vnum = (uint8_t)(psy_min(12u, psy_dsp_envelope_numofpoints(env)));
		if (psy_dsp_envelope_mode(env) == psy_dsp_ENVELOPETIME_SECONDS) {
			convert = (24.f * (float)(songfile->song->properties.bpm)) / 60000.f * 1000.f;
		}
		// Format of FastTracker points is :
		// Point : frame number. ( 1 frame= line*(24/TPB), samplepos= frame*(samplesperrow*TPB/24))
		// Value : 0..64. , divide by 64 to use it as a multiplier.
		int idx = 0;
		for (unsigned int i = 0; i < psy_dsp_envelope_numofpoints(env) && i < 12; i++) {
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
		sampleHeader->ptype = 1;
		const psy_dsp_Envelope* env = &inst->panenvelope;		

		// Max number of envelope points in Fasttracker format is 12.
		sampleHeader->pnum = (uint8_t)(psy_min(12u, psy_dsp_envelope_numofpoints(env)));
		float convert = 1.f;
		if (psy_dsp_envelope_mode(env) == psy_dsp_ENVELOPETIME_SECONDS) {
			convert = (24.f * (float)(songfile->song->properties.bpm)) / 60000.f * 1000.f;
		}
		// Format of FastTracker points is :
		// Point : frame number. ( 1 frame= line*(24/TPB), samplepos= frame*(samplesperrow*TPB/24))
		// Value : 0..64. , divide by 64 to use it as a multiplier.
		int idx = 0;
		for (unsigned int i = 0; i < psy_dsp_envelope_numofpoints(env) && i < 12; i++) {
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

void xmsongexport_setsamplerenvelopes(XMSongExport* self, psy_audio_SongFile* songfile,
	int instrIdx, struct XMSAMPLEHEADER* sampleHeader)
{
	psy_audio_Instrument* instrument;
	psy_audio_InstrumentIndex index;
	psy_audio_LegacyInstrument inst;

	index = psy_audio_instrumentindex_make(0, instrIdx);
	instrument = psy_audio_instruments_at(&songfile->song->instruments,
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
		convert = (float)(songfile->song->properties.bpm) * 24.f / (44100.f * 60.f);
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
