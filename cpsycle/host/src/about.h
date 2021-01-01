// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(ABOUT_H)
#define ABOUT_H

// host
#include "workspace.h"
// ui
#include <uibutton.h>
#include <uiedit.h>
#include <uieditor.h>
#include <uiimage.h>
#include <uilabel.h>
#include <uinotebook.h>
#include <uiscroller.h>

#ifdef __cplusplus
extern "C" {
#endif

// About
//
// Shows information about Psycle.

typedef struct Contrib {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Label asio;
	psy_ui_Edit	sourceforge;
	psy_ui_Edit	psycledelics;
	psy_ui_Label steincopyright;		
	psy_ui_Label headercontrib;
	psy_ui_Edit	contrib;	
} Contrib;

void contrib_init(Contrib*, psy_ui_Component* parent);

INLINE psy_ui_Component* contrib_base(Contrib* self)
{
	assert(self);

	return &self->component;
}

typedef struct Version{
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Label versioninfo;
} Version;

void version_init(Version*, psy_ui_Component* parent);

INLINE psy_ui_Component* version_base(Version* self)
{
	assert(self);

	return &self->component;
}

typedef struct Licence {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Editor licenceinfo;
} Licence;

void licence_init(Licence*, psy_ui_Component* parent);

INLINE psy_ui_Component* licence_base(Licence* self)
{
	assert(self);

	return &self->component;
}

typedef struct About {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Notebook notebook;
	psy_ui_Image image;
	Contrib contrib;
	Version version;
	Licence licence;
	psy_ui_Button contribbutton;
	psy_ui_Button versionbutton;
	psy_ui_Button licencebutton;
	psy_ui_Button okbutton;
	// references
	Workspace* workspace;
} About;

void about_init(About*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* about_base(About* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* ABOUT_H */
