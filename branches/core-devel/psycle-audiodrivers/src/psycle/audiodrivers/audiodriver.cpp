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
#include "audiodriver.h"

#include <diversalis/processor.hpp>

#if !defined DIVERSALIS__PROCESSOR__ENDIAN__LITTLE
	#include <cmath>
#endif

namespace psy { namespace core {

	///\todo move that to psycle-helpers (maybe it's even already there)
	inline int d2i(double d) {
		#if defined DIVERSALIS__PROCESSOR__ENDIAN__LITTLE
			union u {
				double d;
				int i;
			} result;
			double static /* or not static? */ const magic(1.5 * (1 << 26) * (1 << 26));
			result.d = d + magic;
			return result.i;
		#else
			return ::lrintf(d); // C1999 function
		#endif
	}

		AudioDriverInfo::AudioDriverInfo( const std::string& name, const std::string& header, const std::string& description, bool show ) 
		: name_( name ), header_( header ), description_( description ), show_( show )
		{
		}

		///
		/// audio driver setting class
		///
		AudioDriverSettings::AudioDriverSettings()
		:
			deviceName_("unnamed"),
			samplesPerSec_(44100),
			bitDepth_(16),
			channelMode_(3),
			bufferSize_(2048),
			blockSize_(4096),
			blockCount_(7)
		{}

		void AudioDriverSettings::setDeviceName( const std::string& name ) {
			deviceName_ = name;
		}

		void AudioDriverSettings::setBufferSize( int size ) {
			bufferSize_ = size;
		}

		void AudioDriverSettings::setSamplesPerSec( int samples ) {
			samplesPerSec_ = samples;
		}

		void AudioDriverSettings::setChannelMode( int mode ) {
			channelMode_ = mode;
		}

		void AudioDriverSettings::setBitDepth( int depth ) {
			bitDepth_ = depth;
		}

		int AudioDriverSettings::sampleSize() const {
			return ( channelMode_ == 3 ) ? bitDepth_ / 4 : bitDepth_ / 8;
		}

		void AudioDriverSettings::setBlockSize( int size ) {
			blockSize_ = size;   
		}     

		void AudioDriverSettings::setBlockCount( int count ) {
			blockCount_ = count;
		}
		
		///////////////////////////////////////////////////////////////////////////
		// the AudioDriver base class
		
		AudioDriver * AudioDriver::clone( ) const {
			return new AudioDriver(*this);
		}

		void AudioDriver::setSettings( const AudioDriverSettings& settings ) {
			settings_ = settings;
		}

		AudioDriverInfo AudioDriver::info( ) const
		{
			return AudioDriverInfo("silent", "null output driver", "no sound output", true);
		}

		double AudioDriver::frand()
		{
			static long stat = 0x16BA2118;
			stat = (stat * 1103515245 + 12345) & 0x7fffffff;
			return (double)stat * (1.0 / 0x7fffffff);
		}
		void AudioDriver::Quantize16WithDither(float const *pin, std::int16_t *piout, int c)
		{
			do
			{
				int r = d2i(pin[1] + frand());
				if (r < -32768)
				{
					r = -32768;
				}
				else if (r > 32767)
				{
					r = 32767;
				}
				int l = d2i(pin[0] + frand());
				if (l < -32768)
				{
					l = -32768;
				}
				else if (l > 32767)
				{
					l = 32767;
				}
				*piout = static_cast<std::int16_t>(l);
				piout++;
				*piout = static_cast<std::int16_t>(r);
				piout++;
				pin += 2;
			}
			while(--c);
		}
		
		void AudioDriver::Quantize16(float const *pin, std::int16_t *piout, int c)
		{
			do
			{
			  int r = d2i(pin[1]);
				if (r < -32768)
				{
					r = -32768;
				}
				else if (r > 32767)
				{
					r = 32767;
				}
				int l = d2i((double)pin[0]);
				if (l < -32768)
				{
					l = -32768;
				}
				else if (l > 32767)
				{
					l = 32767;
				}
				*piout = static_cast<std::int16_t>(l);
				piout++;
				*piout = static_cast<std::int16_t>(r);
				piout++;
				pin += 2;
			}
			while(--c);
		}

		void AudioDriver::Quantize16AndDeinterlace(float const *pin, std::int16_t *pileft, int strideleft, std::int16_t *piright, int strideright, int c)
		{
			do
			{
				int r = d2i(pin[1]);
				if (r < -32768)
				{
					r = -32768;
				}
				else if (r > 32767)
				{
					r = 32767;
				}
				*piright = static_cast<std::int16_t>(r);
				piright += strideright;
				int l = d2i(pin[0]);
				if (l < -32768)
				{
					l = -32768;
				}
				else if (l > 32767)
				{
					l = 32767;
				}
				*pileft = static_cast<std::int16_t>(l);
				pileft += strideleft;
				pin += 2;
			}
			while(--c);
		}

		void AudioDriver::DeQuantize16AndDeinterlace(int const *pin, float *poutleft,float *poutright,int c)
		{
			do
			{
				*poutleft++ = static_cast<short int>(*pin&0xFFFF);
				*poutright++ = static_cast<short int>((*pin&0xFFFF0000)>>16);
				pin++;
			}
			while(--c);
		}
	}
}