/** @file
 *  @brief Helper Library �w���p���C�u����
 *
 *  ���̃��C�u�����́A�����̃R�[�h��⊮����ړI�ō쐬����܂����B\n
 *  This library was created in order to complement the existing code.
 *  @author S.F. (Satoshi Fujiwara)
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
#include ".\sfhelper.h"



/// �ÓI�ϐ���` static variable
std::auto_ptr<SF::CResource> SF::CResource::m_pResource;
