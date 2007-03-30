/***************************************************************************
 *   Copyright (C) 2006 by  Stefan Nattkemper   *
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
#ifndef MSWAVEOUTPLUGIN_H
#define MSWAVEOUTPLUGIN_H

#if defined __unix__ && !defined __CYGWIN__ && !defined __MSYS__ && !defined _UWIN
#else

#include "psyaudio.h"
#include <windows.h>
#include <mmsystem.h>

#ifdef __cplusplus
extern "C" {
#endif
/* body of header */

#if defined(_MSC_VER)
#ifdef __cplusplus
#define EXPORT extern "C" __declspec (dllexport)
#else
#define EXPORT __declspec (dllexport)
#endif 
#else 
#define EXPORT
#endif

static int msWaveOutInit(void);

static int msWaveOutOpen(void);

static int msWaveOutClose(void);

static int msWaveOutLock(void);

static int msWaveOutUnlock(void);

static int msWaveOutSetCallback( PsyProcessCallback process_callback, void *arg);

static void msWaveOutSetSettings( PsyAudioSettings settings );

static int msWaveOutChannelSize(void);

static float* msWaveOutBuffer( int channel );

static PsyAudioSettings* msWaveOutSettings(void);

PsyAudioOut msWaveOut = 
{
  "mswaveout",
  "Microsoft WaveOut Driver",
  "Microsoft standard output driver",
  1,
  msWaveOutInit,
  msWaveOutOpen,
  msWaveOutClose,
  msWaveOutLock,
  msWaveOutUnlock,
  msWaveOutSetCallback,
  msWaveOutSetSettings,
  msWaveOutSettings,
  msWaveOutChannelSize,
  msWaveOutBuffer
};

EXPORT PsyAudioOut *getPsyAudioOutPlugin(void)
{
	return &msWaveOut;
}
                   
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

/* end of windows detetection */
#endif 

#endif
