// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "styles.h"
// ui
#include <uidefaults.h>

void initdarkstyles(psy_ui_Defaults* defaults)
{
	// Statusbar
	psy_ui_Style* style;
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_make(0x00D1C5B6),	
		psy_ui_colour_make(0x00252525));
	psy_ui_border_init_all(&style->border, psy_ui_BORDER_SOLID,
		psy_ui_BORDER_NONE, psy_ui_BORDER_NONE, psy_ui_BORDER_NONE);
	psy_ui_border_setcolour(&style->border, psy_ui_colour_make(0x00282828));	
	psy_ui_colour_set(&style->border.colour_bottom, psy_ui_colour_make(0x00232323));	
	psy_ui_defaults_setstyle(defaults, STYLE_STATUSBAR, style);
	// seqview buttons
	style = psy_ui_style_allocinit();	
	psy_ui_defaults_setstyle(defaults, STYLE_SEQVIEW_BUTTONS, style);
	// seqview button
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_make(0x00909090),
		psy_ui_colour_make(0x00232323));
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_make(0x003D3D3D));
	psy_ui_border_setradius_px(&style->border, 5.0);
	psy_ui_defaults_setstyle(defaults, STYLE_SEQVIEW_BUTTON, style);
	// seqview button::hover
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_make(0x00EEFFFF),
		psy_ui_colour_make(0x00202020));
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_make(0x00EEFFFF));
	psy_ui_border_setradius_px(&style->border, 5.0);
	psy_ui_defaults_setstyle(defaults, STYLE_SEQVIEW_BUTTON_HOVER, style);
	// seqview button::select
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_make(0x00D1C5B6),
		psy_ui_colour_make(0x00202020));
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_make(0x00444444));
	psy_ui_border_setradius_px(&style->border, 5.0);
	psy_ui_defaults_setstyle(defaults, STYLE_SEQVIEW_BUTTON_SELECT, style);
	// seqlistview
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_make(0x00D1C5B6),	
		psy_ui_colour_make(0x00202121));
	psy_ui_border_init_all(&style->border, psy_ui_BORDER_SOLID,
		psy_ui_BORDER_SOLID, psy_ui_BORDER_SOLID, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border, psy_ui_colour_make(0x000151515));
	psy_ui_colour_set(&style->border.colour_right, psy_ui_colour_make(0x00303030));
	psy_ui_colour_set(&style->border.colour_bottom, psy_ui_colour_make(0x00303030));
	psy_ui_defaults_setstyle(defaults, STYLE_SEQLISTVIEW, style);
	// seqeditor
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00D1C5B6));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_make(0x00252525));
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_make(0x00282828));
	//psy_ui_colour_set(&style->border.colour_bottom,
		//psy_ui_colour_make(0x00333333));			
	psy_ui_defaults_setstyle(defaults, STYLE_SEQEDT, style);
	// sequenceeditor item
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00D1C5B6));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_make(0x00333333));
	psy_ui_border_init(&style->border);
	psy_ui_border_setcolour(&style->border,	
		psy_ui_colour_make(0x00444444));	
	psy_ui_defaults_setstyle(defaults, STYLE_SEQEDT_ITEM, style);
	// sequenceeditor item::selected
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00D1C5B6));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_make(0x00514536));
	psy_ui_border_init(&style->border);
	psy_ui_colour_set(&style->border.colour_top,
		psy_ui_colour_make(0x00555555));
	psy_ui_colour_set(&style->border.colour_right,
		style->border.colour_top);
	psy_ui_colour_set(&style->border.colour_bottom,
		style->border.colour_top);
	psy_ui_colour_set(&style->border.colour_left,
		style->border.colour_top);
	psy_ui_defaults_setstyle(defaults, STYLE_SEQEDT_ITEM_SELECTED, style);
	// sequencetrackbox tab
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00D1C5B6));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_make(0x00232323));
	psy_ui_border_init(&style->border);
	psy_ui_colour_set(&style->border.colour_top,
		psy_ui_colour_make(0x00333333));
	psy_ui_colour_set(&style->border.colour_right,
		style->border.colour_top);
	psy_ui_colour_set(&style->border.colour_bottom,
		style->border.colour_top);
	psy_ui_colour_set(&style->border.colour_left,
		style->border.colour_top);
	psy_ui_defaults_setstyle(defaults, STYLE_SEQ_TAB, style);
	// sequencetrackbox tab::hover
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00D1C5B6));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_make(0x00232323));
	psy_ui_border_init(&style->border);
	psy_ui_colour_set(&style->border.colour_top,
		psy_ui_colour_make(0x00333333));
	psy_ui_colour_set(&style->border.colour_right,
		style->border.colour_top);
	psy_ui_colour_set(&style->border.colour_bottom,
		style->border.colour_top);
	psy_ui_colour_set(&style->border.colour_left,
		style->border.colour_top);
	psy_ui_defaults_setstyle(defaults, STYLE_SEQ_TAB_HOVER, style);
	// sequencetrackbox tab::select
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x009F7B00));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_make(0x00232323));
	psy_ui_border_init(&style->border);
	psy_ui_colour_set(&style->border.colour_top,
		psy_ui_colour_make(0x00B1C8B0));
	psy_ui_colour_set(&style->border.colour_right,
		style->border.colour_top);
	psy_ui_colour_set(&style->border.colour_bottom,
		style->border.colour_top);
	psy_ui_colour_set(&style->border.colour_left,
		style->border.colour_top);
	psy_ui_defaults_setstyle(defaults, STYLE_SEQ_TAB_SELECT, style);
	// sequencetrackbox newtrack
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00444444));
	psy_ui_defaults_setstyle(defaults, STYLE_SEQ_TAB_NEW, style);
	// sequenceview progress
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00D1C5B6));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_make(0x00514536));
	psy_ui_defaults_setstyle(defaults, STYLE_SEQ_PROGRESS, style);
	// machinebox
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00D1C5B6));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_make(0x00202122));
	psy_ui_border_init_all(&style->border, psy_ui_BORDER_SOLID,
		psy_ui_BORDER_SOLID, psy_ui_BORDER_SOLID, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border, psy_ui_colour_make(0x000161616));
	psy_ui_colour_set(&style->border.colour_right, psy_ui_colour_make(0x00282828));
	psy_ui_colour_set(&style->border.colour_bottom, psy_ui_colour_make(0x00303030));
	psy_ui_defaults_setstyle(defaults, STYLE_MACHINEBOX, style);
	// zoombox
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00D1C5B6));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_make(0x00282828));	
	psy_ui_defaults_setstyle(defaults, STYLE_ZOOMBOX, style);
	// machineview
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00D1C5B6));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_make(0x00202020));	
	psy_ui_defaults_setstyle(defaults, STYLE_MACHINEVIEW, style);
	// stepsequencer
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00D1C5B6));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_make(0x00272727));
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border, psy_ui_colour_make(0x00272727));	
	psy_ui_border_setradius_px(&style->border, 10.0);
	psy_ui_defaults_setstyle(defaults, STYLE_STEPSEQUENCER, style);
	// top
	style = psy_ui_style_allocinit();	
	psy_ui_border_init_all(&style->border, psy_ui_BORDER_SOLID,
		psy_ui_BORDER_NONE, psy_ui_BORDER_NONE, psy_ui_BORDER_NONE);
	psy_ui_border_setcolour(&style->border, psy_ui_colour_make(0x00303030));
	psy_ui_defaults_setstyle(defaults, STYLE_TOP, style);	
	// toprows
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_make(0x00232323));	
	psy_ui_defaults_setstyle(defaults, STYLE_TOPROWS, style);
	// toprow0
	style = psy_ui_style_allocinit();	
	psy_ui_defaults_setstyle(defaults, STYLE_TOPROW0, style);
	// toprow1
	style = psy_ui_style_allocinit();	
	psy_ui_defaults_setstyle(defaults, STYLE_TOPROW1, style);
	// toprow2
	style = psy_ui_style_allocinit();	
	psy_ui_defaults_setstyle(defaults, STYLE_TOPROW2, style);
	// trackscopes
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_make(0x00232323));	
	psy_ui_defaults_setstyle(defaults, STYLE_TRACKSCOPES, style);
}

void initlightstyles(psy_ui_Defaults* defaults)
{
	psy_ui_Style* style;	

	// StatusBar	
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00474849));
	psy_ui_colour_set(&style->backgroundcolour, psy_ui_colour_make(0x00F2F2F2));
	psy_ui_defaults_setstyle(defaults, STYLE_STATUSBAR, style);
	// sequencelistview item
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00D1C5B6));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_make(0x00E9E9E9));
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_make(0xb8d6ff));
	psy_ui_defaults_setstyle(defaults, STYLE_SEQLISTVIEW, style);
	// sequenceeditor item
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00444444));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_make(0xb8d6ff));	
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_make(0xB1C2F5));	
	psy_ui_defaults_setstyle(defaults, STYLE_SEQEDT_ITEM, style);
	// sequenceeditor item selected
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0xFFFFFF));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_make(0x4d8ee5));	
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_make(0xb8d6ff));	
	psy_ui_defaults_setstyle(defaults, STYLE_SEQEDT_ITEM_SELECTED,
		style);
	// sequencetrackbox tab
	style = psy_ui_style_allocinit();	
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00D1C5B6));
	psy_ui_border_init(&style->border);
	psy_ui_colour_set(&style->border.colour_left,
		psy_ui_colour_make(0x00B2B2B2));
	psy_ui_defaults_setstyle(defaults, STYLE_SEQ_TAB, style);
	// sequencetrackbox tab::hover
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x1050F0));
	psy_ui_border_init(&style->border);
	psy_ui_colour_set(&style->border.colour_left,
		psy_ui_colour_make(0x00B2B2B2));
	psy_ui_defaults_setstyle(defaults, STYLE_SEQ_TAB_HOVER, style);
	// sequencetrackbox tab::select
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x1050F0));
	psy_ui_border_init(&style->border);
	psy_ui_colour_set(&style->border.colour_left,
		psy_ui_colour_make(0x1050F0));
	psy_ui_defaults_setstyle(defaults, STYLE_SEQ_TAB_SELECT, style);
	// sequencetrackbox newtrack
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00D1C5B6));		
	psy_ui_defaults_setstyle(defaults, STYLE_SEQ_TAB_NEW, style);
	// sequenceeditor event
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x8398F1));	
	psy_ui_border_init(&style->border);
	psy_ui_colour_set(&style->border.colour_left,
		psy_ui_colour_make(0x1b8ff2));	
	psy_ui_defaults_setstyle(defaults, STYLE_SEQEDT_EVENT, style);
	// sequenceview progress
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00444444));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_make(0x0979F7));
	psy_ui_defaults_setstyle(defaults, STYLE_SEQ_PROGRESS, style);
	// machinebox
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00D1C5B6));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_make(0x00E9E9E9));
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_make(0xb8d6ff));
	psy_ui_defaults_setstyle(defaults, STYLE_MACHINEBOX, style);
	// zoombox
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00D1C5B6));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_make(0x00E9E9E9));
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_make(0xb8d6ff));
	psy_ui_defaults_setstyle(defaults, STYLE_ZOOMBOX, style);
}
