// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "uiframe.h"
#include "hashtbl.h"

extern TCHAR szAppClass[];
extern HINSTANCE appInstance;
extern IntHashTable selfmap;

void ui_frame_init(ui_frame* frame, ui_component* parent)
{		
	HWND hWndParent = 0;
	int style = 0;

	if (parent) {
	  hWndParent = parent->hwnd;
	  // style  |= WS_CHILD;
	}
	memset(&frame->events, 0, sizeof(ui_events));	
	ui_component_init_signals(frame);
	frame->doublebuffered = 0;
	frame->hwnd = CreateWindow (szAppClass, TEXT ("Psycle"),
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          CW_USEDEFAULT, CW_USEDEFAULT | style,
                          hWndParent, NULL, appInstance, NULL);     	
	InsertIntHashTable(&selfmap, (int)frame->hwnd, frame);	
	frame->events.target = frame;
	frame->events.cmdtarget = frame;
	ui_component_init_base(frame);
}
