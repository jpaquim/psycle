// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "waveio.h"
#include "fileio.h"
#include <string.h>
#include <stdlib.h> 

void wave_load(psy_audio_Sample* sample, const char* path)
{
	char header[9];
	PsyFile file;

	if (!psyfile_open(&file, path)) {
		return;
	}
	psyfile_read(&file, header, 4);
	header[4] = 0;		
	if (strcmp(&header[0],"RIFF") == 0) {
		unsigned long chunksize;
		unsigned long pcmsize;		

		psyfile_read(&file, &chunksize, sizeof(chunksize));
		psyfile_read(&file, header, 8);
		header[8] = 0;
		if (strcmp(&header[0], "WAVEfmt ") == 0) {
			psy_audio_WaveFormatChunk format;
			psyfile_read(&file, &pcmsize, sizeof(pcmsize));
			psyfile_read(&file, &format.wFormatTag, 2);
			psyfile_read(&file, &format.nChannels, 2);
			psyfile_read(&file, &format.nSamplesPerSec, 4);
			psyfile_read(&file, &format.nAvgBytesPerSec, 4);
			psyfile_read(&file, &format.nBlockAlign, 2);
			psyfile_read(&file, &format.wBitsPerSample, 2);		
			// psyfile_read(&file, &format.cbSize, 2);
			sample->samplerate = format.nSamplesPerSec;
			buffer_resize(&sample->channels, format.nChannels);
			psyfile_read(&file, header, 4);
			header[4] = 0;			
			if (strcmp(&header[0], "data") == 0) {
				int numsamples;
				unsigned int frame;
				int channel;
				char frame8;
				short frame16;
				psyfile_read(&file, &numsamples, 4);
				sample->numframes = numsamples / 
					format.nChannels / (format.wBitsPerSample / 8);
				sample->stereo = (format.nChannels == 2);

				for (channel = 0; channel < format.nChannels; ++channel) {
					sample->channels.samples[channel] = malloc(numsamples * 
						sizeof(psy_dsp_amp_t));
				}
				for (frame = 0; frame < sample->numframes; ++frame) {
					for (channel = 0; channel < format.nChannels; ++channel) {
						switch (format.wBitsPerSample) {
							case 8: 
								psyfile_read(&file, &frame8, 1);
								sample->channels.samples[channel][frame] = frame8;
							break;
							case 16: 
								psyfile_read(&file, &frame16, 2);
								sample->channels.samples[channel][frame] = frame16;
							break;
							default:
							break;
						}
					}
				}				
			}
		}
	}
	psyfile_close(&file);
}

void wave_save(psy_audio_Sample* sample, const char* path)
{
	PsyFile file;
	psy_audio_WaveFormatChunk format;
	int bitspersample = 16;
	uint8_t temp8 = 0;
	uint16_t temp16 = 0;
	uint32_t temp32 = 0;	
	uint32_t begin = 0;
	uint32_t pcmbegin = 0;
	uint32_t channel = 0;
	uint32_t frame = 0;
	uint32_t numsamples = 0;

	if (!psyfile_create(&file, path, 1)) {
		return;
	}
	psyfile_write(&file, "RIFF", 4);	
	begin = psyfile_getpos(&file);
	temp32 = 0;
	psyfile_write(&file, &temp32, sizeof(temp32));
	// Write Format Chunk
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = sample->channels.numchannels;
	format.nSamplesPerSec = sample->samplerate;
	format.nAvgBytesPerSec = sample->channels.numchannels *
		sample->samplerate * bitspersample / 8;
	format.nBlockAlign = sample->channels.numchannels *
		bitspersample / 8;
	format.wBitsPerSample = bitspersample;	
	format.cbSize = 0;
	psyfile_write(&file, "WAVEfmt ", 8);
	temp32 = 0;
	pcmbegin = psyfile_getpos(&file);
	psyfile_write(&file, &temp32, sizeof(temp32));	
	temp16 = format.wFormatTag;
	psyfile_write(&file, &temp16, sizeof(temp16));
	temp16 = format.nChannels;
	psyfile_write(&file, &temp16, sizeof(temp16));
	temp32 = format.nSamplesPerSec;
	psyfile_write(&file, &temp32, sizeof(temp32));
	temp32 = format.nAvgBytesPerSec;
	psyfile_write(&file, &temp32, sizeof(temp32));
	temp16 = format.nBlockAlign;
	psyfile_write(&file, &temp16, sizeof(temp16));
	temp16 = format.wBitsPerSample;
	psyfile_write(&file, &temp16, sizeof(temp16));
	{
		uint32_t pos2;
		uint32_t size;
	
		pos2 = psyfile_getpos(&file); 
		size = (pos2 - pcmbegin - 4);
		psyfile_seek(&file, pcmbegin);
		psyfile_write(&file, &size, sizeof(size));
		psyfile_seek(&file, pos2);
	}
	// Write Data Chunk
	psyfile_write(&file, "data", 4);
	numsamples = sample->numframes * sample->channels.numchannels * (bitspersample / 8);
	temp32 = numsamples;
	psyfile_write(&file, &temp32, sizeof(temp32));
	
	for (frame = 0; frame < sample->numframes; ++frame) {
		for (channel = 0; channel < sample->channels.numchannels; ++channel) {		
			temp16 = (uint16_t) sample->channels.samples[channel][frame];
			psyfile_write(&file, &temp16, sizeof(temp16));				
		}		
	}	

	{
		uint32_t pos2;
		uint32_t size;
	
		pos2 = psyfile_getpos(&file); 
		size = (pos2 - begin - 4);
		psyfile_seek(&file, begin);
		psyfile_write(&file, &size, sizeof(size));
		psyfile_seek(&file, pos2);
	}	
	psyfile_close(&file);
}
