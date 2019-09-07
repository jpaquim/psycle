#include "waveio.h"
#include "fileio.h"
#include <string.h>
#include <stdlib.h> 

void wave_load(Sample* sample, const char* path)
{
	char header[9];
	RiffFile file;
	if (!rifffile_open(&file, path)) {
		return;
	}

	rifffile_read(&file, header, 4);
	header[4] = 0;		
	if (strcmp(&header[0],"RIFF") == 0) {
		unsigned long chunksize;
		unsigned long pcmsize;		

		rifffile_read(&file, &chunksize, sizeof(chunksize));
		rifffile_read(&file, header, 8);
		header[8] = 0;
		if (strcmp(&header[0], "WAVEfmt ") == 0) {
			WaveFormatChunk format;						
			rifffile_read(&file, &pcmsize, sizeof(pcmsize));
			rifffile_read(&file, &format.wFormatTag, 2);
			rifffile_read(&file, &format.nChannels, 2);
			rifffile_read(&file, &format.nSamplesPerSec, 4);
			rifffile_read(&file, &format.nAvgBytesPerSec, 4);
			rifffile_read(&file, &format.nBlockAlign, 2);
			rifffile_read(&file, &format.wBitsPerSample, 2);		
			// rifffile_read(&file, &format.cbSize, 2);
			sample->samplerate = format.nSamplesPerSec;
			buffer_resize(&sample->channels, format.nChannels);
			rifffile_read(&file, header, 4);
			header[4] = 0;			
			if (strcmp(&header[0], "data") == 0) {
				int numsamples;
				int frame;
				int channel;
				char frame8;
				short frame16;
				rifffile_read(&file, &numsamples, 4);
				sample->numframes = numsamples / 
					format.nChannels / (format.wBitsPerSample / 8);	

				for (channel = 0; channel < format.nChannels; ++channel) {
					sample->channels.samples[channel] = malloc(numsamples*sizeof(real));
				}
				for (frame = 0; frame < numsamples; ++frame) {
					for (channel = 0; channel < format.nChannels; ++channel) {
						switch (format.wBitsPerSample) {
							case 8: 
								rifffile_read(&file, &frame8, 1);
								sample->channels.samples[channel][frame] = frame8;
							break;
							case 16: 
								rifffile_read(&file, &frame16, 2);
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
	rifffile_close(&file);
}