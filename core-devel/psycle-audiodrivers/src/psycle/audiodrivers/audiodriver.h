/******************************************************************************
*  copyright 2007 members of the psycle project http://psycle.sourceforge.net *
*                                                                             *
*  This program is free software; you can redistribute it and/or modify       *
*  it under the terms of the GNU General Public License as published by       *
*  the Free Software Foundation; either version 2 of the License, or          *
*  (at your option) any later version.                                        *
*                                                                             *
*  This program is distributed in the hope that it will be useful,            *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of             *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
*  GNU General Public License for more details.                               *
*                                                                             *
*  You should have received a copy of the GNU General Public License          *
*  along with this program; if not, write to the                              *
*  Free Software Foundation, Inc.,                                            *
*  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                  *
******************************************************************************/
#ifndef AUDIODRIVER_H
#define AUDIODRIVER_H
#include <string>
#include <cstdint>
/**
@author  Psycledelics
*/

///\todo write a c api interface for building driver plugins

namespace psy 
{ 
	namespace core 
	{
		/// typedef for work callback
		typedef float * (*AUDIODRIVERWORKFN) (void * context, int & numSamples);

		/// provides some text info about the driver
		class AudioDriverInfo
		{
		public:
			AudioDriverInfo(const std::string & name, const std::string & header, const std::string & description, bool show);
			/// the driver name, used e.g in the configuration for the driver map.
			/// As convention, please use only lower case names here.
			const std::string& name() const { return name_; }
			/// short description
			const std::string& header() const { return header_; }
			/// deeper explanation
			const std::string& description() const { return description_; }
			/// hint, if audiodialog should list the driver
			bool show() const { return show_; }
		private:
			std::string name_;
			std::string header_;
			std::string description_;
			bool show_;
		};

		/// holds the info about sample rate, bit depth, etc
		class AudioDriverSettings
		{
		public:
			AudioDriverSettings();

			///\name getter/setter for device name  ( hw:0 and others )
			///\{
		public:
			const std::string& deviceName() const { return deviceName_; }
			void setDeviceName(const std::string & name);
		private:
			std::string deviceName_;
			///\}
			///\name getter/setter for sample rate
			///\{
		public:
			int samplesPerSec() const { return samplesPerSec_; }
			void setSamplesPerSec( int samples );
		private:
			int samplesPerSec_;
			///\}
			///\name getter/setter for bit depth ( 8, 16, 24. what about ASIO that offers more modes? 
			///\{
		public:
			int bitDepth() const { return bitDepth_; }
			void setBitDepth( int depth );
		private:
			int bitDepth_;
			///\}
			///\name getter/setter for channel mode (mode 3 == stereo, 1 == mono left, 2 == mono right, 0 = mono both channels)
			///\{
		public:
			int channelMode() const { return channelMode_; }
			void setChannelMode( int mode );
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
			void setBufferSize( int size );
		private:
			int bufferSize_;
			///\}
			///\todo: Only used by mmewaveout???
			///\name getter/setter for the audio block size 
			///\{
		public:
			int blockSize() const { return blockSize_; }
			void setBlockSize( int size );
		private:
			int blockSize_;
			///\}
			///\todo: Only used by mmewaveout???
			///\name getter/setter for number of blocks.
			///\{
		public:
			int blockCount() const { return blockCount_; }
			void setBlockCount( int count );
		private:
			int blockCount_;
			///\}
		};

		///\todo doc
		class AudioDriver
		{
			public:
				virtual ~AudioDriver() {}

				/// clones the driver using the copy ctor
				virtual AudioDriver* clone()  const;

				/// gives the driver information
				virtual AudioDriverInfo info() const;

				virtual void Reset(void) {}
				/// enable will start the driver and the calling for the work player function
				virtual bool Enable(bool e) { return false; }
				/// initialize has nothing to do with the driver, it sets only the pointer for a later player work call
				virtual void Initialize(AUDIODRIVERWORKFN pCallback, void * context) {}
				virtual void Configure(void) {}
				virtual bool Initialized(void) { return true; }
				virtual bool Configured(void) { return true; }
				static double frand();
				static void Quantize16WithDither(float const *pin, std::int16_t *piout, int c);
				static void Quantize16(float const *pin, std::int16_t *piout, int c);
				static void Quantize16AndDeinterlace(float const *pin, std::int16_t *pileft, int strideleft, std::int16_t *piright, int strideright, int c);
				static void DeQuantize16AndDeinterlace(int const *pin, float *poutleft,float *poutright,int c);

		///\name settings
		///\{
			public:
				/// here you can set the settings of the driver, like samplerate depth etc
				virtual void setSettings( const AudioDriverSettings& settings );

				/// here you get the special audio driver settings.
				/// In case of some drivers like  e.g jack you must prepare, that a driver can set itself.
				const AudioDriverSettings& settings() const { return settings_; }
			private:
				/// holds the sample rate, bit depth, etc
				AudioDriverSettings settings_; 
		///\}
		};
	}
}
#endif
