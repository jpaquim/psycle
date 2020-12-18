// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "xmsongexport.h"
#include "samplerdefs.h"
#include "xmdefs.h"
#include <string.h>
#include <math.h>

#include "../../detail/portable.h"

// prototypes
static void xmsongexport_writesongheader(XMSongExport*, psy_audio_SongFile*);
static void xmsongexport_savepatterns(XMSongExport*, psy_audio_SongFile*);
static void xmsongexport_savesinglepattern(XMSongExport*, psy_audio_SongFile*, int patIdx);
static void xmsongexport_getcommand(XMSongExport*, psy_audio_SongFile*, int i,
	const psy_audio_PatternEvent*, unsigned char* vol, unsigned char* type,
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

	self->macInstruments = 0;
	bool hasSampler = FALSE;
	for (int i = 0; i < 256; i++) { self->isBlitzorVst[i] = FALSE; }
	for (int i = 0; i < MAX_BUSES; i++) {
		if (psy_audio_machines_at(&songfile->song->machines, i)  != NULL) {
			self->isSampler[i] = hasSampler = (psy_audio_machine_type(psy_audio_machines_at(&songfile->song->machines, i)) == MACH_SAMPLER);
			self->isSampulse[i] = (psy_audio_machine_type(psy_audio_machines_at(&songfile->song->machines, i)) == MACH_XMSAMPLER);
			if (!(self->isSampler[i] || self->isSampulse[i])) {
				self->macInstruments++;
				if (psy_audio_machine_type(psy_audio_machines_at(&songfile->song->machines, i)) == MACH_PLUGIN) {
					//const char* str = reinterpret_cast<Plugin*>(song._pMachine[i])->GetDllName();
					//if (str.find("blitz") != std::string::npos) {
					//	isBlitzorVst[i + 1] = true;
					//}
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
	self->xmInstruments = psy_audio_instruments_size(&songfile->song->instruments, 1); // song.GetHighestXMInstrumentIndex() + 1;
	if (self->xmInstruments > 1 && self->correctionIndex == 0) self->xmInstruments--;
	//If there is a sampler machine, we have to take the samples into account.
	int samInstruments = psy_audio_instruments_size(&songfile->song->instruments, 0); //0; // (hasSampler) ? song.GetHighestInstrumentIndex() + 1 : 0;

	psyfile_write(songfile->file, XM_HEADER, 17);//ID text
	// Module name
	// modulename: PE + songtitle 0..17 (incl.) + spaces
	// no null termination
	memset(modulename, 0x32, 20);
	modulename[0] = 'P';
	modulename[1] = 'E';
	for (i = 0; i < 18 && i < strlen(psy_audio_song_title(songfile->song)); ++i) {
		modulename[i + 2] = psy_audio_song_title(songfile->song)[i];
	}	
	psyfile_write(songfile->file, modulename, sizeof(modulename));
	// The hex  value  0x1A  in  a  normal  XM  file or 0x00  in  a  Stripped  on
	uint16_t temp = 0x1A;
	psyfile_write_uint16(songfile->file, temp);
	// Tracker name
	psyfile_write(songfile->file, "FastTracker v2.00   ", 20);
	// Version number	
	temp = 0x0104;
	psyfile_write_uint32(songfile->file, temp);

	memset(&self->m_Header, 0, sizeof(self->m_Header));
	self->m_Header.size = sizeof(self->m_Header);
	self->m_Header.norder = psy_audio_sequence_maxtracksize(&songfile->song->sequence);
	self->m_Header.restartpos = 0;	
	self->m_Header.channels = psy_min(songfile->song->properties.tracks, 32);
	// Number of patterns stored in file. There should be at least 1 pattern
	// if you expect to hear anything out of the speakers. The maximum value is
	// 256. Don’t confuse this with[Song length]!
	self->m_Header.patterns = psy_min(psy_audio_patterns_size(&songfile->song->patterns), 256);
	self->m_Header.instruments = psy_min(128, self->macInstruments + self->xmInstruments + samInstruments);
	self->m_Header.flags = 0x0001; //Linear frequency.	
	self->m_Header.speed = (int)floor(24.f / songfile->song->properties.lpb) + songfile->song->properties.extraticksperbeat;
	self->m_Header.tempo = (int)songfile->song->properties.bpm;
	// Pattern order table
	track = (psy_audio_SequenceTrack*)songfile->song->sequence.tracks->entry;
	for (t = track->entries; t != 0; t = t->next) {
		psy_audio_SequenceEntry* entry;

		entry = (psy_audio_SequenceEntry*)t->entry;
		self->m_Header.order[i] = entry->patternslot;
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
	// psy_audio_PatternEvent volumeEntries[32];

	struct XMPATTERNHEADER ptHeader;
	memset(&ptHeader, 0, sizeof(ptHeader));
	ptHeader.size = sizeof(ptHeader);
	//ptHeader.packingtype = 0; implicit from memset.
	//// ptHeader.rows = min(256, song.patternLines[patIdx]);
	//ptHeader.packedsize = 0; implicit from memset.

	psyfile_write(songfile->file, &ptHeader, sizeof(ptHeader));
	size_t currentpos = psyfile_getpos(songfile->file);

	int maxtracks = 0; // min(song.SONGTRACKS, 32);
	// check every pattern for validity
	//if (song.IsPatternUsed(patIdx))
	{
		for (int i = 0; i < maxtracks; i++) {
			self->lastInstr[i] = -1;
		}
		self->addTicks = 0;
		for (int j = 0; j < ptHeader.rows; j++) {
			for (int i = 0; i < maxtracks; i++) {
				//// self->extraEntry[i] = NULL;
			}
			for (int i = 0; i < songfile->song->properties.tracks; i++) {
				/*const PatternEntry* pData = reinterpret_cast<const PatternEntry*>(song._ptrackline(patIdx, i, j));
				if (pData->note == psy_audio_NOTECOMMANDS_MIDICC) {
					if (pData->inst < maxtracks) {
						extraEntry[pData->_inst] = pData;
					}
				} else if (pData->note != notecommands::tweak && pData->_note != notecommands::tweakslide) {
					if (pData->cmd == psy_audio_PatternCmd::psy_audio_PATTERNCMD_EXTENDED && (pData->_parameter & 0xF0) == psy_audio_PatternCmd::psy_audio_PATTERNCMD_ROW_EXTRATICKS) {
						addTicks = pData->_parameter & 0x0F;
					}
				}*/
			}
			for (int i = 0; i < maxtracks; i++) {

				/*const PatternEntry* pData = reinterpret_cast<const PatternEntry*>(song._ptrackline(patIdx, i, j));
				if (pData->_note == notecommands::tweak || pData->_note == notecommands::tweakslide) {
					unsigned char compressed = 0x80;
					Write(&compressed, 1);
					continue;
				} else if (pData->_note == notecommands::midicc) {
					if (pData->_inst > 0x7F) {
						unsigned char compressed = 0x80 + 2 + 8 + 16;
						Write(&compressed, 1);
						Write(&pData->_inst, 1);
						unsigned char val = XMCMD::MIDI_MACRO;
						Write(&val, 1);
						Write(&pData->_cmd, 1);
					} else {
						unsigned char compressed = 0x80;
						Write(&compressed, 1);
					}
					continue;
				}
				*/
				psy_audio_Machine* mac = NULL;
				unsigned char instr = 0;
				int instrint = 0xFF;
				/*
				if (pData->_mach < MAX_BUSES) {
					mac = song._pMachine[pData->_mach];
					instrint = pData->_inst;
				} else if (pData->_mach >= MAX_MACHINES && pData->_mach < MAX_VIRTUALINSTS) {
					mac = song.GetMachineOfBus(pData->_mach, instrint);
					if (instrint == -1) instrint = 0xFF;
					if (mac != NULL && pData->_inst != 255) {
						volumeEntries[i]._cmd = (mac->_type == MACH_SAMPLER) ? SAMPLER_CMD_VOLUME : XM_SAMPLER::CMD::SENDTOVOLUME;
						volumeEntries[i]._parameter = pData->_inst;
						extraEntry[i] = &volumeEntries[i];
					}
				}
				if (mac != NULL) {
					if (mac->_type == MACH_SAMPLER) {
						if (instrint != 0xFF) instr = static_cast<unsigned char>(macInstruments + xmInstruments + instrint + 1);
					} else if (mac->_type == MACH_XM_SAMPLER) {
						if (instrint != 0xFF &&
							(pData->_note != notecommands::empty || pData->_mach < MAX_BUSES)) {
							instr = static_cast<unsigned char>(macInstruments + instrint + correctionIndex);
						}
					} else {
						instr = static_cast<unsigned char>(pData->_mach + 1);
					}
					if (instr != 0) lastInstr[i] = instr;
				}
				*/
				unsigned char note;
				/*if (pData->_note >= 12 && pData->_note < 108) {
					if (mac != NULL && mac->_type == MACH_SAMPLER && ((Sampler*)mac)->isDefaultC4() == false)
					{
						note = pData->_note + 1;
					} else {
						note = pData->_note - 11;
					}
				} else if (pData->_note == notecommands::release) {
					note = 0x61;
				} else {*/
					note = 0x00;
				//}

				unsigned char vol = 0;
				unsigned char type = 0;
				unsigned char param = 0;

				// xmsongexport_getcommand(self, songfile, i, pData, vol, type, param);
				//if (extraEntry[i] != NULL) GetCommand(song, i, extraEntry[i], vol, type, param);

				unsigned char bWriteNote = note != 0;
				unsigned char bWriteInstr = instr != 0;
				unsigned char bWriteVol = vol != 0;
				unsigned char bWriteType = type != 0;
				unsigned char bWriteParam = param != 0;

				char compressed = 0x80 + bWriteNote + (bWriteInstr << 1) + (bWriteVol << 2)
					+ (bWriteType << 3) + (bWriteParam << 4);

				if (compressed != 0x9F) psyfile_write_int8(songfile->file, compressed); // 0x9F means to write everything.
				if (bWriteNote) psyfile_write_uint8(songfile->file, note);
				if (bWriteInstr) psyfile_write_uint8(songfile->file, instr);
				if (bWriteVol) psyfile_write_uint8(songfile->file, vol);
				if (bWriteType) psyfile_write_uint8(songfile->file, type);
				if (bWriteParam) psyfile_write_uint8(songfile->file, param);
			}
		}
		ptHeader.packedsize = (uint16_t)((psyfile_getpos(songfile->file) - currentpos) & 0xFFFF);
		psyfile_seek(songfile->file, currentpos - sizeof(ptHeader));
		psyfile_write(songfile->file, &ptHeader, sizeof(ptHeader));
		psyfile_skip(songfile->file, ptHeader.packedsize);
	} /*else {
		Write(&ptHeader, sizeof(ptHeader));
	}*/
}

void xmsongexport_getcommand(XMSongExport* self, psy_audio_SongFile* songfile, int i, const psy_audio_PatternEvent* pData, unsigned char* vol, unsigned char* type, unsigned char* param)
{	
	int singleEffectCharacter = (pData->cmd & 0xF0);
	if (singleEffectCharacter == 0xF0) { //Global commands
		switch (pData->cmd) {
		case psy_audio_PATTERNCMD_SET_TEMPO:
			if (pData->parameter >= 0x20) {
				*type = XMCMD_SETSPEED;
				*param = pData->parameter;
			}
			break;
		case psy_audio_PATTERNCMD_EXTENDED:
			switch (pData->parameter & 0xF0) {
			case psy_audio_PATTERNCMD_SET_LINESPERBEAT0:
			case psy_audio_PATTERNCMD_SET_LINESPERBEAT1:
				*type = XMCMD_SETSPEED;
				*param = floor(24.f / pData->parameter) + self->addTicks;
				self->addTicks = 0;
				break;
			case psy_audio_PATTERNCMD_PATTERN_LOOP:
				*type = XMCMD_EXTENDED;
				*param = XMCMD_E_PATTERN_LOOP + (pData->parameter & 0x0F);
				break;
			case psy_audio_PATTERNCMD_PATTERN_DELAY:
				*type = XMCMD_EXTENDED;
				*param = XMCMD_E_PATTERN_DELAY + (pData->parameter & 0x0F);
				break;
			default:
				break;
			}
			break;
		case psy_audio_PATTERNCMD_JUMP_TO_ORDER:
			*type = XMCMD_POSITION_JUMP;
			*param = pData->parameter;
			break;
		case psy_audio_PATTERNCMD_BREAK_TO_LINE:
			*type = XMCMD_PATTERN_BREAK;
			*param = ((pData->parameter / 10) << 4) + (pData->parameter % 10);
			break;
		case psy_audio_PATTERNCMD_SET_VOLUME:
			if (pData->inst == 255) {
				*type = XMCMD_SET_GLOBAL_VOLUME;
				*param = pData->parameter >> 1;
			}
			break;
		case psy_audio_PATTERNCMD_NOTE_DELAY:
			*type = XMCMD_EXTENDED;
			*param = XMCMD_E_NOTE_DELAY | (pData->parameter * songfile->song->properties.tpb / 256);
			break;
		case psy_audio_PATTERNCMD_ARPEGGIO:
			*type = XMCMD_ARPEGGIO;
			*param = pData->parameter;
			break;
		default:
			break;
		}
	} else if (self->lastInstr[i] != -1 && self->isBlitzorVst[self->lastInstr[i]]) {
		if (singleEffectCharacter == 0xE0) { //Blitz and VST "special" slide up
			//int toneDest = (pData->_cmd & 0x0F); This has no mapping to module commands
			*type = XMCMD_PORTAUP;
			*param = pData->parameter;
		} else if (singleEffectCharacter == 0xD0) { //Blitz and VST "special" slide down
			//int toneDest = (pData->_cmd & 0x0F); This has no mapping to module commands
			*type = XMCMD_PORTADOWN;
			*param = pData->parameter;
		} else if (singleEffectCharacter == 0xC0) { //Blitz
			switch (pData->cmd) {
			case 0xC1: // Blitz slide up
				*type = XMCMD_PORTAUP;
				*param = pData->parameter;
				break;
			case 0xC2: // Blitz slide donw
				*type = XMCMD_PORTADOWN;
				*param = pData->parameter;
				break;
			case 0xC3: // blitz tone portamento and VST porta
				*type = XMCMD_PORTA2NOTE;
				*param = pData->parameter;
				break;
			case 0xCC: // blitz old volume
				*vol = 0x10 + (pData->parameter >> 2);
				break;
			default:
				break;
			}
		} else if (pData->cmd == 0x0C) {
			*vol = 0x10 + (pData->parameter >> 2);
		} else if (pData->cmd > 0) {
			*type = XMCMD_ARPEGGIO;
			*param = pData->cmd;
		}
	} else if (self->lastInstr[i] != -1 && self->lastInstr[i] > self->macInstruments && 
		self->lastInstr[i] <= self->macInstruments + self->xmInstruments)
	{ //Sampulse
		switch (pData->cmd) {
		case XM_SAMPLER_CMD_ARPEGGIO:
			*type = XMCMD_ARPEGGIO;
			*param = pData->parameter;
			break;
		case XM_SAMPLER_CMD_PORTAMENTO_UP:
			if (pData->parameter < 0xF0) {
				*type = XMCMD_PORTAUP;
				*param = pData->parameter;
			} else {
				*type = XMCMD_EXTENDED;
				*param = XMCMD_E_FINE_PORTA_UP | (pData->parameter & 0xF);
			}
			break;
		case XM_SAMPLER_CMD_PORTAMENTO_DOWN:
			if (pData->parameter < 0xF0) {
				*type = XMCMD_PORTADOWN;
				*param = pData->parameter;
			} else {
				*type = XMCMD_EXTENDED;
				*param = XMCMD_E_FINE_PORTA_DOWN | (pData->parameter & 0xF);
			}
			break;
		case XM_SAMPLER_CMD_PORTA2NOTE:
			*type = XMCMD_PORTA2NOTE;
			*param = pData->parameter;
			break;
		case XM_SAMPLER_CMD_VIBRATO:
			*type = XMCMD_VIBRATO;
			*param = pData->parameter;
			break;
		case XM_SAMPLER_CMD_TONEPORTAVOL:
			*type = XMCMD_ARPEGGIO;
			*param = pData->parameter;
			break;
		case XM_SAMPLER_CMD_VIBRATOVOL:
			*type = XMCMD_VIBRATOVOL;
			*param = pData->parameter;
			break;
		case XM_SAMPLER_CMD_TREMOLO:
			*type = XMCMD_TREMOLO;
			*param = pData->parameter;
			break;
		case XM_SAMPLER_CMD_PANNING:
			*type = XMCMD_PANNING;
			*param = pData->parameter;
			break;
		case XM_SAMPLER_CMD_OFFSET:
			*type = XMCMD_OFFSET;
			*param = pData->parameter;
			break;
		case XM_SAMPLER_CMD_VOLUMESLIDE:
			if ((pData->parameter & 0xF) == 0xF) {
				*type = XMCMD_EXTENDED;
				*param = XMCMD_E_FINE_VOLUME_UP | ((pData->parameter & 0xF0) >> 4);
			} else if ((pData->parameter & 0xF0) == 0xF0) {
				*type = XMCMD_EXTENDED;
				*param = XMCMD_E_FINE_VOLUME_DOWN | (pData->parameter & 0xF);
			} else {
				*type = XMCMD_VOLUMESLIDE;
				*param = pData->parameter;
			}
			break;
		case XM_SAMPLER_CMD_VOLUME:
			*vol = 0x10 + ((pData->parameter <= 0x80) ? (pData->parameter >> 1) : 0x40);
			break;
		case XM_SAMPLER_CMD_EXTENDED:
			switch (pData->parameter & 0xF0) {
			case XM_SAMPLER_CMD_E_GLISSANDO_TYPE:
				*type = XMCMD_EXTENDED;
				*param = XMCMD_E_GLISSANDO_STATUS | ((pData->parameter == 0) ? 0 : 1);
				break;
			case XM_SAMPLER_CMD_E_DELAYED_NOTECUT:
				*type = XMCMD_EXTENDED;
				*param = XMCMD_E_DELAYED_NOTECUT | (pData->parameter & 0xF);
				break;
			case XM_SAMPLER_CMD_E_NOTE_DELAY:
				*type = XMCMD_EXTENDED;
				*param = XMCMD_E_NOTE_DELAY | (pData->parameter & 0xf);
				break;
			default:
				break;
			}
			break;
		case XM_SAMPLER_CMD_RETRIG:
			*type = XMCMD_EXTENDED;
			*param = XMCMD_E_MOD_RETRIG | (pData->parameter & 0xF);
			break;
		case XM_SAMPLER_CMD_SENDTOVOLUME:
			/*if (pData->parameter < 0x40) {
				vol = 0x10 + ((pData->parameter < 0x40) ? pData->parameter : 0x40);
			} else if ((pData->parameter & 0xF0) == XM_SAMPLER_CMD_VOL_VOLSLIDEUP) {
				vol = XMVOL_CMD_XMV_VOLUMESLIDEUP | (pData->parameter & 0x0F);
			} else if ((pData->parameter & 0xF0) == XM_SAMPLER_CMD_VOL_VOL_VOLSLIDEDOWN) {
				vol = XMVOL_CMD_XMV_VOLUMESLIDEDOWN | (pData->parameter & 0x0F);
			} else if ((pData->_parameter & 0xF0) == XM_SAMPLER_CMD_VOL_VOL_FINEVOLSLIDEUP) {
				vol = XMVOL_CMD_XMV_FINEVOLUMESLIDEUP | (pData->parameter & 0x0F);
			} else if ((pData->parameter & 0xF0) == XM_SAMPLER_CMD_VOL_VOL_FINEVOLSLIDEDOWN) {
				vol = XMVOL_CMD_XMV_FINEVOLUMESLIDEDOWN | (pData->parameter & 0x0F);
			} else if ((pData->parameter & 0xF0) == XM_SAMPLER_CMD_VOL_VOL_PANNING) {
				vol = XMVOL_CMD_XMV_PANNING | (pData->parameter & 0x0F);
			} else if ((pData->parameter & 0xF0) == XM_SAMPLER_CMD_VOL_VOL_PANSLIDELEFT) {
				vol = XMVOL_CMD_XMV_PANNINGSLIDELEFT | (((pData->parameter & 0x0F) > 3) ? 0xF : (pData->parameter & 0x0F) << 2);
			} else if ((pData->parameter & 0xF0) == XM_SAMPLER_CMD_VOL_VOL_PANSLIDERIGHT) {
				vol = XMVOL_CMD_XMV_PANNINGSLIDERIGHT | (((pData->parameter & 0x0F) > 3) ? 0xF : (pData->parameter & 0x0F) << 2);
			} else if ((pData->parameter & 0xF0) == XM_SAMPLER_CMD_VOL_VOL_VIBRATO) {
				vol = XMVOL_CMD_XMV_VIBRATO | (pData->parameter & 0x0F);
			} else if ((pData->parameter & 0xF0) == XM_SAMPLER_CMD_VOL_VOL_TONEPORTAMENTO) {
				vol = XMVOL_CMD_XMV_PORTA2NOTE | (pData->parameter & 0x0F);
			}*/
			break;
		default:
			break;
		}
	} else if (self->lastInstr[i] != -1 && self->lastInstr[i] > self->macInstruments + self->xmInstruments)
	{ // Sampler
		switch (pData->cmd) {
		case 0x01:
			*type = XMCMD_PORTAUP;
			*param = pData->parameter >> 1; // There isn't a correct conversion. It is non-linear, but acting on speed, not period, and based on sample rate.
			break;
		case 0x02:
			*type = XMCMD_PORTADOWN;
			*param = pData->parameter >> 1; // There isn't a correct conversion. It is non-linear, but acting on speed, not period, and based on sample rate.
			break;
		case 0x08:
			*type = XM_SAMPLER_CMD_PANNING;
			*param = pData->parameter;
			break;
		case 0x09:
			*type = XM_SAMPLER_CMD_OFFSET;
			*param = pData->parameter;  //The offset in sampler is of the whole sample size, not a fixed amount.
			break;
		case 0x0C:
			*vol = 0x10 + (pData->parameter >> 2);
			break;
		case 0x0E: {
			switch (pData->parameter & 0xF0) {
			case 0xC0:
				*type = XMCMD_E_DELAYED_NOTECUT;
				*param = (pData->parameter * songfile->song->properties.lpb) >> 2;
				break;
			case 0xD0:
				*type = XMCMD_E_NOTE_DELAY;
				*param = (pData->parameter * songfile->song->properties.lpb) >> 2;
				break;
			default:
				break;
			}
			break;
		}
		case 0x15:
			*type = XMCMD_RETRIG;
			*param = pData->parameter;
			break;
		default:
			break;
		}
	} else if (pData->cmd == 0x0C) {
		*vol = 0x10 + (pData->parameter >> 2);
	}
}

void xmsongexport_saveinstruments(XMSongExport* self, psy_audio_SongFile* songfile)
{
	for (int i = 0; i < MAX_BUSES; i++) {
		if (psy_audio_machines_at(&songfile->song->machines, i) != NULL && 
			psy_audio_machine_type(psy_audio_machines_at(&songfile->song->machines, i)) != MACH_SAMPLER &&
			psy_audio_machine_type(psy_audio_machines_at(&songfile->song->machines, i)) != MACH_XMSAMPLER) {
			xmsongexport_saveemptyinstrument(self, songfile,
				psy_audio_machine_editname(psy_audio_machines_at(&songfile->song->machines, i)));
		}
	}
	int remaining = self->m_Header.instruments - self->macInstruments;
	for (int j = 0, i = (self->correctionIndex == 0) ? 1 : 0; j < self->xmInstruments && j < remaining; j++, i++) {
		//if (song.xminstruments.IsEnabled(i)) {
			//SaveSampulseInstrument(song, i);
		//} else {
		//xmsongexport_saveemptyinstrument(song.xminstruments.Exists(i) ? song.xminstruments[i].Name() : "");
		//}
	}
	remaining = self->m_Header.instruments - self->macInstruments - self->xmInstruments;
	for (int i = 0; i < remaining; i++) {
		//if (song.samples.IsEnabled(i)) {
			//SaveSamplerInstrument(song, i);
		//} else {
			//SaveEmptyInstrument(song.samples.Exists(i) ? song.samples[i].WaveName() : "");
		//}
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
	/*const XMInstrument& inst = song.xminstruments[instIdx];
	struct XMINSTRUMENTHEADER insHeader;
	memset(&insHeader, 0, sizeof(insHeader));
	strncpy(insHeader.name, inst.Name().c_str(), 22); //Names are not null terminated
	//insHeader.type = 0; Implicit by memset
	struct XMSAMPLEHEADER samphead;
	std::memset(&samphead, 0, sizeof(samphead));

	std::list<unsigned char> sampidxs;
	std::map<unsigned char, unsigned char> sRemap;
	unsigned char sample = 0;
	for (int i = 12; i < 108; i++) {
		const XMInstrument::NotePair& pair = inst.NoteToSample(i);
		if (pair.second != 255) {
			std::map<unsigned char, unsigned char>::const_iterator ite = sRemap.find(pair.second);
			if (ite == sRemap.end()) {
				sampidxs.push_back(pair.second);
				sRemap[pair.second] = sample;
				samphead.snum[i - 12] = sample;
				sample++;
			} else {
				samphead.snum[i - 12] = ite->second;
			}
		}
	}

	//If no samples for this instrument, write it and exit.
	if (sampidxs.size() == 0) {
		insHeader.size = sizeof(insHeader);
		//insHeader.samples = 0; Implicit by memset
		Write(&insHeader, sizeof(insHeader));
	} else {
		insHeader.size = sizeof(insHeader) + sizeof(XMSAMPLEHEADER);
		insHeader.samples = static_cast<uint16_t>(sampidxs.size());
		Write(&insHeader, sizeof(insHeader));

		SetSampulseEnvelopes(song, instIdx, samphead);
		samphead.volfade = static_cast<uint16_t>(floor(inst.VolumeFadeSpeed() * 32768.0f));
		samphead.shsize = sizeof(XMSAMPLESTRUCT);
		Write(&samphead, sizeof(samphead));

		for (std::list<unsigned char>::const_iterator ite = sampidxs.begin(); ite != sampidxs.end(); ++ite) {
			SaveSampleHeader(song, static_cast<int>(*ite));
		}
		for (std::list<unsigned char>::const_iterator ite = sampidxs.begin(); ite != sampidxs.end(); ++ite) {
			SaveSampleData(song, static_cast<int>(*ite));
		}
	}*/
}

void xmsongexport_savesamplerinstrument(XMSongExport* self, psy_audio_SongFile* songfile, int instIdx)
{
	struct XMINSTRUMENTHEADER insHeader;
	memset(&insHeader, 0, sizeof(insHeader));
/*	strncpy(insHeader.name, song.samples[instIdx].WaveName().c_str(), 22); //Names are not null terminated
	//insHeader.type = 0; Implicit by memset

	//If it has samples, add the whole header.
	if (song.samples.IsEnabled(instIdx)) {
		insHeader.size = sizeof(insHeader) + sizeof(XMSAMPLEHEADER);
		insHeader.samples = 1;
		Write(&insHeader, sizeof(insHeader));

		XMSAMPLEHEADER _samph;
		std::memset(&_samph, 0, sizeof(_samph));
		SetSamplerEnvelopes(song, instIdx, _samph);
		_samph.volfade = 0x400;
		_samph.shsize = sizeof(XMSAMPLESTRUCT);
		Write(&_samph, sizeof(_samph));

		SaveSampleHeader(song, instIdx);
		SaveSampleData(song, instIdx);
	} else {
		insHeader.size = sizeof(insHeader);
		//insHeader.samples = 0; Implicit by memset
		Write(&insHeader, sizeof(insHeader));
	}*/
}

void xmsongexport_savesampleheader(XMSongExport* self, psy_audio_SongFile* songfile, int instIdx)
{
	psy_audio_Sample* wave;
	psy_audio_SampleIndex index;

	index = sampleindex_make(instIdx, 0);
	wave = psy_audio_samples_at(&songfile->song->samples,
		index);

	XMSAMPLESTRUCT stheader;
	memset(&stheader, 0, sizeof(stheader));
	strncpy(stheader.name, psy_audio_sample_name(wave), 22); //Names are not null terminated
	// stheader.res Implicitely set at zero by memset

	int tune = wave->tune;
	int finetune = (int)((float)wave->finetune * 1.28);
	if (wave->samplerate != 8363) {
		//correct the tuning
		double newtune = log10((double)(wave->samplerate) / 8363.0) / log10(2.0);
		double floortune = floor(newtune * 12.0);
		tune += (int)(floortune);
		finetune += (int)(floor(((newtune * 12.0) - floortune) * 128.0));
		if (finetune > 127) { tune++; finetune -= 127; }
	}

	//All samples are 16bits in Psycle.
	stheader.samplen = wave->numframes * 2;
	stheader.loopstart = wave->loop.start * 2;
	stheader.looplen = (wave->loop.end - wave->loop.start) * 2;
	stheader.vol = (int)(wave->globalvolume * 255) >> 1;
	stheader.relnote = tune;
	stheader.finetune = finetune;

	uint8_t type = 0;
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
	int length;
	short prev;
	int j;

	index = sampleindex_make(instrIdx, 0);
	wave = psy_audio_samples_at(&songfile->song->samples,
		index);
	// pack sample data
	samples = wave->channels.samples[0];
	length = wave->numframes;
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
	sampleHeader->vtype = 0;
	/*const XMInstrument& inst = song.xminstruments[instrIdx];

	if (inst.AmpEnvelope().IsEnabled()) {
		sampleHeader.vtype = 1;
		const XMInstrument::Envelope& env = inst.AmpEnvelope();

		// Max number of envelope points in Fasttracker format is 12.
		sampleHeader.vnum = std::min(12u, env.NumOfPoints());
		float convert = 1.f;
		if (env.Mode() == XMInstrument::Envelope::Mode::MILIS) {
			convert = (24.f * static_cast<float>(song.BeatsPerMin())) / 60000.f;
		}
		// Format of FastTracker points is :
		// Point : frame number. ( 1 frame= line*(24/TPB), samplepos= frame*(samplesperrow*TPB/24))
		// Value : 0..64. , divide by 64 to use it as a multiplier.
		int idx = 0;
		for (unsigned int i = 0; i < env.NumOfPoints() && i < 12; i++) {
			sampleHeader.venv[idx] = static_cast<uint16_t>(env.GetTime(i) * convert); idx++;
			sampleHeader.venv[idx] = static_cast<uint16_t>(env.GetValue(i) * 64.f); idx++;
		}

		if (env.SustainBegin() != XMInstrument::Envelope::INVALID) {
			sampleHeader.vtype |= 2;
			sampleHeader.vsustain = static_cast<uint16_t>(env.SustainBegin());
		}
		if (env.LoopStart() != XMInstrument::Envelope::INVALID) {
			sampleHeader.vtype |= 4;
			sampleHeader.vloops = static_cast<uint16_t>(env.LoopStart());
			sampleHeader.vloope = static_cast<uint16_t>(env.LoopEnd());
		}
	}
	if (inst.PanEnvelope().IsEnabled()) {
		sampleHeader.ptype = 1;
		const XMInstrument::Envelope& env = inst.PanEnvelope();

		// Max number of envelope points in Fasttracker format is 12.
		sampleHeader.pnum = std::min(12u, env.NumOfPoints());
		float convert = 1.f;
		if (env.Mode() == XMInstrument::Envelope::Mode::MILIS) {
			convert = (24.f * static_cast<float>(song.BeatsPerMin())) / 60000.f;
		}
		// Format of FastTracker points is :
		// Point : frame number. ( 1 frame= line*(24/TPB), samplepos= frame*(samplesperrow*TPB/24))
		// Value : 0..64. , divide by 64 to use it as a multiplier.
		int idx = 0;
		for (unsigned int i = 0; i < env.NumOfPoints() && i < 12; i++) {
			sampleHeader.penv[idx] = static_cast<uint16_t>(env.GetTime(i) * convert); idx++;
			sampleHeader.penv[idx] = static_cast<uint16_t>(32 + (env.GetValue(i) * 32.f)); idx++;
		}

		if (env.SustainBegin() != XMInstrument::Envelope::INVALID) {
			sampleHeader.ptype |= 2;
			sampleHeader.psustain = static_cast<uint16_t>(env.SustainBegin());
		}
		if (env.LoopStart() != XMInstrument::Envelope::INVALID) {
			sampleHeader.ptype |= 4;
			sampleHeader.ploops = static_cast<uint16_t>(env.LoopStart());
			sampleHeader.ploope = static_cast<uint16_t>(env.LoopEnd());
		}
	}*/
}

void xmsongexport_setsamplerenvelopes(XMSongExport* self, psy_audio_SongFile* songfile,
	int instrIdx, struct XMSAMPLEHEADER* sampleHeader)
{
	psy_audio_Instrument* inst;
	psy_audio_InstrumentIndex index;

	index = psy_audio_instrumentindex_make(0, instrIdx);
	inst = psy_audio_instruments_at(&songfile->song->instruments,
		index);
	sampleHeader->vtype = 0;

	//if (inst->ENV_AT != 1 || inst->ENV_DT != 1 || inst->ENV_SL != 100 || inst->ENV_RT != 220) {
	{
		float convert = 0.f;
		int idx;

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
		// sampleHeader->venv[idx] = (uint16_t)(inst->ENV_AT * convert); idx++;
		sampleHeader->venv[idx] = 64; idx++;
		// sampleHeader->venv[idx] = (uint16_t)((inst->ENV_DT + inst->ENV_DT) * convert); idx++;
		// sampleHeader->venv[idx] = (uint16_t)(inst->ENV_SL * 0.64f); idx++;
		// sampleHeader->venv[idx] = (uint16_t)((inst->ENV_RT + inst->ENV_DT + inst->ENV_RT) * convert); idx++;
		sampleHeader->venv[idx] = 0; idx++;
	}
}
