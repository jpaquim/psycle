// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__AUDIODRIVERS__AUDIODRIVER__INCLUDED
#define PSYCLE__AUDIODRIVERS__AUDIODRIVER__INCLUDED
#pragma once

#include <universalis.hpp>
#include <universalis/stdlib/thread.hpp>
#include <universalis/stdlib/mutex.hpp>
#include <string>
#include <exception>
#include <cassert>

namespace psycle { namespace audiodrivers {

using namespace universalis::stdlib;

/// typedef for work callback
typedef float * (*AUDIODRIVERWORKFN) (void * context, int numSamples);

/// max number of samples (frames comprising all channels) that the Work function may ask to return
int const MAX_SAMPLES_WORKFN = 65536;


/// provides some text info about the driver
class AudioDriverInfo {
	public:
		AudioDriverInfo(const std::string & name, const std::string & header, const std::string & description, bool show);
		
	///\name name
	///\{
		public:
			/// the driver name, used e.g in the configuration for the driver map.
			/// As convention, please use only lower case names here.
			std::string const & name() const { return name_; }
		private:
			std::string name_;
	///\}
		
	///\name header title (short description)
	///\{
		public:
			/// short description
			std::string const & header() const { return header_; }
		private:
			std::string header_;
	///\}

	///\name long description
	///\{
		public:
			/// deeper explanation
			std::string const & description() const { return description_; }
		private:
			std::string description_;
	///\}

	///\name hint, if audio dialog should list the driver
	///\{
		public:
			/// hint, if audio dialog should list the driver
			bool show() const { return show_; }
		private:
			bool show_;
	///\}
};

/// holds the info about sample rate, bit depth, etc
class AudioDriverSettings {
	public:
		AudioDriverSettings();
		bool operator!=(AudioDriverSettings const &);
		bool operator==(AudioDriverSettings const & other) { return !((*this) != other); }

	///\name getter/setter for device name  ( hw:0 and others )
	///\{
		public:
			std::string const & deviceName() const { return deviceName_; }
			void setDeviceName(const std::string & name) { deviceName_ = name; }
		private:
			std::string deviceName_;
	///\}

	///\name getter/setter for sample rate
	///\{
		public:
			unsigned int samplesPerSec() const { return samplesPerSec_; }
			void setSamplesPerSec(unsigned int value) { samplesPerSec_ = value; }
		private:
			unsigned int samplesPerSec_;
	///\}

	///\name getter/setter for sample bit depth, per-channel (8, 16, 24. what about ASIO that offers more modes? )
	///\{
		public:
			unsigned int bitDepth() const { return bitDepth_; }
			void setBitDepth(unsigned int value) {
				bitDepth_ = value;
				frameBytes_ = channelMode_ == 3 ? bitDepth_ / 4 : bitDepth_ / 8;
				blockBytes_ = blockFrames_ * frameBytes_;
				totalBufferBytes_ = blockBytes_ * blockCount_;
			}
		private:
		unsigned int bitDepth_;
	///\}

	///\name getter/setter for channel mode (mode 3 == stereo, 1 == mono left, 2 == mono right, 0 = mono both channels)
	///\{
		public:
			unsigned int numChannels() const { return channelMode_ == 3 ? 2 : 1; }
			unsigned int channelMode() const { return channelMode_; }
			void setChannelMode(unsigned int value) {
				channelMode_ = value;
				frameBytes_ = channelMode_ == 3 ? bitDepth_ / 4 : bitDepth_ / 8;
				blockBytes_ = blockFrames_ * frameBytes_;
				totalBufferBytes_ = blockBytes_ * blockCount_;
			}
		private:
			unsigned int channelMode_;
	///\}

	///\name number of bytes per sample, comprising all channels. (should verify if this is valid for 24bits)
	///\{
		public:
			/// getter for number of bytes per sample. (should verify if this is valid for 24bits)
			unsigned int frameBytes() const { return frameBytes_; }
		private:
			unsigned int frameBytes_;
	///\}

	///\name getter for the whole buffer size (in bytes).
	///\{
		public:
			/// getter for the whole buffer size (in bytes).
			unsigned int totalBufferBytes() const { return totalBufferBytes_; }
		private:
			unsigned int totalBufferBytes_;
	///\}

	///\name getter/setter for the audio block size (in bytes)
	///\{
		public:
			unsigned int blockBytes() const { return blockBytes_; }
			void setBlockBytes(unsigned int value) {
				blockBytes_ = value;
				blockFrames_ = blockBytes_ / frameBytes_;
				totalBufferBytes_ = blockBytes_ * blockCount_;
				assert(blockFrames_ < MAX_SAMPLES_WORKFN);
			}
		private:
			unsigned int blockBytes_;
	///\}

	///\name getter/setter for the audio block size (in samples comprising all channels)
	///\{
		public:
			unsigned int blockFrames() const { return blockFrames_; }
			void setBlockFrames(unsigned int value) {
				blockFrames_ = value;
				blockBytes_ = blockFrames_ * frameBytes_;
				totalBufferBytes_ = blockBytes_ * blockCount_;
				assert(blockFrames_ < MAX_SAMPLES_WORKFN);
			}
		private:
			unsigned int blockFrames_;
	///\}

	///\name getter/setter for number of blocks.
	///\{
		public:
			unsigned int blockCount() const { return blockCount_; }
			void setBlockCount(unsigned int value) {
				blockCount_ = value;
				totalBufferBytes_ = blockBytes_ * blockCount_;
			}
		private:
			unsigned int blockCount_;
	///\}
};

/// base class for all audio drivers
class AudioDriver {
	public:
		/// gives the driver information
		virtual AudioDriverInfo info() const = 0;

	///\name ctor/dtor
	///\{
		public:
			AudioDriver();
			virtual ~AudioDriver() throw() {}
		protected:
			/// derived classes must call this in their destructor.
			/// this ensures the driver is closed.
			/// Exceptions are handled internally in this function,
			/// so it's suitable for being called inside a destructor.
			void before_destruction() throw();
	///\}

	///\name callback to the player work function that generates audio
	///\{
		public:
			void set_callback(AUDIODRIVERWORKFN callback, void * context);
		protected:
			float * callback(int numSamples) { return callback_(callback_context_, numSamples); }
		private:
			AUDIODRIVERWORKFN callback_;
			void * callback_context_;
	///\}

	///\name playback settings
	///\{
		public:
			/// here you can set the settings of the driver, like samplerate depth etc
			void setPlaybackSettings(AudioDriverSettings const &);

			/// here you get the special audio driver settings.
			/// In case of some drivers like  e.g jack you must prepare, that a driver can set itself.
			AudioDriverSettings const & playbackSettings() const { return playbackSettings_; }

			virtual/*?*/ int GetOutputLatency() { return playbackSettings_.totalBufferBytes(); }
			virtual int GetWritePos() { return 0; }
			virtual int GetPlayPos() { return 0; }
			virtual void ReadConfig() {}
			virtual void WriteConfig() {}
		protected:
			/// holds the sample rate, bit depth, etc
			AudioDriverSettings playbackSettings_; 
	///\}

	///\name capture settings
	///\{
		public:
			/// here you can set the settings of the driver, like samplerate depth etc
			void setCaptureSettings(AudioDriverSettings const &);

			/// here you get the special audio driver settings.
			/// In case of some drivers like  e.g jack you must prepare, that a driver can set itself.
			AudioDriverSettings const & captureSettings() const { return captureSettings_; }

			virtual/*?*/ int GetInputLatency() { return captureSettings_.totalBufferBytes(); }
		protected:
			/// holds the sample rate, bit depth, etc
			AudioDriverSettings captureSettings_; 
	///\}

	public:
		/// ask the driver to config itself (e.g with a gui, config read)
		virtual void Configure() {}
		/// Audio capture Methods
		virtual void AddCapturePort(int idx) {}
		virtual void RemoveCapturePort(int idx) {}
		virtual void GetReadBuffers(int idx, float **pleft, float **pright,int numsamples) {}

	///\name open/close the device (allocate/deallocate resources)
	///\{
		public:
			/// opens/closes the driver, allocating resources and setting it up according to the settings or deallocating them.
			///\throws std::exception in case of failure to open/close the driver
			///\post opened() == b
			void set_opened(bool b) throw(std::exception);
			/// status, if driver is opened (e.g. a driver handle could be created)
			bool opened() const throw() { return opened_; }
		protected:
			/// opens the driver, allocating resources and setting it up according to the settings.
			///\pre !opened()
			///\throws std::exception in case of failure to open the driver or setting up settings
			virtual void do_open() throw(std::exception) = 0;

			/// closes the driver, deallocating resources.
			///\pre opened()
			///\throws std::exception in case of failure to close the driver
			virtual void do_close() throw(std::exception) = 0;
		private:
			bool opened_;
	///\}

	///\name start/stop the device (enable/disable processing)
	///\{
		public:
			/// starts/stops processing.
			///\throws std::exception in case of failure to start/stop the driver
			///\post started() == b
			void set_started(bool b) throw(std::exception);
			/// status, if driver started (e.g. audiothread is running)
			bool started() const throw() { return started_; }  

		protected:
			/// starts processing
			///\pre !started()
			///\throws std::exception in case of failure to start the driver
			virtual void do_start() throw(std::exception) = 0;

			/// stops processing
			///\pre started()
			///\throws std::exception in case of failure to stop the driver
			virtual void do_stop() throw(std::exception) = 0;
		private:
			bool started_;
	///\}

	///\name utility functions
	///\{
		protected:
			static double frand();
			static void Quantize16WithDither(float const * pin, int16_t * piout, int c);
			static void Quantize16(float const * pin, int16_t * piout, int c);
			static void Quantize16AndDeinterlace(float const * pin, int16_t * pileft, int strideleft, int16_t * piright, int strideright, int c);
			static void DeQuantize16AndDeinterlace(int const * pin, float * poutleft, float * poutright, int c);
			static void Quantize24WithDither(float const * pin, int32_t * piout, int c);
			static void Quantize24(float const * pin, int32_t * piout, int c);
			static void Quantize24AndDeinterlace(float const * pin, int32_t * pileft, int32_t * piright, int c);
			static void DeQuantize24AndDeinterlace(int const * pin, float * poutleft, float * poutright, int c);
	///\}
};

/// a dummy, silent driver
class DummyDriver : public AudioDriver {
	public:
		~DummyDriver() throw();
		/*override*/ AudioDriverInfo info() const;

	protected:
		/*override*/ void do_open() throw(std::exception) {}
		/*override*/ void do_start() throw(std::exception);
		/*override*/ void do_stop() throw(std::exception);
		/*override*/ void do_close() throw(std::exception) {}

	///\name thread
	///\{
		private:
			thread * thread_;
			/// the function executed by the thread
			void thread_function();
			/// whether the thread is asked to terminate
			bool stop_requested_;
			mutex mutex_;
			typedef class unique_lock<mutex> scoped_lock;
	///\}
};

}}
#endif
