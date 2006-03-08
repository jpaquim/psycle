///\implementation psycle::host::AudioDriver
#include <project.private.hpp>
#include "AudioDriver.hpp"
#include "helpers.hpp"
#include "constants.hpp"
namespace psycle
{
	namespace host
	{
		#define SHORT_MIN	-32768
		#define SHORT_MAX	32767

		AudioDriverInfo AudioDriver::_info = { "Silent" };

		// returns random value between 0 and 1
		// i got the magic numbers from csound so they should be ok but 
		// I haven't checked them myself
		inline double frand()
		{
			static long stat = 0x16BA2118;
			stat = (stat * 1103515245 + 12345) & 0x7fffffff;
			return (double)stat * (1.0 / 0x7fffffff);
		}
		AudioDriver::AudioDriver()
			: _samplesPerSec(44100)
			, _bitDepth(16)
			, _channelmode(3)
			, _numBlocks(0)
			, _blockSize(0)
		{}

		void AudioDriver::QuantizeWithDither(float *pin, int *piout, int c)
		{
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
				*piout++ = (r << 16) | (word)l;
				pin += 2;
			}
			while(--c);
		}

		void AudioDriver::Quantize(float *pin, int *piout, int c)
		{
		//	double const d2i = (1.5 * (1 << 26) * (1 << 26));
			
			do
			{
		//		double res = ((double)pin[1]) + d2i;
		//		int r = *(int *)&res;
				int r = f2i(pin[1]);

				if (r < SHORT_MIN)
				{
					r = SHORT_MIN;
				}
				else if (r > SHORT_MAX)
				{
					r = SHORT_MAX;
				}
		//		res = ((double)pin[0]) + d2i;
		//		int l = *(int *)&res;
				int l = f2i(pin[0]);

				if (l < SHORT_MIN)
				{
					l = SHORT_MIN;
				}
				else if (l > SHORT_MAX)
				{
					l = SHORT_MAX;
				}
				*piout++ = (r << 16) | (word)l;
				pin += 2;
			}
			while(--c);
		}
	}
}

