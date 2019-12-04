// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "about.h"
#include <psycleversion.h>

#include <stdio.h>
#include "resources/resource.h"

static void OnSize(About*, ui_component* sender, ui_size*);
static void InitButtons(About* self);
static void OnContributors(About*, ui_component* sender);
static void OnVersion(About*, ui_component* sender);
static void OnShowatstartup(About*, ui_component* sender);
static void Align(About*, ui_component* sender);
static void OnMouseDoubleClick(About*, ui_component* sender, int x, int y, int button);
	
void InitContrib(Contrib* self, ui_component* parent)
{
	ui_margin margin = { 0, 3, 0, 0 };

	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);	
	ui_edit_init(&self->contrib, &self->component, 
		WS_VSCROLL | ES_MULTILINE |ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_READONLY);
	ui_edit_setlinenumber(&self->contrib, 10);
	ui_edit_settext(&self->contrib,						
		"Josep Mª Antolín. [JAZ]/JosepMa\tDeveloper since release 1.5" "\r\n"
		"Johan Boulé [bohan]\t\tDeveloper since release 1.7.3" "\r\n"
		"Stefan Nattkemper\t\t\tDeveloper of the LUA host in release 1.12" "\r\n"
		"James Redfern [alk]\t\tDeveloper and plugin coder" "\r\n"
		"Magnus Jonsson [zealmange]\t\tDeveloper during 1.7.x and 1.9alpha" "\r\n"
		"Jeremy Evers [pooplog]\t\tDeveloper in releases 1.7.x" "\r\n"
		"Daniel Arena\t\t\tDeveloper in release 1.5 & 1.6" "\r\n"
		"Marcin Kowalski [FideLoop]\t\tDeveloper in release 1.6" "\r\n"
		"Mark McCormack\t\t\tMIDI (in) Support in release 1.6" "\r\n"
		"Mats Höjlund\t\t\tMain Developer until release 1.5" "\r\n" // (Internal Recoding) .. doesn't fit in the small box :/
		"Juan Antonio Arguelles [Arguru] (RIP)\tOriginal Developer and maintainer until 1.0" "\r\n"
		"Satoshi Fujiwara\t\t\tBase code for the Sampulse machine\r\n"
		"Hermann Seib\t\t\tBase code for the new VST Host in 1.8.5\r\n"
		"Martin Etnestad Johansen [lobywang]\tCoding Help" "\r\n"
		"Patrick Haworth [TranceMyriad]\tAuthor of the Help File" "\r\n"
		"Budislav Stepanov\t\t\tNew artwork for version 1.11\r\n"
		"Angelus\t\t\t\tNew Skin for 1.10, example songs, beta tester" "\r\n"
		"ikkkle\t\t\t\tNew toolbar graphics for 1.10.0" "\r\n"
		"Hamarr Heylen\t\t\tInitial Graphics" "\r\n"
		"David Buist\t\t\tAdditional Graphics" "\r\n"
		"frown\t\t\t\tAdditional Graphics" "\r\n"
		"/\\/\\ark\t\t\t\tAdditional Graphics" "\r\n"
		"Michael Haralabos\t\t\tInstaller and Debugging help" "\r\n\r\n"
		"This release of Psycle also contains VST plugins from:" "\r\n"
		"Digital Fish Phones\t( http://www.digitalfishphones.com/ )" "\r\n"
		"DiscoDSP\t\t( http://www.discodsp.com/ )" "\r\n"
		"SimulAnalog\t( http://www.simulanalog.org/ )" "\r\n"
		"Jeroen Breebaart\t( http://www.jeroenbreebaart.com/ )" "\r\n"
		"George Yohng\t( http://www.yohng.com/ )" "\r\n"
		"Christian Budde\t( http://www.savioursofsoul.de/Christian/ )" "\r\n"
		"DDMF\t\t( http://www.ddmf.eu/ )" "\r\n"
		"Loser\t\t( http://loser.asseca.com/ )" "\r\n"
		"E-phonic\t\t( http://www.e-phonic.com/ )" "\r\n"
		"Argu\t\t( http://www.aodix.com/ )" "\r\n"
		"Oatmeal by Fuzzpilz\t( http://bicycle-for-slugs.org/ )"
	);	
	ui_edit_init(&self->psycledelics, &self->component, ES_READONLY);
	ui_edit_settext(&self->psycledelics, "http://psycle.pastnotecut.org");	
	ui_edit_init(&self->sourceforge, &self->component, ES_READONLY);
	ui_edit_settext(&self->sourceforge, "http://psycle.sourceforge.net");	
	ui_label_init(&self->steincopyright, &self->component);
	ui_label_settext(&self->steincopyright, "VST Virtual Studio Technology v2.4 (c)1998-2006 Steinberg");	
	
	ui_component_resize(&self->contrib.component, 0, 150);
	ui_component_resize(&self->psycledelics.component, 0, 20);
	ui_component_resize(&self->sourceforge.component, 0, 20);
	ui_component_resize(&self->steincopyright.component, 0, 20);
	{
		List* p;
		List* q;
		for (p = q = ui_component_children(&self->component, 0); p != 0; p = p->next)
		{
			ui_component_setalign((ui_component*)p->entry, UI_ALIGN_TOP);			
			ui_component_setmargin((ui_component*)p->entry, &margin);
		}
		list_free(q);
	}
}

void InitVersion(Version* self, ui_component* parent)
{
	ui_component_init(&self->component, parent);	
	ui_label_init(&self->versioninfo, &self->component);
	ui_label_setstyle(&self->versioninfo, WS_CHILD | WS_VISIBLE | SS_CENTER);
	ui_label_settext(&self->versioninfo, PSYCLE__BUILD__IDENTIFIER("\r\n"));
	ui_component_resize(&self->versioninfo.component, 500, 300);	
	ui_component_setbackgroundcolor(&self->versioninfo.component, 0x00232323);
}

void InitAbout(About* self, ui_component* parent)
{			
	ui_component_init(&self->component, parent);	
	signal_connect(&self->component.signal_size, self, OnSize);	
	InitButtons(self);
	ui_notebook_init(&self->notebook, &self->component);
	ui_image_init(&self->image, &self->notebook.component);	
	self->image.component.preventdefault = 0;
	ui_bitmap_loadresource(&self->image.bitmap, IDB_ABOUT);	
	InitContrib(&self->contrib, &self->notebook.component);
	InitVersion(&self->version, &self->notebook.component);		
	ui_notebook_setpageindex(&self->notebook, 0);
	signal_connect(&self->component.signal_mousedoubleclick, self,
		OnMouseDoubleClick);
}

void InitButtons(About* self)
{
	ui_button_init(&self->contribbutton, &self->component);
	ui_button_settext(&self->contribbutton, "Contributors / Credits");
	signal_connect(&self->contribbutton.signal_clicked, self, OnContributors);
	ui_button_init(&self->versionbutton, &self->component);
	ui_button_settext(&self->versionbutton, PSYCLE__VERSION);
	signal_connect(&self->versionbutton.signal_clicked, self, OnVersion);
	ui_button_init(&self->okbutton, &self->component);
	ui_button_settext(&self->okbutton, "OK");	
}

void Align(About* self, ui_component* sender)
{
	ui_size size;
	int centerx;
	int centery;

	size = ui_component_size(&self->component);
	centerx = (size.width - 500) / 2;	
	centery = (size.height - 385) / 2;
	ui_component_setposition(&self->notebook.component, centerx, centery + 5, 520, 360);	
	if (centery + 365 > size.height - 25) {
		centery = size.height - 365 - 25;
	}
	ui_component_setposition(&self->contribbutton.component, centerx, centery + 365, 120, 20);
	ui_component_setposition(&self->versionbutton.component, centerx + 145, centery + 365, 300, 20);
	ui_component_setposition(&self->okbutton.component,
		centerx + 445,
		centery + 365,
		60,
		20);
}

void OnSize(About* self, ui_component* sender, ui_size* size)
{	
	Align(self, &self->component);
}

void OnContributors(About* self, ui_component* sender) 
{	
	ui_notebook_setpageindex(&self->notebook, 
		ui_notebook_pageindex(&self->notebook) != 1 ? 1 : 0);
	ui_component_invalidate(&self->component);
}

void OnVersion(About* self, ui_component* sender) 
{	
	ui_notebook_setpageindex(&self->notebook, 
		ui_notebook_pageindex(&self->notebook) != 2 ? 2 : 0);
	ui_component_invalidate(&self->component);
}

void OnMouseDoubleClick(About* self, ui_component* sender, int x, int y, int button)
{
	signal_emit(&self->okbutton.signal_clicked, self, 0);
}

