// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "waveio.h"
#include "fileio.h"
#include <string.h>
#include <stdlib.h> 

void wave_load(Sample* sample, const char* path)
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
			WaveFormatChunk format;						
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
				int frame;
				int channel;
				char frame8;
				short frame16;
				psyfile_read(&file, &numsamples, 4);
				sample->numframes = numsamples / 
					format.nChannels / (format.wBitsPerSample / 8);	

				for (channel = 0; channel < format.nChannels; ++channel) {
					sample->channels.samples[channel] = malloc(numsamples*sizeof(amp_t));
				}
				for (frame = 0; frame < numsamples; ++frame) {
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
