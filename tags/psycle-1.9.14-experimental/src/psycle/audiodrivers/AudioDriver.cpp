///\implementation psycle::host::AudioDriver
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/audiodrivers/AudioDriver.hpp>
#include <psycle/helpers/helpers.hpp>
#include <psycle/engine/constants.hpp>
#include <cstdint>
namespace psycle
{
	namespace host
	{

		AudioDriverInfo AudioDriver::_info = { "Silent" };

		namespace
		{
			///\todo use std::numeric_limits<std::int16_t> instead
			int const SHORT_MIN = -32768;
			int const SHORT_MAX =  32767;

			// returns random value between 0 and 1
			// the magic numbers come from csound so they should be ok
			inline double frand()
			{
				///\todo sizeof(signed long int) has to match "0x7fffffff" (32-bit)

				static signed long int stat = 0x16BA2118;
				stat = (stat * 1103515245 + 12345) & 0x7fffffff;
				return (double)stat * (1.0 / 0x7fffffff);
			}
		}

		AudioDriver::AudioDriver()
			: _samplesPerSec(44100)
			, _bitDepth(16)
			, _channelmode(3)
			, _blockSize(0)
		{}

		void AudioDriver::QuantizeWithDither(float *pin, int *piout, int c)
		{
			///\todo sizeof *piout has to be 32-bit

			double const d2i = (1.5 * (1 << 26) * (1 << 26));
			
			do
			{
				double res = ((double)pin[1] + frand()) + d2i;

				int r = *(int *)&res;

				if (r < SHORT_MIN)
				{
					r = SHORT_MIN;
				}
				else if (r > SHORT_MAX)
				{
					r = SHORT_MAX;
				}

				res = ((double)pin[0] + frand()) + d2i;

				int l = *(int *)&res;

				if (l < SHORT_MIN)
				{
					l = SHORT_MIN;
				}
				else if (l > SHORT_MAX)
				{
					l = SHORT_MAX;
				}

				*piout++ = (r << 16) | static_cast<std::uint16_t>(l);
				pin += 2;
			}
			while(--c);
		}

		void AudioDriver::Quantize(float *pin, int *piout, int c)
		{
			do
			{
				int r = f2i(pin[1]);

				if (r < SHORT_MIN)
				{
					r = SHORT_MIN;
				}
				else if (r > SHORT_MAX)
				{
					r = SHORT_MAX;
				}

				int l = f2i(pin[0]);

				if (l < SHORT_MIN)
				{
					l = SHORT_MIN;
				}
				else if (l > SHORT_MAX)
				{
					l = SHORT_MAX;
				}

				*piout++ = (r << 16) | static_cast<std::uint16_t>(l);
				pin += 2;
			}
			while(--c);
		}
	}
}

