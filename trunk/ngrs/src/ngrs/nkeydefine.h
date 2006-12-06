/***************************************************************************
 *   Copyright (C) 2005 by Stefan Nattkemper  *
 *   natti@linux   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifdef __unix__

#include <X11/keysym.h>

const unsigned int NK_Shift = XK_Shift_L;

const unsigned int NK_BackSpace	= XK_BackSpace;
const unsigned int NK_Tab = XK_Tab;
const unsigned int NK_Clear = XK_Clear;
const unsigned int NK_Return = XK_Return;
const unsigned int NK_Pause = XK_Pause;
const unsigned int NK_Scroll_Lock = XK_Scroll_Lock;
const unsigned int NK_Escape = XK_Escape;
const unsigned int NK_Delete = XK_Delete;

// Cursor control & motion

const unsigned int NK_Home = XK_Home;
const unsigned int NK_Left = XK_Left;
const unsigned int NK_Up = XK_Up;
const unsigned int NK_Right = XK_Right;
const unsigned int NK_Down = XK_Down;
const unsigned int NK_Prior = XK_Prior;
const unsigned int NK_Page_Up = XK_Page_Up;
const unsigned int NK_Next = XK_Next;
const unsigned int NK_Page_Down = XK_Page_Down;
const unsigned int NK_End = XK_End;
const unsigned int NK_Begin = XK_Begin;

/* Misc Functions */

const unsigned int NK_Select = XK_Select;
const unsigned int NK_Print = XK_Print;
const unsigned int NK_Execute = XK_Execute;
const unsigned int NK_Insert = XK_Insert;
const unsigned int NK_Menu = XK_Menu;
const unsigned int NK_Cancel = XK_Cancel;
const unsigned int NK_Help = XK_Help;
const unsigned int NK_Break = XK_Break;
const unsigned int NK_Num_Lock = XK_Num_Lock;


// Auxilliary Functions

const unsigned int NK_F1 = XK_F1;
const unsigned int NK_F2 = XK_F2;
const unsigned int NK_F3 = XK_F3;
const unsigned int NK_F4 = XK_F4;
const unsigned int NK_F5 = XK_F5;
const unsigned int NK_F6 = XK_F6;
const unsigned int NK_F7 = XK_F7;
const unsigned int NK_F8 = XK_F8;
const unsigned int NK_F9 = XK_F9;
const unsigned int NK_F10 = XK_F10;
const unsigned int NK_F11 = XK_F11;
const unsigned int NK_F12 = XK_F12;
const unsigned int NK_F13 = XK_F13;
const unsigned int NK_F14 = XK_F14;
const unsigned int NK_F15 = XK_F15;
const unsigned int NK_F16 = XK_F16;
const unsigned int NK_F17 = XK_F17;
const unsigned int NK_F18 = XK_F18;
const unsigned int NK_F19 = XK_F19;
const unsigned int NK_F20 = XK_F20;
const unsigned int NK_F21 = XK_F21;
const unsigned int NK_F22 = XK_F22;
const unsigned int NK_F23 = XK_F23;
const unsigned int NK_F24 = XK_F24;

#else

#pragma once
#include <windows.h>

const unsigned int NK_Shift = VK_SHIFT;


const unsigned int NK_BackSpace	= VK_BACK;
const unsigned int NK_Tab = VK_TAB;
const unsigned int NK_Clear = VK_CLEAR;
const unsigned int NK_Return = VK_RETURN;
const unsigned int NK_Pause = VK_PAUSE;
const unsigned int NK_Scroll_Lock = VK_SCROLL;
const unsigned int NK_Escape = VK_ESCAPE;
const unsigned int NK_Delete = VK_DELETE;

// Cursor control & motion

const unsigned int NK_Home = VK_HOME;
const unsigned int NK_Left = VK_LEFT;
const unsigned int NK_Up = VK_UP;
const unsigned int NK_Right = VK_RIGHT;
const unsigned int NK_Down = VK_DOWN;
const unsigned int NK_Prior = VK_PRIOR ;
const unsigned int NK_Page_Up = VK_PRIOR;
const unsigned int NK_Next = VK_NEXT;
const unsigned int NK_Page_Down = VK_NEXT;
const unsigned int NK_End = VK_END;
const unsigned int NK_Begin = VK_HOME;


/* Misc Functions */

const unsigned int NK_Select = VK_SELECT;
const unsigned int NK_Print = VK_PRINT;
const unsigned int NK_Execute = VK_EXECUTE;
const unsigned int NK_Insert = VK_INSERT;
const unsigned int NK_Menu = VK_MENU;
const unsigned int NK_Cancel = VK_CANCEL;
const unsigned int NK_Help = VK_HELP;
const unsigned int NK_Break = VK_CANCEL;
const unsigned int NK_Num_Lock = VK_NUMLOCK;

// Auxilliary Functions

const unsigned int NK_F1 = VK_F1;
const unsigned int NK_F2 = VK_F2;
const unsigned int NK_F3 = VK_F3;
const unsigned int NK_F4 = VK_F4;
const unsigned int NK_F5 = VK_F5;
const unsigned int NK_F6 = VK_F6;
const unsigned int NK_F7 = VK_F7;
const unsigned int NK_F8 = VK_F8;
const unsigned int NK_F9 = VK_F9;
const unsigned int NK_F10 = VK_F10;
const unsigned int NK_F11 = VK_F11;
const unsigned int NK_F12 = VK_F12;
const unsigned int NK_F13 = VK_F13;
const unsigned int NK_F14 = VK_F14;
const unsigned int NK_F15 = VK_F15;
const unsigned int NK_F16 = VK_F16;
const unsigned int NK_F17 = VK_F17;
const unsigned int NK_F18 = VK_F18;
const unsigned int NK_F19 = VK_F19;
const unsigned int NK_F20 = VK_F20;
const unsigned int NK_F21 = VK_F21;
const unsigned int NK_F22 = VK_F22;
const unsigned int NK_F23 = VK_F23;
const unsigned int NK_F24 = VK_F24;


#endif
