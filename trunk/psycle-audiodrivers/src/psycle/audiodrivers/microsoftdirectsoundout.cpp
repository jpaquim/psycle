// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#if defined PSYCLE__MICROSOFT_DIRECT_SOUND_AVAILABLE
#include "microsoftdirectsoundout.h"
#include <psycle/helpers/math.hpp>
#include <universalis/exception.hpp>
#include <universalis/cpu/exception.hpp>
#include <universalis/os/exceptions/code_description.hpp>
#include <universalis/os/aligned_memory_alloc.hpp>
#include <universalis/os/thread_name.hpp>
#include <universalis/os/loggers.hpp>
#include <universalis/stdlib/cstdint.hpp>
#include <boost/bind.hpp>
#include <stdexcept>

namespace psycle { namespace audiodrivers {

namespace loggers = universalis::os::loggers;
using universalis::exceptions::runtime_error;
using universalis::os::exceptions::code_description;
using namespace helpers::math;

AudioDriverInfo MsDirectSound::info( ) const {
	return AudioDriverInfo("dsound","Microsoft DirectSound Driver","Microsoft output driver",true);
}

void MsDirectSound::error(std::string const & msg, universalis::compiler::location const & location) {
	loggers::exception()(msg, location);
	if(ui_) {
		std::ostringstream s;
		s
			<< msg
			<< "\n\nlocation:\n"
			<< "\tmodule: " << location.module()
			<< "\tfile: " << location.file()
			<< "\tline: " << location.line()
			<< "\tfunction: " << location.function();
		ui_->Error(s.str());
	}
}

MsDirectSound::MsDirectSound(DSoundUiInterface * ui)
:
	device_guid_(), // DSDEVID_DefaultPlayback <- unresolved external.
	direct_sound_(),
	buffer_(),
	ui_(ui)
{}

MsDirectSound::~MsDirectSound() throw() {
	before_destruction();
}

void MsDirectSound::do_open() throw(std::exception) {
	HRESULT result;
	//S_OK means good. S_FALSE means already initialized, so good too.
	if(S_OK != (result = ::CoInitialize(NULL)) && S_FALSE != result)
		throw runtime_error("failed to initialize COM (for DirectSound): " + code_description(result), UNIVERSALIS__COMPILER__LOCATION);

	///\todo: The capture ports logic needs to be rethought for the event of changing
	/// the audiodriver.
	/// Also, prepare the event where a new capture port appears (USB soundcard?)
	cap_enums_.resize(0);
	DirectSoundCaptureEnumerate(DSEnumCallback, &cap_enums_);

	if(DS_OK != (result = ::DirectSoundCreate8(device_guid_ != GUID() ? &device_guid_ : 0, &direct_sound_, 0)))
		throw runtime_error("failed to create DirectSound object: " + code_description(result), UNIVERSALIS__COMPILER__LOCATION);
	try {
		HWND hwnd = ::GetWindow(0, 0);
		if(!hwnd) hwnd = ::GetForegroundWindow();
		if(!hwnd) hwnd = ::GetDesktopWindow();

		if(exclusive_) {
			if(DS_OK != (result = direct_sound_->SetCooperativeLevel(hwnd, DSSCL_WRITEPRIMARY))) {
				direct_sound_->Release();
				direct_sound_ = 0;
				throw runtime_error("Failed to set DirectSound cooperative level: " + code_description(result), UNIVERSALIS__COMPILER__LOCATION);
			}
		} else if(DS_OK != (result = direct_sound_->SetCooperativeLevel(hwnd, DSSCL_PRIORITY))) {
			direct_sound_->Release();
			direct_sound_ = 0;
			throw runtime_error("failed to set DirectSound cooperative level: " + code_description(result), UNIVERSALIS__COMPILER__LOCATION);
		}

		// Set up wave format structure.
		WAVEFORMATEX format;
		std::memset(&format, 0, sizeof format);
		format.wFormatTag = WAVE_FORMAT_PCM;
		format.nChannels = playbackSettings().numChannels();
		format.wBitsPerSample = playbackSettings().bitDepth();
		format.nSamplesPerSec = playbackSettings().samplesPerSec();
		format.nBlockAlign = playbackSettings().frameBytes();
		format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
		format.cbSize = 0;

		// Set up playback buffer structure.
		DSBUFFERDESC desc;
		std::memset(&desc, 0, sizeof desc);
		desc.dwSize = sizeof desc;
		desc.dwFlags = DSBCAPS_LOCSOFTWARE | DSBCAPS_GETCURRENTPOSITION2; // | DSBCAPS_TRUEPLAYPOSITION valid only in vista
		desc.dwFlags |= exclusive_ ? DSBCAPS_PRIMARYBUFFER : DSBCAPS_GLOBALFOCUS;
		desc.dwBufferBytes = exclusive_ ? 0 : playbackSettings().totalBufferBytes();
		desc.dwReserved = 0;
		desc.lpwfxFormat = exclusive_ ? 0 : &format;
		desc.guid3DAlgorithm = GUID_NULL;

		if(DS_OK != (result = direct_sound_->CreateSoundBuffer(&desc, reinterpret_cast<LPDIRECTSOUNDBUFFER*>(&buffer_), 0)))
			throw runtime_error("failed to create DirectSound playback buffer: " + code_description(result), UNIVERSALIS__COMPILER__LOCATION);
		try {
			if(exclusive_) {
				if(DS_OK != (result = buffer_->SetFormat(&format)))
					throw runtime_error("failed to set DirectSound playback buffer format: " + code_description(result), UNIVERSALIS__COMPILER__LOCATION);
				DSBCAPS caps;
				caps.dwSize = sizeof caps;
				if(DS_OK != (result = buffer_->GetCaps(&caps)))
					throw runtime_error("failed to get DirectSound playback buffer capabilities: " + code_description(result), UNIVERSALIS__COMPILER__LOCATION);
				if(caps.dwFlags & DSBCAPS_STATIC)
					throw runtime_error("direct sound buffer get caps, what kind of fancy sound card hardware you got ? it uses on-board memory only !", UNIVERSALIS__COMPILER__LOCATION);
				playbackSettings_.setBlockCount(2);
				playbackSettings_.setBlockBytes(caps.dwBufferBytes / playbackSettings().blockCount());
			}

			for(unsigned int i = 0; i < cap_ports_.size(); ++i)
				CreateCapturePort(cap_ports_[i]);
		} catch(...) {
			buffer_->Release();
			buffer_ = 0;
			throw;
		}
	} catch(...) {
		direct_sound_->Release();
		direct_sound_ = 0;
		throw;
	}
}

void MsDirectSound::do_start() throw(std::exception) {
	HRESULT result;
	if( DSERR_BUFFERLOST == (result = buffer_->Play(0, 0, DSBPLAY_LOOPING))) {
		// ignore
	} else if(DS_OK != result)
		throw runtime_error("fatal error while trying to play DirectSound playback buffer: " + code_description(result), UNIVERSALIS__COMPILER__LOCATION);
	low_mark_ = 0;
	high_mark_ = playbackSettings().blockBytes();
	stop_requested_ = false;
	thread_ = new thread(boost::bind(&MsDirectSound::thread_function, this));
}

void MsDirectSound::thread_function() {
	universalis::os::thread_name thread_name("direct sound");
	universalis::cpu::exception::install_handler_in_thread();
	::SetThreadPriority(::GetCurrentThread(),
		//THREAD_PRIORITY_NORMAL
		//THREAD_PRIORITY_ABOVE_NORMAL
		THREAD_PRIORITY_HIGHEST
		//THREAD_PRIORITY_TIME_CRITICAL
	);
	SetThreadAffinityMask(GetCurrentThread(), 1);

	// prefill buffer
	for(unsigned int i = 0; i < captureSettings().blockCount(); ++i) {
		for(unsigned int i =0; i < cap_ports_.size(); ++i)
			DoBlocksRecording(cap_ports_[i]);
	}
	// todo zero-out playback buffer?

	while(true) {
		HRESULT result;
		while(true) {
			// check whether the thread has been asked to terminate
			{
				scoped_lock lock(mutex_);
				if(stop_requested_) return;
			}

			// [low_mark_, high_mark_[ is the next buffer to be filled.
			// if pos is still inside, we have to wait.
			assert(low_mark_ != high_mark_);
			DWORD pos = 0;
			if(DS_OK != (result = buffer_->GetCurrentPosition(&pos, 0))) {
				error("direct sound: failed to get playback buffer position: " + code_description(result), UNIVERSALIS__COMPILER__LOCATION);
				return;
			}
			if(high_mark_ < low_mark_) {
				if(pos >= low_mark_ || pos < high_mark_) {
					this_thread::sleep(milliseconds(1)); //this_thread::yield();
					continue;
				}
			} else if(pos >= low_mark_ && pos < high_mark_) {
				this_thread::sleep(milliseconds(1)); //this_thread::yield();
				continue;
			}
			break;
		}

		// First, do the capture buffers so that audio is available to wavein machines.
		for(unsigned int i = 0; i < cap_ports_.size(); ++i)
			DoBlocksRecording(cap_ports_[i]);

		// Obtain write pointers.
		void * block_1, * block_2;
		DWORD block_size_1, block_size_2;
		if( DSERR_BUFFERLOST == (result = buffer_->Lock(
				low_mark_, playbackSettings().blockBytes(),
				reinterpret_cast<void** >(&block_1), &block_size_1,
				reinterpret_cast<void** >(&block_2), &block_size_2, 0
			))
		) {
			// restore and retry lock.
			low_mark_ = 0;
			high_mark_ = playbackSettings().blockBytes();
			while(true) {
				result = buffer_->Restore();
				if(DS_OK == result) result = buffer_->Play(0, 0, DSBPLAY_LOOPING);
				if(DS_OK == result) result = buffer_->Lock(
						low_mark_, playbackSettings().blockBytes(),
						reinterpret_cast<void** >(&block_1), &block_size_1,
						reinterpret_cast<void** >(&block_2), &block_size_2, 0
				);
				if(result == DSERR_BUFFERLOST) {
					// application lost focus. retry periodically.
					this_thread::sleep(milliseconds(500));
					// check whether the thread has been asked to terminate
					{
						scoped_lock lock(mutex_);
						if(stop_requested_) return;
					}
				} else if(DS_OK != result) {
					error("direct sound: fatal error while trying to restore/play/lock playback buffer: " + code_description(result), UNIVERSALIS__COMPILER__LOCATION);
					return;
				}
			}
		} else if(DS_OK != result) {
			error("direct sound: fatal error while trying to lock playback buffer: " + code_description(result), UNIVERSALIS__COMPILER__LOCATION);
			return;
		}


		// Generate audio and put it into the buffer
		unsigned int const num_samples = block_size_1 / playbackSettings().frameBytes();
		#if 1
			float * float_block = callback(num_samples);
		#else
			// simple test for debugging
			float static float_block[65536];
			float static t = 0;
			for(unsigned int i = 0; i < num_samples * 2; ++i) {
				float_block[i] = std::sin(t) * 10000;
				t += 0.1f;
			}
			t = std::fmodf(t, 2 * pi);
			std::cout << "xxxxxxxxxxxxxxxx " << t << '\n';
		#endif
		switch(playbackSettings().bitDepth()) {
			case 16: {
				int16_t * block = reinterpret_cast<int16_t*>(block_1);
				if(dither_) Quantize16WithDither(float_block, block, num_samples);
				else Quantize16(float_block, block, num_samples);
			}
			break;
			case 24: {
				int32_t * block = reinterpret_cast<int32_t*>(block_1);
				if(dither_) Quantize24WithDither(float_block, block, num_samples);
				else Quantize24(float_block, block, num_samples);
			}
			break;
			default: {
				std::ostringstream s;
				s << "unhandled bit depth: " << playbackSettings().bitDepth();
				error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				return;
			}
		}

		if(block_size_2 > 0) {
			unsigned int const num_samples = block_size_2 / playbackSettings().frameBytes();
			float * float_block = callback(num_samples);
			switch(playbackSettings().bitDepth()) {
				case 16: {
					int16_t * block = reinterpret_cast<int16_t*>(block_2);
					if(dither_) Quantize16WithDither(float_block, block, num_samples);
					else Quantize16(float_block, block, num_samples);
				}
				break;
				case 24: {
					int32_t * block = reinterpret_cast<int32_t*>(block_2);
					if(dither_) Quantize24WithDither(float_block, block, num_samples);
					else Quantize24(float_block, block, num_samples);
				}
				break;
				default: {
					std::ostringstream s;
					s << "unhandled bit depth: " << playbackSettings().bitDepth();
					error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					return;
				}
			}
		}
		
		// Release the data back to DirectSound.
		if(DS_OK != (result = buffer_->Unlock(block_1, block_size_1, block_2, block_size_2))) {
			error("direct sound: failed to unlock playback buffer: " + code_description(result), UNIVERSALIS__COMPILER__LOCATION);
			return;
		}

		if(block_size_2 > 0) {
			low_mark_ = block_size_2;
			high_mark_ = block_size_2 + playbackSettings().blockBytes();
		} else {
			low_mark_ += block_size_1;
			low_mark_ %= playbackSettings().totalBufferBytes();
			high_mark_ += block_size_1;
			high_mark_ %= playbackSettings().totalBufferBytes();
		}
	}
}

bool MsDirectSound::WantsMoreBlocksRecording(PortCapt & port) {
	// [low_mark_, high_mark_[ is the next buffer to be filled.
	// if pos is still inside, we have to wait.
	DWORD pos = 0;
	if(DS_OK != port.buffer_->GetCurrentPosition(0, &pos)) return false;
	if(port.high_mark_ < port.low_mark_) {
		if(pos >= port.low_mark_ || pos < port.high_mark_) return false;
	} else if(pos >= port.low_mark_ && pos < port.high_mark_) return false;
	return true;
}

void MsDirectSound::DoBlocksRecording(PortCapt & port) {
	int * block_1 , * block_2;
	unsigned long block_size_1, block_size_2;
	HRESULT result = port.buffer_->Lock(
		port.low_mark_, captureSettings().blockBytes(),
		reinterpret_cast<void** >(&block_1), &block_size_1,
		reinterpret_cast<void** >(&block_2), &block_size_2, 0
	);
	if(DSERR_BUFFERLOST == result) {
		port.low_mark_ = 0;
		port.high_mark_ = captureSettings().blockBytes();
		result = port.buffer_->Lock(
			port.low_mark_, captureSettings().blockBytes(),
			reinterpret_cast<void** >(&block_1), &block_size_1,
			reinterpret_cast<void** >(&block_2), &block_size_2, 0);
		if(DS_OK != result) {
			///\todo sleep and loop
		}
	}
	if(DS_OK == result) {
		// Put the audio in our float buffers.

		unsigned int const num_samples = block_size_1 / captureSettings().frameBytes();
		switch(captureSettings().bitDepth()) {
			case 16: DeQuantize16AndDeinterlace(block_1, port.left_, port.right_, num_samples); break;
			case 24: DeQuantize24AndDeinterlace(block_1, port.left_, port.right_, num_samples); break;
			default: {
				std::ostringstream s;
				s << "unhandled bit depth: " << captureSettings().bitDepth();
				error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				return;
			}
		}

		if(block_size_2 > 0) {
			unsigned int const num_samples = block_size_2 / captureSettings().frameBytes();
			switch(captureSettings().bitDepth()) {
				case 16: DeQuantize16AndDeinterlace(block_2, port.left_ + num_samples, port.right_ + num_samples, num_samples); break;
				case 24: DeQuantize24AndDeinterlace(block_2, port.left_ + num_samples, port.right_ + num_samples, num_samples); break;
				default: {
					std::ostringstream s;
					s << "unhandled bit depth: " << captureSettings().bitDepth();
					error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					return;
				}
			}
		}
		
		// Release the data back to DirectSound.
		if(DS_OK != (result = port.buffer_->Unlock(block_1, block_size_1, block_2, block_size_2))) {
			error("direct sound: failed to unlock capture buffer: " + code_description(result), UNIVERSALIS__COMPILER__LOCATION);
			return;
		}
		
		if(block_size_2 > 0) {
			port.low_mark_ = block_size_2;
			port.high_mark_ = block_size_2 + captureSettings().blockBytes();
		} else {
			port.low_mark_ += block_size_1;
			port.low_mark_ %= captureSettings().totalBufferBytes();
			port.high_mark_ += block_size_1;
			port.high_mark_ %= captureSettings().totalBufferBytes();
		}
	}
	port.machine_pos_ = 0;
}

void MsDirectSound::do_stop() throw(std::exception) {
	{ scoped_lock lock(mutex_);
		stop_requested_ = true; // ask the thread to terminate
	}
	thread_->join();
	delete thread_;

	buffer_->Stop();

	for(unsigned int i = 0; i < cap_ports_.size(); ++i)
		cap_ports_[i].buffer_->Stop();
}

void MsDirectSound::do_close() throw(std::exception) {
	if(buffer_) {
		buffer_->Release();
		buffer_ = 0;
	}

	if(direct_sound_) {
		direct_sound_->Release();
		direct_sound_ = 0;
	}

	for(unsigned int i = 0; i < cap_ports_.size(); ++i) {
		cap_ports_[i].buffer_->Release();
		cap_ports_[i].capture_->Release();
		cap_ports_[i].capture_ = 0;
		universalis::os::aligned_memory_dealloc(cap_ports_[i].left_);
		universalis::os::aligned_memory_dealloc(cap_ports_[i].right_);
	}
	cap_ports_.resize(0);

	CoUninitialize(); // release COM
}

void MsDirectSound::CreateCapturePort(PortCapt & port) {
	HRESULT result;
	// don't open a port twice
	if(port.capture_) return;

	// Create IDirectSoundCapture using the indicated capture device
	if(DS_OK != (result = DirectSoundCaptureCreate8(port.guid_, &port.capture_, 0)))
		throw runtime_error("failed to create DirectSound capture object: " + code_description(result), UNIVERSALIS__COMPILER__LOCATION);

	// Set up wave format structure.
	WAVEFORMATEX format;
	std::memset(&format, 0, sizeof format);
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = captureSettings().numChannels();
	format.wBitsPerSample = captureSettings().bitDepth();
	format.nSamplesPerSec = captureSettings().samplesPerSec();
	format.nBlockAlign = captureSettings().frameBytes();
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
	format.cbSize = 0;

	// Set up capture buffer structure.
	DSCBUFFERDESC desc;
	std::memset(&desc, 0, sizeof desc);
	desc.dwSize = sizeof desc;
	desc.dwBufferBytes = exclusive_ ? 0 : captureSettings().totalBufferBytes();
	desc.lpwfxFormat = &format;

	if(DS_OK != (result = port.capture_->CreateCaptureBuffer(&desc, reinterpret_cast<LPDIRECTSOUNDCAPTUREBUFFER*>(&port.buffer_), 0)))
		throw runtime_error("failed to create DirectSound capture buffer: " + code_description(result), UNIVERSALIS__COMPILER__LOCATION);

	DSCBCAPS caps;
	caps.dwSize = sizeof caps;
	if(DS_OK != (result = port.buffer_->GetCaps(&caps)))
		throw runtime_error("failed to get DirectSound capture buffer capabilities: " + code_description(result), UNIVERSALIS__COMPILER__LOCATION);
	if(caps.dwFlags & DSCBCAPS_WAVEMAPPED)
		loggers::warning()("format not supported by device; wave mapper used for conversion", UNIVERSALIS__COMPILER__LOCATION);
	captureSettings_.setBlockBytes(caps.dwBufferBytes / captureSettings().blockCount());

	if(DS_OK != (result = port.buffer_->Start(DSCBSTART_LOOPING)))
		throw runtime_error("failed to start DirectSound capture buffer: " + code_description(result), UNIVERSALIS__COMPILER__LOCATION);

	universalis::os::aligned_memory_alloc(16, port.left_, captureSettings().totalBufferBytes());
	universalis::os::aligned_memory_alloc(16, port.right_, captureSettings().totalBufferBytes());
}

void MsDirectSound::GetReadBuffers(uint32_t idx, float ** left, float ** right, int num_samples) {
	if(!started()) return;
	if(idx >= cap_ports_.size()) return;
	*left  = cap_ports_[port_mapping_[idx]].left_  + cap_ports_[port_mapping_[idx]].machine_pos_;
	*right = cap_ports_[port_mapping_[idx]].right_ + cap_ports_[port_mapping_[idx]].machine_pos_;
	cap_ports_[port_mapping_[idx]].machine_pos_ += num_samples;
}

BOOL CALLBACK MsDirectSound::DSEnumCallback(LPGUID guid, LPCSTR description, LPCSTR module, LPVOID context) {
	std::vector<PortEnums> * ports = static_cast<std::vector<PortEnums>*>(context);
	PortEnums port(guid, description);
	ports->push_back(port);
	return TRUE;
}

void MsDirectSound::GetCapturePorts(std::vector<std::string> & ports) {
	for(unsigned int i = 0;i < cap_enums_.size(); ++i) ports.push_back(cap_enums_[i].port_name_);
}

void MsDirectSound::AddCapturePort(std::uint32_t idx) {
	if(idx >= cap_enums_.size()) return; // throw exception?
	
	for(unsigned int i = 0; i < cap_ports_.size(); ++i)
		if(cap_ports_[i].guid_ == cap_enums_[idx].guid_) return;
	
	PortCapt port;
	port.guid_ = cap_enums_[idx].guid_;

	bool was_started(started());
	set_started(false);

	cap_ports_.push_back(port);
	if(port_mapping_.size() <= idx) port_mapping_.resize(idx + 1);
	port_mapping_[idx] = cap_ports_.size() - 1;

	set_started(was_started);
}

void MsDirectSound::RemoveCapturePort(std::uint32_t idx) {
	if(idx >= cap_enums_.size()) return; // throw exception?
	
	bool was_started(started());
	
	std::vector<PortCapt> new_ports;
	for(unsigned int i = 0; i < cap_ports_.size(); ++i) {
		if(cap_ports_[i].guid_ == cap_enums_[idx].guid_) {
			set_started(false);
		} else {
			///\todo: this assignment is probably wrong. should be checked.
			port_mapping_[new_ports.size()] = port_mapping_[i];
			new_ports.push_back(cap_ports_[i]);
		}
	}
	
	port_mapping_.resize(new_ports.size());
	cap_ports_ = new_ports;
	
	set_started(was_started);
}

void MsDirectSound::ReadConfig() {
	// default configuration
	device_guid_ = GUID(); // DSDEVID_DefaultPlayback <-- unresolved external symbol
	exclusive_ = false;
	dither_ = false;
	playbackSettings_.setBitDepth(16);
	playbackSettings_.setChannelMode(3);
	playbackSettings_.setSamplesPerSec(44100);
	playbackSettings_.setBlockFrames(2048);
	playbackSettings_.setBlockCount(4);
	captureSettings_.setBitDepth(playbackSettings().bitDepth());
	captureSettings_.setChannelMode(playbackSettings().channelMode());
	if(ui_) {
		int tmp_samplespersec, tmp_blockbytes, tmp_block_count;
		ui_->ReadConfig(
			device_guid_,
			exclusive_,
			dither_,
			tmp_samplespersec,
			tmp_blockbytes,
			tmp_block_count
		);
		playbackSettings_.setSamplesPerSec(tmp_samplespersec);
		playbackSettings_.setBlockBytes(tmp_blockbytes);
		playbackSettings_.setBlockCount(tmp_block_count);
	}
	captureSettings_.setSamplesPerSec(playbackSettings().samplesPerSec());
	captureSettings_.setBlockFrames(playbackSettings().blockFrames());
	captureSettings_.setBlockCount(playbackSettings().blockCount());
}

void MsDirectSound::WriteConfig() {
	if(ui_) {
		ui_->WriteConfig(
			device_guid_,
			exclusive_,
			dither_,
			playbackSettings().samplesPerSec(),
			playbackSettings().blockBytes(),
			playbackSettings().blockCount()
		);
	}
}

int MsDirectSound::GetPlayPos() {
	if(!started()) return 0;
	DWORD pos;
	if(DS_OK != buffer_->GetCurrentPosition(&pos, 0)) return 0;
	return pos;
}

int MsDirectSound::GetWritePos() {
	if(!started()) return 0;
	DWORD pos;
	if(DS_OK != buffer_->GetCurrentPosition(0, &pos)) return 0;
	return pos;
}

void MsDirectSound::Configure() {
	// 1. reads the config from persistent storage
	// 2. opens the gui to let the user edit the settings
	// 3. writes the config to persistent storage

	ReadConfig();

	if(!ui_) return;

	ui_->SetValues(
		device_guid_,
		exclusive_,
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
	GUID device_guid = this->device_guid_;
	bool exclusive = this->exclusive_;
	bool dither = this->dither_;
	int samplesPerSec = playbackSettings().samplesPerSec();
	int bufferSize = playbackSettings().blockBytes();
	int numBuffers = playbackSettings().blockCount();

	int tmp_samplespersec, tmp_blockbytes, tmp_block_count;
	ui_->GetValues(
		this->device_guid_,
		this->exclusive_,
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
		this->device_guid_ = device_guid;
		this->exclusive_ = exclusive;
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
#endif // defined PSYCLE__MICROSOFT_DIRECT_SOUND_AVAILABLE
