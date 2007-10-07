/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *  
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */

#ifndef _FLUID_MIDI_H
#define _FLUID_MIDI_H

#include "fluidsynth_priv.h"
#include "fluid_sys.h"
#include "fluid_list.h"

typedef struct _fluid_midi_parser_t fluid_midi_parser_t;

fluid_midi_parser_t* new_fluid_midi_parser(void);
int delete_fluid_midi_parser(fluid_midi_parser_t* parser);
fluid_midi_event_t* fluid_midi_parser_parse(fluid_midi_parser_t* parser, unsigned char c);

int fluid_midi_send_event(fluid_synth_t* synth, fluid_player_t* player, fluid_midi_event_t* evt);


/***************************************************************
 *
 *                   CONSTANTS & ENUM
 */


#define MAX_NUMBER_OF_TRACKS 128

enum fluid_midi_event_type {
  /* channel messages */
  NOTE_OFF = 0x80,
  NOTE_ON = 0x90,
  KEY_PRESSURE = 0xa0,
  CONTROL_CHANGE = 0xb0,
  PROGRAM_CHANGE = 0xc0,
  CHANNEL_PRESSURE = 0xd0,
  PITCH_BEND = 0xe0,
  /* system exclusive */
  MIDI_SYSEX = 0xf0,
  /* system common - never in midi files */
  MIDI_TIME_CODE = 0xf1,
  MIDI_SONG_POSITION = 0xf2,
  MIDI_SONG_SELECT = 0xf3,
  MIDI_TUNE_REQUEST = 0xf6,
  MIDI_EOX = 0xf7,
  /* system real-time - never in midi files */
  MIDI_SYNC = 0xf8,
  MIDI_TICK = 0xf9,
  MIDI_START = 0xfa,
  MIDI_CONTINUE = 0xfb,
  MIDI_STOP = 0xfc,
  MIDI_ACTIVE_SENSING = 0xfe,
  MIDI_SYSTEM_RESET = 0xff,
  /* meta event - for midi files only */
  MIDI_META_EVENT = 0xff
};

enum fluid_midi_control_change {
  BANK_SELECT_MSB = 0x00,
  MODULATION_MSB = 0x01,
  BREATH_MSB = 0x02,
  FOOT_MSB = 0x04,
  PORTAMENTO_TIME_MSB = 0x05,
  DATA_ENTRY_MSB = 0x06,
  VOLUME_MSB = 0x07,
  BALANCE_MSB = 0x08,
  PAN_MSB = 0x0A,
  EXPRESSION_MSB = 0x0B,
  EFFECTS1_MSB = 0x0C,
  EFFECTS2_MSB = 0x0D,
  GPC1_MSB = 0x10, /* general purpose controller */
  GPC2_MSB = 0x11,
  GPC3_MSB = 0x12,
  GPC4_MSB = 0x13,
  BANK_SELECT_LSB = 0x20,
  MODULATION_WHEEL_LSB = 0x21,
  BREATH_LSB = 0x22,
  FOOT_LSB = 0x24,
  PORTAMENTO_TIME_LSB = 0x25,
  DATA_ENTRY_LSB = 0x26,
  VOLUME_LSB = 0x27,
  BALANCE_LSB = 0x28,
  PAN_LSB = 0x2A,
  EXPRESSION_LSB = 0x2B,
  EFFECTS1_LSB = 0x2C,
  EFFECTS2_LSB = 0x2D,
  GPC1_LSB = 0x30,
  GPC2_LSB = 0x31,
  GPC3_LSB = 0x32,
  GPC4_LSB = 0x33,
  SUSTAIN_SWITCH = 0x40,
  PORTAMENTO_SWITCH = 0x41,
  SOSTENUTO_SWITCH = 0x42,
  SOFT_PEDAL_SWITCH = 0x43,
  LEGATO_SWITCH = 0x45,
  HOLD2_SWITCH = 0x45,
  SOUND_CTRL1 = 0x46,
  SOUND_CTRL2 = 0x47,
  SOUND_CTRL3 = 0x48,
  SOUND_CTRL4 = 0x49,
  SOUND_CTRL5 = 0x4A,
  SOUND_CTRL6 = 0x4B,
  SOUND_CTRL7 = 0x4C,
  SOUND_CTRL8 = 0x4D,
  SOUND_CTRL9 = 0x4E,
  SOUND_CTRL10 = 0x4F,
  GPC5 = 0x50,
  GPC6 = 0x51,
  GPC7 = 0x52,
  GPC8 = 0x53,
  PORTAMENTO_CTRL = 0x54,
  EFFECTS_DEPTH1 = 0x5B,
  EFFECTS_DEPTH2 = 0x5C,
  EFFECTS_DEPTH3 = 0x5D,
  EFFECTS_DEPTH4 = 0x5E,
  EFFECTS_DEPTH5 = 0x5F,
  DATA_ENTRY_INCR = 0x60,
  DATA_ENTRY_DECR = 0x61,
  NRPN_LSB = 0x62,
  NRPN_MSB = 0x63,
  RPN_LSB = 0x64,
  RPN_MSB = 0x65,
  ALL_SOUND_OFF = 0x78,
  ALL_CTRL_OFF = 0x79,
  LOCAL_CONTROL = 0x7A,
  ALL_NOTES_OFF = 0x7B,
  OMNI_OFF = 0x7C,
  OMNI_ON = 0x7D,
  POLY_OFF = 0x7E,
  POLY_ON = 0x7F
};

enum midi_meta_event {
  MIDI_COPYRIGHT = 0x02,
  MIDI_TRACK_NAME = 0x03,
  MIDI_INST_NAME = 0x04,
  MIDI_LYRIC = 0x05,
  MIDI_MARKER = 0x06,
  MIDI_CUE_POINT = 0x07,
  MIDI_EOT = 0x2f,
  MIDI_SET_TEMPO = 0x51,
  MIDI_SMPTE_OFFSET = 0x54,
  MIDI_TIME_SIGNATURE = 0x58,
  MIDI_KEY_SIGNATURE = 0x59,
  MIDI_SEQUENCER_EVENT = 0x7f
};

enum fluid_player_status 
{
  FLUID_PLAYER_READY,
  FLUID_PLAYER_PLAYING,
  FLUID_PLAYER_DONE
};

enum fluid_driver_status 
{
  FLUID_MIDI_READY,
  FLUID_MIDI_LISTENING,
  FLUID_MIDI_DONE
};

/***************************************************************
 *
 *         TYPE DEFINITIONS & FUNCTION DECLARATIONS
 */

/* From ctype.h */
#define fluid_isascii(c)    (((c) & ~0x7f) == 0)  



/*
 * fluid_midi_event_t
 */
struct _fluid_midi_event_t {
  fluid_midi_event_t* next;  /* Don't use it, it will dissappear. Used in midi tracks.  */
  unsigned int dtime;       /* Delay (ticks) between this and previous event. midi tracks. */
  unsigned char type;       /* MIDI event type */
  unsigned char channel;    /* MIDI channel */
  unsigned int param1;      /* First parameter */
  unsigned int param2;      /* Second parameter */
};


/*
 * fluid_track_t
 */
struct _fluid_track_t {
  char* name;
  int num;
  fluid_midi_event_t *first;
  fluid_midi_event_t *cur;
  fluid_midi_event_t *last;
  unsigned int ticks;
};

typedef struct _fluid_track_t fluid_track_t;

fluid_track_t* new_fluid_track(int num);
int delete_fluid_track(fluid_track_t* track);
int fluid_track_set_name(fluid_track_t* track, char* name);
char* fluid_track_get_name(fluid_track_t* track);
int fluid_track_add_event(fluid_track_t* track, fluid_midi_event_t* evt);
fluid_midi_event_t* fluid_track_first_event(fluid_track_t* track);
fluid_midi_event_t* fluid_track_next_event(fluid_track_t* track);
int fluid_track_get_duration(fluid_track_t* track);
int fluid_track_reset(fluid_track_t* track);

int fluid_track_send_events(fluid_track_t* track, 
			   fluid_synth_t* synth,
			   fluid_player_t* player,
			   unsigned int ticks);

#define fluid_track_eot(track)  ((track)->cur == NULL)


/*
 * fluid_player
 */
struct _fluid_player_t {
  int status;
  int loop;
  int ntracks;
  fluid_track_t *track[MAX_NUMBER_OF_TRACKS];
  fluid_synth_t* synth;
  fluid_timer_t* timer;
  fluid_list_t* playlist;
  char* current_file;
  char send_program_change; /* should we ignore the program changes? */
  int start_ticks;          /* the number of tempo ticks passed at the last tempo change */
  int cur_ticks;            /* the number of tempo ticks passed */
  int begin_msec;           /* the time (msec) of the beginning of the file */
  int start_msec;           /* the start time of the last tempo change */
  int cur_msec;             /* the current time */
  int miditempo;            /* as indicated by MIDI SetTempo: n 24th of a usec per midi-clock. bravo! */
  double deltatime;         /* milliseconds per midi tick. depends on set-tempo */
  unsigned int division;
};

int fluid_player_add_track(fluid_player_t* player, fluid_track_t* track);
int fluid_player_callback(void* data, unsigned int msec);
int fluid_player_count_tracks(fluid_player_t* player);
fluid_track_t* fluid_player_get_track(fluid_player_t* player, int i);
int fluid_player_reset(fluid_player_t* player);
int fluid_player_load(fluid_player_t* player, char *filename);


/*
 * fluid_midi_file
 */
typedef struct {
  fluid_file fp;
  int running_status;
  int c;
  int type;
  int ntracks;
  int uses_smpte;
  unsigned int smpte_fps;
  unsigned int smpte_res;
  unsigned int division;       /* If uses_SMPTE == 0 then division is 
				  ticks per beat (quarter-note) */
  double tempo;                /* Beats per second (SI rules =) */
  int tracklen;
  int trackpos;
  int eot;
  int varlen;
} fluid_midi_file;

fluid_midi_file* new_fluid_midi_file(char* filename);
void delete_fluid_midi_file(fluid_midi_file* mf);
int fluid_midi_file_read_mthd(fluid_midi_file* midifile);
int fluid_midi_file_load_tracks(fluid_midi_file* midifile, fluid_player_t* player);
int fluid_midi_file_read_track(fluid_midi_file* mf, fluid_player_t* player, int num);
int fluid_midi_file_read_event(fluid_midi_file* mf, fluid_track_t* track);
int fluid_midi_file_read_varlen(fluid_midi_file* mf);
int fluid_midi_file_getc(fluid_midi_file* mf);
int fluid_midi_file_push(fluid_midi_file* mf, int c);
int fluid_midi_file_read(fluid_midi_file* mf, void* buf, int len);
int fluid_midi_file_skip(fluid_midi_file* mf, int len);
int fluid_midi_file_read_tracklen(fluid_midi_file* mf);
int fluid_midi_file_eot(fluid_midi_file* mf);
int fluid_midi_file_get_division(fluid_midi_file* midifile);
int fluid_midi_event_length(unsigned char status);

/* How many parameters may a MIDI event have? */
#define FLUID_MIDI_PARSER_MAX_PAR 3

/*
 * fluid_midi_parser_t
 */
struct _fluid_midi_parser_t {
  unsigned char status;           /* Identifies the type of event, that is currently received ('Noteon', 'Pitch Bend' etc). */
  unsigned char channel;          /* The channel of the event that is received (in case of a channel event) */
  unsigned int nr_bytes;          /* How many bytes have been read for the current event? */
  unsigned int nr_bytes_total;    /* How many bytes does the current event type include? */
  unsigned short p[FLUID_MIDI_PARSER_MAX_PAR]; /* The parameters */
  fluid_midi_event_t event;        /* The event, that is returned to the MIDI driver. */
};

int fluid_isasciistring(char* s);
long fluid_getlength(unsigned char *s);



int fluid_midi_router_send_event(fluid_midi_router_t* router, fluid_midi_event_t* event);


#endif /* _FLUID_MIDI_H */
