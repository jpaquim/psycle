/*****************************************************************************/
/* EffectWnd.h : Interface for CEffectWnd class. (Base plugin window)		 */
/*****************************************************************************/

/*****************************************************************************/
/* Work Derived from the LGPL host "vsthost (1.16l)".						 */
/* (http://www.hermannseib.com/english/vsthost.htm)"						 */
/* vsthost has the following lincense:										 *

Copyright (C) 2006  Hermann Seib

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
******************************************************************************/

#pragma once
#include "CVSTHost.Seib.hpp"
namespace seib {
	namespace vst{

typedef struct
{
	UINT vkWin;
	unsigned char vstVirt;
} VkeysT;

class CWnd;
class CSecEditWnd;
class CParamEditWnd;

class CEffectWnd
{
protected:
	CEffectWnd(){};
public:
	CEffectWnd(CEffect* effect);

// Operations
public:
	virtual ~CEffectWnd();
    CEffect& GetEffect() { return *pEffect; }
	void SetTitleText(const char* lpszText = NULL) { sTitle = (lpszText) ? lpszText : ""; UpdateTitle(); }

protected:
	void ConvertToVstKeyCode(UINT nChar, UINT nRepCnt, UINT nFlags, VstKeyCode &keyCode);
//	virtual bool GetWindowSize(CRect &rcFrame, CRect &rcClient, ERect *pRect = NULL);

	void KeyDown(VstKeyCode keyCode) { pEffect->KeyDown(keyCode); }
	void KeyUp(VstKeyCode keyCode) { pEffect->KeyUp(keyCode); }
	void ChangeProgram(int index) { pEffect->SetProgram(index); }
	bool SaveBank(std::string file);
	bool SaveProgram(std::string file);


// Overridables
public:
	virtual void* OpenEditorWnd()=0;
	virtual void CloseEditorWnd()=0;
	virtual void ResizeWindow(int width, int height){};
	virtual void RefreshUI(){};
	virtual bool OnSetParameterAutomated(long index, float value) { return false; }
	virtual CWnd* OpenSecondaryWnd(VstWindow& window) { return 0; }
	virtual bool CloseSecondaryWnd(VstWindow& window) { return false; }

protected:
	virtual void UpdateTitle() { ; }
	virtual CWnd * CreateView() { return NULL; }
	virtual void CreateMenu() { ; }

protected:
	CEffect * pEffect;
	std::string sTitle;
	static VkeysT VKeys[];
};
	}
}
