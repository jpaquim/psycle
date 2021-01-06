// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "midiloader.h"
// local
#include "pattern.h"
#include "patterns.h"
#include "song.h"
// std
#include <assert.h>
// platform
#include "../../detail/portable.h"

// MidiFile stores in big endian. The swap functions convert to little endian
// todo: combine duplicate code (asiodriver.cpp)
#define swaplong(v) ((((v)>>24)&0xFF)|(((v)>>8)&0xFF00)|(((v)&0xFF00)<<8)|(((v)&0xFF)<<24)) 
#define swapshort(v) ((((v)>>8)&0xFF)|(((v)&0xFF)<<8))

void miditrackstate_init(MidiTrackState* self)
{       
    miditrackstate_reset(self);
}

void miditrackstate_dispose(MidiTrackState* self)
{       
}

void miditrackstate_reset(MidiTrackState* self)
{    
    uint16_t v;

    self->trackidx = 0;
    self->tracknode = NULL;
    self->pattern = NULL;
    self->patternnode = NULL;    
    self->channel = 0;
    self->automationchannel = 1;
    self->position = 0.0;
    self->patternoffset = 0.0;
    self->runningstatus = 0;
    for (v = 0; v < MAX_MIDIFILE_POLYPHONY; ++v) {
        psy_audio_PatternEvent ev;

        psy_audio_patternevent_init(&ev);
        self->channels[v].tracknote = ev;
        self->channels[v].time = 0.0;
        self->channels[v].noteoff = FALSE;
    }    
}

// MidiLoader
// prototypes
static void midiloader_reset(MidiLoader*);
// Midi File
int midiloader_readmthd(MidiLoader*);
// Midi Track
static int midiloader_readtrk(MidiLoader*, MCHUNK, uintptr_t trackidx);
static void midiloader_appendtrack(MidiLoader*, uintptr_t trackidx);
static int midiloader_readtrackevents(MidiLoader*, MCHUNK, uintptr_t trackidx);
// Midi Events
static int midiloader_readdeltatime(MidiLoader*);
static int midiloader_readstatusbyte(MidiLoader*);
static int midiloader_readnoteon(MidiLoader*);
static int midiloader_readnoteoff(MidiLoader*);
static int midiloader_marknoteoff(MidiLoader*, uint8_t note);
static int midiloader_readpolyphonicpressure(MidiLoader*);
static int midiloader_readcontroller(MidiLoader*);
static int midiloader_readprogramchange(MidiLoader*);
static int midiloader_readchannelpressure(MidiLoader*);
static int midiloader_readpitchbend(MidiLoader*);
// Midi Meta Events
static int midiloader_readmetaevent(MidiLoader*, bool* rv_eoftrack);
static int midiloader_readmeta_text(MidiLoader*);
static int midiloader_readmeta_copyright(MidiLoader*);
static int midiloader_readmeta_sequencetrackname(MidiLoader*);
static int midiloader_readmeta_instrument(MidiLoader*);
static int midiloader_readmeta_lyric(MidiLoader*);
static int midiloader_readmeta_marker(MidiLoader*);
static int midiloader_readmeta_cuepoint(MidiLoader*);
static int midiloader_readmeta_programpatchname(MidiLoader*);
static int midiloader_readmeta_deviceportname(MidiLoader*);
static int midiloader_readmeta_midiport(MidiLoader*);
static int midiloader_readmeta_endoftrack(MidiLoader*);
static int midiloader_readmeta_tempo(MidiLoader*);
static int midiloader_readmeta_smpte(MidiLoader*);
static int midiloader_readmeta_timesignature(MidiLoader*);
static int midiloader_readmeta_keysignature(MidiLoader*);
static int midiloader_readmeta_properietaryevent(MidiLoader*);
// Misc methods
static  void midiloader_writepatternevent(MidiLoader*, psy_audio_PatternEvent);
static int midiloader_readchunk(MidiLoader*, MCHUNK* rv);
static int midiloader_readbyte1(MidiLoader*, uint8_t* rv_int);
int readvarlen(PsyFile* fp, uint32_t* rv);
static int midiloader_readvarlentext(PsyFile*, char_dyn_t** rv_text);

// implementation
void midiloader_init(MidiLoader* self, psy_audio_SongFile* songfile)
{
	assert(self);

	self->songfile = songfile;	        
	midiloader_reset(self);
}

void midiloader_dispose(MidiLoader* self)
{    
	assert(self);	
}

void midiloader_reset(MidiLoader* self)
{
    miditrackstate_reset(&self->currtrack);
}

int midiloader_load(MidiLoader* self)
{
    int status;    
    uint16_t currtrack;    
    
    status = PSY_OK;
    psy_audio_song_setbpm(self->songfile->song, 120.0);
    status = midiloader_readmthd(self);    
    currtrack = 0;
    while (!psyfile_eof(self->fp) && currtrack < self->mthd.numtracks) {
        MCHUNK mtrkchunk;
        char idstr[5];
        uint32_t chunkstart;

        if (status = midiloader_readchunk(self, &mtrkchunk)) {
            break;
        }
        chunkstart = psyfile_getpos(self->fp);
        memcpy(idstr, mtrkchunk.id, 4);
        idstr[4] = '\0';
        if (strcmp(idstr, "MTrk") == 0) {
            if (status = midiloader_readtrk(self, mtrkchunk, currtrack)) {
                break;
            }
            ++currtrack;
        }
        if (mtrkchunk.length > (psyfile_getpos(self->fp) - chunkstart)) {
            psyfile_skip(self->fp,
                mtrkchunk.length - (psyfile_getpos(self->fp) - chunkstart));
        }
    }    
    return status;
}

int midiloader_readmthd(MidiLoader* self)
{
    MCHUNK mthdchunk;
    int status;
    
    self->fp = self->songfile->file;
    if (status = midiloader_readchunk(self, &mthdchunk)) {
        return status;
    }
    if (!psyfile_read(self->fp, &self->mthd.format, 2)) {
        return PSY_ERRFILE;
    }
    self->mthd.format = swapshort(self->mthd.format);
    if (!psyfile_read(self->fp, &self->mthd.numtracks, 2)) {
        return PSY_ERRFILE;
    }
    self->mthd.numtracks = swapshort(self->mthd.numtracks);
    if (!psyfile_read(self->fp, &self->mthd.division, 2)) {
        return PSY_ERRFILE;
    }
    self->mthd.division = swapshort(self->mthd.division);
    return PSY_OK;
}

int midiloader_readtrk(MidiLoader* self, MCHUNK chunk, uintptr_t trackidx)
{    
    int status;    
    bool eoftrack;    
       
    status = PSY_OK;
    eoftrack = FALSE;       
    midiloader_appendtrack(self, trackidx);
    if (status = midiloader_readtrackevents(self, chunk, trackidx)) {
        return status;
    }
    if (self->currtrack.pattern) {
        if (self->currtrack.patternnode) {
            psy_audio_PatternEntry* lastentry;

            lastentry = (psy_audio_PatternEntry*)self->currtrack.patternnode->entry;
            psy_audio_pattern_setlength(self->currtrack.pattern, lastentry->offset + 1.0);
        } else {
            psy_audio_pattern_setlength(self->currtrack.pattern, 16.0);
        }
    }
    return status;
}

void midiloader_appendtrack(MidiLoader* self, uintptr_t trackidx)
{
    psy_audio_Pattern* pattern;
    psy_audio_SequencePosition sequenceposition;
    psy_audio_Song* song;
    uintptr_t patidx;
    
    song = self->songfile->song;
    // reset trackstate
    miditrackstate_reset(&self->currtrack);
    self->currtrack.trackidx = trackidx;    
    // create pattern
    pattern = psy_audio_pattern_allocinit();
    psy_audio_pattern_setname(pattern, "unnamed");
    patidx = psy_audio_patterns_append(&song->patterns, pattern);
    // append new sequence track 
    sequenceposition.tracknode = psy_audio_sequence_appendtrack(
        &song->sequence, psy_audio_sequencetrack_allocinit());
    sequenceposition.trackposition = psy_audio_sequence_begin(&song->sequence,
        sequenceposition.tracknode, 0.0);
    psy_audio_sequence_insert(&song->sequence, sequenceposition, patidx);
    // prepare currpattern            
    self->currtrack.pattern = pattern;
    self->currtrack.tracknode = sequenceposition.tracknode;
    // insert pattern    
    self->currtrack.position = 0.0;
    self->currtrack.patternnode = NULL;    
}

int midiloader_readtrackevents(MidiLoader* self, MCHUNK chunk, uintptr_t trackidx)
{
    int status;
    uint32_t currentpos;
    bool eoftrack;

    status = PSY_OK;
    eoftrack = FALSE;
    currentpos = psyfile_getpos(self->fp);
    // insert events   
    while (!eoftrack && !psyfile_eof(self->fp) ||
            (psyfile_getpos(self->fp) - currentpos < chunk.length)) {
        uint32_t ln;
        uint32_t hn;

        if (status = midiloader_readdeltatime(self)) {
            return status;
        }        
        if (status = midiloader_readstatusbyte(self)) {
            return status;
        }
        ln = self->currtrack.runningstatus & 0x0F;
        hn = (self->currtrack.runningstatus & 0xF0) >> 4;
        self->currtrack.channel = (uint8_t)ln;
        switch (hn) {
        case 0x8:
            status = midiloader_readnoteoff(self);
            break;
        case 0x9:
            status = midiloader_readnoteon(self);
            break;
        case 0xA:
            status = midiloader_readpolyphonicpressure(self);
            break;
        case 0xB:
            status = midiloader_readcontroller(self);
            break;
        case 0xC:
            status = midiloader_readprogramchange(self);
            break;
        case 0xD:
            status = midiloader_readchannelpressure(self);
            break;
        case 0xE:
            status = midiloader_readpitchbend(self);
            break;
        case 0xF:
            if (ln == 0xF) {
                status = midiloader_readmetaevent(self, &eoftrack);
            } else {
                psyfile_skip(self->fp, 1);
            }
            break;
        default:
            if (!psyfile_eof(self->fp)) {
                psyfile_skip(self->fp, 1);
            }
            break;
        }
    }
    if (self->currtrack.pattern) {
        if (self->currtrack.patternnode) {
            psy_audio_PatternEntry* lastentry;

            lastentry = (psy_audio_PatternEntry*)self->currtrack.patternnode->entry;
            psy_audio_pattern_setlength(self->currtrack.pattern, lastentry->offset + 1.0);
        } else {
            psy_audio_pattern_setlength(self->currtrack.pattern, 16.0);
        }
    }
    return status;
}

int midiloader_readstatusbyte(MidiLoader* self)
{    
    uint8_t statusbyte;

    if (!psyfile_read(self->fp, &statusbyte, 1)) {
        return PSY_ERRFILE;
    }
    if (statusbyte < 0x80) {        
        self->currtrack.hasrunningstatus = TRUE;
        self->currtrack.byte1 = statusbyte;
    } else {
        self->currtrack.runningstatus = statusbyte;
        self->currtrack.hasrunningstatus = FALSE;
    }
    return PSY_OK;
}

int midiloader_readdeltatime(MidiLoader* self)
{
    int status;
    uint32_t deltatime;
    psy_dsp_big_beat_t offset;    

    if (status = readvarlen(self->fp, &deltatime)) {
        return status;
    }
    offset = deltatime / (psy_dsp_big_beat_t)self->mthd.division;
    self->currtrack.position += offset;      
    return PSY_OK;
}

int midiloader_readnoteon(MidiLoader* self)
{
    int status;
    uint8_t note;
    uint8_t vol;    
    psy_audio_PatternEvent ev;

    if (status = midiloader_readbyte1(self, &note)) {
        return status;
    }
    if (!psyfile_read(self->fp, &vol, 1)) {
        return PSY_ERRFILE;
    }
    if (vol == 0) {
        midiloader_marknoteoff(self, note);
    } else {
        psy_audio_patternevent_init(&ev);
        ev.note = note;
        ev.mach = self->currtrack.channel;
        if (vol != 64) {
            ev.cmd = 0xC;
            ev.parameter = (vol / (float)0x7F) * 0x80;
        }
        midiloader_writepatternevent(self, ev);
    }
    return PSY_OK;
}

int midiloader_readnoteoff(MidiLoader* self)
{
    int status;
    uint8_t note;
    uint8_t vol;    

    if (status = midiloader_readbyte1(self, &note)) {
        return status;
    }
    if (!psyfile_read(self->fp, &vol, 1)) {
        return PSY_ERRFILE;
    }
    midiloader_marknoteoff(self, note);
    return PSY_OK;
}

int midiloader_marknoteoff(MidiLoader* self, uint8_t note)
{
    uint16_t voice;
    voice = 0;
    for (voice = 0; voice < MAX_MIDIFILE_POLYPHONY; ++voice) {
        if (self->currtrack.channels[voice].tracknote.note == note) {
            self->currtrack.channels[voice].noteoff = TRUE;
            self->currtrack.channels[voice].time = self->currtrack.position;
            break;
        }
    }
    return voice;
}

int midiloader_readpolyphonicpressure(MidiLoader* self)
{
    int status;
    uint8_t byte1;
    uint8_t byte2;        

    if (status = midiloader_readbyte1(self, &byte1)) {
        return status;
    }
    if (!psyfile_read(self->fp, &byte2, 1)) {
        return PSY_ERRFILE;
    }
    return PSY_OK;
}

int midiloader_readcontroller(MidiLoader* self)
{    
    int status;
    uint8_t controller;
    uint8_t value;    
    psy_audio_PatternEvent ev;
    psy_audio_PatternNode* node;
    psy_audio_PatternNode* prev;
    
    if (status = midiloader_readbyte1(self, &controller)) {
        return status;
    }
    if (!psyfile_read(self->fp, &value, 1)) {
        return PSY_ERRFILE;
    }
    psy_audio_patternevent_init(&ev);
    ev.note = psy_audio_NOTECOMMANDS_MIDICC;
    ev.inst = self->currtrack.runningstatus;
    ev.mach = self->currtrack.channel;
    ev.cmd = controller;
    ev.parameter = value;            
    node = psy_audio_pattern_findnode(self->currtrack.pattern, 0,
        self->currtrack.position, 0.01, &prev);
    if (node) {
        psy_audio_PatternEntry* entry;

        entry = (psy_audio_PatternEntry*)node->entry;
        psy_audio_patternentry_addevent(entry, &ev);
    } else {
        psy_audio_pattern_insert(
            self->currtrack.pattern, self->currtrack.patternnode,
            0, self->currtrack.position - self->currtrack.patternoffset, &ev);
        self->currtrack.patternnode = self->currtrack.pattern->events->tail;
    }
    return PSY_OK;
}

int midiloader_readprogramchange(MidiLoader* self)
{
    int status;
    uint8_t byte1;    

    if (status = midiloader_readbyte1(self, &byte1)) {
        return status;
    }    
    return PSY_OK;
}

int midiloader_readchannelpressure(MidiLoader* self)
{
    int status;
    uint8_t byte1;

    if (status = midiloader_readbyte1(self, &byte1)) {
        return status;
    }
    return PSY_OK;
}

int midiloader_readpitchbend(MidiLoader* self)
{
    int status;
    uint8_t byte1;
    uint8_t byte2;    

    if (status = midiloader_readbyte1(self, &byte1)) {
        return status;
    }
    if (!psyfile_read(self->fp, &byte2, 1)) {
        return PSY_ERRFILE;
    }    
    return PSY_OK;
}

int midiloader_readmetaevent(MidiLoader* self, bool* rv_eoftrack)
{
    int status;
    uint32_t len;
    uint8_t byte1;    
    
    status = PSY_OK;
    *rv_eoftrack = FALSE;
    if (status = midiloader_readbyte1(self, &byte1)) {
        return status;
    }
    switch (byte1) {
        case 0x1:
            status = midiloader_readmeta_text(self);
            break;
        case 0x2:
            status = midiloader_readmeta_copyright(self);
            break;
        case 0x3:
            status = midiloader_readmeta_sequencetrackname(self);
            break;
        case 0x4:
            status = midiloader_readmeta_instrument(self);
            break;
        case 0x5:
            status = midiloader_readmeta_lyric(self);
            break;
        case 0x6:
            status = midiloader_readmeta_marker(self);
            break;
        case 0x7:
            status = midiloader_readmeta_cuepoint(self);
            break;
        case 0x8:
            status = midiloader_readmeta_programpatchname(self);
            break;
        case 0x9:
            status = midiloader_readmeta_deviceportname(self);
            break;
        case 0x21:
            status = midiloader_readmeta_midiport(self);
            break;
        case 0x2F:
            status = midiloader_readmeta_endoftrack(self);
            *rv_eoftrack = TRUE;
            break;
        case 0x51:
            status = midiloader_readmeta_tempo(self);
            break;
        case 0x54:
            status = midiloader_readmeta_smpte(self);
            break;
        case 0x58:
            status = midiloader_readmeta_timesignature(self);
            break;
        case 0x59:
            status = midiloader_readmeta_keysignature(self);
            break;
        case 0x7F:
            status = midiloader_readmeta_properietaryevent(self);
            break;    
        default:
            status = readvarlen(self->fp, &len);
            if (status == PSY_OK) {
                psyfile_skip(self->fp, len);
            }
            break;
    }    
    return status;
}

int midiloader_readmeta_text(MidiLoader* self)
{
    int status;
    char_dyn_t* text;

    if (status = midiloader_readvarlentext(self->fp, &text)) {
        return status;
    }
    psy_audio_song_setcomments(self->songfile->song, text);
    free(text);
    return PSY_OK;
}

int midiloader_readmeta_copyright(MidiLoader* self)
{
    int status;
    char_dyn_t* text;

    if (status = midiloader_readvarlentext(self->fp, &text)) {
        return status;
    }
    psy_audio_song_setcredits(self->songfile->song, text);
    free(text);
    return PSY_OK;
}
 
int midiloader_readmeta_sequencetrackname(MidiLoader* self)
{
    int status;
    char_dyn_t* text;
    psy_audio_SequenceTrack* track;

    if (status = midiloader_readvarlentext(self->fp, &text)) {
        return status;
    }
    track = (psy_audio_SequenceTrack*)self->currtrack.tracknode->entry;
    psy_audio_sequencetrack_setname(track, text);
    psy_audio_pattern_setname(self->currtrack.pattern, text);
    free(text);
    return PSY_OK;
}

int midiloader_readmeta_instrument(MidiLoader* self)
{
    int status;
    char_dyn_t* text;

    if (status = midiloader_readvarlentext(self->fp, &text)) {
        return status;
    }
    free(text);
    return PSY_OK;
}
 
int midiloader_readmeta_lyric(MidiLoader* self)
{
    int status;
    char_dyn_t* text;

    if (status = midiloader_readvarlentext(self->fp, &text)) {
        return status;
    }
    free(text);
    return PSY_OK;
}
 
int midiloader_readmeta_marker(MidiLoader* self)
{
    int status;
    char_dyn_t* text;

    if (status = midiloader_readvarlentext(self->fp, &text)) {
        return status;
    }
    free(text);
    return PSY_OK;
}
 
int midiloader_readmeta_cuepoint(MidiLoader* self)
{
    int status;
    char_dyn_t* text;

    if (status = midiloader_readvarlentext(self->fp, &text)) {
        return status;
    }
    free(text);
    return PSY_OK;
}
 
int midiloader_readmeta_programpatchname(MidiLoader* self)
{
    int status;
    char_dyn_t* text;

    if (status = midiloader_readvarlentext(self->fp, &text)) {
        return status;
    }
    free(text);
    return PSY_OK;
}
 
int midiloader_readmeta_deviceportname(MidiLoader* self)
{
    int status;
    char_dyn_t* text;

    if (status = midiloader_readvarlentext(self->fp, &text)) {
        return status;
    }
    free(text);
    return PSY_OK;
}
 
int midiloader_readmeta_midiport(MidiLoader* self)
{
    uint8_t pp;    

    psyfile_skip(self->fp, 1);
    if (!psyfile_read(self->fp, &pp, 1)) {
        return PSY_ERRFILE;
    }
    return PSY_OK;
}

int midiloader_readmeta_endoftrack(MidiLoader* self)
{
    psyfile_skip(self->fp, 1);
    return PSY_OK;
}

int midiloader_readmeta_tempo(MidiLoader* self)
{    
    uint8_t tt1;
    uint8_t tt2;
    uint8_t tt3;
    uint32_t microsecsperquarternote;
    psy_dsp_big_beat_t bpm;

    psyfile_skip(self->fp, 1);
    if (!psyfile_read(self->fp, &tt1, 1)) {
        return PSY_ERRFILE;
    }
    if (!psyfile_read(self->fp, &tt2, 1)) {
        return PSY_ERRFILE;
    }
    if (!psyfile_read(self->fp, &tt3, 1)) {
        return PSY_ERRFILE;
    }
    microsecsperquarternote = tt3 | (tt2 << 8) | (tt1 << 16);
    bpm = 60.0 * 1000000.0 / (microsecsperquarternote);
    bpm = (uintptr_t)(bpm * 1000) / 1000.0f;
    if (self->currtrack.position == 0.0) {
        // tempo event at track start: set change as song tempo
        psy_audio_song_setbpm(self->songfile->song, bpm);
    } else {
        // tempo event inside track: insert pattern bpm change
        psy_audio_PatternEvent ev;

        psy_audio_patternevent_init(&ev);
        ev.cmd = psy_audio_PATTERNCMD_SET_TEMPO;
        ev.parameter = (uint8_t)bpm;
        self->currtrack.patternnode = psy_audio_pattern_insert(self->currtrack.pattern,
            self->currtrack.patternnode, self->currtrack.automationchannel,
            self->currtrack.position - self->currtrack.patternoffset, &ev);
    }
    return PSY_OK;
}

int midiloader_readmeta_smpte(MidiLoader* self)
{    
    uint8_t hr;
    uint8_t mn;
    uint8_t se;
    uint8_t fr;
    uint8_t ff;

    psyfile_skip(self->fp, 1);    
    if (!psyfile_read(self->fp, &hr, 1)) {
        return PSY_ERRFILE;
    }
    if (!psyfile_read(self->fp, &mn, 1)) {
        return PSY_ERRFILE;
    }
    if (!psyfile_read(self->fp, &se, 1)) {
        return PSY_ERRFILE;
    }
    if (!psyfile_read(self->fp, &fr, 1)) {
        return PSY_ERRFILE;
    }
    if (!psyfile_read(self->fp, &ff, 1)) {
        return PSY_ERRFILE;
    }
    return PSY_OK;
}

int midiloader_readmeta_timesignature(MidiLoader* self)
{
    // Time Signature
    uint8_t nn;
    uint8_t dd;
    uint8_t cc;
    uint8_t bb;

    psyfile_skip(self->fp, 1);
    if (!psyfile_read(self->fp, &nn, 1)) {
        return PSY_ERRFILE;
    }
    if (!psyfile_read(self->fp, &dd, 1)) {
        return PSY_ERRFILE;
    }
    if (!psyfile_read(self->fp, &cc, 1)) {
        return PSY_ERRFILE;
    }
    if (!psyfile_read(self->fp, &bb, 1)) {
        return PSY_ERRFILE;
    }
    return PSY_OK;
}

int midiloader_readmeta_keysignature(MidiLoader* self)
{
    uint8_t sf;
    uint8_t mi;

    // Key Signature
    psyfile_skip(self->fp, 1);
    if (!psyfile_read(self->fp, &sf, 1)) {
        return PSY_ERRFILE;
    }
    if (!psyfile_read(self->fp, &mi, 1)) {
        return PSY_ERRFILE;
    }
    return PSY_OK;
}

int midiloader_readmeta_properietaryevent(MidiLoader* self)
{   
    int status;
    uint32_t length;

    if (status = readvarlen(self->fp, &length)) {
        return status;
    }
    psyfile_skip(self->fp, length);
    return PSY_OK;
}

// misc midi file read
int midiloader_readchunk(MidiLoader* self, MCHUNK* rv)
{    
    assert(rv);
    
    if (!psyfile_read(self->fp, &rv->id, sizeof(rv->id))) {
        return PSY_ERRFILE;
    }
    if (!psyfile_read(self->fp, &rv->length, sizeof(rv->length))) {
        return PSY_ERRFILE;
    }
    rv->length = swaplong(rv->length);
    return PSY_OK;
}

int midiloader_isvalid(MidiLoader* self)
{			
	return FALSE;
}

// reads the first byte or uses the running status
int midiloader_readbyte1(MidiLoader* self, uint8_t* rv)
{
    if (self->currtrack.hasrunningstatus) {
        *rv = self->currtrack.byte1;
    } else if (!psyfile_read(self->fp, rv, 1)) {
        return PSY_ERRFILE;
    }
    return PSY_OK;
}

void midiloader_writepatternevent(MidiLoader* self, psy_audio_PatternEvent ev)
{
    uint16_t voice;
    uint16_t channelvoice;

    channelvoice = MAX_MIDIFILE_POLYPHONY;
    if (self->currtrack.patternoffset != self->currtrack.position) {
        for (voice = 0; voice < MAX_MIDIFILE_POLYPHONY; ++voice) {
            if (self->currtrack.channels[voice].noteoff) {
                if (self->currtrack.channels[voice].time < self->currtrack.position) {
                    if (self->currtrack.channels[voice].time >= self->currtrack.patternoffset) {
                        psy_audio_PatternEvent noteoff;
                        psy_audio_PatternNode* node;
                        psy_audio_PatternNode* prev;

                        psy_audio_patternevent_init(&noteoff);
                        noteoff.note = psy_audio_NOTECOMMANDS_RELEASE;
                        noteoff.mach = self->currtrack.channel;
                        node = psy_audio_pattern_findnode(self->currtrack.pattern,
                            voice, self->currtrack.channels[voice].time - self->currtrack.patternoffset, 0.05,
                            &prev);
                        if (!node) {
                            node = psy_audio_pattern_insert(self->currtrack.pattern,
                                prev, voice,
                                self->currtrack.channels[voice].time - self->currtrack.patternoffset,
                                &noteoff);
                            self->currtrack.patternnode = self->currtrack.pattern->events->tail;
                        }
                    } else {
                        // noteoff was in previous patttern
                        // todo
                    }                
                }
                channelvoice = voice;
                self->currtrack.channels[voice].noteoff = FALSE;
            }
        }
    }
    if (channelvoice == MAX_MIDIFILE_POLYPHONY) {
        for (channelvoice = 1; channelvoice < MAX_MIDIFILE_POLYPHONY; ++channelvoice) {
            if (self->currtrack.channels[channelvoice].tracknote.note == psy_audio_NOTECOMMANDS_EMPTY) {
                break;
            }
        }
    }
    if (channelvoice != MAX_MIDIFILE_POLYPHONY) {
        self->currtrack.channels[channelvoice].tracknote = ev;
        self->currtrack.patternnode = psy_audio_pattern_insert(
            self->currtrack.pattern, self->currtrack.patternnode,
            channelvoice,
            self->currtrack.position - self->currtrack.patternoffset, &ev);
    }
}


// midi variable length fileio funtions
int midiloader_readvarlentext(PsyFile* fp, char_dyn_t** rv)
{
    int status;
    uint32_t length;

    assert(rv);
    *rv = NULL;
    if (status = readvarlen(fp, &length)) {
        return status;
    }
    if (length) {        
        uint32_t size;

        size = length + 1;
        *rv = (char_dyn_t*)malloc(size);
        if (*rv) {
            uint32_t i;

            memset(*rv, 0, size);
            for (i = 0; i < length; ++i) {
                if (!psyfile_read(fp, &((*rv)[i]), 1)) {
                    free(*rv);
                    return PSY_ERRFILE;
                }
            }            
        } else {
            psyfile_skip(fp, length);
            return PSY_ERRFILE;
        }
    }
    return PSY_OK;
}

int readvarlen(PsyFile* fp, uint32_t* rv)
{
    uint32_t value;
    uint8_t temp;
    unsigned char c;

    if (!psyfile_read(fp, &temp, 1)) {
        *rv = 0;
        return PSY_ERRFILE;
    }
    value = temp;
    if (value & 0x80)
    {
        value &= 0x7F;
        do
        {
            if (!psyfile_read(fp, &temp, 1)) {
                return PSY_ERRFILE;
            }
            value = (value << 7) + ((c = temp) & 0x7F);
        } while (c & 0x80);
    }
    *rv = value;
    return PSY_OK;
}
