// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "styles.h"
// ui
#include <uidefaults.h>

void initdarkstyles(psy_ui_Defaults* defaults)
{
	psy_ui_Style* style;

	// sequenceeditor item
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00D1C5B6));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_make(0x00333333));
	psy_ui_border_init(&style->border);
	psy_ui_colour_set(&style->border.colour_top,
		psy_ui_colour_make(0x00444444));
	psy_ui_colour_set(&style->border.colour_right,
		style->border.colour_top);
	psy_ui_colour_set(&style->border.colour_bottom,
		style->border.colour_top);
	psy_ui_colour_set(&style->border.colour_left,
		style->border.colour_top);
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
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00B1C8B0));
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
}

void initlightstyles(psy_ui_Defaults* defaults)
{
	psy_ui_Style* style;	

	// sequenceeditor item
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00444444));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_make(0xb8d6ff));
	psy_ui_border_init(&style->border);
	psy_ui_colour_set(&style->border.colour_top,
		psy_ui_colour_make(0xB1C2F5));
	psy_ui_colour_set(&style->border.colour_right,
		style->border.colour_top);
	psy_ui_colour_set(&style->border.colour_bottom,
		style->border.colour_top);
	psy_ui_colour_set(&style->border.colour_left,
		style->border.colour_top);
	psy_ui_defaults_setstyle(defaults, STYLE_SEQEDT_ITEM, style);
	// sequenceeditor item selected
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0xFFFFFF));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_make(0x4d8ee5));
	psy_ui_border_init(&style->border);
	psy_ui_colour_set(&style->border.colour_top,
		psy_ui_colour_make(0xb8d6ff));
	psy_ui_colour_set(&style->border.colour_right,
		style->border.colour_top);
	psy_ui_colour_set(&style->border.colour_bottom,
		style->border.colour_top);
	psy_ui_colour_set(&style->border.colour_left,
		style->border.colour_top);
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
}
