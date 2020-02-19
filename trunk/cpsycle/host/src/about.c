// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "about.h"
#include "../../detail/psydef.h"

#include <stdio.h>
#include <string.h>
#include "resources/resource.h"

static void about_initbuttons(About* self);
static void about_oncontributors(About*, psy_ui_Component* sender);
static void about_onversion(About*, psy_ui_Component* sender);
static void about_onmousedoubleclick(About*, psy_ui_MouseEvent*);
static void about_onalign(About*);

static psy_ui_ComponentVtable about_vtable;
static int about_vtable_initialized = 0;

static void about_vtable_init(About* self)
{
	if (!about_vtable_initialized) {
		about_vtable = *(self->component.vtable);		
		about_vtable.onalign = (psy_ui_fp_onalign) about_onalign;
		about_vtable.onmousedoubleclick = (psy_ui_fp_onmousedoubleclick)
			about_onmousedoubleclick;
		about_vtable_initialized = 1;
	}
}

void contrib_init(Contrib* self, psy_ui_Component* parent)
{	
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);	
	psy_ui_edit_multiline_init(&self->contrib, &self->component);
	psy_ui_edit_preventedit(&self->contrib);
	psy_ui_edit_setlinenumber(&self->contrib, 10);
	psy_ui_edit_settext(&self->contrib,						
		"Josep M� Antol�n. [JAZ]/JosepMa\tDeveloper since release 1.5" "\r\n"
		"Johan Boul� [bohan]\t\tDeveloper since release 1.7.3" "\r\n"
		"Stefan Nattkemper\t\t\tDeveloper of the LUA host in release 1.12" "\r\n"
		"James Redfern [alk]\t\tDeveloper and plugin coder" "\r\n"
		"Magnus Jonsson [zealmange]\t\tDeveloper during 1.7.x and 1.9alpha" "\r\n"
		"Jeremy Evers [pooplog]\t\tDeveloper in releases 1.7.x" "\r\n"
		"Daniel Arena\t\t\tDeveloper in release 1.5 & 1.6" "\r\n"
		"Marcin Kowalski [FideLoop]\t\tDeveloper in release 1.6" "\r\n"
		"Mark McCormack\t\t\tMIDI (in) Support in release 1.6" "\r\n"
		"Mats H�jlund\t\t\tMain Developer until release 1.5" "\r\n" // (Internal Recoding) .. doesn't fit in the small box :/
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
	psy_ui_edit_init(&self->psycledelics, &self->component);
	psy_ui_edit_preventedit(&self->psycledelics);
	psy_ui_edit_settext(&self->psycledelics, "http://psycle.pastnotecut.org");	
	psy_ui_edit_init(&self->sourceforge, &self->component);
	psy_ui_edit_preventedit(&self->sourceforge);
	psy_ui_edit_settext(&self->sourceforge, "http://psycle.sourceforge.net");	
	psy_ui_label_init(&self->steincopyright, &self->component);
	psy_ui_label_settext(&self->steincopyright, "VST Virtual Studio Technology v2.4 (c)1998-2006 Steinberg");	
	
	psy_ui_component_resize(&self->contrib.component, 0, 150);
	psy_ui_component_resize(&self->psycledelics.component, 0, 20);
	psy_ui_component_resize(&self->sourceforge.component, 0, 20);
	psy_ui_component_resize(&self->steincopyright.component, 0, 20);
	{
		psy_ui_Margin margin;

		psy_ui_margin_init(&margin, psy_ui_value_makepx(0),
			psy_ui_value_makepx(0), psy_ui_value_makeeh(0.5),
			psy_ui_value_makepx(0));
		psy_list_free(psy_ui_components_setalign(
			psy_ui_component_children(&self->component, 0),
			psy_ui_ALIGN_TOP,
			&margin));				
	}
}

void version_init(Version* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent);	
	psy_ui_label_init(&self->versioninfo, &self->component);
	psy_ui_label_settextalignment(&self->versioninfo, psy_ui_ALIGNMENT_CENTER_HORIZONTAL);
	psy_ui_label_settext(&self->versioninfo, PSYCLE__BUILD__IDENTIFIER("\r\n"));
	psy_ui_component_resize(&self->versioninfo.component, 500, 300);	
	psy_ui_component_setbackgroundcolor(&self->versioninfo.component, 0x00232323);
}

void about_init(About* self, psy_ui_Component* parent)
{			
	psy_ui_component_init(&self->component, parent);	
	about_vtable_init(self);
	self->component.vtable = &about_vtable;
	psy_ui_component_enablealign(&self->component);	
	about_initbuttons(self);
	psy_ui_notebook_init(&self->notebook, &self->component);
	psy_ui_image_init(&self->image, psy_ui_notebook_base(&self->notebook));	
	self->image.component.preventdefault = 0;
	psy_ui_bitmap_loadresource(&self->image.bitmap, IDB_ABOUT);	
	contrib_init(&self->contrib, psy_ui_notebook_base(&self->notebook));
	version_init(&self->version, psy_ui_notebook_base(&self->notebook));		
	psy_ui_notebook_setpageindex(&self->notebook, 0);
}

void about_initbuttons(About* self)
{
	psy_ui_button_init(&self->contribbutton, &self->component);
	psy_ui_button_settext(&self->contribbutton, "Contributors / Credits");
	psy_signal_connect(&self->contribbutton.signal_clicked, self,
		about_oncontributors);
	psy_ui_button_init(&self->versionbutton, &self->component);
	psy_ui_button_settext(&self->versionbutton, PSYCLE__VERSION);
	psy_signal_connect(&self->versionbutton.signal_clicked, self,
		about_onversion);
	psy_ui_button_init(&self->okbutton, &self->component);
	psy_ui_button_settext(&self->okbutton, "OK");	
}

void about_onalign(About* self)
{
	psy_ui_Size size;
	psy_ui_Size bitmapsize;
	psy_ui_TextMetric tm;
	int centerx;
	int centery;
	psy_ui_Size contribbuttonsize;
	psy_ui_Size versionbuttonsize;
	psy_ui_Size okbuttonsize;
	int charmargin = 25;
	int width = 0;	
	int margin;
	int cpx;

	size = psy_ui_component_size(&self->component);	
	tm = psy_ui_component_textmetric(&self->component);
	bitmapsize = psy_ui_bitmap_size(&self->image.bitmap);
	bitmapsize.height += tm.tmHeight * 4;
	centerx = (size.width - bitmapsize.width) / 2;
	centery = (size.height - bitmapsize.height) / 2;	
	contribbuttonsize = psy_ui_component_preferredsize(&self->contribbutton.component, &size);
	versionbuttonsize = psy_ui_component_preferredsize(&self->versionbutton.component, &size);
	okbuttonsize = psy_ui_component_preferredsize(&self->okbutton.component, &size);
	if (centery + bitmapsize.height + okbuttonsize.height > size.height) {
		bitmapsize.height = size.height - okbuttonsize.height * 2;
		centery = (size.height - bitmapsize.height) / 2;
	}
	psy_ui_component_setposition(psy_ui_notebook_base(&self->notebook),
		centerx, centery, bitmapsize.width, bitmapsize.height);	
	do {
		margin = tm.tmAveCharWidth * charmargin;
		width = contribbuttonsize.width + versionbuttonsize.width + okbuttonsize.width + margin * 2;
		--charmargin;
	} while (width > size.width && charmargin > 0);	
	cpx = (size.width - width) / 2;
	psy_ui_component_setposition(&self->contribbutton.component,
		cpx, centery + bitmapsize.height,
		contribbuttonsize.width, contribbuttonsize.height);
	psy_ui_component_setposition(&self->versionbutton.component,
		cpx + contribbuttonsize.width + margin, centery + bitmapsize.height,
		versionbuttonsize.width, versionbuttonsize.height);
	psy_ui_component_setposition(&self->okbutton.component,
		cpx + contribbuttonsize.width + versionbuttonsize.width + margin * 2,
		centery + bitmapsize.height,
		okbuttonsize.width, okbuttonsize.height);
}

void about_oncontributors(About* self, psy_ui_Component* sender) 
{	
	psy_ui_notebook_setpageindex(&self->notebook, 
		psy_ui_notebook_pageindex(&self->notebook) != 1 ? 1 : 0);
	psy_ui_component_invalidate(&self->component);
}

void about_onversion(About* self, psy_ui_Component* sender) 
{	
	psy_ui_notebook_setpageindex(&self->notebook, 
		psy_ui_notebook_pageindex(&self->notebook) != 2 ? 2 : 0);
	psy_ui_component_invalidate(&self->component);
}

void about_onmousedoubleclick(About* self, psy_ui_MouseEvent* ev)
{
	psy_signal_emit(&self->okbutton.signal_clicked, self, 0);
}
