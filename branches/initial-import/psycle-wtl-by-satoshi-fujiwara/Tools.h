///////////////////////////////////////////////////////////////////////////////
//
// Tools.h : header file
//
///////////////////////////////////////////////////////////////////////////////

#pragma once


///////////////////////////////////////////////////////////////////////////////
// Usefull macros
//
#define KEYDOWN(Key) ((GetKeyState(Key)&0x8000)!=0)


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class CClientRect : public CRect
{
public:
    CClientRect (HWND hWnd)
    {
        ::GetClientRect (hWnd, this);
    };
    CClientRect (const CWindow* pWnd)
    {
        ::GetClientRect (pWnd->m_hWnd, this);
    };
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class CWindowRect : public CRect
{
public:
    CWindowRect (HWND hWnd)
    {
        ::GetWindowRect (hWnd, this);
    };
    CWindowRect (const CWindow* pWnd)
    {
        ::GetWindowRect (pWnd->m_hWnd, this);
    };
};
