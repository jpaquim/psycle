
/**********************************************************************************************
	Copyright 2007-2008 members of the psycle project http://psycle.sourceforge.net

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
	You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**********************************************************************************************/

#pragma once
#include <universalis/stdlib/mutex.hpp>
#include <universalis/stdlib/condition.hpp>
#include <universalis/stdlib/cstdint.hpp>
#include <string>
namespace psycle { namespace core {

using namespace universalis::stdlib;

/// typedef for work callback
typedef float * (*AUDIODRIVERWORKFN) (void * context, int numSamples);

/// max number of samples (per channel) that the Work function may ask to return
int const AUDIODRIVERWORKFN_MAX_BUFFER_LENGTH = 256;

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
			uint32_t samplesPerSec() const { return samplesPerSec_; }
			void setSamplesPerSec(uint32_t samples) { samplesPerSec_ = samples; }
		private:
			uint32_t samplesPerSec_;
	///\}

	///\name getter/setter for bit depth ( 8, 16, 24. what about ASIO that offers more modes? 
	///\{
		public:
			uint32_t bitDepth() const { return bitDepth_; }
			void setBitDepth(uint32_t depth) { bitDepth_ = depth; }
		private:
		uint32_t bitDepth_;
	///\}

	///\name getter/setter for channel mode (mode 3 == stereo, 1 == mono left, 2 == mono right, 0 = mono both channels)
	///\{
		public:
			uint32_t channelMode() const { return channelMode_; }
			void setChannelMode(uint32_t mode) { channelMode_ = mode; }
			uint32_t numChannels() const { return (channelMode_ == 3) ? 2 : 1; }
		private:
			uint32_t channelMode_;
	///\}

	/// getter for number of bytes per sample. (should verify if this is valid for 24bits)
		public:
			int sampleSize() const {
			return (channelMode_ == 3) ? bitDepth_ / 4 : bitDepth_ / 8;
		}


	///\name getter/setter for the whole buffer size (in bytes).
	///\{
		public:
			uint32_t totalBufferBytes() const { return bufferSize_; }
			void setTotalBufferBytes(uint32_t size) { bufferSize_ = size; }
		private:
			uint32_t bufferSize_;
	///\}

	///\name getter/setter for the audio block size (in samples)
	///\{
		public:
			uint32_t blockSamples() const { return blockSize_; }
			void setBlockSamples(uint32_t size) { blockSize_ = size; }
		private:
			uint32_t blockSize_;
	///\}

	///\name getter/setter for number of blocks.
	///\{
		public:
			uint32_t blockCount() const { return blockCount_; }
			void setBlockCount(uint32_t count) { blockCount_ = count; }
		private:
			uint32_t blockCount_;
	///\}
};

/// base class for all audio drivers
class AudioDriver {
	public:
		AudioDriver();

		virtual ~AudioDriver() {}

		/// gives the driver information
		virtual AudioDriverInfo info() const;

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

	public:
		virtual bool Enabled() { return false; }
		/// enable will start the driver and the calls to the work player function
		virtual bool Enable(bool /*e*/) { return true; }

		virtual void Configure() {}
		virtual bool Configured() { return true; }

		virtual int GetInputLatency() { return captureSettings_.totalBufferBytes(); }
		virtual int GetOutputLatency() { return playbackSettings_.totalBufferBytes(); }

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

	///\name playback settings
	///\{
		public:
			/// here you can set the settings of the driver, like samplerate depth etc
			virtual void setPlaybackSettings(AudioDriverSettings const & settings ) { playbackSettings_ = settings; }

			/// here you get the special audio driver settings.
			/// In case of some drivers like  e.g jack you must prepare, that a driver can set itself.
			AudioDriverSettings const & playbackSettings() const { return playbackSettings_; }
		protected:
			/// holds the sample rate, bit depth, etc
			AudioDriverSettings playbackSettings_; 
	///\}
	///\name capture settings
	///\{
		public:
			/// here you can set the settings of the driver, like samplerate depth etc
			virtual void setCaptureSettings(AudioDriverSettings const & settings ) { captureSettings_ = settings; }

			/// here you get the special audio driver settings.
			/// In case of some drivers like  e.g jack you must prepare, that a driver can set itself.
			AudioDriverSettings const & captureSettings() const { return captureSettings_; }
		protected:
			/// holds the sample rate, bit depth, etc
			AudioDriverSettings captureSettings_; 
	///\}
};

/// a dummy, silent driver
class DummyDriver : public AudioDriver {
	public:
		DummyDriver();
		~DummyDriver();
		/*override*/ AudioDriverInfo info() const;
		/*override*/ bool Enable(bool e) { if(e) start(); else stop(); return true; }

	private:
		void start();
		void stop();

		///\name thread
		///\{
			/// the function executed by the alsa thread
			void thread_function();
			/// whether the thread is running
			bool running_;
			/// whether the thread is asked to terminate
			bool stop_requested_;
			/// a mutex to synchronise accesses to running_ and stop_requested_
			mutex mutex_;
			typedef class scoped_lock<mutex> scoped_lock;
			/// a condition variable to wait until notified that the value of running_ has changed
			condition<scoped_lock> condition_;
		///\}
};

}}
