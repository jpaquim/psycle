/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.4 $
 */
#include "stdafx.h"
#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#include "AudioDriver.h"
#include "helpers.h"
#include "constants.h"

#define SHORT_MIN	-32768
#define SHORT_MAX	32767

namespace model {
	namespace audiodriver {

		string DummyAudioDriver::m_DriverName = _T("Silent");
		string DummyAudioDriver::m_DriverDescription = _T("Silent Driver");

		// returns random value between 0 and 1
		// i got the magic numbers from csound so they should be ok but 
		// I haven't checked them myself
		inline double frand()
		{
			static long stat = 0x16BA2118;
			stat = (stat * 1103515245 + 12345) & 0x7fffffff;
			return (double)stat * (1.0 / 0x7fffffff);
		}

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



