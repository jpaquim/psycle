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

/*
#if defined(_WINAMP_PLUGIN_)
	#include "global.h"
#endif // _WINAMP_PLUGIN_
*/
#include "Filter.h"



FilterCoeff Filter::_coeffs;

Filter::Filter(const int samplesPerSec)
{
	_coeffs.Init(samplesPerSec);
	Init();
	_x1 = _x2 = _y1 = _y2 = 0;
	_a1 = _a2 = _b1 = _b2 = 0;
}

void Filter::Init(void)
{
}

void Filter::Update(void)
{
	_coeff0 = _coeffs._coeffs[_type][_cutoff][_q][0];
	_coeff1 = _coeffs._coeffs[_type][_cutoff][_q][1];
	_coeff2 = _coeffs._coeffs[_type][_cutoff][_q][2];
	_coeff3 = _coeffs._coeffs[_type][_cutoff][_q][3];
	_coeff4 = _coeffs._coeffs[_type][_cutoff][_q][4];
}
