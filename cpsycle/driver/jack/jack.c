// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-20222 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../audiodriversettings.h"

// includes
#include <jack/jack.h>
#include <string.h>
#include "../audiodriver.h"
#include <stdio.h>
#include "../../detail/portable.h"

#define PSY_AUDIODRIVER_JACK_GUID 0x0006

typedef struct {		
	psy_AudioDriver driver;
	psy_AudioDriverSettings settings;
	psy_Property* configuration;
	
	bool running_;

	// jack variables
	jack_port_t *output_port_1;
	jack_port_t *output_port_2;

	jack_client_t *client;
	const char **ports;

	char* clientName_;
	char* serverName_;
	
	int (*error)(int, const char*);
} JackDriver;

static void driver_deallocate(psy_AudioDriver*);
static int driver_init(psy_AudioDriver*);
static int driver_open(psy_AudioDriver*);
static bool driver_opened(const psy_AudioDriver*);
static int driver_close(psy_AudioDriver*);
static int driver_dispose(psy_AudioDriver*);
static void driver_refresh_ports(psy_AudioDriver* self) { }
static void driver_configure(psy_AudioDriver*, psy_Property*);
static const psy_Property* driver_configuration(const psy_AudioDriver*);
static psy_dsp_big_hz_t driver_samplerate(psy_AudioDriver*);
static const psy_AudioDriverInfo* driver_info(psy_AudioDriver*);
static uint32_t playposinsamples(psy_AudioDriver*);
static bool addcaptureport(psy_AudioDriver* self, int idx) { }
static bool removecaptureport(psy_AudioDriver* self, int idx) { return TRUE; }
static void readbuffers(psy_AudioDriver* self, int idx, float** pleft,
	float** pright,
	uintptr_t numsamples)
{
	*pleft = NULL;
	*pright = NULL;
}
static const char* capturename(psy_AudioDriver* self, int index)
{
	return "undefined";
}
static int numcaptures(psy_AudioDriver* self)
{
	return 0;
}
static const char* playbackname(psy_AudioDriver* self, int index)
{
	return "undefined";
}
static int numplaybacks(psy_AudioDriver* self)
{
	return 0;
}

static void thread_function(void* driver);
static void init_properties(psy_AudioDriver*);
static int process(jack_nframes_t nframes, void *arg);
static int FillBuffer(JackDriver*, jack_nframes_t nframes);
static void do_stop(JackDriver*);
static int on_error(int err, const char* msg);

static psy_AudioDriverVTable vtable;
static int vtable_initialized = 0;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.open = driver_open;
		vtable.opened = driver_opened;
		vtable.deallocate = driver_deallocate;
		vtable.close = driver_close;
		vtable.dispose = driver_dispose;
		vtable.refresh_ports = driver_refresh_ports;
		vtable.configure = driver_configure;
		vtable.configuration = driver_configuration;
		vtable.samplerate = driver_samplerate;		
		vtable.addcapture = addcaptureport;
		vtable.removecapture = removecaptureport;
		vtable.readbuffers = readbuffers;
		vtable.capturename = capturename;
		vtable.numcaptures = numcaptures;
		vtable.playbackname = playbackname;
		vtable.numplaybacks = numplaybacks;
		vtable.playposinsamples = playposinsamples;
		vtable.info = driver_info;
		vtable_initialized = 1;
	}
}

int on_error(int err, const char* msg)
{
	fprintf(stderr, "Jack driver ", msg);
	return 0;
}

EXPORT psy_AudioDriverInfo const * __cdecl GetPsycleDriverInfo(void)
{
	static psy_AudioDriverInfo info;

	info.guid = PSY_AUDIODRIVER_JACK_GUID;
	info.Flags = 0;
	info.Name = "Jack Low Latency audio driver";
	info.ShortName = "jack";
	info.Version = 0;
	return &info;
}

EXPORT psy_AudioDriver* __cdecl driver_create(void)
{
	JackDriver* d = (JackDriver*)malloc(sizeof(JackDriver));
	if (d != NULL) {		
		driver_init(&d->driver);
		return &d->driver;
	}
	return NULL;
}

void driver_deallocate(psy_AudioDriver* driver)
{
	driver_dispose(driver);
	free(driver);
}

int driver_init(psy_AudioDriver* driver)
{
	JackDriver* self = (JackDriver*) driver;	

	memset(self, 0, sizeof(JackDriver));
	vtable_init();
	self->driver.vtable = &vtable;
	self->error = on_error;	
		
	self->clientName_ = psy_strdup("psycle");
	self->serverName_ = psy_strdup(""); // maybe not needed
	self->running_ = 0;
	psy_audiodriversettings_init(&self->settings);
	init_properties(driver);
	return 0;
}

int driver_dispose(psy_AudioDriver* driver)
{
	JackDriver* self = (JackDriver*)driver;
	driver_close(self);
	psy_property_deallocate(self->configuration);
	self->configuration = NULL;	
	return 0;
}

void init_properties(psy_AudioDriver* driver)
{	
	JackDriver* self;
	psy_Property* property;	
	char key[256];

	self = (JackDriver*)driver;
	psy_snprintf(key, 256, "jack-guid-%d", PSY_AUDIODRIVER_JACK_GUID);
	self->configuration = psy_property_preventtranslate(psy_property_settext(
		psy_property_allocinit_key(key), "Jack Interface"));	
	psy_property_sethint(psy_property_append_int(self->configuration,
		"guid", PSY_AUDIODRIVER_JACK_GUID, 0, 0),
		PSY_PROPERTY_HINT_HIDE);
	psy_property_setreadonly(
		psy_property_append_str(self->configuration, "name", "jack"),
		TRUE);
	psy_property_setreadonly(
		psy_property_append_str(self->configuration, "vendor",
		"Psycedelics"),
		TRUE);
	psy_property_setreadonly(
		psy_property_append_str(self->configuration, "version", "1.0"),
		TRUE);
	property = psy_property_append_choice(self->configuration, "device", -1);	
	psy_property_append_int(self->configuration, "bitdepth", 16, 0, 32);
	psy_property_append_int(self->configuration, "samplerate", 44100, 0, 0);
	psy_property_append_int(self->configuration, "dither", 0, 0, 1);
	psy_property_append_int(self->configuration, "numbuf", 8, 6, 8);
	psy_property_append_int(self->configuration, "numsamples", 4096, 128, 8193);	
}

void driver_configure(psy_AudioDriver* driver, psy_Property* config)
{
	JackDriver* self;

	self = (JackDriver*)driver;
	if (self->configuration && config) {
		psy_property_sync(self->configuration, config);
	}	
}

psy_dsp_big_hz_t driver_samplerate(psy_AudioDriver* self)
{
	// return psy_audiodriversettings_samplespersec(&((DXDriver*)self)->settings);
	return 44100.0;
}

int driver_open(psy_AudioDriver* driver)
{
    JackDriver* self = (JackDriver*) driver;
	char* device_name;
	unsigned int chn;
	int err;
	int threadid;

	// return immediatly if the thread is already running
	if (self->running_) return 0;
	if (driver->callback == NULL) {
		return FALSE;
	}	
	device_name = "default";	
	
	
		//jack_options_t options = JackNullOption;
	//jack_status_t status;
	// try to become a client of the JACK server
	const char* registerCPtr = self->clientName_;
	// std::string( clientName_ +" "+serverName_  ).c_str();

	if ( (self->client = jack_client_new( registerCPtr )) == 0) {
		fprintf(stderr, "jack server not running?\n");
		return FALSE;
	}

	/*if ( (client = jack_client_open( clientName_.c_str(),options,&status,serverName_.c_str())) == NULL ) {
		std::cerr << "jack server not running?\n" << std::endl;
	}*/

	// tell the JACK server to call `process()' whenever
	// there is work to be done.

	jack_set_process_callback(self->client, process, (void*)self);

	// display the current sample rate. 

	printf("engine sample rate: %d\n", 
		(int)jack_get_sample_rate(self->client));

	// create output port
	self->output_port_1 = jack_port_register(self->client, "output_l",
		JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	self->output_port_2 = jack_port_register(self->client, "output_r",
		JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);	

	// tell the JACK server that we are ready to roll
	if (jack_activate(self->client)) {
		fprintf(stderr, "cannot activate client\n");
		return FALSE;
	}
	psy_audiodriversettings_setsamplespersec(&self->settings,
		jack_get_sample_rate(self->client));
	psy_audiodriversettings_setvalidbitdepth(&self->settings,
		16); // hardcoded so far
	///\todo inform the player that the sample rate is different

	if ((self->ports = jack_get_ports(self->client, NULL, NULL, JackPortIsPhysical|JackPortIsInput)) == NULL) {
		fprintf(stderr, "Cannot find any physical playback ports\n");		
		return FALSE;
	}	
	if (jack_connect(self->client, jack_port_name(self->output_port_1), self->ports[0])) {
		fprintf(stderr, "Cannot connect output port 1\n");
		return FALSE;		
	}	
	if (jack_connect(self->client, jack_port_name(self->output_port_2), self->ports[1])) {
		fprintf(stderr, "Cannot connect output port 2\n");
		return FALSE;
	}
	free(self->ports);
	printf("jack enabled\n");
	self->running_ = TRUE;	
	return TRUE;
}

bool driver_opened(const psy_AudioDriver* driver)
{
	JackDriver* self;

	self = (JackDriver*)driver;
	return (self->running_ != FALSE);
}

int process(jack_nframes_t nframes, void *arg)
{
	JackDriver* self = (JackDriver*)arg;
	FillBuffer(self, nframes);
	return 0;  
}

void do_stop(JackDriver* self)
{
	// return immediatly if the thread is not running
	if (!self->running_) return;
	jack_client_close(self->client);
}

int driver_close(psy_AudioDriver* self)
{
    do_stop((JackDriver*)self);
}

int FillBuffer(JackDriver* self, jack_nframes_t nframes )
{
	int num;
	int hostisplaying;
	float* input;
	
	jack_default_audio_sample_t *out_1 = (jack_default_audio_sample_t *) jack_port_get_buffer (self->output_port_1, nframes);
	jack_default_audio_sample_t *out_2 = (jack_default_audio_sample_t *) jack_port_get_buffer (self->output_port_2, nframes);	
	num = nframes;
	input = self->driver.callback(
		self->driver.callbackcontext, &num, &hostisplaying);
    unsigned int count = 0;
	while(count < nframes) {
		out_1[count] = *input++  / 32768.0f;
		out_2[count] = *input++  / 32768.0f;
		++count;
	}
	return 0;
}

const psy_AudioDriverInfo* driver_info(psy_AudioDriver* self)
{
	return GetPsycleDriverInfo();
}

const psy_Property* driver_configuration(const psy_AudioDriver* driver)
{
	JackDriver* self = (JackDriver*)driver;
	
	return self->configuration;
}

uint32_t playposinsamples(psy_AudioDriver* driver)
{
	JackDriver* self = (JackDriver*)driver;

	return 0;
}
