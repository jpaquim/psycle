// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(TRANSFORMPATTERNVIEW_H)
#define TRANSFORMPATTERNVIEW_H

// host
#include "workspace.h"
// ui
#include <uibutton.h>
#include <uicheckbox.h>
#include <uicombobox.h>
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

// TransformPatternView view
//
// The search and replace view, where some sustitutions can be made on all the
// patterns.

typedef struct TransformPatternView {
	// inherits
	psy_ui_Component component;
	// ui elements	
	// search
	psy_ui_Component search;
	psy_ui_Component searchtop;
	psy_ui_Button hide;
	psy_ui_Label searchtitle;
	psy_ui_Label searchnotedesc;
	psy_ui_Label searchinstdesc;
	psy_ui_Label searchmachdesc;
	psy_ui_ComboBox searchnote;	
	psy_ui_ComboBox searchinst;
	psy_ui_ComboBox searchmach;	
	// replace
	psy_ui_Component replace;
	psy_ui_Label replacetitle;
	psy_ui_Label replacenotedesc;
	psy_ui_Label replaceinstdesc;
	psy_ui_Label replacemachdesc;
	psy_ui_ComboBox replacenote;
	psy_ui_ComboBox replaceinst;
	psy_ui_ComboBox replacemach;
	// searchon
	psy_ui_Component searchon;
	psy_ui_Label searchontitle;
	psy_ui_Component searchonchoice;
	psy_ui_Label entire;
	psy_ui_Label currpattern;
	psy_ui_Label currselection;
	// actions	
	psy_ui_Component actions;
	psy_ui_Button dosearch;
	psy_ui_Button doreplace;
	psy_ui_Button docancel;
	// internal data
	psy_ui_Margin sectionmargin;
	int applyto;
	psy_audio_PatternSelection patternselection;
	// references
	Workspace* workspace;
} TransformPatternView;

void transformpatternview_init(TransformPatternView*, psy_ui_Component* parent,
	Workspace*);

void transformpatternview_setpatternselection(TransformPatternView*,
	const psy_audio_PatternSelection*);

INLINE psy_ui_Component* transformpatternview_base(TransformPatternView* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* TRANSFORMPATTERNVIEW_H */
