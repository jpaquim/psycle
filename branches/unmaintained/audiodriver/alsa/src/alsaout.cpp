/***************************************************************************
  *   Copyright (C) 2006 by Stefan   *
  *   natti@linux   *
  *                                                                         *
  *   This program is free software; you can redistribute it and/or modify  *
  *   it under the terms of the GNU General Public License as published by  *
  *   the Free Software Foundation; either version 2 of the License, or     *
  *   (at your option) any later version.                                   *
  *                                                                         *
  *   This program is distributed in the hope that it will be useful,       *
  *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
  *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
  *   GNU General Public License for more details.                          *
  *                                                                         *
  *   You should have received a copy of the GNU General Public License     *
  *   along with this program; if not, write to the                         *
  *   Free Software Foundation, Inc.,                                       *
  *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
  ***************************************************************************/
#if defined XPSYCLE__CONFIGURATION
	#include <xpsycle/alsa_conditional_build.h>
#endif
#if !defined XPSYCLE__NO_ALSA

#include "alsaout.h"
#include "global.h"
#include "configuration.h"
#include "player.h"

namespace psycle { namespace host {

int AlsaOut::enablePlayer = 0;
bool AlsaOut::running = false;


AlsaOut::AlsaOut( ) :
    AudioDriver(),
//    _running(false),
    _initialized(false)
//   , _timerActive(false)
{
	setDefault();
}

AlsaOut::~AlsaOut()
{
}

AlsaOut * AlsaOut::clone( ) const
{
  return new AlsaOut(*this);
}

AudioDriverInfo AlsaOut::info( ) const
{
  return AudioDriverInfo("alsa","Alsa Driver","Low Latency audio driver",true);
}

void AlsaOut::Initialize(AUDIODRIVERWORKFN pCallback, void * context )
{
  _pCallback = pCallback;
  _callbackContext = context;
  _initialized = true;
  running = false;
  audioStart();
}

bool AlsaOut::Enable( bool e )
{
  return e ? Start() : Stop();
}

bool AlsaOut::Start( )
{
  if(running) return true;
  AlsaOut::enablePlayer = running = 1;
}

bool AlsaOut::Stop( )
{
//  _timerActive = false;
  audioStop();
}



void AlsaOut::configure( )
{
}


////////////// Begin audio.c to c++



void AlsaOut::setDefault( )
{
		rate = 44100; // stream rate
    format = SND_PCM_FORMAT_S16; // sample format
    channels = 2; // count of channels
    buffer_time = 80000; // ring buffer length in us
    period_time = 20000; // period time in us

    buffer_size;
    period_size;
    output = NULL;
//    AlsaOut::enablePlayer = 1; // has stopped, 0: stop!, 1: play!, 2: is playing

}

int AlsaOut::audioStop( )
{
  AlsaOut::enablePlayer = 0;

  while (running) {
      usleep(100);
  }
}

int AlsaOut::audioStart(  )
{
  int err;
  snd_pcm_hw_params_t *hwparams;
  snd_pcm_sw_params_t *swparams;
  unsigned int chn;

  snd_pcm_hw_params_alloca(&hwparams);
  snd_pcm_sw_params_alloca(&swparams);

  err = snd_output_stdio_attach(&output, stdout, 0);
  if (err < 0) {
      printf("Output failed: %s\n", snd_strerror(err));
      return 0;
  }

  printf("Playback device is %s\n", settings().deviceName().c_str() );
  printf("Stream parameters are %iHz, %s, %i channels\n", rate, snd_pcm_format_name(format), channels);
  printf("Using transfer method: %s\n", "write");

	std::cout << "step1" << std::endl;

  if ((err = snd_pcm_open(&handle, settings().deviceName().c_str() , SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    printf("Playback open error: %s\n", snd_strerror(err));
    return 0;
  }

  std::cout << "step2" << std::endl;

  if ((err = set_hwparams(handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    printf("Setting of hwparams failed: %s\n", snd_strerror(err));
    exit(EXIT_FAILURE);
  }

	std::cout << "step3" << std::endl;

  if ((err = set_swparams(handle, swparams)) < 0) {
    printf("Setting of swparams failed: %s\n", snd_strerror(err));
    exit(EXIT_FAILURE);
  }

	std::cout << "step4" << std::endl;

  samples = (short int*) malloc((period_size * channels * snd_pcm_format_width(format)) / 8);
  if (samples == NULL) {
      printf("No enough memory\n");
      exit(EXIT_FAILURE);
  }

	std::cout << "step5" << std::endl;

  areas = (snd_pcm_channel_area_t*) calloc(channels, sizeof(snd_pcm_channel_area_t));
  if (areas == NULL) {
      printf("No enough memory\n");
      exit(EXIT_FAILURE);
  }

	std::cout << "step6" << std::endl;

  for (chn = 0; chn < channels; chn++) {
    areas[chn].addr = samples;
    areas[chn].first = chn * 16;
    areas[chn].step = channels * 16;
  }

	std::cout << "step7" << std::endl;

  if (0 == pthread_create(&threadid, NULL, (void*(*)(void*))audioOutThread, (void*) this))
  {	
	  AlsaOut::enablePlayer = 1;
	  return EXIT_SUCCESS;
  }
  return 1;
}

void AlsaOut::FillBuffer(const snd_pcm_channel_area_t * areas, snd_pcm_uframes_t offset, int count )
{
  signed short *samples[channels];
  int steps[channels];
  unsigned int chn;
  // verify and prepare the contents of areas */
  for (chn = 0; chn < channels; chn++) {
    if ((areas[chn].first % 8) != 0) {
      printf("areas[%i].first == %i, aborting...\n", chn, areas[chn].first);
      exit(EXIT_FAILURE);
    }
    samples[chn] = (signed short *)(((unsigned char *)areas[chn].addr) + (areas[chn].first / 8));
    if ((areas[chn].step % 16) != 0) {
      printf("areas[%i].step == %i, aborting...\n", chn, areas[chn].step);
      exit(EXIT_FAILURE);
    }
    steps[chn] = areas[chn].step / 16;
    samples[chn] += offset * steps[chn];
  }
  // fill the channel areas

  float const * input(_pCallback(_callbackContext, count));

  while (count-- > 0) {
      *samples[0] = static_cast<short int>( *input++ );
      samples[0] += steps[0];
      *samples[1] = static_cast<short int>( *input++ );
      samples[1] += steps[1];
  }
}

int AlsaOut::set_hwparams(snd_pcm_t *handle,
                        snd_pcm_hw_params_t *params,
                        snd_pcm_access_t access)
{
    unsigned int rrate;
    snd_pcm_uframes_t size;
    int err, dir;

		std::cout << "step2.1" << std::endl;
    // choose all parameters
    err = snd_pcm_hw_params_any(handle, params);
    if (err < 0) {
                printf("Broken configuration for playback: no configurations available: %s\n", snd_strerror(err));
                return err;
        }
    /* set hardware resampling
    err = snd_pcm_hw_params_set_rate_resample(handle, params, resample);
    if (err < 0) {
          printf("Resampling setup failed for playback: %s\n", snd_strerror(err));
                return err;
        }
    */
		std::cout << "step2.2" << std::endl;
        // set the interleaved read/write format
    err = snd_pcm_hw_params_set_access(handle, params, access);
    if (err < 0) {
                printf("Access type not available for playback: %s\n", snd_strerror(err));
                return err;
    }
		std::cout << "step2.3" << std::endl;
    // set the sample format
    err = snd_pcm_hw_params_set_format(handle, params, format);
    if (err < 0) {
          printf("Sample format not available for playback: %s\n", snd_strerror(err));
          return err;
    }

		std::cout << "step2.4" << std::endl;
    // set the count of channels
    err = snd_pcm_hw_params_set_channels(handle, params, channels);
    if (err < 0) {
          printf("Channels count (%i) not available for playbacks: %s\n", channels, snd_strerror(err));
          return err;
    }
    // set the stream rate
		std::cout << "step2.5" << std::endl;
    rrate = rate;		
    err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
    if (err < 0) {
        printf("Rate %iHz not available for playback: %s\n", rate, snd_strerror(err));
        return err;
    }

		std::cout << "step2.6" << std::endl;
    if (rrate != rate) {
          printf("Rate doesn't match (requested %iHz, get %iHz)\n", rate, err);
          return -EINVAL;
    }

		std::cout << "step2.7" << std::endl;
    // set the buffer time
    err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &buffer_time, 0);
    if (err < 0) {
          printf("Unable to set buffer time %i for playback: %s\n", buffer_time, snd_strerror(err));
          return err;
    }

		std::cout << "step2.8" << std::endl;
    err = snd_pcm_hw_params_get_buffer_size(params, &size);
    if (err < 0) {
          printf("Unable to get buffer size for playback: %s\n", snd_strerror(err));
          return err;
    }

		std::cout << "step2.9" << std::endl;
    buffer_size = size;
		std::cout << "step2.9.1" << std::endl;
    // set the period time
    err = snd_pcm_hw_params_set_period_time_near(handle, params, &period_time, 0);
    if (err < 0) {
          printf("Unable to set period time %i for playback: %s\n", period_time, snd_strerror(err));
          return err;
    }

		std::cout << "step2.10" << std::endl;
    err = snd_pcm_hw_params_get_period_size(params, &size, 0);
    if (err < 0) {
        printf("Unable to get period size for playback: %s\n", snd_strerror(err));
        return err;
    }

		std::cout << "step2.10" << std::endl;
    period_size = size;
		std::cout << "step2.10.1" << std::endl;
    // write the parameters to device
    err = snd_pcm_hw_params(handle, params);
    if (err < 0) {
        printf("Unable to set hw params for playback: %s\n", snd_strerror(err));
        return err;
    }
    return 0;
}



int AlsaOut::set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams)
{
        int err;

        // get the current swparams
        err = snd_pcm_sw_params_current(handle, swparams);
        if (err < 0) {
                printf("Unable to determine current swparams for playback: %s\n", snd_strerror(err));
                return err;
        }
        // start the transfer when the buffer is almost full:
        // (buffer_size / avail_min) * avail_min
        err = snd_pcm_sw_params_set_start_threshold(handle, swparams, (buffer_size / period_size) * period_size);
        if (err < 0) {
                printf("Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
                return err;
        }
        // allow the transfer when at least period_size samples can be processed
        err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_size);
        if (err < 0) {
                printf("Unable to set avail min for playback: %s\n", snd_strerror(err));
                return err;
        }
        // align all transfers to 1 sample
        err = snd_pcm_sw_params_set_xfer_align(handle, swparams, 1);
        if (err < 0) {
                printf("Unable to set transfer align for playback: %s\n", snd_strerror(err));
                return err;
        }
        // write the parameters to the playback device
        err = snd_pcm_sw_params(handle, swparams);
        if (err < 0) {
                printf("Unable to set sw params for playback: %s\n", snd_strerror(err));
                return err;
        }
        return 0;
}


//
//   Underrun and suspend recovery
//

int AlsaOut::xrun_recovery(snd_pcm_t *handle, int err)
{
  if (err == -EPIPE) {    /* under-run */
    err = snd_pcm_prepare(handle);
    if (err < 0) printf("Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
    return 0;
  } else if (err == -ESTRPIPE) {
    while ((err = snd_pcm_resume(handle)) == -EAGAIN)
        sleep(1);       /* wait until the suspend flag is released */
        if (err < 0) {
          err = snd_pcm_prepare(handle);
          if (err < 0) printf("Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
        }
        return 0;
  }
  return err;
}

int AlsaOut::write_loop(snd_pcm_t *handle, signed short *samples, snd_pcm_channel_area_t *areas)
{
  signed short *ptr;
  int err, cptr;
  if (enablePlayer > 0) enablePlayer = 2;
  while (1) {
//    in = true;
    if (enablePlayer >0) {
    FillBuffer(areas, 0, period_size);
    ptr = samples;
    cptr = period_size;
    while (cptr > 0) {
      err = snd_pcm_writei(handle, ptr, cptr);
      if (err == -EAGAIN)
        continue;
      if (err < 0) {
        if (xrun_recovery(handle, err) < 0) {
          printf("Write error: %s\n", snd_strerror(err));
          exit(EXIT_FAILURE);
        }
        break;  /* skip one period */
      }
      ptr += err * channels;
      cptr -= err;
    }
   }
	else
	{
		usleep(1000);
	   running = false;
	}
  }
}

struct transfer_method {
  const char *name;
  snd_pcm_access_t access;
  int (*transfer_loop)(snd_pcm_t *handle, signed short *samples, snd_pcm_channel_area_t *areas);
};

int AlsaOut::AlsaOut::audioOutThread( void * ptr )
{
  AlsaOut* pDriver = (AlsaOut*) ptr;
  int err = pDriver->write_loop(pDriver->handle,pDriver->samples,pDriver->areas);
  if (err < 0) printf("Transfer failed: %s\n", snd_strerror(err));
  pDriver->enablePlayer = -2;
  pthread_exit(0);
  return err;
}


bool AlsaOut::Initialized( )
{
  return _initialized;
}

}}



#endif // !defined XPSYCLE__NO_ALSA