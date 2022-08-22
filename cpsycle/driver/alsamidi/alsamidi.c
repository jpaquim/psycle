/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"

#include "../eventdriver.h"

/* platform */
#include <alsa/asoundlib.h>
#include <pthread.h>
#include "../../detail/portable.h"

#define __cdecl

#define DEVICE_NONE 0
#define PSY_EVENTDRIVER_ALSAMIDI_GUID 0x00020

typedef struct psy_AlsaMidiDriver {
	psy_EventDriver driver;	
	snd_seq_t *seq_handle;
	int npolldesc;
	struct pollfd *ppolldesc;	
	int (*error)(int, const char*);
	psy_EventDriverCmd lastinput;
	/// whether the thread is running
	bool running_;
	/// whether the thread is asked to terminate
	bool stop_requested_;
	// HANDLE hEvent;	
	psy_Property* configuration;
} psy_AlsaMidiDriver;

static void driver_free(psy_EventDriver*);
static int driver_init(psy_EventDriver*);
static int driver_open(psy_EventDriver*);
static int driver_close(psy_EventDriver*);
static int driver_dispose(psy_EventDriver*);
static const psy_EventDriverInfo* driver_info(psy_EventDriver*);
static void driver_configure(psy_EventDriver*, psy_Property*);
static const psy_Property* driver_configuration(const psy_EventDriver*);
static void driver_cmd(psy_EventDriver*, const char* section,
	psy_EventDriverInput, psy_EventDriverCmd*);
static psy_EventDriverCmd driver_getcmd(psy_EventDriver*, const char* section);
static void setcmddef(psy_EventDriver* self, const psy_Property* property) {  }
static void driver_idle(psy_EventDriver* self) { }
static void process_event(psy_AlsaMidiDriver*);

static psy_EventDriverInput driver_input(psy_EventDriver* context)
{
	psy_EventDriverInput input;

	psy_AlsaMidiDriver* self = (psy_AlsaMidiDriver*)context;
	if (self->lastinput.type == psy_EVENTDRIVER_MIDI) {
		input.message = self->lastinput.midi.byte0;
		input.param1 = self->lastinput.midi.byte1;
		input.param2 = self->lastinput.midi.byte2;
	} else {
		input.message = 0;
		input.param1 = 0;
		input.param2 = 0;
	}
	return input;
}

static void driver_makeconfig(psy_EventDriver*);
static int onerror(int err, const char* msg);

static void MidiCallback(void* driver);

static psy_EventDriverVTable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.open = driver_open;
		vtable.deallocate = driver_free;
		vtable.open = driver_open;
		vtable.close = driver_close;
		vtable.dispose = driver_dispose;
		vtable.info = driver_info;
		vtable.configure = driver_configure;
		vtable.configuration = driver_configuration;
		vtable.error = onerror;
		vtable.cmd = driver_cmd;
		vtable.getcmd = driver_getcmd;		
		vtable.setcmddef = setcmddef;
		vtable.idle = driver_idle;
		vtable.input = driver_input;
		vtable_initialized = TRUE;
	}
}

int onerror(int err, const char* msg)
{
	fprintf(stderr, "Alsa Midi driver ", msg);
	return 0;	
}

EXPORT psy_EventDriverInfo const *  psy_eventdriver_moduleinfo(void)
{
	static psy_EventDriverInfo info;

	info.guid = PSY_EVENTDRIVER_ALSAMIDI_GUID;
	info.Flags = 0;
	info.Name = "Alsa Midi Driver";
	info.ShortName = "alsamidi";
	info.Version = 0;
	return &info;
}

EXPORT psy_EventDriver* __cdecl psy_eventdriver_create(void)
{
	psy_AlsaMidiDriver* mme;
	
	mme = (psy_AlsaMidiDriver*)malloc(sizeof(psy_AlsaMidiDriver));
	if (mme) {			
		driver_init(&mme->driver);
		return &mme->driver;
	}
	return NULL;
}

void driver_free(psy_EventDriver* driver)
{
	free(driver);
}

int driver_init(psy_EventDriver* driver)
{
	psy_AlsaMidiDriver* self;
	
	assert(driver);

	self = (psy_AlsaMidiDriver*)driver;
	memset(self, 0, sizeof(psy_AlsaMidiDriver));
	vtable_init();
	self->driver.vtable = &vtable;
	// self->deviceid = DEVICE_NONE;
	// self->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	self->seq_handle = 0;	
	driver_makeconfig(&self->driver);
	self->driver.callback = NULL;
	self->driver.callbackcontext = NULL;
	self->lastinput.type = psy_EVENTDRIVER_CMD;
	self->lastinput.id = -1;
	self->running_ = FALSE;
	self->stop_requested_ = FALSE;
	return 0;
}

int driver_dispose(psy_EventDriver* driver)
{
	psy_AlsaMidiDriver* self;

	assert(driver);
	
	self = (psy_AlsaMidiDriver*)driver;
	if (self->seq_handle) {
		driver_close(driver);
	}
	psy_property_deallocate(self->configuration);
	self->configuration = NULL;	
	// CloseHandle(self->hEvent);	
	return 0;
}

const psy_EventDriverInfo* driver_info(psy_EventDriver* self)
{
	return psy_eventdriver_moduleinfo();
}

void driver_makeconfig(psy_EventDriver* context)
{		
	psy_AlsaMidiDriver* self;
	psy_Property* devices;
	char key[256];
	uint32_t i;
	uint32_t n;

	self = (psy_AlsaMidiDriver*)context;
	psy_snprintf(key, 256, "alsamidi-guid-%d", PSY_EVENTDRIVER_ALSAMIDI_GUID);
	self->configuration = psy_property_preventtranslate(
		psy_property_allocinit_key(key));
	psy_property_hide(psy_property_append_int(self->configuration,
		"guid", PSY_EVENTDRIVER_ALSAMIDI_GUID, 0, 0));
	psy_property_append_str(self->configuration, "name", "alsa midi");
	psy_property_append_str(self->configuration, "version", "1.0");
	// devices = psy_property_append_choice(self->configuration, "device", 0);
	// psy_property_set_hint(devices, PSY_PROPERTY_HINT_COMBO);
	// n = midiInGetNumDevs();	
	// for (i = 0; i < n; ++i) {
	//	char text[256];
		
	//	MIDIINCAPS caps;
	//	midiInGetDevCaps(i, &caps, sizeof(MIDIINCAPS));
	//	psy_snprintf(text, 256, "%d:%s", i, caps.szPname);
	//	psy_property_append_int(devices, text, i, 0, 0);
	// }
}

void driver_configure(psy_EventDriver* driver, psy_Property* config)
{
	psy_AlsaMidiDriver* self;
	psy_Property* p;

	self = (psy_AlsaMidiDriver*)driver;
	if (self->configuration && config) {
		psy_property_sync(self->configuration, config);
	}
	p = psy_property_at(self->configuration, "device",
		PSY_PROPERTY_TYPE_NONE);
	if (p) {
		// self->deviceid = (uint32_t)psy_property_item_int(p);
	}
}

int driver_open(psy_EventDriver* driver)
{
	psy_AlsaMidiDriver* self = (psy_AlsaMidiDriver*)driver;	
	int success = 1;
	pthread_t threadid;
	
	// return immediatly if the thread is already running
	if (self->running_) return 0;

	self->lastinput.type = psy_EVENTDRIVER_CMD;
	self->lastinput.id = -1;
	
	//\todo: change "default" to something configurable.
	if (snd_seq_open(&self->seq_handle, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
			fprintf(stderr, "Error opening ALSA sequencer.\n");
			success = 0;
			return success;
	}
	snd_seq_set_client_name(self->seq_handle, "psycle");
	if (snd_seq_create_simple_port(self->seq_handle, "midiIO",
			SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
			SND_SEQ_PORT_TYPE_APPLICATION) < 0) {
			fprintf(stderr, "Error creating sequencer port.\n");
			success = 0;
			return success;
	}
	self->npolldesc = snd_seq_poll_descriptors_count(self->seq_handle, POLLIN);
	self->ppolldesc = (struct pollfd *)alloca(sizeof(struct pollfd) * self->npolldesc);
	snd_seq_poll_descriptors(self->seq_handle, self->ppolldesc,
		self->npolldesc, POLLIN);
	self->stop_requested_ = FALSE;
	if (pthread_create(&threadid, NULL, (void*(*)(void*))MidiCallback,
		(void*) driver) == 0)
	{		
		return TRUE;	  
	}
	
	return success;	
}

int driver_close(psy_EventDriver* driver)
{
	psy_AlsaMidiDriver* self = (psy_AlsaMidiDriver*)driver;
	int success = 1;

	self->stop_requested_ = TRUE;
	usleep(20000);
	snd_seq_close (self->seq_handle);
	self->seq_handle = NULL;
	return success;
}

void driver_cmd(psy_EventDriver* driver, const char* section,
	psy_EventDriverInput input, psy_EventDriverCmd* cmd)
{	
	psy_AlsaMidiDriver* self;

	if (!cmd) {
		return;
	}
	self = (psy_AlsaMidiDriver*)driver;
	if (psy_strlen(section) == 0) {
		cmd->type = psy_EVENTDRIVER_MIDI;
		cmd->midi.byte0 = (uint8_t)input.message;
		cmd->midi.byte1 = (uint8_t)input.param1;
		cmd->midi.byte2 = (uint8_t)input.param1;
		*cmd = self->lastinput;
	} else {
		cmd->id = -1;
		cmd->type = psy_EVENTDRIVER_CMD;
	}
}

psy_EventDriverCmd driver_getcmd(psy_EventDriver* context, const char* section)
{	
	psy_AlsaMidiDriver* self;
	psy_EventDriverCmd cmd;

	self = (psy_AlsaMidiDriver*)(context);
	if (psy_strlen(section) == 0) {
		return self->lastinput;
	}
	cmd.id = -1;
	cmd.type = psy_EVENTDRIVER_CMD;	
	return cmd;
}

const psy_Property* driver_configuration(const psy_EventDriver* driver)
{
	psy_AlsaMidiDriver* self;

	self = (psy_AlsaMidiDriver*)driver;
	return self->configuration;
}

void MidiCallback(void* driver)
{
	psy_AlsaMidiDriver* self = (psy_AlsaMidiDriver*) driver;
	//int numSamples

	pthread_detach(pthread_self());
	self->running_ = TRUE;	
	printf("Enter Alsa Midi Thread function\n");
	while (TRUE) {
		if (self->stop_requested_) goto notify_termination;
		if (poll (self->ppolldesc, self->npolldesc, 1000) > 0) {
			int l1;
			
			for (l1 = 0; l1 < self->npolldesc; l1++) {
				if (self->ppolldesc[l1].revents > 0) {
					process_event(self);
				}
			}
		}
		usleep(100);
	}
notify_termination:	
	self->running_ = FALSE;
	// }
	// pthread_mutex_unlock(&self->mutex);		
	printf("Leave Alsa Thread\n");
	pthread_exit(0);
}

void process_event(psy_AlsaMidiDriver* self)
{	
    snd_seq_event_t* ev;
    snd_midi_event_t *midi_event;
    unsigned char buf[256];	
		
		
	self->lastinput.type = psy_EVENTDRIVER_CMD;
	self->lastinput.id = -1;
	
	if (snd_midi_event_new(256 /* ? */, &midi_event) < 0) {
		return;
	}	
	do{
		snd_seq_event_input(self->seq_handle, &ev);
		if (self->driver.callback) {			
			snd_midi_event_decode(midi_event, buf, sizeof(buf), ev);
			self->lastinput.midi.byte0 = buf[0];
			self->lastinput.midi.byte1 = buf[1];
			self->lastinput.midi.byte2 = buf[2];					
			self->driver.callback(self->driver.callbackcontext, &self->driver);
			snd_midi_event_free(midi_event);			
		}
	} while (snd_seq_event_input_pending(self->seq_handle, 0) > 0);
}
/*
SND_SEQ_EVENT_SYSTEM                 system status; event data type = snd_seq_result_t
SND_SEQ_EVENT_RESULT                 returned result status; event data type = snd_seq_result_t
SND_SEQ_EVENT_NOTE                   note on and off with duration; event data type = snd_seq_ev_note_t
SND_SEQ_EVENT_NOTEON                 note on; event data type = snd_seq_ev_note_t
SND_SEQ_EVENT_NOTEOFF                note off; event data type = snd_seq_ev_note_t
SND_SEQ_EVENT_KEYPRESS               key pressure change (aftertouch); event data type = snd_seq_ev_note_t
SND_SEQ_EVENT_CONTROLLER             controller; event data type = snd_seq_ev_ctrl_t
SND_SEQ_EVENT_PGMCHANGE              program change; event data type = snd_seq_ev_ctrl_t
SND_SEQ_EVENT_CHANPRESS              channel pressure; event data type = snd_seq_ev_ctrl_t
SND_SEQ_EVENT_PITCHBEND              pitchwheel; event data type = snd_seq_ev_ctrl_t; data is from -8192 to 8191)
SND_SEQ_EVENT_CONTROL14              14 bit controller value; event data type = snd_seq_ev_ctrl_t
SND_SEQ_EVENT_NONREGPARAM            14 bit NRPN; event data type = snd_seq_ev_ctrl_t
SND_SEQ_EVENT_REGPARAM               14 bit RPN; event data type = snd_seq_ev_ctrl_t
SND_SEQ_EVENT_SONGPOS                SPP with LSB and MSB values; event data type = snd_seq_ev_ctrl_t
SND_SEQ_EVENT_SONGSEL                Song Select with song ID number; event data type = snd_seq_ev_ctrl_t
SND_SEQ_EVENT_QFRAME                 midi time code quarter frame; event data type = snd_seq_ev_ctrl_t
SND_SEQ_EVENT_TIMESIGN               SMF Time Signature event; event data type = snd_seq_ev_ctrl_t
SND_SEQ_EVENT_KEYSIGN                SMF Key Signature event; event data type = snd_seq_ev_ctrl_t
SND_SEQ_EVENT_START                  MIDI Real Time Start message; event data type = snd_seq_ev_queue_control_t
SND_SEQ_EVENT_CONTINUE               MIDI Real Time Continue message; event data type = snd_seq_ev_queue_control_t
SND_SEQ_EVENT_STOP                   MIDI Real Time Stop message; event data type = snd_seq_ev_queue_control_t
SND_SEQ_EVENT_SETPOS_TICK            Set tick queue position; event data type = snd_seq_ev_queue_control_t
SND_SEQ_EVENT_SETPOS_TIME            Set real-time queue position; event data type = snd_seq_ev_queue_control_t
SND_SEQ_EVENT_TEMPO                  (SMF) Tempo event; event data type = snd_seq_ev_queue_control_t
SND_SEQ_EVENT_CLOCK                  MIDI Real Time Clock message; event data type = snd_seq_ev_queue_control_t
SND_SEQ_EVENT_TICK MIDI              Real Time Tick message; event data type = snd_seq_ev_queue_control_t
SND_SEQ_EVENT_QUEUE_SKEW             Queue timer skew; event data type = snd_seq_ev_queue_control_t
SND_SEQ_EVENT_SYNC_POS               Sync position changed; event data type = snd_seq_ev_queue_control_t
SND_SEQ_EVENT_TUNE_REQUEST           Tune request; event data type = none
SND_SEQ_EVENT_RESET                  Reset to power-on state; event data type = none
SND_SEQ_EVENT_SENSING                Active sensing event; event data type = none
SND_SEQ_EVENT_ECHO                   Echo-back event; event data type = any type
SND_SEQ_EVENT_OSS                    OSS emulation raw event; event data type = any type
SND_SEQ_EVENT_CLIENT_START           New client has connected; event data type = snd_seq_addr_t
SND_SEQ_EVENT_CLIENT_EXIT            Client has left the system; event data type = snd_seq_addr_t
SND_SEQ_EVENT_CLIENT_CHANGE          Client status/info has changed; event data type = snd_seq_addr_t
SND_SEQ_EVENT_PORT_START             New port was created; event data type = snd_seq_addr_t
SND_SEQ_EVENT_PORT_EXIT              Port was deleted from system; event data type = snd_seq_addr_t
SND_SEQ_EVENT_PORT_CHANGE            Port status/info has changed; event data type = snd_seq_addr_t
SND_SEQ_EVENT_PORT_SUBSCRIBED        Ports connected; event data type = snd_seq_connect_t
SND_SEQ_EVENT_PORT_UNSUBSCRIBED      Ports disconnected; event data type = snd_seq_connect_t
SND_SEQ_EVENT_SAMPLE                 Sample select; event data type = snd_seq_ev_sample_control_t
SND_SEQ_EVENT_SAMPLE_CLUSTER         Sample cluster select; event data type = snd_seq_ev_sample_control_t
SND_SEQ_EVENT_SAMPLE_START           voice start
SND_SEQ_EVENT_SAMPLE_STOP            voice stop
SND_SEQ_EVENT_SAMPLE_FREQ            playback frequency
SND_SEQ_EVENT_SAMPLE_VOLUME          volume and balance
SND_SEQ_EVENT_SAMPLE_LOOP            sample loop
SND_SEQ_EVENT_SAMPLE_POSITION        sample position
SND_SEQ_EVENT_SAMPLE_PRIVATE1        private (hardware dependent) event
SND_SEQ_EVENT_USR0                   user-defined event; event data type = any (fixed size)
SND_SEQ_EVENT_USR1                   user-defined event; event data type = any (fixed size)
SND_SEQ_EVENT_USR2                   user-defined event; event data type = any (fixed size)
SND_SEQ_EVENT_USR3                   user-defined event; event data type = any (fixed size)
SND_SEQ_EVENT_USR4                   user-defined event; event data type = any (fixed size)
SND_SEQ_EVENT_USR5                   user-defined event; event data type = any (fixed size)
SND_SEQ_EVENT_USR6                   user-defined event; event data type = any (fixed size)
SND_SEQ_EVENT_USR7                   user-defined event; event data type = any (fixed size)
SND_SEQ_EVENT_USR8                   user-defined event; event data type = any (fixed size)
SND_SEQ_EVENT_USR9                   user-defined event; event data type = any (fixed size)
SND_SEQ_EVENT_INSTR_BEGIN            begin of instrument management
SND_SEQ_EVENT_INSTR_END              end of instrument management
SND_SEQ_EVENT_INSTR_INFO             query instrument interface info
SND_SEQ_EVENT_INSTR_INFO_RESULT      result of instrument interface info
SND_SEQ_EVENT_INSTR_FINFO            query instrument format info
SND_SEQ_EVENT_INSTR_FINFO_RESULT     result of instrument format info
SND_SEQ_EVENT_INSTR_RESET            reset instrument instrument memory
SND_SEQ_EVENT_INSTR_STATUS           get instrument interface status
SND_SEQ_EVENT_INSTR_STATUS_RESULT    result of instrument interface status
SND_SEQ_EVENT_INSTR_PUT              put an instrument to port
SND_SEQ_EVENT_INSTR_GET              get an instrument from port
SND_SEQ_EVENT_INSTR_GET_RESULT       result of instrument query
SND_SEQ_EVENT_INSTR_FREE             free instrument(s)
SND_SEQ_EVENT_INSTR_LIST             get instrument list
SND_SEQ_EVENT_INSTR_LIST_RESULT      result of instrument list
SND_SEQ_EVENT_INSTR_CLUSTER          set cluster parameters
SND_SEQ_EVENT_INSTR_CLUSTER_GET      get cluster parameters
SND_SEQ_EVENT_INSTR_CLUSTER_RESULT   result of cluster parameters
SND_SEQ_EVENT_INSTR_CHANGE           instrument change
SND_SEQ_EVENT_SYSEX                  system exclusive data (variable length); event data type = snd_seq_ev_ext_t
SND_SEQ_EVENT_BOUNCE                 error event; event data type = snd_seq_ev_ext_t
SND_SEQ_EVENT_USR_VAR0               reserved for user apps; event data type = snd_seq_ev_ext_t
SND_SEQ_EVENT_USR_VAR1               reserved for user apps; event data type = snd_seq_ev_ext_t
SND_SEQ_EVENT_USR_VAR2               reserved for user apps; event data type = snd_seq_ev_ext_t
SND_SEQ_EVENT_USR_VAR3               reserved for user apps; event data type = snd_seq_ev_ext_t
SND_SEQ_EVENT_USR_VAR4               reserved for user apps; event data type = snd_seq_ev_ext_t
SND_SEQ_EVENT_NONE                   NOP; ignored in any case
*/



