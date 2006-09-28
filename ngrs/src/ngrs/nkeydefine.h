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

const unsigned int NK_BackSpace	= XK_BackSpace;
const unsigned int NK_Tab = XK_Tab;
const unsigned int NK_Clear = XK_Clear;
const unsigned int NK_Return = XK_Return;
const unsigned int NK_Pause = XK_Pause;
const unsigned int NK_Scroll_Lock = XK_Scroll_Lock;
const unsigned int NK_Escape = XK_Escape;
const unsigned int NK_Delete = XK_Escape;

/* Cursor control & motion */

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

#else

const unsigned int NK_BackSpace	= VK_BACK;
const unsigned int NK_Tab = VK_TAB;
const unsigned int NK_Clear = VK_CLEAR;
const unsigned int NK_Return = VK_RETURN;
const unsigned int NK_Pause = VK_PAUSE;
const unsigned int NK_Scroll_Lock = VK_SCROLL;
const unsigned int NK_Escape = VK_ESCAPE;
const unsigned int NK_Delete = VK_DELETE;

/* Cursor control & motion */

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

#endif