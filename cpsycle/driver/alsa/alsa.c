// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../audiodriversettings.h"

// includes
#include <alsa/asoundlib.h>
#include <string.h>
#include "../audiodriver.h"
#include <stdio.h>
#include "quantize.h"
#include <pthread.h>
#include "../../detail/portable.h"

#define PSY_AUDIODRIVER_ALSA_GUID 0x0005

typedef struct {		
	psy_AudioDriver driver;
	psy_AudioDriverSettings settings;
	psy_Property* configuration;
	/// stream rate
	unsigned int rate;
	/// sample format
	snd_pcm_format_t format;
	/// count of channels
	unsigned int channels;
	/// ring buffer length in us
	unsigned int buffer_time;
	/// period time in us
	unsigned int period_time;
	snd_pcm_sframes_t buffer_size;
	snd_pcm_sframes_t period_size;
	snd_output_t* output;
	snd_pcm_t* handle;
	/// 0:WRITE 1:WRITE&POLL 2:ASYNC 3:async_direct 4:direct_interleaved
	/// 5:direct_noninterleaved 6:DIRECT_WRITE
	int method;
	int16_t* samples;
	snd_pcm_channel_area_t* areas;	
	int id;
	/// left out (getSample should change this non-stop if audio was started
	int16_t left;
	/// right out (getSample should change this non-stop if audio was started)
	int16_t right;	
	/// whether the thread is running
	bool running_;
	/// whether the thread is asked to terminate
	bool stop_requested_;
	/// a mutex to synchronise accesses to running_ and stop_requested_
	// mutex mutex_;
	// typedef class unique_lock<mutex> scoped_lock;
	/// a condition variable to wait until notified that the value of running_ has changed
	// condition_variable condition_;
	///\}
	int (*error)(int, const char*);
	pthread_mutex_t mutex;
} AlsaDriver;

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
static void set_hwparams(AlsaDriver* self, snd_pcm_hw_params_t* params,
    snd_pcm_access_t access);
static void set_swparams(AlsaDriver* self, snd_pcm_sw_params_t* swparams);
static void FillBuffer(AlsaDriver* self, snd_pcm_uframes_t offset, int count);
static int xrun_recovery(AlsaDriver* self, int err);
static void do_stop(AlsaDriver* self);
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
		vtable.configure =
			(psy_audiodriver_fp_configure)
			driver_configure;
		vtable.configuration = driver_configuration;
		vtable.samplerate = driver_samplerate;		
		vtable.addcapture =
			(psy_audiodriver_fp_addcapture)
			addcaptureport;
		vtable.removecapture =
			(psy_audiodriver_fp_removecapture)
			removecaptureport;
		vtable.readbuffers = readbuffers;
		vtable.capturename = capturename;
		vtable.numcaptures = numcaptures;
		vtable.playbackname = playbackname;
		vtable.numplaybacks = numplaybacks;
		vtable.playposinsamples =
			(psy_audiodriver_fp_playposinsamples)
			playposinsamples;
		vtable.info = driver_info;
		vtable_initialized = 1;
	}
}

int on_error(int err, const char* msg)
{
	fprintf(stderr, "Alsa driver ", msg);
	return 0;
}

EXPORT psy_AudioDriverInfo const * __cdecl GetPsycleDriverInfo(void)
{
	static psy_AudioDriverInfo info;

	info.guid = PSY_AUDIODRIVER_ALSA_GUID;
	info.Flags = 0;
	info.Name = "Alsa Low Latency audio driver";
	info.ShortName = "alsa";
	info.Version = 0;
	return &info;
}

EXPORT psy_AudioDriver* __cdecl driver_create(void)
{
	AlsaDriver* dx = (AlsaDriver*)malloc(sizeof(AlsaDriver));
	if (dx != NULL) {		
		driver_init(&dx->driver);
		return &dx->driver;
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
	AlsaDriver* self = (AlsaDriver*) driver;	
	pthread_mutexattr_t recursiveattr;

	memset(self, 0, sizeof(AlsaDriver));
	vtable_init();
	self->driver.vtable = &vtable;
	self->error = on_error;
	memset(&self->format, 0, sizeof(self->format));

	self->rate = 44100; // stream rate
	self->format = SND_PCM_FORMAT_S16; // sample format
	self->channels = 2; // count of channels

	// safe default values
	self->buffer_time = 80000;						
	self->period_time = 20000;

	self->buffer_size = 0;
	self->period_size = 0;
	self->output = 0;
	
	self->running_ = FALSE;
	self->stop_requested_ = FALSE;
	
	pthread_mutexattr_init(&recursiveattr);
	pthread_mutexattr_settype(&recursiveattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&self->mutex, &recursiveattr);
	pthread_mutexattr_destroy(&recursiveattr);	
	psy_audiodriversettings_init(&self->settings);
	init_properties(driver);
	return 0;
}

int driver_dispose(psy_AudioDriver* driver)
{
	AlsaDriver* self = (AlsaDriver*) driver;
	psy_property_deallocate(self->configuration);
	self->configuration = NULL;
	pthread_mutex_destroy(&self->mutex);
	return 0;
}

void init_properties(psy_AudioDriver* driver)
{	
	AlsaDriver* self;
	psy_Property* property;	
	char key[256];

	self = (AlsaDriver*)driver;
	psy_snprintf(key, 256, "alsa-guid-%d", PSY_AUDIODRIVER_ALSA_GUID);
	self->configuration = psy_property_preventtranslate(
		psy_property_set_text(
		psy_property_allocinit_key(key), "Linux ALSA Interface"));	
	psy_property_hide(psy_property_append_int(self->configuration,
		"guid", PSY_AUDIODRIVER_ALSA_GUID, 0, 0));
	psy_property_setreadonly(
		psy_property_append_str(self->configuration, "name", "alsa"),
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
	AlsaDriver* self;

	self = (AlsaDriver*)driver;
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
    AlsaDriver* self = (AlsaDriver*) driver;
	char* device_name;
	unsigned int chn;
	int err;
	pthread_t threadid;

	// return immediatly if the thread is already running	
	if (self->running_) return 0;	
	if (driver->callback == NULL) {
		return FALSE;
	}	
	snd_pcm_hw_params_t* hwparams;
	snd_pcm_sw_params_t* swparams;

	snd_pcm_hw_params_alloca(&hwparams);
	snd_pcm_sw_params_alloca(&swparams);

	err = snd_output_stdio_attach(&self->output, stdout, 0);
	if (err < 0) {		
		fprintf(stderr, "psycle: alsa: attaching output failed: %s\n", snd_strerror(err));
		return -1;
	}

	device_name = "default";	

	printf("psycle: alsa: playback device is: %s\n", device_name);
	printf("psycle: alsa: stream parameters are: %u Hz, channels %s\n",
        self->rate, snd_pcm_format_name(self->format),
        (int)self->channels);
	printf("psycle: alsa: using transfer method: write\n"); ///\todo parametrable?

	if ((err = snd_pcm_open(&self->handle, device_name, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {		
		fprintf(stderr, "psycle: alsa: playback open error: %s\n", snd_strerror(err));
		return -1;
	}

	set_hwparams(self, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
	set_swparams(self, swparams);

	self->samples = (int16_t*)malloc((self->period_size * self->channels *
        snd_pcm_format_width(self->format)) / 8);
	if (!self->samples) {
		fprintf(stderr, "psycle: alsa: out of memory alloc samples\n");		
	}

	self->areas = (snd_pcm_channel_area_t*) calloc(self->channels,
        sizeof(snd_pcm_channel_area_t));
	if (!self->areas) {
		fprintf(stderr, "psycle: alsa: out of memory alloc areas\n");
	}

	for (chn = 0; chn < self->channels; ++chn) {
		self->areas[chn].addr = self->samples;
		self->areas[chn].first = chn * 16;
		self->areas[chn].step = self->channels * 16;
	}	
	
	// wait for the thread to be running
	// { scoped_lock lock(mutex_);
	// while (!running_) condition_.wait(lock);
	// }	
	self->stop_requested_ = FALSE;
	if (pthread_create(&threadid, NULL, (void*(*)(void*))thread_function,
		(void*) driver) == 0)
	{		
		return TRUE;	  
	}
	return FALSE;
}

bool driver_opened(const psy_AudioDriver* driver)
{
	AlsaDriver* self;

	self = (AlsaDriver*)driver;
	return (self->running_ != FALSE);
}


void thread_function(void* driver) {
	AlsaDriver* self = (AlsaDriver*) driver;
	// notify that the thread is now running
	// { scoped_lock lock(mutex_);	
	pthread_mutex_lock(&self->mutex);
	pthread_detach(pthread_self());
	self->running_ = TRUE;	
	//}
	//condition_.notify_one();
	printf("Enter Alsa Thread function\n");
	while (TRUE) {
		int16_t* ptr;
		int cptr;

		// check whether the thread has been asked to terminate
		// { scoped_lock lock(mutex_);
		if (self->stop_requested_) goto notify_termination;
		// }		
		FillBuffer(self, 0, self->period_size);
		ptr = self->samples;
		cptr = self->period_size;
		while (cptr > 0) {
			int err = snd_pcm_writei(self->handle, ptr, cptr);
			if (err == -EAGAIN) continue;
			if (err < 0) {
				if (xrun_recovery(self, err) < 0) {
					//if (loggers::exception()) {						
					fprintf(stderr, "psycle: alsa: write error: %s\n", snd_strerror(err));					
					goto notify_termination;
				}
				break; // skip one period
			}
			ptr += err * self->channels;
			cptr -= err;
		}
//		usleep(100);
	//	printf("write\n");
		// this_thread::yield(); ///\todo is this useful?
	}

	// notify that the thread is not running anymore
notify_termination:	
	self->running_ = FALSE;
	// }
	pthread_mutex_unlock(&self->mutex);		
	printf("Leave Alsa Thread\n");
	pthread_exit(0);
}

/// Underrun and suspend recovery
int xrun_recovery(AlsaDriver* self, int err)
{
	if (err == -EPIPE) { // under-run
		err = snd_pcm_prepare(self->handle);
		if (err < 0) {
			//if (loggers::warning()) {				
				fprintf(stderr, "psycle: alsa: cannot recover from under-run, prepare failed: %s\n", snd_strerror(err));				
			//}
		}
		return 0;
	}
	else if (err == -ESTRPIPE) {
		while ((err = snd_pcm_resume(self->handle)) == -EAGAIN)
			// wait until the suspend flag is released
			// this_thread::yield(); ///\todo any other way?
		if (err < 0) {
			err = snd_pcm_prepare(self->handle);
			if (err < 0) {
				// if (loggers::warning()) {					
				fprintf(stderr, "psycle: alsa: cannot recover from suspend, prepare failed: %s\n", snd_strerror(err));				
			}
		}
		return 0;
	}
	return err;
}

void do_stop(AlsaDriver* self)
{
	// return immediatly if the thread is not running	
	if (!self->running_) return;	

	// ask the thread to terminate
	// { scoped_lock lock(mutex_);
	self->stop_requested_ = TRUE;
	// }
	// condition_.notify_one();

	/// join the thread
	// { scoped_lock lock(mutex_);
	// while (running_) condition_.wait(lock);
	pthread_mutex_lock(&self->mutex);
	self->stop_requested_ = FALSE;
	pthread_mutex_unlock(&self->mutex);
	//}
}

int driver_close(psy_AudioDriver* driver)
{
	AlsaDriver* self = (AlsaDriver*)driver;
    do_stop(self);    
	if (self->handle) {
		free(self->areas);
		free(self->samples);		
		snd_pcm_close(self->handle);
		snd_config_update_free_global();
	}	
}

void FillBuffer(AlsaDriver* self, snd_pcm_uframes_t offset, int count)
{
	int16_t* samples[self->channels];
	int steps[self->channels];
	float* input;
	int hostisplaying;
    unsigned int chn;
    int num;

	// verify and prepare the contents of areas
	for (chn = 0; chn < self->channels; ++chn) {
		if (self->areas[chn].first % 8) {			
			fprintf(stderr, "psycle: alsa: areas %u.first == %d, aborting. %s\n",
                chn, (int)self->areas[chn].first );			
		}
		samples[chn] = (int16_t*)(((unsigned char*)self->areas[chn].addr)
            + (self->areas[chn].first / 8));
		if (self->areas[chn].step % 16) {			
			fprintf(stderr, "psycle: alsa: areas[%u].step == %d, aborting. \n",
                chn, self->areas[chn].step);			
		}
		steps[chn] = self->areas[chn].step / 16;
		samples[chn] += offset * steps[chn];
	}
	// fill the channel areas
	num = count;
	input = self->driver.callback(
		self->driver.callbackcontext, &num, &hostisplaying);
	while (count-- > 0) {
		*samples[0] = (int16_t)(*input++);
		samples[0] += steps[0];
		*samples[1] = (int16_t)(*input++);
		samples[1] += steps[1];
	}
}

void set_hwparams(AlsaDriver* self, snd_pcm_hw_params_t* params, snd_pcm_access_t access)
{
	snd_pcm_uframes_t size;
	int err;

	// choose all parameters
	err = snd_pcm_hw_params_any(self->handle, params);
	if (err < 0) {		
		fprintf(stderr, "psycle: alsa: broken configuration for playback: no configurations available: %s\n", snd_strerror(err));		
	}
#if 0
	// set hardware resampling
	err = snd_pcm_hw_params_set_rate_resample(handle, params, resample);
	if (err < 0) {
		std::ostringstream s;
		s << "psycle: alsa: resampling setup failed for playback: " << snd_strerror(err);
		throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
#endif
	// set the interleaved read/write format
	err = snd_pcm_hw_params_set_access(self->handle, params, access);
	if (err < 0) {		
		fprintf(stderr, "psycle: alsa: access type not available for playback: %s\n", snd_strerror(err));		
	}
	// set the sample format
	err = snd_pcm_hw_params_set_format(self->handle, params, self->format);
	if (err < 0) {		
		fprintf(stderr, "psycle: alsa: sample format not available for playback: %s\n", snd_strerror(err));		
	}
	// set the count of channels
	err = snd_pcm_hw_params_set_channels(self->handle, params, self->channels);
	if (err < 0) {		
		fprintf(stderr, "psycle: alsa: channels count (%d) not available for playback: %s\n",
            self->channels, snd_strerror(err));		
	}

	// set the stream rate
	{
		unsigned int rrate = self->rate;
		err = snd_pcm_hw_params_set_rate_near(self->handle, params, &rrate, 0);
		if (err < 0) {			
			fprintf(stderr, "psycle: alsa: rate %d Hz not available for playback: ",
                self->rate, snd_strerror(err));			
		}
		if (rrate != self->rate) {
			///\todo don't throw an exception, but inform the caller that the actual settings differ.			
			fprintf(stderr, "psycle: alsa: rate does not match (requested %d Hz, got %d Hz %d",
                (int)self->rate, (int) rrate, err);			
		}
	}

	// set the buffer time
	err = snd_pcm_hw_params_set_buffer_time_near(self->handle, params,
        &self->buffer_time, 0);
	if (err < 0) {		
		fprintf(stderr, "psycle: alsa: unable to set buffer time %d for playback: ",
            self->buffer_time, snd_strerror(err));		
	}

	err = snd_pcm_hw_params_get_buffer_size(params, &size);
	if (err < 0) {		
		fprintf(stderr, "psycle: alsa: unable to get buffer size for playback: %s\n",
            snd_strerror(err));		
	}

	// if (loggers::trace()) {
//		std::ostringstream s;
		fprintf(stderr, "psycle: alsa: buffer size: %u\n", size);		
	//}
	self->buffer_size = size;

	// set the period time
	err = snd_pcm_hw_params_set_period_time_near(self->handle, params,
        &self->period_time, 0);
	if (err < 0) {		
		fprintf(stderr, "psycle: alsa: unable to set period time %d for playback: \n",
            (int)self->period_time, snd_strerror(err));		
	}

	err = snd_pcm_hw_params_get_period_size(params, &size, 0);
	if (err < 0) {		
		fprintf(stderr, "psycle: alsa: unable to get period size for playback: %s\n",
            snd_strerror(err));
	}

	self->period_size = size;

	// write the parameters to device
	err = snd_pcm_hw_params(self->handle, params);
	if (err < 0) {		
		fprintf(stderr, "psycle: alsa: unable to set hw params for playback: %s\n",
            snd_strerror(err));		
	}
}

void set_swparams(AlsaDriver* self, snd_pcm_sw_params_t* swparams) {
	int err;
	// get the current swparams
	err = snd_pcm_sw_params_current(self->handle, swparams);
	if (err < 0) {		
		fprintf(stderr, "psycle: alsa: unable to determine current swparams for playback: %s\n",
            snd_strerror(err));
	}
	// start the transfer when the buffer is almost full:
	// (buffer_size / avail_min) * avail_min
	err = snd_pcm_sw_params_set_start_threshold(self->handle, swparams,
        (self->buffer_size / self->period_size) * self->period_size);
	if (err < 0) {
		fprintf(stderr, "psycle: alsa: unable to set start threshold mode for playback: %s\n",
            snd_strerror(err));
	}
	// allow the transfer when at least period_size samples can be processed
	err = snd_pcm_sw_params_set_avail_min(self->handle, swparams, self->period_size);
	if (err < 0) {		
		fprintf(stderr, "psycle: alsa: unable to set avail min for playback: %s\n",
            snd_strerror(err));
	}
	// align all transfers to 1 sample
#if SND_LIB_VERSION >= 0x10010 // 1.0.16
	// snd_pcm_sw_params_set_xfer_align() is deprecated, alignment is always 1
#else
	err = snd_pcm_sw_params_set_xfer_align(self->handle, swparams, 1);
	if (err < 0) {		
		fprintf(stderr, "psycle: alsa: unable to set transfer align for playback: %s\n",
            snd_strerror(err));
	}
#endif
	// write the parameters to the playback device
	err = snd_pcm_sw_params(self->handle, swparams);
	if (err < 0) {		
		fprintf(stderr, "psycle: alsa: unable to set sw params for playback: %s\n",
            snd_strerror(err));
	}
}

const psy_AudioDriverInfo* driver_info(psy_AudioDriver* self)
{
	return GetPsycleDriverInfo();
}

const psy_Property* driver_configuration(const psy_AudioDriver* driver)
{
	AlsaDriver* self = (AlsaDriver*)driver;
	
	return self->configuration;
}

uint32_t playposinsamples(psy_AudioDriver* driver)
{
	AlsaDriver* self = (AlsaDriver*)driver;

	return 0;
}
