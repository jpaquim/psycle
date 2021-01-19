// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "itmodule2.h"
// local
#include "constants.h"
#include "instrument.h"
#include "instruments.h"
#include "machinefactory.h"
#include "sample.h"
#include "song.h"
#include "xmsampler.h"
// dsp
#include <valuemapper.h>
#include <operations.h>
// std
#include <assert.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

#ifndef ASSERT
#define ASSERT assert
#endif

// default sampulse group
#define INSTRUMENTGROUP 1

/*
#ifndef BOOL
typedef bool BOOL;
#endif*/


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

// BitsBlock

bool bitsblock_readblock(BitsBlock* self, PsyFile* file)
{
	// block layout : uint16 size, <size> bytes data
	uint16_t size;
	int status;

	psyfile_read(file, &size, 2);
	self->pdata = (uint8_t*)malloc(size);
	if (!self->pdata) {
		return FALSE;
	}
	if (status = psyfile_read(file, self->pdata, size)) {
		free(self->pdata);
		return FALSE;
	}
	self->rpos = self->pdata;
	self->rend = self->pdata + size;
	self->rembits = 8;
	return TRUE;
}

uint32_t bitsblock_readbits(BitsBlock* self, unsigned char bitwidth)
{
	uint32_t val;
	int32_t b;

	val = 0;
	b = 0;
	// If reached the end of the buffer, exit.
	if (self->rpos >= self->rend) {
		return val;
	}
	// while we have more bits to read than the remaining bits in this byte
	while (bitwidth > self->rembits) {
		// add to val, the data-bits from rpos, shifting the necessary number
		// of bits.
		val |= *(self->rpos++) << b;
		//if reached the end, exit.
		if (self->rpos >= self->rend) {
			return val;
		}
		// increment the shift
		b += self->rembits;
		// decrease the remaining bits to read
		bitwidth -= self->rembits;
		// set back the number of bits.
		self->rembits = 8;
	}
	// Filter the bottom-most bitwidth bytes from rpos, and shift them by b to add
	// to the final value.
	val |= ((*self->rpos) & ((1 << bitwidth) - 1)) << b;
	// shift down the remaining bits so that they are read the next time.
	*(self->rpos) >>= bitwidth;
	// reduce the remaining bits.
	self->rembits -= bitwidth;
	return val;
}

// ITModule2
// prototypes
static void itmodule2_reset(ITModule2*);
static void itmodule2_setsongcomments(ITModule2*);
static bool itmodule2_makexmsampler(ITModule2*);
static void itmodule2_setsamplerslidemode(ITModule2*);
static void itmodule2_setglobalvolume(ITModule2*);
static void itmodule2_setupchannels(ITModule2*);
static int itmodule2_readmidiembedded(ITModule2*);
static int itmodule2_readmessage(ITModule2*);
static void itmodule2_tweaksampler(ITModule2*, int twk, int val);
static int itmodule2_readsequence(ITModule2*);
static bool itmodule2_loaditpattern(ITModule2*, int patIdx, int* numchans);
static bool itmodule_writepatternentry(ITModule2*,
	psy_audio_PatternNode**, bool append, psy_audio_Pattern*,
	const int row, const int col, const psy_audio_PatternEvent*);
static bool itmodule2_parseeffect(ITModule2*, psy_audio_PatternEvent*,
	psy_audio_PatternNode**, psy_audio_Pattern*, int patIdx,
	int row, int command, int param, int channel);
static bool itmodule2_loads3mpatternx(ITModule2*, uint16_t patidx);
static int itmodule2_loadolditinst(ITModule2*, const itInsHeader1x* curh,
	psy_audio_Instrument*);
static int itmodule2_loaditinst(ITModule2*, const itInsHeader2x* curh,
	psy_audio_Instrument*);
static bool itmodule2_loaditsample(ITModule2*, psy_audio_Sample*);
static bool itmodule2_loaditsampledata(ITModule2*, psy_audio_Sample*,
	uint32_t iLen, bool bstereo, bool b16Bit, unsigned char convert);
static bool itmodule2_loaditcompresseddata(ITModule2*,
	psy_dsp_amp_t* pWavedata, uint32_t iLen, bool b16Bit, unsigned char convert);

// implementation
void itmodule2_init(ITModule2* self, psy_audio_SongFile* songfile)
{
	assert(self);

	self->songfile = songfile;
	self->fp = songfile->file;
	self->song = songfile->song;
	self->embeddeddata = NULL;
	psy_table_init(&self->xmtovirtual);
	itmodule2_reset(self);
}

void itmodule2_dispose(ITModule2* self)
{
	assert(self);

	psy_table_dispose(&self->xmtovirtual);
	self->songfile = NULL;
	free(self->embeddeddata);
	self->embeddeddata = NULL;
}

void itmodule2_reset(ITModule2* self)
{
	self->extracolumn = 0;
	self->maxextracolumn = 0;
	free(self->embeddeddata);
	self->embeddeddata = NULL;
	psy_table_clear(&self->xmtovirtual);
}

int itmodule2_load(ITModule2* self)
{	
	int status;

	if (status = psyfile_read(self->fp, &self->fileheader, sizeof(self->fileheader))) {
		return status;
	}
	if (psyfile_seek(self->fp, 0) == -1) {
		return PSY_ERRFILE;
	}
	if (self->fileheader.tag == IMPM_ID) {
		return itmodule2_loaditmodule(self);
	}
	if (status = psyfile_read(self->fp, &self->s3mFileH, sizeof(self->s3mFileH))) {
		return status;
	}
	if (psyfile_seek(self->fp, 0) == -1) {
		return PSY_ERRFILE;
	}
	if (self->s3mFileH.tag == SCRM_ID && self->s3mFileH.type == 0x10) {		
		//return itmodule2_loads3mmodulex(self);
		return PSY_ERRFILE;
	}
	return PSY_OK;
}

int itmodule2_loaditmodule(ITModule2* self)
{	
	uint32_t i; // , j;
	int status;
	uint32_t* pointersi;	
	uint32_t* pointerss;	
	uint32_t* pointersp;
	int virtualInst;
	int numchans;	

	assert(self);
	assert(self->fp);
	assert(self->song);	

	itmodule2_reset(self);
	if (status = psyfile_read(self->fp, &self->fileheader, sizeof(self->fileheader))) {
		return status;
	}
	psy_audio_song_settitle(self->song, self->fileheader.songName);
	itmodule2_setsongcomments(self);	
	if (!itmodule2_makexmsampler(self)) {
		return PSY_ERRFILE;
	}
	itmodule2_setsamplerslidemode(self);
	itmodule2_setglobalvolume(self);
	itmodule2_setupchannels(self);
	if (status = itmodule2_readsequence(self)) {
		return status;
	}		
	if (self->fileheader.insNum) {
		pointersi = malloc(self->fileheader.insNum * sizeof(uint32_t));
		if (status = psyfile_read(self->fp, pointersi, self->fileheader.insNum * sizeof(uint32_t))) {
			// clean up
			free(pointersi);
			return status;
		}
	} else {
		pointersi = NULL;
	}
	if (self->fileheader.sampNum) {
		pointerss = malloc(self->fileheader.sampNum * sizeof(uint32_t));
		if (status = psyfile_read(self->fp, pointerss, self->fileheader.sampNum * sizeof(uint32_t))) {
			// clean up
			free(pointersi);
			free(pointerss);
			return status;
		}
	} else {
		pointerss = NULL;
	}
	if (self->fileheader.patNum) {
		pointersp = malloc(self->fileheader.patNum * sizeof(uint32_t));
		if (status = psyfile_read(self->fp, pointersp, self->fileheader.patNum * sizeof(uint32_t))) {
			// clean up
			free(pointersi);
			free(pointerss);
			free(pointersp);
			return status;
		}
	} else {
		pointersp = NULL;
	}
	if (status = itmodule2_readmidiembedded(self)) {
		return status;
	}
	if (status = itmodule2_readmessage(self)) {
		return status;
	}
	virtualInst = MAX_MACHINES;
	for (i = 0; i < self->fileheader.insNum; ++i)
	{			
		psy_audio_Instrument* instrument;

		if (psyfile_seek(self->fp, pointersi[i]) == -1) {
			// clean up
			free(pointersi);
			free(pointerss);
			free(pointersp);
			return PSY_ERRFILE;
		}
		instrument = psy_audio_instrument_allocinit();
		psy_audio_instruments_insert(&self->songfile->song->instruments,
			instrument, psy_audio_instrumentindex_make(INSTRUMENTGROUP, i));
		if (self->fileheader.ffv < 0x200) {
			itInsHeader1x curh;
			int status;

			if (status = psyfile_read(self->fp, &curh, sizeof(curh))) {
				return status;
			}
			if (status = itmodule2_loadolditinst(self, &curh, instrument)) {
				return status;
			}
		} else {
			itInsHeader2x curh;
			int status;

			if (status = psyfile_read(self->fp, &curh, sizeof(curh))) {
				return status;
			}
			if (status = itmodule2_loaditinst(self, &curh, instrument)) {
				return status;
			}
		}
		psy_table_insert(&self->xmtovirtual, i,
			(void*)(uintptr_t)virtualInst);
		psy_audio_song_insertvirtualgenerator(self->songfile->song,
			virtualInst++, 0, i);			
	}
	for (i = 0; i < self->fileheader.sampNum; i++)
	{
		psy_audio_Sample* wave;
		psy_audio_Instrument* instr;
		psy_audio_InstrumentEntry instentry;
		bool created;

		if (psyfile_seek(self->fp, pointerss[i]) == -1) {
			// clean up
			free(pointersi);
			free(pointerss);
			free(pointersp);
			return PSY_ERRFILE;
		}
		wave = psy_audio_sample_allocinit(1);
		psy_audio_samples_insert(&self->songfile->song->samples, wave,
			sampleindex_make(i, 0));			
		created = itmodule2_loaditsample(self, wave);
		// If this IT file doesn't use Instruments, we need to map the notes manually.
		if (created && !(self->fileheader.flags & IT2_FLAGS_USEINSTR))
		{			
		//	if (song.xminstruments.IsEnabled(i) == false) {
		//		XMInstrument instr;
		//		instr.Init();
		//		song.xminstruments.SetInst(instr, i);
		//	}
		//	XMInstrument& instrument = song.xminstruments.get(i);
		//	instrument.Name(wave.WaveName());
		//	XMInstrument::NotePair npair;
		//	npair.second = i;
		//	for (int j = 0; j < XMInstrument::NOTE_MAP_SIZE; j++) {
		//		npair.first = j;
		//		instrument.NoteToSample(j, npair);
		//	}
		//	instrument.ValidateEnabled();

		//	ittovirtual[i] = virtualInst;
		//	song.SetVirtualInstrument(virtualInst++, 0, i);
		}	
		// create instrument entry
		instr = psy_audio_instruments_at(&self->songfile->song->instruments,
			psy_audio_instrumentindex_make(INSTRUMENTGROUP, i));
		if (instr) {
			psy_audio_instrument_clearentries(instr);
			psy_audio_instrumententry_init(&instentry);
			instentry.sampleindex = sampleindex_make(i, 0);
			psy_audio_instrument_addentry(instr, &instentry);
		}
	}
	numchans = self->maxextracolumn;
	self->maxextracolumn = 0;
	for (i = 0; i < self->fileheader.patNum; i++)
	{
		if (pointersp[i] == 0)
		{
			psy_audio_Pattern* pattern;

			pattern = psy_audio_pattern_allocinit();
			psy_audio_pattern_setname(pattern, "unnamed");
			psy_audio_patterns_insert(&self->songfile->song->patterns,
				i, pattern);
			psy_audio_pattern_setlength(pattern,
				64 * 1.0 / self->songfile->song->properties.lpb);				
		} else {
			psyfile_seek(self->fp, pointersp[i]);
			itmodule2_loaditpattern(self, i, &numchans);				
		}
	}
	psy_audio_song_setnumsongtracks(self->song,
		 psy_max(numchans + 1, (int)self->maxextracolumn));
	psy_audio_reposition(&self->songfile->song->sequence);
	free(pointersi);
	free(pointerss);
	free(pointersp);	
	return PSY_OK;
}

void itmodule2_setsongcomments(ITModule2* self)
{
	char_dyn_t* comments;

	comments = strdup("Imported from Impulse Tracker Module: ");
	comments = psy_strcat_realloc(comments, self->songfile->path);
	psy_audio_song_setcomments(self->song, comments);
	free(comments);
	comments = NULL;
}

bool itmodule2_makexmsampler(ITModule2* self)
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
			wire, psy_dsp_map_128_1((self->fileheader.mVol > 128)
					? 128
					: self->fileheader.mVol));
	}
	return self->sampler != NULL;
}

void itmodule2_setsamplerslidemode(ITModule2* self)
{
	itmodule2_tweaksampler(self, XM_SAMPLER_TWK_AMIGASLIDES,
		(self->fileheader.flags & IT2_FLAGS_LINEARSLIDES)
		? FALSE
		: TRUE);	
}

void itmodule2_setglobalvolume(ITModule2* self)
{
	itmodule2_tweaksampler(self, XM_SAMPLER_TWK_GLOBALVOLUME,
		self->fileheader.gVol);
}

void itmodule2_setupchannels(ITModule2* self)
{
	bool stereo;
	int i;

	// Flags:   Bit 0: On = Stereo, Off = Mono
	// 			Bit 1: Vol0MixOptimizations - If on, no mixing occurs if
	// 			the volume at mixing time is 0 (redundant v1.04+)
	// 			Bit 2: On = Use instruments, Off = Use samples.
	// 			Bit 3: On = Linear slides, Off = Amiga slides.
	// 			Bit 4: On = Old Effects, Off = IT Effects
	// Differences:
	// 		- Vibrato is updated EVERY frame in IT mode, whereas
	// 			it is updated every non-row frame in other formats.
	// 			Also, it is two times deeper with Old Effects ON
	// 			- Command Oxx will set the sample offset to the END
	// 			of a sample instead of ignoring the command under
	// 			old effects mode.
	// 			- (More to come, probably)
	// 			Bit 5: On = Link Effect G's memory with Effect E/F. Also
	// 			Gxx with an instrument present will cause the
	// 			envelopes to be retriggered. If you change a
	// 			sample on a row with Gxx, it'll adjust the
	// 			frequency of the current note according to:
	// 
	// 		  NewFrequency = OldFrequency * NewC5 / OldC5;
	// 		  Bit 6: Use MIDI pitch controller, Pitch depth given by PWD
	// 			  Bit 7: Request embedded MIDI configuration
	// 			  (Coded this way to permit cross-version saving)
	// 
	// Special:  Bit 0: On = song message attached.
	// 		  Song message:
	// 		  Stored at offset given by "Message Offset" field.
	// 			  Length = MsgLgth.
	// 			  NewLine = 0Dh (13 dec)
	// 			  EndOfMsg = 0
	// 
	// 		Note: v1.04+ of IT may have song messages of up to
	// 			8000 bytes included.
	// 			Bit 1: Reserved
	// 			Bit 2: Reserved
	// 			Bit 3: MIDI configuration embedded
	// 			Bit 4-15: Reserved

	stereo = self->fileheader.flags & IT2_FLAGS_STEREO;

	for (i = 0; i < 64; ++i)
	{
		// if (stereo && !(itFileH.chanPan[i] & ChanFlags::IS_DISABLED))
		//{
		//	if (itFileH.chanPan[i] == ChanFlags::IS_SURROUND)
		//	{
		//		sampler->rChannel(i).DefaultPanFactorFloat(0.5f, true);
		//		sampler->rChannel(i).DefaultIsSurround(true);
		//	} else {
		//		sampler->rChannel(i).DefaultPanFactorFloat((itFileH.chanPan[i] & 0x7F) / 64.0f, true);
		//	}
		//  else {
		//	  sampler->rChannel(i).DefaultPanFactorFloat(0.5f, true);
		//  }
		//sampler->rChannel(i).DefaultVolumeFloat(itFileH.chanVol[i] / 64.0f);
		if ((self->fileheader.chanPan[i] & IT2_CHAN_FLAGS_IS_DISABLED))
		{
			// sampler->rChannel(i).DefaultIsMute(true);
		} else {
			//sampler->rChannel(i).DefaultIsMute(false);
			self->maxextracolumn = i;
		}
		// sampler->rChannel(i).DefaultFilterType(dsp::F_ITLOWPASS);
	}
	if (self->maxextracolumn == 63) {
		self->maxextracolumn = 15;
	}
}

int itmodule2_readmidiembedded(ITModule2* self)
{
	if (self->fileheader.special & IT2_SPECIAL_FLAGS_MIDIEMBEDDED)
	{
		int16_t skipnum;
		int i;
		int status;

		self->embeddeddata = (EmbeddedMIDIData*)malloc(sizeof(EmbeddedMIDIData));
		if (status = psyfile_read(self->fp, &skipnum, sizeof(int16_t))) {
			return status;
		}
		// This is some strange data. It is not documented.
		if (psyfile_skip(self->fp, skipnum * 8) == -1) {
			return PSY_ERRFILE;
		}
		if (status = psyfile_read(self->fp, self->embeddeddata, sizeof(EmbeddedMIDIData))) {
			return status;
		}

		for (i = 0; i < 128; i++)
		{
			const char* zxx = self->embeddeddata->Zxx[i];
			char zxx2[6];

			memset(zxx2, 0, 6);
			memcpy(zxx2, zxx, 5);
			if (strcmp(zxx2, "F0F00") == 0)
			{
				int mode = 0;
				int tmp = 0;

				if (self->embeddeddata->Zxx[i][5] >= '0' && self->embeddeddata->Zxx[i][5] <= '9')
					mode = self->embeddeddata->Zxx[i][5] - '0';
				else if (self->embeddeddata->Zxx[i][5] >= 'A' && self->embeddeddata->Zxx[i][5] <= 'F')
					mode = self->embeddeddata->Zxx[i][5] - 'A' + 0xA;

				if (self->embeddeddata->Zxx[i][6] >= '0' && self->embeddeddata->Zxx[i][6] <= '9')
					tmp = (self->embeddeddata->Zxx[i][6] - '0') * 0x10;
				else if (self->embeddeddata->Zxx[i][6] >= 'A' && self->embeddeddata->Zxx[i][6] <= 'F')
					tmp = (self->embeddeddata->Zxx[i][6] - 'A' + 0xA) * 0x10;
				if (self->embeddeddata->Zxx[i][7] >= '0' && self->embeddeddata->Zxx[i][7] <= '9')
					tmp += (self->embeddeddata->Zxx[i][7] - '0');
				else if (self->embeddeddata->Zxx[i][7] >= 'A' && self->embeddeddata->Zxx[i][7] <= 'F')
					tmp += (self->embeddeddata->Zxx[i][7] - 'A' + 0xA);
				itmodule2_tweaksampler(self, XM_SAMPLER_TWK_SETZXXMACRO_INDEX, i);
				itmodule2_tweaksampler(self, XM_SAMPLER_TWK_SETZXXMACRO_MODE, mode);
				itmodule2_tweaksampler(self, XM_SAMPLER_TWK_SETZXXMACRO_VALUE, tmp);
			}
		}
	}
	return PSY_OK;
}

int itmodule2_readmessage(ITModule2* self)
{
	if ((self->fileheader.special & IT2_SPECIAL_FLAGS_HASMESSAGE) != 0 &&
		self->fileheader.msgLen > 0) {
		char_dyn_t* comments;
		int status;

		if (psyfile_seek(self->fp, self->fileheader.msgOffset) == -1) {
			return PSY_ERRFILE;
		}
		// NewLine = 0Dh (13 dec)
		// EndOfMsg = 0
		comments = malloc(self->fileheader.msgLen + 2);
		if (status = psyfile_read(self->fp, comments, self->fileheader.msgLen)) {
			free(comments);
			return status;
		}
		comments[self->fileheader.msgLen + 1] = '\0';
		psy_audio_songproperties_setcomments(
			&self->songfile->song->properties, comments);
		free(comments);
	}
	return PSY_OK;
}

void itmodule2_tweaksampler(ITModule2* self, int twk, int val)
{
	psy_audio_MachineParam* param;

	param = psy_audio_machine_tweakparameter(self->sampler, twk);
	if (param) {
		psy_audio_machine_parameter_tweak_scaled(self->sampler, param, val);
	}
}

int itmodule2_loadolditinst(ITModule2* self, const itInsHeader1x* curh, psy_audio_Instrument* xins)
{	
	psy_audio_instrument_setname(xins, curh->sName);

	xins->volumefadespeed = curh->fadeout / 512.0f;

	xins->nna = (psy_audio_NewNoteAction)curh->NNA;
	if (curh->DNC)
	{
		xins->dct = psy_audio_DUPECHECK_NOTE;
		xins->dca = psy_audio_NNA_STOP;
	}
	psy_dsp_envelope_clear(&xins->volumeenvelope);
	if (curh->flg & IT2_ENV_USE_ENVELOPE) {
		int i;

		psy_dsp_envelope_setmode(&xins->volumeenvelope, psy_dsp_ENVELOPETIME_TICK);
		psy_dsp_envelope_setenabled(&xins->volumeenvelope, TRUE);

		if (curh->flg & IT2_ENV_USE_SUSTAIN) {
			psy_dsp_envelope_setsustainbegin(&xins->volumeenvelope,
				curh->sustainS);
			psy_dsp_envelope_setsustainend(&xins->volumeenvelope,
				curh->sustainE);
		}
		if (curh->flg & IT2_ENV_USE_LOOP) {
			psy_dsp_envelope_setloopstart(&xins->volumeenvelope,
				curh->loopS);
			psy_dsp_envelope_setloopend(&xins->volumeenvelope,
				curh->loopE);
		}

		for (i = 0; i < 25; i++) {
			uint8_t tick = curh->nodepair[i].first;
			uint8_t value = curh->nodepair[i].second;
			if (value == 0xFF || tick == 0xFF) break;

			psy_dsp_envelope_append(&xins->volumeenvelope,
				psy_dsp_envelopepoint_make(tick, (float)value / 64.0f));
		}
	}
	psy_dsp_envelope_clear(&xins->panenvelope);
	psy_dsp_envelope_clear(&xins->pitchenvelope);
	psy_dsp_envelope_clear(&xins->filterenvelope);	
	//xins.ValidateEnabled();
	return PSY_OK;
}

int itmodule2_loaditinst(ITModule2* self, const itInsHeader2x* curh, psy_audio_Instrument* xins)
{
	int envelope_point_num;
	int i;

	assert(self);
	assert(curh);
	assert(xins);
			
	psy_audio_instrument_setname(xins, curh->sName);
	xins->nna = (psy_audio_NewNoteAction)curh->NNA;
	xins->dct = (psy_audio_DupeCheck)curh->DCT;
	switch (curh->DCA) {
		case IT2_DCACTION_NOTEOFF:
			xins->dca = psy_audio_NNA_NOTEOFF;
			break;
		case IT2_DCACTION_FADEOUT:
			xins->dca = psy_audio_NNA_FADEOUT;
			break;
		case IT2_DCACTION_STOP://fallthrough
		default:
			xins->dca = psy_audio_NNA_NOTEOFF;
			break;
	}

	xins->initpan = (curh->defPan & 0x7F) / 64.0f;
	xins->panenabled = (curh->defPan & 0x80) ? FALSE : TRUE;
	xins->notemodpancenter = (curh->pPanCenter);
	xins->notemodpansep = (curh->pPanSep);
	xins->globalvolume = (curh->gVol / 127.0f);
	xins->volumefadespeed = (curh->fadeout / 1024.0f);
	xins->randomvolume = (curh->randVol / 100.f);
	xins->randompanning = (curh->randPan / 64.f);
	if ((curh->inFC & 0x80) != 0)
	{
		psy_audio_instrument_setfiltertype(xins,
			F_ITLOWPASS);
		psy_audio_instrument_setfiltercutoff(xins,
			(curh->inFC & 0x7F) / 127.f);
	}
	if ((curh->inFR & 0x80) != 0)
	{
		psy_audio_instrument_setfiltertype(xins,
			F_ITLOWPASS);
		psy_audio_instrument_setfilterresonance(xins,
			(float)(curh->inFR & 0x7F));
	}

	//XMInstrument::NotePair npair;
	//int i;
	//for (i = 0; i < XMInstrument::NOTE_MAP_SIZE; i++) {
	//	npair.first = curh.notes[i].first;
	//	npair.second = curh.notes[i].second - 1;
	//	xins.NoteToSample(i, npair);
	//}

	// volume envelope
	psy_dsp_envelope_clear(&xins->volumeenvelope);
	psy_dsp_envelope_setmode(&xins->volumeenvelope, psy_dsp_ENVELOPETIME_TICK);
	psy_dsp_envelope_setenabled(&xins->volumeenvelope,
		curh->volEnv.flg & IT2_ENV_USE_ENVELOPE);		
	if (curh->volEnv.flg & IT2_ENV_ENABLE_CARRY) {
		psy_dsp_envelope_setcarry(&xins->volumeenvelope, TRUE);		
	}
	if (curh->volEnv.flg & IT2_ENV_USE_SUSTAIN) {
		psy_dsp_envelope_setsustainbegin(&xins->volumeenvelope,
			curh->volEnv.sustainS);
		psy_dsp_envelope_setsustainend(&xins->volumeenvelope,
			curh->volEnv.sustainE);		
	}
	if (curh->volEnv.flg & IT2_ENV_USE_LOOP) {
		psy_dsp_envelope_setloopstart(&xins->volumeenvelope,
			curh->volEnv.loopS);
		psy_dsp_envelope_setloopend(&xins->volumeenvelope,
			curh->volEnv.loopE);	
	}

	envelope_point_num = curh->volEnv.numP;
	if (envelope_point_num > 25) {
		envelope_point_num = 25;
	}

	for (i = 0; i < envelope_point_num; i++) {
		short envtmp;
		
		envtmp = curh->volEnv.nodes[i].secondlo | (curh->volEnv.nodes[i].secondhi << 8);
		psy_dsp_envelope_append(&xins->volumeenvelope,
			psy_dsp_envelopepoint_make(envtmp, (float)curh->volEnv.nodes[i].first / 64.0f));
	}

	//// Pan envelope
	psy_dsp_envelope_clear(&xins->panenvelope);
	psy_dsp_envelope_setmode(&xins->panenvelope, psy_dsp_ENVELOPETIME_TICK);
	psy_dsp_envelope_setenabled(&xins->panenvelope,
		curh->panEnv.flg & IT2_ENV_USE_ENVELOPE);
	if (curh->panEnv.flg & IT2_ENV_ENABLE_CARRY) {
		psy_dsp_envelope_setcarry(&xins->panenvelope, TRUE);
	}
	if (curh->panEnv.flg & IT2_ENV_USE_SUSTAIN) {
		psy_dsp_envelope_setsustainbegin(&xins->panenvelope,
			curh->panEnv.sustainS);
		psy_dsp_envelope_setsustainend(&xins->panenvelope,
			curh->panEnv.sustainE);
	}
	if (curh->panEnv.flg & IT2_ENV_USE_LOOP) {
		psy_dsp_envelope_setloopstart(&xins->panenvelope,
			curh->panEnv.loopS);
		psy_dsp_envelope_setloopend(&xins->panenvelope,
			curh->panEnv.loopE);
	}

	envelope_point_num = curh->panEnv.numP;
	if (envelope_point_num > 25) {
		envelope_point_num = 25;
	}

	for (i = 0; i < envelope_point_num; i++) {
		short envtmp;

		envtmp = curh->panEnv.nodes[i].secondlo | (curh->panEnv.nodes[i].secondhi << 8);
		psy_dsp_envelope_append(&xins->panenvelope,
			psy_dsp_envelopepoint_make(envtmp, (float)curh->panEnv.nodes[i].first / 32.0f));
	}

	// Pitch/Filter envelope
	psy_dsp_envelope_clear(&xins->pitchenvelope);
	psy_dsp_envelope_setmode(&xins->pitchenvelope, psy_dsp_ENVELOPETIME_TICK);
	psy_dsp_envelope_clear(&xins->filterenvelope);
	psy_dsp_envelope_setmode(&xins->filterenvelope, psy_dsp_ENVELOPETIME_TICK);

	envelope_point_num = curh->pitchEnv.numP;
	if (envelope_point_num > 25) { // Max number of envelope points in Impulse format is 25.
		envelope_point_num = 25;
	}

	if (curh->pitchEnv.flg & IT2_ENV_ISFILTER)
	{
		psy_audio_instrument_setfiltertype(xins, F_ITLOWPASS);
		psy_dsp_envelope_setenabled(&xins->filterenvelope,
			curh->pitchEnv.flg & IT2_ENV_USE_ENVELOPE);
		psy_dsp_envelope_setenabled(&xins->pitchenvelope, FALSE);
		if (curh->pitchEnv.flg & IT2_ENV_ENABLE_CARRY) {
			psy_dsp_envelope_setcarry(&xins->filterenvelope, TRUE);
		}
		if (curh->pitchEnv.flg & IT2_ENV_USE_SUSTAIN) {
			psy_dsp_envelope_setsustainbegin(&xins->filterenvelope,
				curh->pitchEnv.sustainS);
			psy_dsp_envelope_setsustainend(&xins->filterenvelope,
				curh->pitchEnv.sustainE);
		}
		if (curh->pitchEnv.flg & IT2_ENV_USE_LOOP) {
			psy_dsp_envelope_setloopstart(&xins->filterenvelope,
				curh->pitchEnv.loopS);
			psy_dsp_envelope_setloopend(&xins->filterenvelope,
				curh->pitchEnv.loopE);
		}

		for (i = 0; i < envelope_point_num; i++) {
			short envtmp;

			envtmp = curh->pitchEnv.nodes[i].secondlo | (curh->pitchEnv.nodes[i].secondhi << 8);
			psy_dsp_envelope_append(&xins->filterenvelope,
				psy_dsp_envelopepoint_make(envtmp, (float)curh->pitchEnv.nodes[i].first / 64.0f));
		}
	} else {
		psy_dsp_envelope_setenabled(&xins->pitchenvelope,
			curh->pitchEnv.flg& IT2_ENV_USE_ENVELOPE);
		psy_dsp_envelope_setenabled(&xins->filterenvelope, FALSE);
		if (curh->pitchEnv.flg & IT2_ENV_ENABLE_CARRY) {
			psy_dsp_envelope_setcarry(&xins->pitchenvelope, TRUE);
		}
		if (curh->pitchEnv.flg & IT2_ENV_USE_SUSTAIN) {
			psy_dsp_envelope_setsustainbegin(&xins->pitchenvelope,
				curh->pitchEnv.sustainS);
			psy_dsp_envelope_setsustainend(&xins->pitchenvelope,
				curh->pitchEnv.sustainE);
		}
		if (curh->pitchEnv.flg & IT2_ENV_USE_LOOP) {
			psy_dsp_envelope_setloopstart(&xins->pitchenvelope,
				curh->pitchEnv.loopS);
			psy_dsp_envelope_setloopend(&xins->pitchenvelope,
				curh->pitchEnv.loopE);
		}

		for (i = 0; i < envelope_point_num; i++) {
			short envtmp;

			envtmp = curh->pitchEnv.nodes[i].secondlo | (curh->pitchEnv.nodes[i].secondhi << 8);
			psy_dsp_envelope_append(&xins->pitchenvelope,
				psy_dsp_envelopepoint_make(envtmp, (float)curh->pitchEnv.nodes[i].first / 32.0f));
		}
	}
	//xins.ValidateEnabled();
	return PSY_OK;
}

int itmodule2_readsequence(ITModule2* self)
{
	int status;
	uint16_t i;	
		
	status = PSY_OK;
	psy_audio_sequence_appendtrack(&self->songfile->song->sequence,
		psy_audio_sequencetrack_allocinit());
	for (i = 0; i < self->fileheader.ordNum; ++i) {
		uint8_t patidx;
		
		if (status = psyfile_read(self->fp, &patidx, sizeof(uint8_t))) {			
			break;
		}			
		psy_audio_sequence_insert(&self->songfile->song->sequence,
			psy_audio_orderindex_make(0, i), patidx);
	}
	return status;
}

bool itmodule2_loaditsample(ITModule2* self, psy_audio_Sample* _wave)
{
	itSampleHeader curh;
	psy_audio_Song* song;
	PsyFile* fp;
	char renamed[26];
	int i;
	bool bstereo;
	bool b16Bit;
	bool bcompressed;
	bool bLoop;
	bool bsustainloop;

	song = self->songfile->song;
	fp = self->songfile->file;

	psyfile_read(fp, &curh, sizeof(curh));
	renamed[26];
	for (i = 0; i < 25; i++) {
		if (curh.sName[i] == '\0') renamed[i] = ' ';
		else renamed[i] = curh.sName[i];
	}
	renamed[25] = '\0';	

	// Flg: Bit 0. On = sample associated with header.
	//      Bit 1. On = 16 bit, Off = 8 bit.
	//      Bit 2. On = stereo, Off = mono. Stereo samples not supported yet
	//      Bit 3. On = compressed samples.
	//      Bit 4. On = Use loop
	//      Bit 5. On = Use sustain loop
	//      Bit 6. On = Ping Pong loop, Off = Forwards loop
	//      Bit 7. On = Ping Pong Sustain loop, Off = Forwards Sustain loop
	
	bstereo = curh.flg & IT2_SAMPLE_FLAGS_ISSTEREO;
	b16Bit = curh.flg & IT2_SAMPLE_FLAGS_IS16BIT;
	bcompressed = curh.flg & IT2_SAMPLE_FLAGS_ISCOMPRESSED;
	bLoop = curh.flg & IT2_SAMPLE_FLAGS_USELOOP;
	bsustainloop = curh.flg & IT2_SAMPLE_FLAGS_USESUSTAIN;

	if (curh.flg & IT2_SAMPLE_FLAGS_HAS_SAMPLE)
	{
		int exchwave[4] = {
			psy_audio_WAVEFORMS_SINUS,
			psy_audio_WAVEFORMS_SAWDOWN,
			psy_audio_WAVEFORMS_SQUARE,
			psy_audio_WAVEFORMS_RANDOM
		};

		_wave->numframes = curh.length;
		if (bstereo) {
			psy_audio_sample_resize(_wave, 2);
		}
		psy_audio_sample_allocwavedata(_wave);		

		_wave->loop.start = curh.loopB;
		_wave->loop.end = curh.loopE;
		if (bLoop) {
			if (curh.flg & IT2_SAMPLE_FLAGS_ISLOOPPINPONG)
			{
				_wave->loop.type = psy_audio_SAMPLE_LOOP_BIDI;
			} else _wave->loop.type = psy_audio_SAMPLE_LOOP_NORMAL;
		} else {
			_wave->loop.type = psy_audio_SAMPLE_LOOP_DO_NOT;
		}
		_wave->sustainloop.start = curh.sustainB;
		_wave->sustainloop.end = curh.sustainE;
		if (bsustainloop)
		{
			if (curh.flg & IT2_SAMPLE_FLAGS_ISLOOPPINPONG)
			{
				_wave->sustainloop.type = psy_audio_SAMPLE_LOOP_BIDI;
			} else _wave->sustainloop.type = psy_audio_SAMPLE_LOOP_NORMAL;
		} else {
			_wave->sustainloop.type = psy_audio_SAMPLE_LOOP_DO_NOT;
		}

		_wave->defaultvolume = curh.vol * 2;
		_wave->globalvolume = (curh.gVol / 64.0f);


		//				Older method. conversion from speed to tune. Replaced by using the samplerate directly
		//				double tune = log10(double(curh.c5Speed)/8363.0f)/log10(2.0);
		//				double maintune = floor(tune*12);
		//				double finetune = floor(((tune*12)-maintune)*100);

		//				_wave.WaveTune(maintune);
		//				_wave.WaveFineTune(finetune);
		_wave->samplerate = curh.c5Speed;
		psy_audio_sample_setname(_wave, renamed);
		_wave->panenabled = (curh.dfp & 0x80);
		_wave->panfactor = (curh.dfp & 0x7F) / 64.0f;
		_wave->vibrato.attack = (curh.vibR == 0 ? 1 : curh.vibR);
		_wave->vibrato.speed = (curh.vibS);
		_wave->vibrato.depth = (curh.vibD);
		_wave->vibrato.type = (exchwave[curh.vibT & 3]);

		if (curh.length > 0) {
			psyfile_seek(fp, curh.smpData);
			if (bcompressed) {				
				itmodule2_loaditcompresseddata(self, _wave->channels.samples[0], curh.length, b16Bit, curh.cvt);
				if (bstereo) itmodule2_loaditcompresseddata(self, _wave->channels.samples[1], curh.length, b16Bit, curh.cvt);
			} else itmodule2_loaditsampledata(self, _wave, curh.length, bstereo, b16Bit, curh.cvt);
		}
		return TRUE;
	}

	return FALSE;
}

bool itmodule2_loaditsampledata(ITModule2* self, psy_audio_Sample* _wave, uint32_t iLen, bool bstereo, bool b16Bit, unsigned char convert)
{
	signed short wNew, wTmp;
	int offset = (convert & IT2_SAMPLE_CONVERT_IS_SIGNED) ? 0 : -32768;
	int lobit = (convert & IT2_SAMPLE_CONVERTIS_MOTOROLA) ? 8 : 0;
	int hibit = 8 - lobit;
	uint32_t j, out;
	psy_audio_Song* song;
	PsyFile* fp;
	unsigned char* smpbuf;

	song = self->songfile->song;
	fp = self->songfile->file;

	if (b16Bit) iLen *= 2;
	smpbuf = malloc(iLen);
	psyfile_read(fp, smpbuf, iLen);

	out = 0; wNew = 0;
	if (b16Bit) {
		for (j = 0; j < iLen; j += 2) {
			wTmp = ((smpbuf[j] << lobit) | (smpbuf[j + 1] << hibit)) + offset;
			wNew = (convert & IT2_SAMPLE_CONVERTIS_DELTA) ? wNew + wTmp : wTmp;
			_wave->channels.samples[0][out] = (psy_dsp_amp_t)wNew;						
			out++;
		}
		if (bstereo) {
			psyfile_read(fp, smpbuf, iLen);
			out = 0;
			for (j = 0; j < iLen; j += 2) {
				wTmp = ((smpbuf[j] << lobit) | (smpbuf[j + 1] << hibit)) + offset;
				wNew = (convert & IT2_SAMPLE_CONVERTIS_DELTA) ? wNew + wTmp : wTmp;
				_wave->channels.samples[0][out] = (psy_dsp_amp_t)wNew;				
				out++;
			}
		}
	} else {		
		for (j = 0; j < iLen; j++) {
			wNew = (convert & IT2_SAMPLE_CONVERTIS_DELTA) ? wNew + smpbuf[j] : smpbuf[j];			
			_wave->channels.samples[0][j] = (psy_dsp_amp_t)((wNew << 8) + offset);			
		}
		if (bstereo) {			
			psyfile_read(fp, smpbuf, iLen);
			for (j = 0; j < iLen; j++) {
				wNew = (convert & IT2_SAMPLE_CONVERTIS_DELTA) ? wNew + smpbuf[j] : smpbuf[j];
				_wave->channels.samples[1][j] = (psy_dsp_amp_t)((wNew << 8) + offset);					
			}
		}
	}
	free(smpbuf); smpbuf = 0;
	return TRUE;
}

bool itmodule2_loaditcompresseddata(ITModule2* self, psy_dsp_amp_t* pWavedata, uint32_t iLen, bool b16Bit, unsigned char convert)
{
	unsigned char bitwidth, packsize, maxbitsize;
	uint32_t topsize, val, j;
	short d1, d2, wNew;
	char d18, d28;
	psy_audio_Song* song;
	PsyFile* fp;
	bool deltapack;

	song = self->songfile->song;
	fp = self->songfile->file;

	deltapack = (self->fileheader.ffv >= 0x215 && convert & IT2_SAMPLE_CONVERTIS_DELTA); // Impulse Tracker 2.15 added a delta packed compressed sample format

	if (b16Bit) {
		topsize = 0x4000;		packsize = 4;	maxbitsize = 16;
	} else {
		topsize = 0x8000;		packsize = 3;	maxbitsize = 8;
	}

	j = 0;
	while (j < iLen) // While we haven't decompressed the whole sample
	{
		BitsBlock block;
		uint32_t blocksize;
		uint32_t blockpos;

		bitsblock_init(&block);
		if (!bitsblock_readblock(&block, fp)) {
			bitsblock_dispose(&block);
			return FALSE;
		}

		// Size of the block of data to process, in blocks of max size=0x8000bytes ( 0x4000 samples if 16bits)
		blocksize = (iLen - j < topsize) ? iLen - j : topsize;
		blockpos = 0;

		bitwidth = maxbitsize + 1;
		d1 = d2 = 0;
		d18 = d28 = 0;

		//Start the decompression:
		while (blockpos < blocksize) {
			val = bitsblock_readbits(&block, bitwidth);

			//Check if value contains a bitwidth change. If it does, change and proceed with next value.
			if (bitwidth < 7) { //Method 1:
				if (val == (uint32_t)(1 << (bitwidth - 1))) { // if the value == topmost bit set.
					val = bitsblock_readbits(&block, packsize) + 1;
					bitwidth = (val < bitwidth) ? val : val + 1;
					continue;
				}
			} else if (bitwidth < maxbitsize + 1) { //Method 2
				unsigned short border = (((1 << maxbitsize) - 1) >> (maxbitsize + 1 - bitwidth)) - (maxbitsize / 2);

				if ((val > border) && (val <= (uint32_t)(border + maxbitsize))) {
					val -= border;
					bitwidth = val < bitwidth ? val : val + 1;
					continue;
				}
			} else if (bitwidth == maxbitsize + 1) { //Method 3
				if (val & (1 << maxbitsize)) {
					bitwidth = (val + 1) & 0xFF;
					continue;
				}
			} else { //Illegal width, abort ?
				bitsblock_dispose(&block);
				return FALSE;
			}

			//If we reach here, val contains a value to decompress, so do it.
			{
				if (b16Bit)
				{
					short v; //The sample value:
					if (bitwidth < maxbitsize) {
						unsigned char shift = maxbitsize - bitwidth;
						v = (short)(val << shift);
						v >>= shift;
					} else
						v = (short)val;

					//And integrate the sample value
					d1 += v;
					d2 += d1;
					wNew = deltapack ? d2 : d1;
				} else
				{
					char v; //The sample value:
					if (bitwidth < maxbitsize) {
						unsigned char shift = maxbitsize - bitwidth;
						v = (val << shift);
						v >>= shift;
					} else
						v = (char)val;

					d18 += v;
					d28 += d18;
					wNew = deltapack ? d28 : d18;
					wNew <<= 8;
				}
			}

			//Store the decompressed value to Wave pointer.
			pWavedata[j + blockpos] = (psy_dsp_amp_t)wNew;			

			blockpos++;
		}
		j += blocksize;
		bitsblock_dispose(&block);
	}
	return FALSE;
}

bool itmodule2_loaditpattern(ITModule2* self, int patidx, int* numchans)
{
	psy_audio_PatternEvent pempty;
	psy_audio_PatternEvent pent;
	unsigned char newEntry;
	uint8_t lastnote[64];
	uint8_t lastinst[64];
	uint8_t lastmach[64];
	uint8_t lastvol[64];
	uint8_t lastcom[64];
	uint8_t lasteff[64];
	uint8_t mask[64];	
	int16_t rowCount;
	psy_audio_Pattern* pattern;
	psy_audio_PatternNode* node;
	PsyFile* fp;
	psy_audio_Song* song;
	bool append;
	int row;
	int status;
	
	fp = self->songfile->file;
	song = self->songfile->song;
	memset(lastnote, 255, sizeof(char) * 64);
	memset(lastinst, 255, sizeof(char) * 64);
	memset(lastmach, 255, sizeof(char) * 64);
	memset(lastvol, 255, sizeof(char) * 64);
	memset(lastcom, 255, sizeof(char) * 64);
	memset(lasteff, 255, sizeof(char) * 64);
	memset(mask, 255, sizeof(char) * 64);
	
	pempty.note = 255;
	pempty.inst = 255;
	pempty.mach = 255;	
	pempty.vol = psy_audio_NOTECOMMANDS_VOL_EMPTY;
	pempty.cmd = 0;
	pempty.parameter = 0;

	pent = pempty;

	psyfile_skip(fp, 2); // packedSize
	if (status = psyfile_read(self->fp, &rowCount, sizeof(int16_t))) {
		return status;
	}	
	psyfile_skip(fp, 4); // unused
	if (rowCount > MAX_LINES) rowCount = MAX_LINES;
	else if (rowCount < 0) rowCount = 0;

	pattern = psy_audio_pattern_allocinit();
	psy_audio_pattern_setname(pattern, "unnamed");
	psy_audio_patterns_insert(&song->patterns, patidx, pattern);
	psy_audio_pattern_setlength(pattern,
		rowCount * 1.0 / self->songfile->song->properties.lpb);
	
	//char* packedpattern = new char[packedSize];
	//Read(packedpattern, packedSize);
	node = NULL;	
	for (row = 0; row < rowCount; row++)
	{
		self->extracolumn = *numchans + 1;
		psyfile_read(fp, &newEntry, 1);
		append = TRUE;
		while (newEntry)
		{
			unsigned char channel;
			unsigned char volume;			

			channel = (newEntry - 1) & 0x3F;
			if (channel >= self->extracolumn) { self->extracolumn = channel + 1; }
			if (newEntry & 0x80) {
				uint8_t tmp_u8;

				if (status = psyfile_read(self->fp, &tmp_u8, sizeof(uint8_t))) {
					return status;
				}
				mask[channel] = tmp_u8;
			}
			volume = 255;
			if (mask[channel] & 1)
			{
				uint8_t note;
				
				if (status = psyfile_read(self->fp, &note, sizeof(uint8_t))) {
					return status;
				}
				if (note == 255) pent.note = psy_audio_NOTECOMMANDS_RELEASE;
				else if (note == 254) pent.note = psy_audio_NOTECOMMANDS_RELEASE; //\todo: Attention ! Psycle doesn't have a note-cut note.
				else pent.note = note;
				lastnote[channel] = pent.note;
			} else if (mask[channel] & 0x10) {
				pent.note = lastnote[channel];
			}
			if (mask[channel] & 2) {
				uint8_t tmp_u8;

				if (status = psyfile_read(self->fp, &tmp_u8, sizeof(uint8_t))) {
					return status;
				}
				pent.inst = tmp_u8 - 1;
			} else if (mask[channel] & 0x20) {
				pent.inst = lastinst[channel];
			}
			if (mask[channel] & 4)
			{
				uint8_t tmp;				

				if (status = psyfile_read(self->fp, &tmp, sizeof(uint8_t))) {
					return status;
				}
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
				if (tmp < 65) {
					volume = tmp < 64 ? tmp : 63;
				} else if (tmp < 75) {
					volume = XM_SAMPLER_CMD_VOL_FINEVOLSLIDEUP | (tmp - 65);
				} else if (tmp < 85) {
					volume = XM_SAMPLER_CMD_VOL_FINEVOLSLIDEDOWN | (tmp - 75);
				} else if (tmp < 95) {
					volume = XM_SAMPLER_CMD_VOL_VOLSLIDEUP | (tmp - 85);
				} else if (tmp < 105) {
					volume = XM_SAMPLER_CMD_VOL_VOLSLIDEDOWN | (tmp - 95);
				} else if (tmp < 115) {
					volume = XM_SAMPLER_CMD_VOL_PITCH_SLIDE_DOWN | (tmp - 105);
				} else if (tmp < 125) {
					volume = XM_SAMPLER_CMD_VOL_PITCH_SLIDE_UP | (tmp - 115);
				} else if (tmp < 193) {
					tmp = (tmp == 192) ? 15 : (tmp - 128) / 4;
					volume = XM_SAMPLER_CMD_VOL_PANNING | tmp;
				} else if (tmp < 203) {
					volume = XM_SAMPLER_CMD_VOL_TONEPORTAMENTO | (tmp - 193);
				} else if (tmp < 213) {
					volume = XM_SAMPLER_CMD_VOL_VIBRATO | (tmp - 203);
				} else {
					volume = 255;
				}
				lastvol[channel] = volume;
			} else if (mask[channel] & 0x40) {
				volume = lastvol[channel];
			}
			if (mask[channel] & 8)
			{
				uint8_t command;
				uint8_t param;

				if (status = psyfile_read(self->fp, &command, sizeof(uint8_t))) {
					return status;
				}
				if (status = psyfile_read(self->fp, &param, sizeof(uint8_t))) {
					return status;
				}
				if (command != 0) pent.parameter = param;
				append = itmodule2_parseeffect(self, &pent, &node, pattern, patidx, row,
					command, param, channel);
				lastcom[channel] = pent.cmd;
				lasteff[channel] = pent.parameter;
			} else if (mask[channel] & 0x80)
			{
				pent.cmd = lastcom[channel];
				pent.parameter = lasteff[channel];
			}

			// If empty, do not inform machine
			if (pent.note == psy_audio_NOTECOMMANDS_EMPTY && pent.inst == 255 && volume == 255 && pent.cmd == 00 && pent.parameter == 00) {
				pent.mach = 255;
			}
			// if instrument without note, or note without instrument, cannot use virtual instrument, so use sampulse directly
			else if ((pent.note == psy_audio_NOTECOMMANDS_EMPTY && pent.inst != 255) || (pent.note < psy_audio_NOTECOMMANDS_RELEASE && pent.inst == 255)) {
				pent.mach = 0;
				if (pent.inst != 255) {
					lastinst[channel] = (uint8_t)pent.inst;
					//We cannot use the virtual instrument, but we should remember which instrument it is.										
					if (psy_table_exists(&self->xmtovirtual, pent.inst)) {
						lastmach[channel] = (uint8_t)(uintptr_t)psy_table_at(&self->xmtovirtual, pent.inst);
					}
				}
			}
			//default behaviour, let's find the virtual instrument.
			else {				
				if (!psy_table_exists(&self->xmtovirtual, pent.inst)) {						
					if (pent.inst != 255) {
						pent.mach = 0;
						lastmach[channel] = pent.mach;
						lastinst[channel] = (uint8_t)pent.inst;
					} else if (lastmach[channel] != 255) {
						pent.mach = lastmach[channel];
					} else if (volume != 255) {
						pent.mach = 0;
					} else {
						pent.mach = 255;
					}
				} else {
					lastinst[channel] = (uint8_t)pent.inst;
					pent.mach = (uint8_t)(uintptr_t)psy_table_at(&self->xmtovirtual, pent.inst);
					pent.inst = 255;
					lastmach[channel] = pent.mach;
				}
			}
			if (pent.mach == 0) { // fallback to the old behaviour. This will happen only if an unused instrument is present in the pattern.
				if (pent.cmd != 0 || pent.parameter != 0) {
					if (volume != 255 && self->extracolumn < MAX_TRACKS) {
						psy_audio_PatternEvent e;
						
						e.note = psy_audio_NOTECOMMANDS_MIDICC;
						e.inst = channel;
						e.mach = pent.mach;
						e.cmd = XM_SAMPLER_CMD_SENDTOVOLUME;
						e.parameter = volume;
						append = FALSE;
						itmodule_writepatternentry(self, &node, append, pattern,
							row, self->extracolumn, &e);
						self->extracolumn++;						
					}
				} else if (volume < 0x40) {
					pent.cmd = XM_SAMPLER_CMD_VOLUME;
					pent.parameter = volume * 2;
				} else if (volume != 255) {
					pent.cmd = XM_SAMPLER_CMD_SENDTOVOLUME;
					pent.parameter = volume;
				}
			} else {
				pent.inst = volume;
			}
							
			itmodule_writepatternentry(self, &node, append, pattern, row, channel,
				&pent);
			pent = pempty;

			*numchans = psy_max((int)channel, *numchans);

			psyfile_read(fp, &newEntry, 1);
		}
		self->maxextracolumn = psy_max(self->maxextracolumn, self->extracolumn);
	}
	return TRUE;
}

bool itmodule_writepatternentry(ITModule2* self,
	psy_audio_PatternNode** node, bool append,
	psy_audio_Pattern* pattern, const int row, const int col,
	const psy_audio_PatternEvent* e)
{
	assert(self);
	assert(pattern && node && e);

	TRACE_INT(row);
	TRACE(", ");
	TRACE_INT(col);
	TRACE("\n");

	if (!psy_audio_patternevent_empty(e)) {
		psy_audio_PatternEvent ev;
		psy_audio_PatternNode* insert;
		psy_audio_PatternNode* prev;

		ev = *e;
		if (e->inst == psy_audio_NOTECOMMANDS_EMPTY) {
			// until now empty instruments has been marked with 255,
			// correct it to psy_audio_NOTECOMMANDS_INST_EMPTY (16bit)
			ev.inst = psy_audio_NOTECOMMANDS_INST_EMPTY;
		}		

		if (!append) {
			insert = psy_audio_pattern_findnode(pattern, col,
				(psy_dsp_beat_t)(row * 1.0 / self->songfile->song->properties.lpb),
				1.0 / self->songfile->song->properties.lpb,
				&prev);
		} else {
			insert = NULL;
		}
		if (!insert) {
			*node = psy_audio_pattern_insert(pattern, *node, col,
				(psy_dsp_beat_t)(row * 1.0 / self->songfile->song->properties.lpb),
				&ev);
		} else {
			psy_audio_PatternEntry* entry;

			entry = psy_audio_patternnode_entry(insert);
			assert(psy_audio_patternentry_front(entry));
			*psy_audio_patternentry_front(entry) = ev;			
		}
	}
	return TRUE;
}

bool itmodule2_parseeffect(ITModule2* self, psy_audio_PatternEvent* pent,
	psy_audio_PatternNode** node, psy_audio_Pattern* pattern, int patIdx,
	int row, int command, int param, int channel)
{
	bool rv_append;
	int exchwave[4] = {
		psy_audio_WAVEFORMS_SINUS,
		psy_audio_WAVEFORMS_SAWDOWN,
		psy_audio_WAVEFORMS_SQUARE,
		psy_audio_WAVEFORMS_RANDOM
	};
	
	rv_append = TRUE;
	switch (command) {
	case IT2_CMD_SET_SPEED:
	{
		int extraticks;
		
		extraticks = 0;
		pent->cmd = psy_audio_PATTERNCMD_EXTENDED;
		extraticks = 0;
		pent->parameter = calclpbfromspeed(param, &extraticks);
		if (extraticks != 0 && self->extracolumn < MAX_TRACKS) {
			psy_audio_PatternEvent e;			
			
			e.note = psy_audio_NOTECOMMANDS_EMPTY;
			e.inst = 255;
			e.mach = pent->mach;
			e.cmd = psy_audio_PATTERNCMD_EXTENDED;
			e.parameter = psy_audio_PATTERNCMD_ROW_EXTRATICKS | extraticks;			
			rv_append = FALSE;
			itmodule_writepatternentry(self, node, rv_append, pattern, row,
				self->extracolumn, &e);
			self->extracolumn++;
		}
	}
	break;
	case IT2_CMD_JUMP_TO_ORDER:
		pent->cmd = psy_audio_PATTERNCMD_JUMP_TO_ORDER;
		break;
	case IT2_CMD_BREAK_TO_ROW:
		pent->cmd = psy_audio_PATTERNCMD_BREAK_TO_LINE;
		break;
	case IT2_CMD_VOLUME_SLIDE:
		pent->cmd = XM_SAMPLER_CMD_VOLUMESLIDE;
		break;
	case IT2_CMD_PORTAMENTO_DOWN:
		pent->cmd = XM_SAMPLER_CMD_PORTAMENTO_DOWN;
		break;
	case IT2_CMD_PORTAMENTO_UP:
		pent->cmd = XM_SAMPLER_CMD_PORTAMENTO_UP;
		break;
	case IT2_CMD_TONE_PORTAMENTO:
		pent->cmd = XM_SAMPLER_CMD_PORTA2NOTE;
		break;
	case IT2_CMD_VIBRATO:
		pent->cmd = XM_SAMPLER_CMD_VIBRATO;
		break;
	case IT2_CMD_TREMOR:
		pent->cmd = XM_SAMPLER_CMD_TREMOR;
		break;
	case IT2_CMD_ARPEGGIO:
		pent->cmd = XM_SAMPLER_CMD_ARPEGGIO;
		break;
	case IT2_CMD_VOLSLIDE_VIBRATO:
		pent->cmd = XM_SAMPLER_CMD_VIBRATOVOL;
		break;
	case IT2_CMD_VOLSLIDE_TONEPORTA:
		pent->cmd = XM_SAMPLER_CMD_TONEPORTAVOL;
		break;
	case IT2_CMD_SET_CHANNEL_VOLUME: // IT
		pent->cmd = XM_SAMPLER_CMD_SET_CHANNEL_VOLUME;
		break;
	case IT2_CMD_CHANNEL_VOLUME_SLIDE: // IT
		pent->cmd = XM_SAMPLER_CMD_CHANNEL_VOLUMESLIDE;
		break;
	case IT2_CMD_SET_SAMPLE_OFFSET:
		pent->cmd = XM_SAMPLER_CMD_OFFSET | self->highoffset[channel];
		break;
	case IT2_CMD_PANNING_SLIDE: // IT
		pent->cmd = XM_SAMPLER_CMD_PANNINGSLIDE;
		break;
	case IT2_CMD_RETRIGGER_NOTE:
		pent->cmd = XM_SAMPLER_CMD_RETRIG;
		break;
	case IT2_CMD_TREMOLO:
		pent->cmd = XM_SAMPLER_CMD_TREMOLO;
		break;
	case IT2_CMD_S:
		switch (param & 0xf0) {
		case IT2_CMD_S_SET_FILTER:
			pent->cmd = XM_SAMPLER_CMD_NONE;
			break;
		case IT2_CMD_S_SET_GLISSANDO_CONTROL:
			pent->cmd = XM_SAMPLER_CMD_EXTENDED;
			pent->parameter = XM_SAMPLER_CMD_E_GLISSANDO_TYPE | (param & 0xf);
			break;
		case IT2_CMD_S_FINETUNE:
			pent->cmd = XM_SAMPLER_CMD_NONE;
			break;
		case IT2_CMD_S_SET_VIBRATO_WAVEFORM:
			pent->cmd = XM_SAMPLER_CMD_EXTENDED;
			pent->parameter = XM_SAMPLER_CMD_E_VIBRATO_WAVE | exchwave[(param & 0x3)];
			break;
		case IT2_CMD_S_SET_TREMOLO_WAVEFORM:
			pent->cmd = XM_SAMPLER_CMD_EXTENDED;
			pent->parameter = XM_SAMPLER_CMD_E_TREMOLO_WAVE | exchwave[(param & 0x3)];
			break;
		case IT2_CMD_S_SET_PANBRELLO_WAVEFORM: // IT
			pent->cmd = XM_SAMPLER_CMD_EXTENDED;
			pent->parameter = XM_SAMPLER_CMD_E_PANBRELLO_WAVE | exchwave[(param & 0x3)];
			break;
		case IT2_CMD_S_FINE_PATTERN_DELAY: // IT
			break;
		case IT2_CMD_S7: // IT
			pent->cmd = XM_SAMPLER_CMD_EXTENDED;
			pent->parameter = XM_SAMPLER_CMD_EE | (param & 0x0F);
			break;
		case IT2_CMD_S_SET_PAN:
			pent->cmd = XM_SAMPLER_CMD_EXTENDED;
			pent->parameter = XM_SAMPLER_CMD_E_SET_PAN | (param & 0xf);
			break;
		case IT2_CMD_S9: // IT
			pent->cmd = XM_SAMPLER_CMD_EXTENDED;
			pent->parameter = XM_SAMPLER_CMD_E9 | (param & 0x0F);
			break;
		case IT2_CMD_S_SET_HIGH_OFFSET: // IT
			self->highoffset[channel] = param & 0x0F;
			pent->cmd = XM_SAMPLER_CMD_NONE;
			pent->parameter = 0;
			break;
		case IT2_CMD_S_PATTERN_LOOP:
			pent->cmd = psy_audio_PATTERNCMD_EXTENDED;
			pent->parameter = psy_audio_PATTERNCMD_PATTERN_LOOP | (param & 0xf);
			break;
		case IT2_CMD_S_DELAYED_NOTE_CUT:
			pent->cmd = XM_SAMPLER_CMD_EXTENDED;
			pent->parameter = XM_SAMPLER_CMD_E_DELAYED_NOTECUT | (param & 0xf);
			break;
		case IT2_CMD_S_NOTE_DELAY:
			pent->cmd = XM_SAMPLER_CMD_EXTENDED;
			pent->parameter = XM_SAMPLER_CMD_E_NOTE_DELAY | (param & 0xf);
			break;
		case IT2_CMD_S_PATTERN_DELAY:
			pent->cmd = psy_audio_PATTERNCMD_EXTENDED;
			pent->parameter = psy_audio_PATTERNCMD_PATTERN_DELAY | (param & 0xf);
			break;
		case IT2_CMD_S_SET_MIDI_MACRO:
			pent->cmd = XM_SAMPLER_CMD_EXTENDED;
			if (self->embeddeddata)
			{
				//std_string zxx = self->embeddeddata->SFx[(param & 0xF)];
				//std_string zxx2 = zxx.substr(0, 5);
				//if (zxx2 == "F0F00")
				//{
				//	int val = (embeddeddata->SFx[(param & 0xF)][5] - '0');
				//	if (val < 2) {
				//		pent->parameter = XM_SAMPLER_CMD_E_SET_MIDI_MACRO | val;
				//	}
				//}
			} else if ((param & 0xF) < 2) {
				pent->parameter = XM_SAMPLER_CMD_E_SET_MIDI_MACRO | (param & 0xf);
			}
			break;
		}
		break;
	case IT2_CMD_SET_SONG_TEMPO:
		pent->cmd = psy_audio_PATTERNCMD_SET_TEMPO;
		break;
	case IT2_CMD_FINE_VIBRATO:
		pent->cmd = XM_SAMPLER_CMD_FINE_VIBRATO;
		break;
	case IT2_CMD_SET_GLOBAL_VOLUME:
		pent->cmd = XM_SAMPLER_CMD_SET_GLOBAL_VOLUME;
		break;
	case IT2_CMD_GLOBAL_VOLUME_SLIDE: // IT
		pent->cmd = XM_SAMPLER_CMD_GLOBAL_VOLUME_SLIDE;
		break;
	case IT2_CMD_SET_PANNING: // IT
		pent->cmd = XM_SAMPLER_CMD_PANNING;
		break;
	case IT2_CMD_PANBRELLO: // IT
		pent->cmd = XM_SAMPLER_CMD_PANBRELLO;
		break;
	case IT2_CMD_MIDI_MACRO:
		if (param < 127)
		{
			pent->parameter = param;
		}
		pent->cmd = XM_SAMPLER_CMD_MIDI_MACRO;
		break;
	default:
		pent->cmd = XM_SAMPLER_CMD_NONE;
		break;
	}
	return rv_append;
}

//////////////////////////////////////////////////////////////////////////
//     S3M Module Members

static bool itmodule2_loads3msampledatax(ITModule2*, psy_audio_Sample*, uint32_t iLen, bool bstereo, bool b16Bit, bool packed);
static bool itmodule2_loads3msamplex(ITModule2* self, psy_audio_Sample* _wave, s3mSampleHeader* currHeader);

bool itmodule2_loads3minstx(ITModule2* self, uint16_t iSampleIdx)
{
	bool result = FALSE;
	s3mInstHeader curh;	
	psy_audio_Instrument* instr;
	PsyFile* fp;
	psy_audio_Song* song;

	fp = self->songfile->file;
	song = self->songfile->song;

	psyfile_read(fp, &curh, sizeof(curh));
	
	instr = psy_audio_instrument_allocinit();
	psy_audio_instrument_setindex(instr, iSampleIdx);
	psy_audio_instruments_insert(&song->instruments, instr,
		psy_audio_instrumentindex_make(0, iSampleIdx));
		
	psy_audio_instrument_setname(instr, curh.sName);	

	if (curh.tag == SCRS_ID && curh.type == 1)
	{
		psy_audio_Sample* wave;

		wave = psy_audio_sample_allocinit(1);
		psy_audio_samples_insert(&song->samples, wave,
			sampleindex_make(iSampleIdx, 0));
		result = itmodule2_loads3msamplex(self, wave, (s3mSampleHeader*)(&curh));		
		if (result) {
			// instr.SetDefaultNoteMap(iSampleIdx);
			// instr.ValidateEnabled();
		}
	} else if (curh.tag == SCRI_ID && curh.type != 0)
	{
		//reinterpret_cast<s3madlibheader*>(&curh)
	}
	//song.xminstruments.SetInst(instr, iSampleIdx);
	return result;
}

bool itmodule2_loads3msamplex(ITModule2* self, psy_audio_Sample* _wave, s3mSampleHeader* currHeader)
{
	bool bLoop = currHeader->flags & S3M_SAMPLE_FLAGS_LOOP;
	bool bstereo = currHeader->flags & S3M_SAMPLE_FLAGS_STEREO;
	bool b16Bit = currHeader->flags & S3M_SAMPLE_FLAGS_IS16BIT;
	PsyFile* fp;
	psy_audio_Song* song;
	int newpos;

	fp = self->songfile->file;
	song = self->songfile->song;

	//_wave.Init();

	_wave->loop.start = currHeader->loopb;
	_wave->loop.end = currHeader->loope;
	if (bLoop) {
		_wave->loop.type = psy_audio_SAMPLE_LOOP_NORMAL;
	} else {
		_wave->loop.type = psy_audio_SAMPLE_LOOP_DO_NOT;
	}

	_wave->globalvolume = (currHeader->vol * 2) / 255.f; // todo check scale

	_wave->samplerate = currHeader->c2speed;
	
	psy_audio_sample_setname(_wave, currHeader->filename);

	newpos = ((currHeader->hiMemSeg << 16) + currHeader->lomemSeg) << 4;
	psyfile_seek(fp, newpos);
	return itmodule2_loads3msampledatax(self, _wave, currHeader->length, bstereo, b16Bit, currHeader->packed);
}

bool itmodule2_loads3msampledatax(ITModule2* self, psy_audio_Sample* wave, uint32_t iLen, bool bstereo, bool b16Bit, bool packed)
{
	PsyFile* fp;
	psy_audio_Song* song;

	fp = self->songfile->file;
	song = self->songfile->song;

	if (!packed) // Looks like the packed format never existed.
	{
		char* smpbuf;
		int16_t wNew;
		int16_t offset;
		
		if (b16Bit)
		{
			smpbuf = malloc(iLen * 2);
			psyfile_read(fp, smpbuf, iLen * 2);
		} else {
			smpbuf = malloc(iLen);
			psyfile_read(fp, smpbuf, iLen);
		}
		
		if (self->s3mFileH.trackerInf == 1) offset = 0; // 1=[VERY OLD] signed samples, 2=unsigned samples
		else offset = -32768;

		//+2=stereo (after Length bytes for LEFT channel, another Length bytes for RIGHT channel)
		//+4=16-bit sample (intel LO-HI byteorder) (+2/+4 not supported by ST3.01)
		if (bstereo) {
			psy_audio_sample_resize(wave, 2);
		}
		wave->numframes = iLen;
		psy_audio_sample_allocwavedata(wave);
		if (b16Bit) {
			int out = 0;
			unsigned int j;

			for (j = 0; j < iLen * 2; j += 2, out++)
			{
				wNew = (0xFF & smpbuf[j] | smpbuf[j + 1] << 8) + offset;
				wave->channels.samples[0][out] = (psy_dsp_amp_t)wNew;				
			}
			if (bstereo) {
				out = 0;
				psyfile_read(fp, smpbuf, iLen * 2);
				for (j = 0; j < iLen * 2; j += 2, out++)
				{
					wNew = (0xFF & smpbuf[j] | smpbuf[j + 1] << 8) + offset;
					wave->channels.samples[1][out] = (psy_dsp_amp_t)wNew;					
				}
			}
		} else {// 8 bit sample
			unsigned int j;

			for (j = 0; j < iLen; j++)
			{
				wNew = (smpbuf[j] << 8) + offset;				
				wave->channels.samples[0][j] = (psy_dsp_amp_t)wNew;
			}
			if (bstereo) {
				psyfile_read(fp, smpbuf, iLen);
				for (j = 0; j < iLen; j++)
				{
					wNew = (smpbuf[j] << 8) + offset;
					wave->channels.samples[0][j] = (psy_dsp_amp_t)wNew;					
				}
			}
		}

		// cleanup
		free(smpbuf); smpbuf = 0;
		return TRUE;
	}
	return FALSE;
}

bool itmodule2_loads3mpatternx(ITModule2* self, uint16_t patidx)
{
	unsigned char lastmach[64];
	uint8_t newEntry;
	psy_audio_PatternEvent pempty;
	psy_audio_PatternEvent pent;
	psy_audio_Pattern* pattern;
	psy_audio_PatternNode* node;
	bool append;
	PsyFile* fp;
	psy_audio_Song* song;
	int row;

	fp = self->songfile->file;
	song = self->songfile->song;

	pempty.note = 255;
	pempty.inst = 255;
	pempty.mach = 255;	
	pempty.vol = psy_audio_NOTECOMMANDS_VOL_EMPTY;
	pempty.cmd = 0;
	pempty.parameter = 0;

	pent = pempty;
	memset(lastmach, 255, sizeof(char) * 64);	

	pattern = psy_audio_pattern_allocinit();
	psy_audio_pattern_setname(pattern, "unnamed");
	psy_audio_patterns_insert(&song->patterns, patidx, pattern);
	psy_audio_pattern_setlength(pattern,
		64 * 1.0 / self->songfile->song->properties.lpb);

	psyfile_skip(fp, 2);//int packedSize=ReadInt(2);
//			char* packedpattern = new char[packedsize];
//			Read(packedpattern,packedsize);
	node = NULL;
	for (row = 0; row < 64; row++)
	{
		self->extracolumn = (int16_t)psy_audio_song_numsongtracks(self->song);
		psyfile_read(fp, &newEntry, 1);
		append = TRUE;
		while (newEntry)
		{
			uint8_t channel = newEntry & 31;
			uint8_t volume = 255;
			if (newEntry & 32)
			{
				uint8_t note;
				uint8_t tmp;
				int status;
				
				// hi=oct, lo=note, 255=empty note,	254=key off
				if (status = psyfile_read(self->fp, &note, sizeof(uint8_t))) {
					return status;
				} 
				if (note == 254) pent.note = psy_audio_NOTECOMMANDS_RELEASE;
				else if (note == 255) pent.note = 255;
				else pent.note = ((note >> 4) * 12 + (note & 0xF) + 12);  // +12 since ST3 C-4 is Psycle's C-5
				if (status = psyfile_read(self->fp, &tmp, sizeof(uint8_t))) {
					return status;
				}
				pent.inst = tmp - 1;
			}
			if (newEntry & 64)
			{
				uint8_t tmp;
				int status;
				
				if (status = psyfile_read(self->fp, &tmp, sizeof(uint8_t))) {
					return status;
				} 
				volume = (tmp < 64) ? tmp : 63;
			}
			if (newEntry & 128)
			{
				uint8_t command;
				uint8_t param;
				int status;

				if (status = psyfile_read(self->fp, &command, sizeof(uint8_t))) {
					return status;
				}
				if (status = psyfile_read(self->fp, &param, sizeof(uint8_t))) {
					return status;
				}
				if (command != 0) pent.parameter = param;
				append = itmodule2_parseeffect(self, &pent, &node, pattern,
					patidx, row, command, param, channel);
				if (pent.cmd == psy_audio_PATTERNCMD_BREAK_TO_LINE)
				{
					pent.parameter = ((pent.parameter & 0xF0) >> 4) * 10 + (pent.parameter & 0x0F);
				} else if (pent.cmd == XM_SAMPLER_CMD_SET_GLOBAL_VOLUME)
				{
					pent.parameter = (pent.parameter < 0x40) ? pent.parameter * 2 : 0x80;
				} else if (pent.cmd == XM_SAMPLER_CMD_PANNING)
				{
					if (pent.parameter < 0x80) pent.parameter = pent.parameter * 2;
					else if (pent.parameter == 0x80) pent.parameter = 255;
					else if (pent.parameter == 0xA4)
					{
						pent.cmd = XM_SAMPLER_CMD_EXTENDED;
						pent.parameter = XM_SAMPLER_CMD_E9 | 1;
					}
				}
			}
			// If empty, do not inform machine
			if (pent.note == 255 && pent.cmd == 255 && volume == 255 && pent.cmd == 00 && pent.parameter == 00) {
				pent.mach = 255;
			}
			// if instrument without note, or note without instrument, cannot use virtual instrument, so use sampulse directly
			else if ((pent.note == 255 && pent.cmd != 255) || (pent.note < psy_audio_NOTECOMMANDS_RELEASE && pent.cmd == 255)) {
				pent.mach = 0;
				if (pent.cmd != 255) {
					//We cannot use the virtual instrument, but we should remember which it is.
					if (psy_table_exists(&self->xmtovirtual, pent.inst)) {
						lastmach[channel] = (uint8_t)(uintptr_t)psy_table_at(&self->xmtovirtual, pent.inst);
					}
				}
			}
			//default behaviour, let's find the virtual instrument.
			else {
				if (!psy_table_exists(&self->xmtovirtual, pent.inst)) {
					if (pent.inst != 255) {					
						pent.mach = 0;
						lastmach[channel] = pent.mach;
					} else if (lastmach[channel] != 255) {
						pent.mach = lastmach[channel];
					} else if (volume != 255) {
						pent.mach = 0;
					} else {
						pent.mach = 255;
					}
				} else {
					pent.mach = (uint8_t)(uintptr_t)psy_table_at(&self->xmtovirtual, pent.inst);
					pent.inst = 255;
					lastmach[channel] = pent.mach;
				}
			}

			if (pent.mach == 0) { // fallback to the old behaviour. This will happen only if an unused instrument is present in the pattern.
				if (pent.cmd != 0 || pent.parameter != 0) {
					if (volume != 255) {
						psy_audio_PatternEvent e;

						e.note = psy_audio_NOTECOMMANDS_MIDICC;
						e.inst = channel;
						e.mach = pent.mach;
						e.cmd = XM_SAMPLER_CMD_SENDTOVOLUME;
						e.parameter = volume;
						append = FALSE;
						itmodule_writepatternentry(self, &node, append, pattern,
							row, self->extracolumn, &e);
						self->extracolumn++;
					}
				} else if (volume < 0x40) {
					pent.cmd = XM_SAMPLER_CMD_VOLUME;
					pent.parameter = volume * 2;
				} else if (volume != 255) {
					pent.cmd = XM_SAMPLER_CMD_SENDTOVOLUME;
					pent.parameter = volume;
				}
			} else {
				pent.cmd = volume;
			}
			if (channel < psy_audio_song_numsongtracks(self->song)) {
				itmodule_writepatternentry(self, &node, append, pattern,
					row, channel, &pent);
			}
			pent = pempty;

			psyfile_read(fp, &newEntry, 1);
		}
		self->maxextracolumn = psy_max(self->maxextracolumn, self->extracolumn);
	}
	return TRUE;
}
