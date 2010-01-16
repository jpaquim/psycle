// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#if defined PSYCLE__MICROSOFT_MME_AVAILABLE
#include "microsoftmmewaveout.h"
#include <universalis/os/aligned_memory_alloc.hpp>

namespace psycle { namespace audiodrivers {

int const SHORT_MIN = -32768;
int const SHORT_MAX =  32767;

bool MsWaveOut::_running = 0;
CRITICAL_SECTION MsWaveOut::waveCriticalSection;
WAVEHDR*         MsWaveOut::waveBlocks;
volatile int     MsWaveOut::waveFreeBlockCount;
int              MsWaveOut::waveCurrentBlock;

AudioDriverInfo MsWaveOut::info( ) const {
	return AudioDriverInfo("mmewaveout","Microsoft MME WaveOut Driver","Microsoft legacy output driver",true);
}      

MsWaveOut::MsWaveOut() {
	hWaveOut = 0; // use this audio handle to detect if the driver is working
	_running = 0; // running condition for the thread loop
	buf = 0;
}

MsWaveOut::~MsWaveOut() throw() {
	before_destruction();
}

WAVEHDR* MsWaveOut::allocateBlocks( ) {
	unsigned char* buffer;
	std::uint32_t i;
	WAVEHDR* blocks;
	DWORD totalBufferSize = (playbackSettings().blockBytes() + sizeof(WAVEHDR)) * playbackSettings().blockCount();

	//
	// allocate memory for the entire set in one go
	//
	if((buffer = static_cast<unsigned char*>( HeapAlloc(
		GetProcessHeap(),
		HEAP_ZERO_MEMORY,
		totalBufferSize
		))) == NULL) {
			std::cerr << "Memory allocation error\n" << std::endl;
			return 0;
	}
	//
	// and set up the pointers to each bit
	//
	blocks = (WAVEHDR*)buffer;
	buffer += sizeof(WAVEHDR) * playbackSettings().blockCount();
	for(i = 0; i < playbackSettings().blockCount(); i++) {
		blocks[i].dwBufferLength = playbackSettings().blockBytes();
		blocks[i].lpData = (CHAR*)( buffer );
		buffer += playbackSettings().blockBytes();
	}

	return blocks;
}

void MsWaveOut::freeBlocks(WAVEHDR* blockArray) {
	//
	// and this is why allocateBlocks works the way it does
	//
	HeapFree( GetProcessHeap(), 0, blockArray );
}

void MsWaveOut::writeAudio( HWAVEOUT hWaveOut, LPSTR data, int size ) {
	WAVEHDR* current;
	int remain;

	current = &waveBlocks[waveCurrentBlock];

	while(size > 0) {
		//
		// first make sure the header we're going to use is unprepared
		//
		if(current->dwFlags & WHDR_PREPARED)
			waveOutUnprepareHeader(hWaveOut, current, sizeof(WAVEHDR));

		if( size < static_cast<int>( playbackSettings().blockBytes() - current->dwUser) ) {
			memcpy(current->lpData + current->dwUser, data, size);
			current->dwUser += size;
			break;
		}

		remain = playbackSettings().blockBytes() - current->dwUser;
		memcpy(current->lpData + current->dwUser, data, remain);
		size -= remain;
		data += remain;
		current->dwBufferLength = playbackSettings().blockBytes();

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
		waveCurrentBlock %= playbackSettings().blockCount();

		current = &waveBlocks[waveCurrentBlock];
		current->dwUser = 0;
	}
}


// called by the waveOut interface as different thread!
// thus we need CriticalSection for the block counter
void CALLBACK MsWaveOut::waveOutProc(
	HWAVEOUT hWaveOut,
	UINT uMsg,
	DWORD dwInstance,
	DWORD dwParam1,
	DWORD dwParam2
) {
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

void MsWaveOut::fillBuffer() {
	// this protects freeBlockCounter, that is manipulated from two threads.
	// the waveOut interface callback WM_Done thread in waveOutProc and in writeAudio
	InitializeCriticalSection( &waveCriticalSection );
	while ( _running ) {
		float const * input(callback(playbackSettings().blockSamples()));
		Quantize16(input,buf,playbackSettings().blockSamples());
		writeAudio(hWaveOut, (CHAR*) buf, playbackSettings().blockSamples()*playbackSettings().numChannels()*sizeof(std::int16_t));
	}
}

DWORD WINAPI MsWaveOut::audioOutThread( void *pWaveOut ) {
	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );
	MsWaveOut * pThis = reinterpret_cast<MsWaveOut*>( pWaveOut ) ;
	pThis->fillBuffer();
	return 0;
}

void MsWaveOut::do_open()
{
	if(hWaveOut) throw std::runtime_error("............");
	// WAVEFORMATEX is defined in mmsystem.h
	// this structure is used, to define the sampling rate, sampling resolution
	// and the number of channles
	// some informations are redundant, but you need to define them
	WAVEFORMATEX format;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.wBitsPerSample = playbackSettings().bitDepth();
	format.nSamplesPerSec = playbackSettings().samplesPerSec();
	format.nChannels = 2;
	format.nBlockAlign = ( format.nChannels * format.wBitsPerSample ) >> 3;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
	format.cbSize = 0;

	_dither = 0;
	// the buffer block variables
	if(!(waveBlocks = allocateBlocks())) throw std::runtime_error("............");
	waveFreeBlockCount = playbackSettings().blockCount();
	waveCurrentBlock = 0;
	// this will protect the monitor buffer counter variable
	if( waveOutOpen( &hWaveOut, WAVE_MAPPER, &format, (DWORD_PTR)waveOutProc,
		(DWORD_PTR)&waveFreeBlockCount,
		CALLBACK_FUNCTION ) != MMSYSERR_NOERROR
	) throw std::runtime_error("waveOutOpen() failed");
	universalis::os::aligned_memory_alloc(16, buf, playbackSettings().blockSamples()*playbackSettings().numChannels());
}

void MsWaveOut::do_close() {
	if(waveOutClose(hWaveOut) != MMSYSERR_NOERROR) {
		hWaveOut=0;
		throw std::runtime_error("waveOutClose() failed");
	}
	hWaveOut=0;
	universalis::os::aligned_memory_dealloc(buf);
	buf=0;
}

void MsWaveOut::do_start() {
	_running = true;
	DWORD dwThreadId;
	_hThread = CreateThread( NULL, 0, audioOutThread, this, 0, &dwThreadId );
}

void MsWaveOut::do_stop() {
	if(!_running) return;
	_running=false;
	///\todo: some threadlocking mechanism. For now adding these sleeps
#if defined _WIN32
	Sleep(1000);
#else
	sleep(1);
#endif
	TerminateThread( _hThread, 0 ); // just in case
	if(::waveOutReset(hWaveOut) != MMSYSERR_NOERROR) {
		hWaveOut=0;
		throw std::runtime_error("waveOutReset() failed");
	}
}


}}
#endif
