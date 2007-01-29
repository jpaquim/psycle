/***************************************************************************
*   Copyright (C) 2007 by  Stefan Nattkemper and others                   *
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

#if defined _WIN64 || defined _WIN32 || defined __CYGWIN__ || defined __MSYS__ || defined _UWIN

#include "mswaveoutplugin.h"
#include <mmsystem.h>

/* is plugin initialized ? */
static int _initialized = 0;
/* mme variables */
/* use this audio handle to detect if the driver is working*/
static HWAVEOUT hWaveOut = 0;
/* device handle */
static CRITICAL_SECTION waveCriticalSection;
/* array of header structure */
/* that points to a block buffer */
static WAVEHDR*         waveBlocks; 
static volatile int     waveFreeBlockCount;
static int              waveCurrentBlock;
// mme functions
// waveOut interface notifies about device is opened, closed, 
// and what we handle here, when a block finishes.
static void CALLBACK waveOutProc(HWAVEOUT, UINT, DWORD, DWORD, DWORD);      
static WAVEHDR* allocateBlocks();
static void freeBlocks( WAVEHDR* blockArray );
// writes a intermediate buffer into a ring buffer to the sound card
static void writeAudio( HWAVEOUT hWaveOut, LPSTR data, int size );
// thread , the writeAudio loop is in
// note : waveOutproc is a different, thread, too, but we cant
// use all winapi calls there we need due to restrictions of the winapi
static HANDLE hThread_;
static DWORD WINAPI audioOutThread( void *pWaveOut );
/* running condition for the thread loop */
/* check, if thread loop should be left */
static int _running = 0; 
static void fillBuffer(void);
static int _dither;
static int start(void);
static int stop(void);
static void quantizeWithDither(float *pin, int *piout, int c);
static void quantize(float *pin, int *piout, int c);
static PsyProcessCallback _pCallback = 0;
static PsyAudioSettings settings_;
static void* arg_ = 0;

static int msWaveOutInit(void) {
  return 1;
}

static int msWaveOutOpen( PsyAudioSettings settings ) {
  settings_ = settings;
  return start();
}

static int msWaveOutClose(void) {
  return stop();
}

static int msWaveOutPause(void) {
  /* todo */
  return 1;
}

static int msWaveOutSetCallback( PsyProcessCallback processCallback, void *arg) {
  _pCallback = processCallback;
  arg_ = arg;
  return 1;
}

static int const SHORT_MIN = -32768;
static int const SHORT_MAX =  32767;


WAVEHDR* allocateBlocks( )
{

  unsigned char* buffer;
  int i;
  WAVEHDR* blocks;
  DWORD totalBufferSize = (settings_.blockSize + sizeof(WAVEHDR)) * settings_.blockCount;

  //
  // allocate memory for the entire set in one go
  //
  if((buffer = (unsigned char*)( HeapAlloc(
    GetProcessHeap(), 
    HEAP_ZERO_MEMORY, 
    totalBufferSize
    ))) == NULL) {
      return 0;
  }
  //
  // and set up the pointers to each bit
  //
  blocks = (WAVEHDR*)buffer;
  buffer += sizeof(WAVEHDR) * settings_.blockCount;
  for(i = 0; i < settings_.blockCount; i++) {
    blocks[i].dwBufferLength = settings_.blockSize;
    blocks[i].lpData = (CHAR*)( buffer );
    buffer += settings_.blockSize;
  }

  return blocks;
}

void freeBlocks(WAVEHDR* blockArray)
{
  //
  // and this is why allocateBlocks works the way it does
  // 
  HeapFree( GetProcessHeap(), 0, blockArray );
}	       

void writeAudio( HWAVEOUT hWaveOut, LPSTR data, int size )
{
  WAVEHDR* current;
  DWORD_PTR remain;

  current = &waveBlocks[waveCurrentBlock];

  while(size > 0) {
    // 
    // first make sure the header we're going to use is unprepared
    //
    if(current->dwFlags & WHDR_PREPARED) 
      waveOutUnprepareHeader(hWaveOut, current, sizeof(WAVEHDR));

    if( size < (int)( settings_.blockSize - current->dwUser) ) {
      memcpy(current->lpData + current->dwUser, data, size);
      current->dwUser += size;
      break;
    }

    remain = settings_.blockSize - current->dwUser;
    memcpy(current->lpData + current->dwUser, data, remain);
    size -= remain;
    data += remain;
    current->dwBufferLength = settings_.blockSize;

    waveOutPrepareHeader(hWaveOut, current, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, current, sizeof(WAVEHDR));

    EnterCriticalSection(&waveCriticalSection);
    waveFreeBlockCount--;
    LeaveCriticalSection(&waveCriticalSection);

    //
    // wait for a block to become free
    //
    while(!waveFreeBlockCount) {
      Sleep(10);
    }

    //
    // point to the next block
    //
    waveCurrentBlock++;
    waveCurrentBlock %= settings_.blockCount;

    current = &waveBlocks[waveCurrentBlock];
    current->dwUser = 0;
  }
}


// called by the waveOut interface as different thread!
// thus we need CriticalSection for the block counter
void CALLBACK waveOutProc(
                          HWAVEOUT hWaveOut, 
                          UINT uMsg, 
                          DWORD dwInstance,  
                          DWORD dwParam1,    
                          DWORD dwParam2     
                          )
{
  int* freeBlockCounter = (int*)dwInstance;
  //
  // ignore calls that occur due to openining and closing the
  // device.
  //
  if(uMsg != WOM_DONE)
    return;

  EnterCriticalSection(&waveCriticalSection);
  (*freeBlockCounter)++;
  LeaveCriticalSection(&waveCriticalSection);
}

void fillBuffer() {
  /* this protects freeBlockCounter, that is manipulated from two threads. */
  /* the waveOut interface callback WM_Done thread in waveOutProc and in writeAudio */
  //          InitializeCriticalSection( &waveCriticalSection );
  int bufSize;
  bufSize = 1024 / 2;
  //          psy::tr1::int16_t buf[1024 / 2];
  //          int newCount = bufSize / 2;        
  /*          while ( _running ) {
  float const * input(_pCallback(_callbackContext, newCount));              
  for (int i = 0; i < bufSize; i++) {
  buf[i] = *input++;
  }       
  writeAudio(hWaveOut, (CHAR*) buf, sizeof(buf) );
  }*/
}

DWORD WINAPI audioOutThread( void* pWaveOut ) {
  SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );
  fillBuffer();
  return 0;
}

static WAVEFORMATEX format;
static DWORD dwThreadId;

int start()
{
  if ( hWaveOut ) return 0;   // do not start again
  if(!_pCallback) return 1;  // no player callback

  // WAVEFORMATEX is defined in mmsystem.h
  // this structure is used, to define the sampling rate, sampling resolution
  // and the number of channles
  // some informations are redundant, but you need to define them
  format.wFormatTag = WAVE_FORMAT_PCM;
  format.wBitsPerSample = settings_.bitDepth;
  format.nSamplesPerSec = settings_.samplesPerSec;
  format.nChannels = 2;
  format.nBlockAlign = ( format.nChannels * format.wBitsPerSample ) >> 3;
  format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
  format.cbSize = 0;

  _dither = 0;

  // the buffer block variables
  if (!(waveBlocks = allocateBlocks())) return 0; // memory error
  waveFreeBlockCount =  settings_.blockCount;
  waveCurrentBlock   = 0;

  // this will protect the monitor buffer counter variable 


  if( waveOutOpen( &hWaveOut, WAVE_MAPPER, &format, (DWORD_PTR)waveOutProc, 
    (DWORD_PTR)&waveFreeBlockCount, 
    CALLBACK_FUNCTION ) != MMSYSERR_NOERROR)
  {
    return 0;
  }

  _running = 1;

  hThread_ = CreateThread( NULL, 0, audioOutThread, 0, 0, &dwThreadId );

  return 1;
}



int stop()
{
  if(!_running) return 1;
  TerminateThread( hThread_, 0 );
  return 1;
}


void quantize(float *pin, int *piout, int c)
{
  /*do
  {
  int r = f2i( (pin[1]) );

  if (r < SHORT_MIN)
  {
  r = SHORT_MIN;
  }
  else if (r > SHORT_MAX)
  {
  r = SHORT_MAX;
  }

  int l = f2i( (pin[0]) );

  if (l < SHORT_MIN)
  {
  l = SHORT_MIN;
  }
  else if (l > SHORT_MAX)
  {
  l = SHORT_MAX;
  }

  *piout++ = (r << 16) | static_cast<psy::tr1::uint16_t>(l);
  pin += 2;
  }
  while(--c);*/
}


#endif // windows platform
