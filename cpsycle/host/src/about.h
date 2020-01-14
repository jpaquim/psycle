// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(ABOUT_H)
#define ABOUT_H

#include <uibutton.h>
#include <uiedit.h>
#include <uiimage.h>
#include <uilabel.h>
#include <uinotebook.h>

typedef struct {	
	psy_ui_Component component;		
	psy_ui_Label asio;
	psy_ui_Edit	sourceforge;
	psy_ui_Edit	psycledelics;
	psy_ui_Label steincopyright;		
	psy_ui_Label headercontrib;
	psy_ui_Edit	contrib;	
} Contrib;

void contrib_init(Contrib*, psy_ui_Component* parent);

typedef struct {
	psy_ui_Component component;
	psy_ui_Label versioninfo;
} Version;

void version_init(Version*, psy_ui_Component* parent);

typedef struct {	
	psy_ui_Component component;
	psy_ui_Notebook notebook;
	psy_ui_Image image;
	Contrib contrib;
	Version version;
	psy_ui_Button contribbutton;
	psy_ui_Button versionbutton;
	psy_ui_Button okbutton;		
} About;

void about_init(About*, psy_ui_Component* parent);

#endif
