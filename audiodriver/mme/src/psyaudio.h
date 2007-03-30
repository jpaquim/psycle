/***************************************************************************
*   Copyright (C) 2007 by  Stefan Nattkemper   *
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
#ifndef PSYAUDIO_H
#define PSYAUDIO_H

#ifdef __cplusplus
extern "C" {
#endif
/* body of header */


/* type definition */
typedef int(* 	PsyProcessCallback )(unsigned int nframes, void *arg);
typedef struct PsyAudioOut PsyAudioOut;
typedef struct PsyAudioSettings PsyAudioSettings;

struct PsyAudioSettings {
  int samplesPerSec;
  int bitDepth;
  int channelSize;
  int bufferSize;
  int blockSize;
  int blockCount;
  char* deviceName;
};

struct PsyAudioOut {
  // the driver name, used e.g in the configuration for the driver map
  // as convention, please use only lower case names here
  char* name;
  // short description      
  char* header;
  // deeper explanation
  char* description; 
  // hint, if audiodialog should list the driver
  int show; 

  int (*init)  (void);  // inits the plugin
  int (*open)  (void);  // open and starts the device
  int (*close) (void);  // closes the device
  int (*lock)   (void);  // locks the callback
  int (*unlock) (void);  // unlocks the callback
  int (*setCallback) ( PsyProcessCallback process_callback, void *arg); // sets the callback to the work method
  void (*setSettings) ( PsyAudioSettings settings );
  PsyAudioSettings* (*settings) (void);
  int (*channelSize) (void); // how many channels has the device available
  float* (*buffer) (int); // returns float array start to the buffer of a channel 
                          // if device is stereo 0 is left, and 1 shall be right
};


/*psyAudioDevice* psyNewAudio();
void psyAudioStart( psyAudioDevice* device );
void psyAudioStop( psyAudioDevice* device );
float* psyAudioGetBuffer( psyAudioDevice* device, int portHandle, unsigned int frames );
char** psyAudioPorts( psyAudioDevice* device);

// this defines the callback to a work function
int psySetCallback( psyAudioDevice *device, PsyProcessCallback process_callback, void *arg);*/

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif


#endif
