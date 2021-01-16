// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instrumentview.h"
// ui
#include <uiopendialog.h>
#include <uisavedialog.h>
// audio
#include <exclusivelock.h>
#include <instruments.h>
#include <songio.h>
#include <xmsongloader.h>
#include <machinefactory.h>
#include <virtualgenerator.h>
// dsp
#include <valuemapper.h>
// std
#include <math.h>

#include "../../detail/portable.h"

static void virtualgeneratorbox_updategenerator(VirtualGeneratorsBox*);
static void virtualgeneratorbox_ongeneratorschanged(VirtualGeneratorsBox*,
	psy_ui_Component* sender, int slot);
static void virtualgeneratorbox_onsamplerschanged(VirtualGeneratorsBox*,
	psy_ui_Component* sender, int slot);
static void virtualgeneratorbox_onactivechanged(VirtualGeneratorsBox*,
	psy_ui_Component* sender);
static void virtualgeneratorbox_update(VirtualGeneratorsBox*);

void virtualgeneratorbox_init(VirtualGeneratorsBox* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent);
	self->workspace = workspace;
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_checkbox_init(&self->active, &self->component);
	psy_ui_checkbox_settext(&self->active, "Virtual generator");
	psy_signal_connect(&self->active.signal_clicked, self,
		virtualgeneratorbox_onactivechanged);
	psy_ui_combobox_init(&self->generators, &self->component);
	psy_signal_connect(&self->generators.signal_selchanged, self,
		virtualgeneratorbox_ongeneratorschanged);
	psy_ui_label_init_text(&self->on, &self->component, "on");
	psy_ui_combobox_setcharnumber(&self->generators, 10);
	psy_ui_combobox_init(&self->samplers, &self->component);
	psy_signal_connect(&self->samplers.signal_selchanged, self,
		virtualgeneratorbox_onsamplerschanged);
	psy_ui_combobox_setcharnumber(&self->samplers, 20);
	virtualgeneratorbox_updategenerators(self);
}

void virtualgeneratorbox_updatesamplers(VirtualGeneratorsBox* self)
{
	if (self->workspace->song) {
		uintptr_t i;
		uintptr_t c;
		uintptr_t maxkey;

		maxkey = psy_table_maxkey(&self->workspace->song->machines.slots);

		for (c = 0, i = 0; i < maxkey; ++i) {
			psy_audio_Machine* machine;

			machine = (psy_audio_Machine*)psy_audio_machines_at(
				&self->workspace->song->machines, i);
			if (machine && machine_supports(machine,
				MACH_SUPPORTS_INSTRUMENTS)) {
				char text[512];

				psy_snprintf(text, 512, "%X: %s", (int)i,
					psy_audio_machine_editname(machine));
				psy_ui_combobox_addtext(&self->samplers, text);
				psy_ui_combobox_setitemdata(&self->samplers,  c++, i);
			}
		}
	}
}

void virtualgeneratorbox_updategenerators(VirtualGeneratorsBox* self)
{
	int slot;
	int start;
	int end;

	start = 0x81;
	end = 0xFE;
	for (slot = start; slot <= end; ++slot) {
		char text[512];

		psy_snprintf(text, 512, "%X", (int)slot);
		psy_ui_combobox_addtext(&self->generators, text);
		psy_ui_combobox_setitemdata(&self->generators, slot - start, slot);
	}
}

void virtualgeneratorbox_ongeneratorschanged(VirtualGeneratorsBox* self, psy_ui_Component* sender, int slot)
{
	virtualgeneratorbox_updategenerator(self);
}

void virtualgeneratorbox_onsamplerschanged(VirtualGeneratorsBox* self, psy_ui_Component* sender, int slot)
{
	virtualgeneratorbox_updategenerator(self);
}

void virtualgeneratorbox_onactivechanged(VirtualGeneratorsBox* self, psy_ui_Component* sender)
{
	virtualgeneratorbox_updategenerator(self);
}

void virtualgeneratorbox_updategenerator(VirtualGeneratorsBox* self)
{
	if (psy_ui_checkbox_checked(&self->active)) {
		if (psy_ui_combobox_cursel(&self->generators) != -1 &&
				psy_ui_combobox_cursel(&self->samplers) != -1) {			
			psy_audio_song_insertvirtualgenerator(self->workspace->song,
				psy_ui_combobox_cursel(&self->generators) + 0x81,
				(uintptr_t)psy_ui_combobox_itemdata(&self->samplers,
					psy_ui_combobox_cursel(&self->samplers)),
				psy_audio_instruments_selected(
					&self->workspace->song->instruments).subslot);
		}
	} else if (psy_ui_combobox_cursel(&self->generators) != -1) {
		psy_audio_Machine* generator;

		generator = psy_audio_machines_at(&self->workspace->song->machines,
			psy_ui_combobox_cursel(&self->generators) + 0x81);
		if (generator) {
			psy_audio_machines_remove(&self->workspace->song->machines,
				psy_ui_combobox_cursel(&self->generators) + 0x81);
		}		
	}	
}

void virtualgeneratorbox_update(VirtualGeneratorsBox* self)
{
	psy_TableIterator it;

	psy_ui_checkbox_disablecheck(&self->active);
	for (it = psy_audio_machines_begin(&self->workspace->song->machines);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		psy_audio_Machine* machine;

		machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
		if (psy_audio_machine_type(machine) == MACH_VIRTUALGENERATOR) {
			psy_audio_MachineParam* param;
			
			param = psy_audio_machine_parameter(machine, 0);
			if (param) {
				intptr_t index;

				index = psy_audio_machine_parameter_scaledvalue(machine, param);
				if (index == psy_audio_instruments_selected(&self->workspace->song->instruments).subslot) {
					param = psy_audio_machine_parameter(machine, 1);
					if (param) {
						index = psy_audio_machine_parameter_scaledvalue(machine, param);
						if (index == psy_audio_instruments_selected(
								&self->workspace->song->instruments).subslot) {
							psy_ui_combobox_setcursel(&self->samplers, index);							
						}
					}
					psy_ui_combobox_setcursel(&self->generators,
						psy_audio_machine_slot(machine) - 0x81);
					psy_ui_checkbox_check(&self->active);
					break;
				}
			}
		}		
	}	
}

// InstrumentPredefsBar
static void instrumentpredefsbar_onpredefs(InstrumentPredefsBar* self, psy_ui_Button* sender);

void instrumentpredefsbar_init(InstrumentPredefsBar* self, psy_ui_Component* parent,
	psy_audio_Instrument* instrument, InstrumentView* view, Workspace* workspace)
{
	psy_ui_Button* buttons[] = { &self->predef_1, &self->predef_2,
		&self->predef_3, &self->predef_4, &self->predef_5, &self->predef_6,
		NULL};	
	int c;

	psy_ui_component_init(&self->component, parent);
	self->instrument = instrument;
	self->workspace = workspace;
	self->view = view;
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HORIZONTALEXPAND);
	psy_ui_label_init(&self->predefs, &self->component);	
	psy_ui_label_preventtranslation(&self->predefs);
	psy_ui_label_settext(&self->predefs, "Predef.");	
	for (c = 0; buttons[c] != NULL; ++c) {
		char text[2];

		psy_snprintf(text, 2, "%i", c + 1);
		psy_ui_button_init_text(buttons[c], &self->component, text);
		psy_ui_button_allowrightclick(buttons[c]);		
		psy_signal_connect(&buttons[c]->signal_clicked, self,
			instrumentpredefsbar_onpredefs);		
	}	
}

void instrumentpredefsbar_onpredefs(InstrumentPredefsBar* self, psy_ui_Button* sender)
{
	psy_ui_Button* buttons[] = { &self->predef_1, &self->predef_2,
		&self->predef_3, &self->predef_4, &self->predef_5, &self->predef_6,
		NULL };
	int index;

	if (self->instrument) {
		index = 0;
		while (buttons[index] != NULL && sender != buttons[index]) ++index;
		if (!buttons[index]) {
			return;
		}
		if (psy_ui_button_clickstate(sender) == 1) {
			predefsconfig_predef(&self->workspace->config.predefs,
				index, &self->instrument->volumeenvelope);		
		} else {
			predefsconfig_storepredef(&self->workspace->config.predefs,
				index, &self->instrument->volumeenvelope);		
		}
		psy_ui_component_invalidate(&self->view->component);
	}
}

// InstrumentHeaderView
// prototypes
static void instrumentheaderview_onprevinstrument(InstrumentHeaderView*, psy_ui_Component* sender);
static void instrumentheaderview_onnextinstrument(InstrumentHeaderView*, psy_ui_Component* sender);
static void instrumentheaderview_oneditinstrumentname(InstrumentHeaderView*, psy_ui_Edit* sender);
// implementation
void instrumentheaderview_init(InstrumentHeaderView* self, psy_ui_Component* parent,
	psy_audio_Instruments* instruments, InstrumentView* view,
	Workspace* workspace)
{	
	psy_ui_Margin margin;
	psy_ui_Margin tab;	

	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(0.5), psy_ui_value_makeeh(0.5),
		psy_ui_value_makepx(0));
	self->view = view;
	self->instrument = 0;
	self->instruments = instruments;
	psy_ui_component_init(&self->component, parent);
	psy_ui_label_init_text(&self->namelabel, &self->component,
		"instrumentview.instrument-name");
	psy_ui_edit_init(&self->nameedit, &self->component);
	psy_ui_edit_setcharnumber(&self->nameedit, 20);	
	psy_signal_connect(&self->nameedit.signal_change, self,
		instrumentheaderview_oneditinstrumentname);
	psy_ui_button_init_connect(&self->prevbutton, &self->component,
		self, instrumentheaderview_onprevinstrument);
	psy_ui_button_seticon(&self->prevbutton, psy_ui_ICON_LESS);
	psy_ui_button_init_connect(&self->nextbutton, &self->component,
		self, instrumentheaderview_onnextinstrument);
	psy_ui_button_seticon(&self->nextbutton, psy_ui_ICON_MORE);
	virtualgeneratorbox_init(&self->virtualgenerators, &self->component,
		workspace);
	instrumentpredefsbar_init(&self->predefs, &self->component, NULL,
		view, workspace);
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
			&margin));		
	tab = margin;
	tab.right = psy_ui_value_makeew(4.0);
	psy_ui_component_setmargin(&self->virtualgenerators.component, &tab);
}

void instrumentheaderview_setinstrument(InstrumentHeaderView* self,
	psy_audio_Instrument* instrument)
{
	self->instrument = instrument;
	self->predefs.instrument = instrument;
	psy_signal_prevent(&self->nameedit.signal_change, self,
		instrumentheaderview_oneditinstrumentname);
	psy_ui_edit_settext(&self->nameedit,
		(instrument)
		? instrument->name
		: "");
	psy_signal_enable(&self->nameedit.signal_change, self,
		instrumentheaderview_oneditinstrumentname);
}

void instrumentheaderview_oneditinstrumentname(InstrumentHeaderView* self,
	psy_ui_Edit* sender)
{
	if (self->instrument) {
		char text[40];
		psy_audio_InstrumentIndex index;
		
		index = instrumentsbox_selected(&self->view->instrumentsbox);
		psy_audio_instrument_setname(self->instrument, psy_ui_edit_text(sender));
		psy_snprintf(text, 20, "%02X:%s", 
			(int)index.subslot, psy_audio_instrument_name(self->instrument));
		psy_ui_listbox_settext(&self->view->instrumentsbox.instrumentlist, text,
			index.subslot);
	}
}

void instrumentheaderview_onprevinstrument(InstrumentHeaderView* self, psy_ui_Component* sender)
{
	psy_audio_InstrumentIndex index;

	index = psy_audio_instruments_selected(self->instruments);
	if (!psy_audio_instrumentindex_invalid(&index) &&
			psy_audio_instrumentindex_subslot(&index) > 0) {
		index.subslot = psy_audio_instrumentindex_subslot(&index) - 1;
		psy_audio_instruments_select(self->instruments, index);
	}
}

void instrumentheaderview_onnextinstrument(InstrumentHeaderView* self, psy_ui_Component* sender)
{
	psy_audio_InstrumentIndex index;

	index = psy_audio_instruments_selected(self->instruments);
	if (!psy_audio_instrumentindex_invalid(&index)) {
		index.subslot = psy_audio_instrumentindex_subslot(&index) + 1;
		psy_audio_instruments_select(self->instruments, index);
	}
}

// InstrumentViewButtons
// implementation
void instrumentviewbuttons_init(InstrumentViewButtons* self,
	psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);	
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);
	psy_ui_button_init_text(&self->create, &self->component, "file.new");
	psy_ui_button_init_text(&self->load, &self->component, "file.load");
	psy_ui_button_init_text(&self->save, &self->component, "file.save");
	psy_ui_button_init_text(&self->duplicate, &self->component, "edit.duplicate");
	psy_ui_button_init_text(&self->del, &self->component, "edit.delete");
}

// InstrumentEmpty
void instrumentempty_init(InstrumentEmpty* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_label_init_text(&self->empty, &self->component,
		"No Instrument");
	psy_ui_component_setalign(psy_ui_label_base(&self->empty),
		psy_ui_ALIGN_CENTER);
}

// InstrumentViewBar
static void instrumentsviewbar_onsongchanged(InstrumentsViewBar*, Workspace*,
	int flag, psy_audio_SongFile*);

void instrumentsviewbar_init(InstrumentsViewBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(instrumentsviewbar_base(self), parent);
	self->workspace = workspace;
	psy_ui_component_setdefaultalign(instrumentsviewbar_base(self),
		psy_ui_ALIGN_LEFT, psy_ui_margin_make(
			psy_ui_value_makepx(0), psy_ui_value_makeew(4),
			psy_ui_value_makepx(0), psy_ui_value_makepx(0)));		
	psy_ui_label_init(&self->status, instrumentsviewbar_base(self));
	psy_ui_label_preventtranslation(&self->status);
	psy_ui_label_setcharnumber(&self->status, 44);
	instrumentsviewbar_settext(self, "instrumentviewbar");
	psy_ui_component_doublebuffer(psy_ui_label_base(&self->status));
	psy_signal_connect(&workspace->signal_songchanged, self,
		instrumentsviewbar_onsongchanged);
}

void instrumentsviewbar_settext(InstrumentsViewBar* self, const char* text)
{
	psy_ui_label_settext(&self->status, text);
}

void instrumentsviewbar_onsongchanged(InstrumentsViewBar* self, Workspace* workspace,
	int flag, psy_audio_SongFile* songfile)
{	
}

// InstrumentView
// prototypes
// instruments
static void instrumentview_oncreateinstrument(InstrumentView*,
	psy_ui_Component* sender);
static void instrumentview_onloadinstrument(InstrumentView*,
	psy_ui_Component* sender);
static void instrumentview_ondeleteinstrument(InstrumentView*,
	psy_ui_Component* sender);
static void instrumentview_oninstrumentinsert(InstrumentView*,
	psy_audio_Instruments* sender,
	const psy_audio_InstrumentIndex* index);
static void instrumentview_oninstrumentremoved(InstrumentView*,
	psy_audio_Instruments* sender,
	const psy_audio_InstrumentIndex* index);
static void instrumentview_oninstrumentslotchanged(InstrumentView*,
	psy_audio_Instrument* sender, const psy_audio_InstrumentIndex*);
static void instrumentview_setinstrument(InstrumentView*,
	psy_audio_InstrumentIndex index);
static void instrumentview_onmachinesinsert(InstrumentView*,
	psy_audio_Machines* sender, int slot);
static void instrumentview_onmachinesremoved(InstrumentView*,
	psy_audio_Machines* sender, int slot);
static void instrumentview_onsongchanged(InstrumentView*,
	Workspace* sender, int flag, psy_audio_SongFile*);
static void instrumentview_onstatuschanged(InstrumentView*,
	psy_ui_Component* sender, char* text);
// implementation
void instrumentview_init(InstrumentView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{
	psy_ui_Margin margin;
	psy_ui_Margin leftmargin;

	psy_ui_component_init(&self->component, parent);	
	psy_ui_component_init(&self->viewtabbar, tabbarparent);
	self->statusbar = NULL;
	self->player = &workspace->player;
	self->workspace = workspace;
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(2), psy_ui_value_makepx(0),
		psy_ui_value_makeew(0));
	psy_ui_margin_init_all(&leftmargin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0),
		psy_ui_value_makeew(3));
	// header
	instrumentheaderview_init(&self->header, &self->component,
		&workspace->song->instruments, self, workspace);
	psy_ui_component_setmargin(&self->header.component,
		&leftmargin);
	psy_ui_component_setalign(&self->header.component, psy_ui_ALIGN_TOP);
	// left
	psy_ui_component_init(&self->left, &self->component);
	psy_ui_component_setalign(&self->left, psy_ui_ALIGN_LEFT);
	psy_ui_component_setmargin(&self->left, &leftmargin);
	instrumentsbox_init(&self->instrumentsbox, &self->left,
		&workspace->song->instruments, workspace);
	psy_ui_component_setalign(&self->instrumentsbox.component,
		psy_ui_ALIGN_CLIENT);	
	instrumentviewbuttons_init(&self->buttons,
		&self->instrumentsbox.groupheader, workspace);
	psy_ui_component_setalign(&self->buttons.component, psy_ui_ALIGN_TOP);	
	// notebook
	psy_ui_notebook_init(&self->clientnotebook, &self->component);
	psy_ui_component_setalign(psy_ui_notebook_base(&self->clientnotebook),
		psy_ui_ALIGN_CLIENT);
	{
		psy_ui_Margin margin;

		psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
			psy_ui_value_makepx(0), psy_ui_value_makepx(0),
			psy_ui_value_makeew(2.0));
		psy_ui_component_setmargin(psy_ui_notebook_base(&self->clientnotebook),
			&margin);
	}
	// empty
	instrumentempty_init(&self->empty,
		psy_ui_notebook_base(&self->clientnotebook));
	// client
	psy_ui_component_init(&self->client,
		psy_ui_notebook_base(&self->clientnotebook));
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0),
		psy_ui_value_makeew(2.0));
	psy_ui_component_setmargin(&self->client, &margin);
	psy_ui_component_setalign(&self->client, psy_ui_ALIGN_CLIENT);
	tabbar_init(&self->tabbar, &self->client);
	psy_ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_TOP);	
	tabbar_append_tabs(&self->tabbar, "instrumentview.general",
		"instrumentview.volume", "instrumentview.pan", "instrumentview.filter",
		"instrumentview.pitch", NULL);
	psy_ui_notebook_init(&self->notebook, &self->client);
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	instrumentgeneralview_init(&self->general,
		psy_ui_notebook_base(&self->notebook), &workspace->song->instruments,
		workspace);
	instrumentvolumeview_init(&self->volume,
		psy_ui_notebook_base(&self->notebook), &workspace->song->instruments,
		workspace);
	instrumentpanview_init(&self->pan, psy_ui_notebook_base(&self->notebook),
		&workspace->song->instruments, workspace);
	instrumentfilterview_init(&self->filter,
		psy_ui_notebook_base(&self->notebook),
		&workspace->song->instruments, workspace);	
	instrumentpitchview_init(&self->pitch,
		psy_ui_notebook_base(&self->notebook),
		&workspace->song->instruments, workspace);
	// connect signals
	psy_ui_notebook_connectcontroller(&self->notebook,
		&self->tabbar.signal_change);
	psy_signal_connect(&workspace_song(self->workspace)->instruments.signal_insert, self,
		instrumentview_oninstrumentinsert);
	psy_signal_connect(&workspace_song(self->workspace)->instruments.signal_removed, self,
		instrumentview_oninstrumentremoved);
	psy_signal_connect(&workspace_song(self->workspace)->instruments.signal_slotchange, self,
		instrumentview_oninstrumentslotchanged);
	psy_signal_connect(&workspace_song(self->workspace)->machines.signal_insert, self,
		instrumentview_onmachinesinsert);
	psy_signal_connect(&workspace_song(self->workspace)->machines.signal_removed, self,
		instrumentview_onmachinesremoved);
	psy_ui_notebook_select(&self->notebook, 0);
	psy_signal_connect(&workspace->signal_songchanged, self, instrumentview_onsongchanged);
	samplesbox_setsamples(&self->general.notemapview.samplesbox, &workspace->song->samples);	
	psy_signal_connect(&self->buttons.create.signal_clicked, self,
		instrumentview_oncreateinstrument);
	psy_signal_connect(&self->buttons.load.signal_clicked, self,
		instrumentview_onloadinstrument);
	psy_signal_connect(&self->buttons.del.signal_clicked, self,
		instrumentview_ondeleteinstrument);	
	psy_signal_connect(&self->volume.signal_status, self,
		instrumentview_onstatuschanged);
	psy_signal_connect(&self->pan.signal_status, self,
		instrumentview_onstatuschanged);
	psy_signal_connect(&self->pitch.signal_status, self,
		instrumentview_onstatuschanged);
	psy_signal_connect(&self->filter.signal_status, self,
		instrumentview_onstatuschanged);
	psy_ui_notebook_select(&self->clientnotebook, 0);
}

void instrumentview_oninstrumentinsert(InstrumentView* self, psy_audio_Instruments* sender,
	const psy_audio_InstrumentIndex* index)
{
	if (index) {
		instrumentview_setinstrument(self, *index);
	}
}

void instrumentview_oninstrumentremoved(InstrumentView* self, psy_audio_Instruments* sender,
	const psy_audio_InstrumentIndex* index)
{
	if (index) {
		instrumentview_setinstrument(self, *index);
	}
}

void instrumentview_oninstrumentslotchanged(InstrumentView* self, psy_audio_Instrument* sender,
	const psy_audio_InstrumentIndex* index)
{
	if (index) {
		instrumentview_setinstrument(self, *index);
	}
}

void instrumentview_onmachinesinsert(InstrumentView* self, psy_audio_Machines* sender,
	int slot)
{
	virtualgeneratorbox_updatesamplers(&self->header.virtualgenerators);
}

void instrumentview_onmachinesremoved(InstrumentView* self, psy_audio_Machines* sender,
	int slot)
{
	virtualgeneratorbox_updatesamplers(&self->header.virtualgenerators);
	virtualgeneratorbox_update(&self->header.virtualgenerators);
}

void instrumentview_setinstrument(InstrumentView* self, psy_audio_InstrumentIndex index)
{
	psy_audio_Instrument* instrument;

	instrument = psy_audio_instruments_at(&workspace_song(self->workspace)->instruments, index);
	instrumentheaderview_setinstrument(&self->header, instrument);
	instrumentgeneralview_setinstrument(&self->general, instrument);
	instrumentvolumeview_setinstrument(&self->volume, instrument);
	instrumentpanview_setinstrument(&self->pan, instrument);
	instrumentfilterview_setinstrument(&self->filter, instrument);	
	instrumentpitchview_setinstrument(&self->pitch, instrument);
	virtualgeneratorbox_update(&self->header.virtualgenerators);
	if (instrument) {
		psy_ui_notebook_select(&self->clientnotebook, 1);
	} else {
		psy_ui_notebook_select(&self->clientnotebook, 0);
	}
}

void instrumentview_onsongchanged(InstrumentView* self, Workspace* workspace, int flag, psy_audio_SongFile* songfile)
{
	if (workspace->song) {
		self->header.instruments = &workspace->song->instruments;
		self->general.instruments = &workspace->song->instruments;
		self->volume.instruments = &workspace->song->instruments;
		self->pan.instruments = &workspace->song->instruments;
		self->filter.instruments = &workspace->song->instruments;		
		psy_signal_connect(&workspace->song->instruments.signal_slotchange, self,
			instrumentview_oninstrumentslotchanged);
		psy_signal_connect(&workspace->song->instruments.signal_insert, self,
			instrumentview_oninstrumentinsert);
		psy_signal_connect(&workspace->song->instruments.signal_removed, self,
			instrumentview_oninstrumentremoved);
		psy_signal_connect(&workspace_song(self->workspace)->machines.signal_insert, self,
			instrumentview_onmachinesinsert);
		psy_signal_connect(&workspace_song(self->workspace)->machines.signal_removed, self,
			instrumentview_onmachinesremoved);
		instrumentsbox_setinstruments(&self->instrumentsbox,
			&workspace->song->instruments);
		samplesbox_setsamples(&self->general.notemapview.samplesbox, &workspace->song->samples);		
	} else {
		instrumentsbox_setinstruments(&self->instrumentsbox, 0);
		samplesbox_setsamples(&self->general.notemapview.samplesbox, NULL);
	}
	virtualgeneratorbox_updatesamplers(&self->header.virtualgenerators);
	instrumentview_setinstrument(self, psy_audio_instrumentindex_make(0, 0));
}

// instruments
void instrumentview_oncreateinstrument(InstrumentView* self, psy_ui_Component* sender)
{
	if (workspace_song(self->workspace)) {
		psy_audio_Instrument* instrument;
		psy_audio_InstrumentIndex selected;

		selected = instrumentsbox_selected(&self->instrumentsbox);
		instrument = psy_audio_instrument_allocinit();
		psy_audio_instrument_setname(instrument, "Untitled");
		psy_audio_instrument_setindex(instrument, selected.subslot);
		psy_audio_exclusivelock_enter();
		psy_audio_instruments_insert(&workspace_song(self->workspace)->instruments, instrument,
			selected);
		psy_audio_instruments_select(&workspace_song(self->workspace)->instruments, selected);
		psy_audio_exclusivelock_leave();
	}
}

void instrumentview_ondeleteinstrument(InstrumentView* self, psy_ui_Component* sender)
{
	psy_audio_exclusivelock_enter();
	psy_audio_instruments_remove(&workspace_song(self->workspace)->instruments,
		instrumentsbox_selected(&self->instrumentsbox));
	psy_audio_exclusivelock_leave();
}

void instrumentview_onloadinstrument(InstrumentView* self, psy_ui_Component* sender)
{
	if (workspace_song(self->workspace)) {
		psy_ui_OpenDialog dialog;
		static char filter[] =
			"Instrument (*.xi)|*.xi";

		psy_ui_opendialog_init_all(&dialog, 0, "Load Instrument", filter, "XI",
			dirconfig_samples(&self->workspace->config.directories));
		if (psy_ui_opendialog_execute(&dialog)) {
			// psy_audio_SampleIndex index;
			psy_audio_SongFile songfile;
			PsyFile file;

			psy_audio_songfile_init(&songfile);
			songfile.song = workspace_song(self->workspace);
			songfile.file = &file;
			if (psyfile_open(&file, psy_path_full(psy_ui_opendialog_path(&dialog)))) {
				XMSongLoader xmsongloader;
				int status;

				xmsongloader_init(&xmsongloader, &songfile);
				if (status = xmsongloader_loadxi(&xmsongloader,
						psy_audio_instruments_selected(
							&workspace_song(self->workspace)->instruments))) {
					psy_audio_songfile_errfile(&songfile);
				}
				xmsongloader_dispose(&xmsongloader);
			}
			psyfile_close(&file);
			psy_audio_songfile_dispose(&songfile);
			instrumentsbox_rebuild(&self->instrumentsbox);
		}
		psy_ui_opendialog_dispose(&dialog);
	}
}

void instrumentview_onstatuschanged(InstrumentView* self,
	psy_ui_Component* sender, char* text)
{
	if (self->statusbar) {
		instrumentsviewbar_settext(self->statusbar, text);
	}
}
