// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#if defined PSYCLE__MICROSOFT_MME_AVAILABLE
#include "microsoftmmewaveout.h"
#include <universalis/os/aligned_memory_alloc.hpp>

namespace psycle { namespace audiodrivers {

bool MsWaveOut::running_ = 0;
CRITICAL_SECTION MsWaveOut::waveCriticalSection;
WAVEHDR*         MsWaveOut::waveBlocks;
volatile int     MsWaveOut::waveFreeBlockCount;
int              MsWaveOut::waveCurrentBlock;

AudioDriverInfo MsWaveOut::info( ) const {
	return AudioDriverInfo("mmewaveout","Microsoft MME WaveOut Driver","Microsoft legacy output driver",true);
}      

MsWaveOut::MsWaveOut(MMEUiInterface* ui) {
	hWaveOut = 0; // use this audio handle to detect if the driver is working
	running_ = 0; // running condition for the thread loop
	buf = 0;
	device_idx_ = WAVE_MAPPER;
	dither_ = false;
	ui_ = ui;
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
	DWORD_PTR dwInstance,
	DWORD_PTR dwParam1,
	DWORD_PTR dwParam2
) {
	int* freeBlockCounter = reinterpret_cast<int*>(dwInstance);
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
	while(running_) {
		float const * input(callback(playbackSettings().blockFrames()));
		if (dither_) {
			Quantize16WithDither(input, buf, playbackSettings().blockFrames());
		}
		else {
			Quantize16(input, buf, playbackSettings().blockFrames());
		}
		writeAudio(hWaveOut, reinterpret_cast<char*>(buf), playbackSettings().blockBytes());
	}
}

DWORD WINAPI MsWaveOut::audioOutThread( void *pWaveOut ) {
	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );
	MsWaveOut * pThis = reinterpret_cast<MsWaveOut*>( pWaveOut ) ;
	pThis->fillBuffer();
	return 0;
}

void MsWaveOut::do_open() throw(std::exception) {
	if(hWaveOut) throw std::runtime_error("............");
	// WAVEFORMATEX is defined in mmsystem.h
	// this structure is used, to define the sampling rate, sampling resolution
	// and the number of channles
	// some informations are redundant, but you need to define them
	WAVEFORMATEX format;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.wBitsPerSample = playbackSettings().bitDepth();
	format.nSamplesPerSec = playbackSettings().samplesPerSec();
	format.nChannels = playbackSettings().numChannels();
	format.nBlockAlign = playbackSettings().frameBytes();
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
	format.cbSize = 0;

	// the buffer block variables
	if(!(waveBlocks = allocateBlocks())) throw std::runtime_error("............");
	waveFreeBlockCount = playbackSettings().blockCount();
	waveCurrentBlock = 0;
	// this will protect the monitor buffer counter variable
	if(waveOutOpen(&hWaveOut, device_idx_, &format, (DWORD_PTR) waveOutProc, (DWORD_PTR)(&waveFreeBlockCount),
		CALLBACK_FUNCTION) != MMSYSERR_NOERROR
	) throw std::runtime_error("waveOutOpen() failed");
	universalis::os::aligned_memory_alloc(16, buf, playbackSettings().blockFrames() * playbackSettings().numChannels());
}

void MsWaveOut::do_close() throw(std::exception) {
	if(waveOutClose(hWaveOut) != MMSYSERR_NOERROR) {
		hWaveOut=0;
		throw std::runtime_error("waveOutClose() failed");
	}
	hWaveOut=0;
	universalis::os::aligned_memory_dealloc(buf);
	buf=0;
}

void MsWaveOut::do_start() throw(std::exception) {
	running_ = true;
	DWORD dwThreadId;
	_hThread = CreateThread( NULL, 0, audioOutThread, this, 0, &dwThreadId );
}

void MsWaveOut::do_stop() throw(std::exception) {
	if(!running_) return;
	running_=false;
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

void MsWaveOut::ReadConfig() {
	// default configuration
	device_idx_ = WAVE_MAPPER;
	dither_ = false;
	playbackSettings_.setBitDepth(16);
	playbackSettings_.setChannelMode(3);
	playbackSettings_.setSamplesPerSec(44100);
	playbackSettings_.setBlockFrames(2048);
	playbackSettings_.setBlockCount(8);
	if(ui_) {
		int tmp_samplespersec, tmp_blockbytes, tmp_block_count;
		ui_->ReadConfig(
			device_idx_,
			dither_,
			tmp_samplespersec,
			tmp_blockbytes,
			tmp_block_count
		);
		playbackSettings_.setSamplesPerSec(tmp_samplespersec);
		playbackSettings_.setBlockBytes(tmp_blockbytes);
		playbackSettings_.setBlockCount(tmp_block_count);
	}
}

void MsWaveOut::WriteConfig() {
	if(ui_) {
		ui_->WriteConfig(
			device_idx_,
			dither_,
			playbackSettings().samplesPerSec(),
			playbackSettings().blockBytes(),
			playbackSettings().blockCount()
		);
	}
}
void MsWaveOut::Configure() {
	// 1. reads the config from persistent storage
	// 2. opens the gui to let the user edit the settings
	// 3. writes the config to persistent storage

	ReadConfig();

	if(!ui_) return;

	ui_->SetValues(
		device_idx_,
		dither_,
		playbackSettings().samplesPerSec(),
		playbackSettings().blockBytes(),
		playbackSettings().blockCount()
	);
	if(ui_->DoModal() != IDOK) return;

	bool was_opened(opened());
	bool was_started(started());

	try {
		set_opened(false);
	} catch(std::exception e) {
		std::ostringstream s;
		s << "failed to close driver: " << e.what();
		ui_->Error(s.str());
		return;
	}

	// save the settings to be able to rollback if it doesn't work
	int device_idx = device_idx_;
	bool dither = this->dither_;
	int samplesPerSec = playbackSettings().samplesPerSec();
	int bufferSize = playbackSettings().blockBytes();
	int numBuffers = playbackSettings().blockCount();

	int tmp_samplespersec, tmp_blockbytes, tmp_block_count;
	ui_->GetValues(
		this->device_idx_,
		this->dither_,
		tmp_samplespersec,
		tmp_blockbytes, 
		tmp_block_count
	);
	playbackSettings_.setSamplesPerSec(tmp_samplespersec);
	playbackSettings_.setBlockBytes(tmp_blockbytes);
	playbackSettings_.setBlockCount(tmp_block_count);

	// try the settings
	bool failed(false);
	try {
		set_started(true);
	} catch(std::exception e) {
		failed = true;
		std::ostringstream s;
		s << "settings failed: " << e.what();
		ui_->Error(s.str());
	}

	if(failed) {
		// rollback settings
		try {
			set_opened(false);
		} catch(std::exception e) {
			std::ostringstream s;
			s << "failed to rollback driver settings: " << e.what();
			s << "\nDriver is totally screwed, in an inconsistent state. Restart the app!";
			ui_->Error(s.str());
			return;
		}
		this->device_idx_ = device_idx;
		this->dither_ = dither;
		playbackSettings_.setSamplesPerSec(samplesPerSec);
		playbackSettings_.setBlockBytes(bufferSize);
		playbackSettings_.setBlockCount(numBuffers);

		try {
			set_opened(was_opened);
			set_started(was_started);
		} catch(std::exception e) {
			std::ostringstream s;
			s << "failed to rollback driver settings: " << e.what();
			s << "\nDriver is totally screwed, in an inconsistent state. Restart the app!";
			ui_->Error(s.str());
		}
		return;
	}

	WriteConfig();
}


}}
#endif
