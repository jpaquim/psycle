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

/* type definition */

typedef int(* 	PsyProcessCallback )(unsigned int nframes, void *arg)
typedef struct _psyAudioDevice  psyDevice_t;

/* body of header */

psyDevice_t* psyNewAudio();
void psyAudioStart( psyDevice_t* device );
void psyAudioStop( psyDevice_t* device );
float* psyAudioGetBuffer( psyDevice_t* device, int portHandle, unsigned int frames );
char** psyAudioPorts( psyDevice_t* device);

// this defines the callback to a work function
int psySetCallback( psyDevice_t *device, PsyProcessCallback process_callback, void *arg);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif


#endif
