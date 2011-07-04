/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.3 $
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
#include "Dsp.h"

namespace model {

int Cubic::_resolution;
float Cubic::_aTable[CUBIC_RESOLUTION];
float Cubic::_bTable[CUBIC_RESOLUTION];
float Cubic::_cTable[CUBIC_RESOLUTION];
float Cubic::_dTable[CUBIC_RESOLUTION];
float Cubic::_lTable[CUBIC_RESOLUTION];

Cubic::Cubic()
{
	_resolution = CUBIC_RESOLUTION;
	// Initialize tables
	//
	for (int i=0; i<_resolution; i++)
	{
		float x = (float)i/(float)_resolution;
		_aTable[i] = float(-0.5*x*x*x +     x*x - 0.5*x);
		_bTable[i] = float( 1.5*x*x*x - 2.5*x*x         + 1);
		_cTable[i] = float(-1.5*x*x*x + 2.0*x*x + 0.5*x);
		_dTable[i] = float( 0.5*x*x*x - 0.5*x*x);
		_lTable[i] = x;
	}
}

}