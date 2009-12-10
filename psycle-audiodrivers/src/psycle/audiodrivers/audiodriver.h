
/**********************************************************************************************
	Copyright 2007-2008 members of the psycle project http://psycle.sourceforge.net

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
	You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**********************************************************************************************/

#pragma once
#include <string>
#include <mutex>
#include <condition>
#include <cstdint>
namespace psy { namespace core {

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
			int samplesPerSec() const { return samplesPerSec_; }
			void setSamplesPerSec(int samples) { samplesPerSec_ = samples; }
		private:
			int samplesPerSec_;
	///\}

	///\name getter/setter for bit depth ( 8, 16, 24. what about ASIO that offers more modes? 
	///\{
		public:
			int bitDepth() const { return bitDepth_; }
			void setBitDepth(int depth) { bitDepth_ = depth; }
		private:
		int bitDepth_;
	///\}

	///\name getter/setter for channel mode (mode 3 == stereo, 1 == mono left, 2 == mono right, 0 = mono both channels)
	///\{
		public:
			int channelMode() const { return channelMode_; }
			void setChannelMode(int mode) { channelMode_ = mode; }
		private:
			int channelMode_;
	///\}

	/// getter for number of bits per sample
	public:
		int sampleSize() const;

	///\todo: Unused????
	///\name getter/setter for buffer size
	///\{
		public:
			int bufferSize() const { return bufferSize_; }
			void setBufferSize(int size) { bufferSize_ = size; }
		private:
			int bufferSize_;
	///\}

	///\todo: Only used by mmewaveout???
	///\name getter/setter for the audio block size 
	///\{
		public:
			int blockSize() const { return blockSize_; }
			void setBlockSize(int size) { blockSize_ = size; }
		private:
			int blockSize_;
	///\}

	///\todo: Only used by mmewaveout???
	///\name getter/setter for number of blocks.
	///\{
		public:
			int blockCount() const { return blockCount_; }
			void setBlockCount(int count) { blockCount_ = count; }
		private:
			int blockCount_;
	///\}
};

/// base class for all audio drivers
class AudioDriver {
	public:
		virtual ~AudioDriver() {}

		/// gives the driver information
		virtual AudioDriverInfo info() const;

		/// enable will start the driver and the calls to the work player function
		virtual bool Enable(bool /*e*/) { return true; }

		/// initialize has nothing to do with the driver, it sets only the pointer for a later player work call
		virtual void Initialize(AUDIODRIVERWORKFN /*pCallback*/, void * /*context*/) {}
		virtual bool Initialized() { return true; }

		virtual void Configure() {}
		virtual bool Configured() { return true; }

		static double frand();
		static void Quantize16WithDither(float const * pin, std::int16_t * piout, int c);
		static void Quantize16(float const * pin, std::int16_t * piout, int c);
		static void Quantize16AndDeinterlace(float const * pin, std::int16_t * pileft, int strideleft, std::int16_t * piright, int strideright, int c);
		static void DeQuantize16AndDeinterlace(int const * pin, float * poutleft, float * poutright, int c);

	///\name settings
	///\{
		public:
			/// here you can set the settings of the driver, like samplerate depth etc
			void virtual setSettings(AudioDriverSettings const & settings ) { settings_ = settings; }

			/// here you get the special audio driver settings.
			/// In case of some drivers like  e.g jack you must prepare, that a driver can set itself.
			AudioDriverSettings const & settings() const { return settings_; }
		private:
			/// holds the sample rate, bit depth, etc
			AudioDriverSettings settings_; 
	///\}
};

/// a dummy, silent driver
class DummyDriver : public AudioDriver {
	public:
		DummyDriver();
		~DummyDriver();
		virtual AudioDriverInfo info() const;
		virtual void Initialize(AUDIODRIVERWORKFN callback_function, void * callback_context);
		virtual bool Initialized() { return initialized_; }
		virtual bool Enable(bool e) { if(e) start(); else stop(); return true; }

	private:
		AUDIODRIVERWORKFN callback_function_;
		void * callback_context_;
		bool initialized_;
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
			std::mutex mutex_;
			typedef std::scoped_lock<std::mutex> scoped_lock;
			/// a condition variable to wait until notified that the value of running_ has changed
			std::condition<scoped_lock> condition_;
		///\}
};

}}