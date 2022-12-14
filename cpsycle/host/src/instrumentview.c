/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "instrumentview.h"
/* host */
#include "styles.h"
/* ui */
#include <uiopendialog.h>
#include <uisavedialog.h>
/* audio */
#include <exclusivelock.h>
#include <instruments.h>
#include <songio.h>
#include <xmsongloader.h>
#include <machinefactory.h>
#include <virtualgenerator.h>
/* dsp */
#include <valuemapper.h>
/* std */
#include <math.h>
/* platform */
#include "../../detail/portable.h"


/* VirtualGeneratorsBox */

/* prototypes */
static void virtualgeneratorbox_updategenerator(VirtualGeneratorsBox*);
static void virtualgeneratorbox_ongeneratorschanged(VirtualGeneratorsBox*,
	psy_ui_Component* sender, int slot);
static void virtualgeneratorbox_onsamplerschanged(VirtualGeneratorsBox*,
	psy_ui_Component* sender, int slot);
static void virtualgeneratorbox_onactivechanged(VirtualGeneratorsBox*,
	psy_ui_Component* sender);

/* implementation */
void virtualgeneratorbox_init(VirtualGeneratorsBox* self,
	psy_ui_Component* parent, Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_checkbox_init(&self->active, &self->component);
	psy_ui_checkbox_set_text(&self->active, "Virtual generator");
	psy_signal_connect(&self->active.signal_clicked, self,
		virtualgeneratorbox_onactivechanged);
	psy_ui_combobox_init(&self->generators, &self->component);
	psy_signal_connect(&self->generators.signal_selchanged, self,
		virtualgeneratorbox_ongeneratorschanged);
	psy_ui_label_init_text(&self->on, &self->component, "on");
	psy_ui_combobox_set_char_number(&self->generators, 10);
	psy_ui_combobox_init(&self->samplers, &self->component);
	psy_signal_connect(&self->samplers.signal_selchanged, self,
		virtualgeneratorbox_onsamplerschanged);
	psy_ui_combobox_set_char_number(&self->samplers, 20);
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
				psy_audio_SUPPORTS_INSTRUMENTS)) {
				char text[512];

				psy_snprintf(text, 512, "%X: %s", (int)i,
					psy_audio_machine_editname(machine));
				psy_ui_combobox_add_text(&self->samplers, text);
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
		psy_ui_combobox_add_text(&self->generators, text);
		psy_ui_combobox_setitemdata(&self->generators, slot - start, slot);
	}
}

void virtualgeneratorbox_ongeneratorschanged(VirtualGeneratorsBox* self,
	psy_ui_Component* sender, int slot)
{
	virtualgeneratorbox_updategenerator(self);
}

void virtualgeneratorbox_onsamplerschanged(VirtualGeneratorsBox* self,
	psy_ui_Component* sender, int slot)
{
	virtualgeneratorbox_updategenerator(self);
}

void virtualgeneratorbox_onactivechanged(VirtualGeneratorsBox* self,
	psy_ui_Component* sender)
{
	virtualgeneratorbox_updategenerator(self);
}

void virtualgeneratorbox_updategenerator(VirtualGeneratorsBox* self)
{
	if (psy_ui_checkbox_checked(&self->active)) {
		if (psy_ui_combobox_cursel(&self->generators) != -1 &&
				psy_ui_combobox_cursel(&self->samplers) != -1) {			
			psy_audio_song_insert_virtual_generator(self->workspace->song,
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
				psy_ui_combobox_cursel(&self->generators) + 0x81, FALSE);
		}		
	}	
}

void virtualgeneratorbox_update(VirtualGeneratorsBox* self)
{
	psy_TableIterator it;	

	psy_ui_combobox_select(&self->generators, -1);
	psy_ui_combobox_select(&self->samplers, -1);
	psy_ui_checkbox_disable_check(&self->active);
	for (it = psy_audio_machines_begin(&self->workspace->song->machines);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		psy_audio_Machine* machine;

		machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
		if (psy_audio_machine_type(machine) == psy_audio_VIRTUALGENERATOR) {
			psy_audio_MachineParam* param;
			
			param = psy_audio_machine_parameter(machine, 0);
			if (param) {
				intptr_t instindex;

				instindex = psy_audio_machine_parameter_scaledvalue(machine,
					param);
				if (instindex == psy_audio_instruments_selected(
						&self->workspace->song->instruments).subslot) {
					param = psy_audio_machine_parameter(machine, 1);
					if (param) {
						uintptr_t macindex;

						macindex = psy_audio_machine_parameter_scaledvalue(
							machine, param);
						psy_ui_combobox_select(&self->samplers, macindex);					
					}
					psy_ui_combobox_select(&self->generators,
						psy_audio_machine_slot(machine) - 0x81);
					psy_ui_checkbox_check(&self->active);
					break;
				}
			} 
		}		
	}	
}

/* InstrumentPredefsBar */

/* prototypes */
static void instrumentpredefsbar_onpredefs(InstrumentPredefsBar* self,
	psy_ui_Button* sender);
	
/* implementation */
void instrumentpredefsbar_init(InstrumentPredefsBar* self,
	psy_ui_Component* parent, psy_audio_Instrument* instrument,
	InstrumentView* view, Workspace* workspace)
{
	psy_ui_Button* buttons[] = { &self->predef_1, &self->predef_2,
		&self->predef_3, &self->predef_4, &self->predef_5, &self->predef_6,
		NULL};	
	int c;

	psy_ui_component_init(&self->component, parent, NULL);
	self->instrument = instrument;
	self->workspace = workspace;
	self->view = view;
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_component_set_align_expand(&self->component,
		psy_ui_HEXPAND);
	psy_ui_label_init(&self->predefs, &self->component);	
	psy_ui_label_prevent_translation(&self->predefs);
	psy_ui_label_set_text(&self->predefs, "Predef.");	
	for (c = 0; buttons[c] != NULL; ++c) {
		char text[2];

		psy_snprintf(text, 2, "%i", c + 1);
		psy_ui_button_init_text(buttons[c], &self->component, text);
		psy_ui_button_allowrightclick(buttons[c]);		
		psy_signal_connect(&buttons[c]->signal_clicked, self,
			instrumentpredefsbar_onpredefs);		
	}	
}

void instrumentpredefsbar_onpredefs(InstrumentPredefsBar* self,
	psy_ui_Button* sender)
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

/* InstrumentHeaderView */

/* prototypes */
static void instrumentheaderview_onprevinstrument(InstrumentHeaderView*,
	psy_ui_Component* sender);
static void instrumentheaderview_onnextinstrument(InstrumentHeaderView*,
	psy_ui_Component* sender);
static void instrumentheaderview_oneditaccept(InstrumentHeaderView*,
	psy_ui_TextArea* sender);
static void instrumentheaderview_oneditreject(InstrumentHeaderView*,
	psy_ui_TextArea* sender);

/* implementation */
void instrumentheaderview_init(InstrumentHeaderView* self,
	psy_ui_Component* parent, psy_audio_Instruments* instruments,
	InstrumentView* view, Workspace* workspace)
{
	self->view = view;
	self->instrument = NULL;
	self->instruments = instruments;
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_style_type(&self->component, STYLE_VIEW_HEADER);
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_label_init_text(&self->namelabel, &self->component,
		"instrumentview.instrument-name");
	psy_ui_textarea_init_single_line(&self->nameedit, &self->component);
	psy_ui_textarea_enable_input_field(&self->nameedit);
	psy_ui_textarea_set_char_number(&self->nameedit, 20);
	psy_signal_connect(&self->nameedit.pane.signal_accept, self,
		instrumentheaderview_oneditaccept);	
	psy_signal_connect(&self->nameedit.pane.signal_reject, self,
		instrumentheaderview_oneditreject);
	psy_ui_button_init_connect(&self->prevbutton, &self->component,
		self, instrumentheaderview_onprevinstrument);
	psy_ui_button_set_icon(&self->prevbutton, psy_ui_ICON_LESS);
	psy_ui_button_init_connect(&self->nextbutton, &self->component,
		self, instrumentheaderview_onnextinstrument);
	psy_ui_button_set_icon(&self->nextbutton, psy_ui_ICON_MORE);
	psy_ui_component_init(&self->more, &self->component, NULL);
	psy_ui_component_set_default_align(&self->more, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	virtualgeneratorbox_init(&self->virtualgenerators, &self->more,
		workspace);
	instrumentpredefsbar_init(&self->predefs, &self->more, NULL,
		view, workspace);		
	/* tab = margin;
	tab.right = psy_ui_value_make_ew(4.0);
	psy_ui_component_set_margin(&self->more, tab); */
	psy_ui_component_hide(&self->more);	
}

void instrumentheaderview_setinstrument(InstrumentHeaderView* self,
	psy_audio_Instrument* instrument)
{
	self->instrument = instrument;
	self->predefs.instrument = instrument;	
	psy_ui_textarea_set_text(&self->nameedit,
		(instrument)
		? instrument->name
		: "");	
	if (instrument) {
		psy_ui_component_show_align(&self->more);
	} else {
		psy_ui_component_hide(&self->more);
	}
}

void instrumentheaderview_oneditaccept(InstrumentHeaderView* self,
	psy_ui_TextArea* sender)
{
	if (self->instrument) {
		char text[40];
		psy_audio_InstrumentIndex index;
		
		index = instrumentsbox_selected(&self->view->instrumentsbox);
		if (psy_strlen(psy_ui_textarea_text(sender)) == 0) {
			psy_ui_textarea_set_text(sender, "Untitled");
		}
		psy_audio_instrument_setname(self->instrument,
			psy_ui_textarea_text(sender));
		psy_snprintf(text, 20, "%02X:%s", 
			(int)index.subslot, psy_audio_instrument_name(self->instrument));
		psy_ui_listbox_set_text(&self->view->instrumentsbox.instrumentlist,
			text, index.subslot);		
	} else if (psy_strlen(psy_ui_textarea_text(sender)) > 0) {
		if (workspace_song(self->view->workspace)) {
			psy_audio_Instrument* instrument;
			psy_audio_InstrumentIndex selected;

			selected = instrumentsbox_selected(&self->view->instrumentsbox);
			instrument = psy_audio_instrument_allocinit();
			psy_audio_instrument_setname(instrument,
				psy_ui_textarea_text(sender));
			psy_audio_instrument_setindex(instrument, selected.subslot);
			psy_audio_exclusivelock_enter();
			psy_audio_instruments_insert(
				&workspace_song(self->view->workspace)->instruments, instrument,
				selected);
			psy_audio_instruments_select(
				&workspace_song(self->view->workspace)->instruments, selected);
			psy_audio_exclusivelock_leave();
		}
	}
	psy_ui_component_set_focus(psy_ui_component_parent(&self->component));
}

void instrumentheaderview_oneditreject(InstrumentHeaderView* self,
	psy_ui_TextArea* sender)
{
	if (self->instrument) {
		psy_ui_textarea_set_text(&self->nameedit,
			(self->instrument)
			? self->instrument->name
			: "");
	}
	psy_ui_component_set_focus(psy_ui_component_parent(&self->component));
}

void instrumentheaderview_onprevinstrument(InstrumentHeaderView* self,
	psy_ui_Component* sender)
{
	psy_audio_InstrumentIndex index;

	index = psy_audio_instruments_selected(self->instruments);
	if (!psy_audio_instrumentindex_invalid(&index) &&
			psy_audio_instrumentindex_subslot(&index) > 0) {
		index.subslot = psy_audio_instrumentindex_subslot(&index) - 1;
		psy_audio_instruments_select(self->instruments, index);
	}
}

void instrumentheaderview_onnextinstrument(InstrumentHeaderView* self,
	psy_ui_Component* sender)
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
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_component_set_style_type(&self->component, STYLE_BUTTONS);	
	psy_ui_button_init_text(&self->create, &self->component, "file.new");
	psy_ui_button_init_text(&self->load, &self->component, "file.load");
	psy_ui_button_init_text(&self->save, &self->component, "file.save");
	psy_ui_button_init_text(&self->duplicate, &self->component,
		"edit.duplicate");
	psy_ui_button_init_text(&self->del, &self->component, "edit.delete");
}

/* InstrumentViewBar */
static void instrumentsviewbar_on_song_changed(InstrumentsViewBar*,
	psy_audio_Player* sender);

void instrumentsviewbar_init(InstrumentsViewBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(instrumentsviewbar_base(self), parent, NULL);
	psy_ui_component_set_id(instrumentsviewbar_base(self),
		VIEW_ID_INSTRUMENTSVIEW);	
	self->workspace = workspace;
	psy_ui_component_set_default_align(instrumentsviewbar_base(self),
		psy_ui_ALIGN_LEFT, psy_ui_margin_make(
			psy_ui_value_make_px(0), psy_ui_value_make_ew(4),
			psy_ui_value_make_px(0), psy_ui_value_make_px(0)));		
	psy_ui_label_init(&self->status, instrumentsviewbar_base(self));
	psy_ui_label_prevent_translation(&self->status);
	psy_ui_label_set_char_number(&self->status, 44);		
	psy_signal_connect(&workspace->player.signal_song_changed, self,
		instrumentsviewbar_on_song_changed);
}

void instrumentsviewbar_settext(InstrumentsViewBar* self, const char* text)
{
	psy_ui_label_set_text(&self->status, text);
}

void instrumentsviewbar_on_song_changed(InstrumentsViewBar* self,
	psy_audio_Player* sender)
{	
}

/* InstrumentView */

/* prototypes */
static void instrumentview_on_destroyed(InstrumentView*);
static void instrumentview_on_create_instrument(InstrumentView*,
	psy_ui_Component* sender);
static void instrumentview_on_load_instrument_button(InstrumentView*,
	psy_ui_Component* sender);
static void instrumentview_on_load_instrument(InstrumentView*,
	psy_Property* sender);
static void instrumentview_load_instrument(InstrumentView*, const char* path);
static void instrumentview_on_save_instrument_button(InstrumentView*,
	psy_ui_Component* sender);
static void instrumentview_on_save_instrument(InstrumentView*,
	psy_Property* sender);
static void instrumentview_save_instrument(InstrumentView*, const char* path,
	psy_audio_Instrument* instrument);
static void instrumentview_on_delete_instrument(InstrumentView*,
	psy_ui_Component* sender);
static void instrumentview_on_instrument_insert(InstrumentView*,
	psy_audio_Instruments* sender,
	const psy_audio_InstrumentIndex* index);
static void instrumentview_on_instrument_removed(InstrumentView*,
	psy_audio_Instruments* sender,
	const psy_audio_InstrumentIndex* index);
static void instrumentview_on_instrument_slot_changed(InstrumentView*,
	psy_audio_Instrument* sender, const psy_audio_InstrumentIndex*);
static void instrumentview_set_instrument(InstrumentView*,
	psy_audio_InstrumentIndex index);
static void instrumentview_on_machines_insert(InstrumentView*,
	psy_audio_Machines* sender, int slot);
static void instrumentview_on_machines_removed(InstrumentView*,
	psy_audio_Machines* sender, int slot);
static void instrumentview_on_song_changed(InstrumentView*,
	psy_audio_Player* sender);
static void instrumentview_on_status_changed(InstrumentView*,
	psy_ui_Component* sender, char* text);
	
/* vtable */
static psy_ui_ComponentVtable instrumentview_vtable;
static bool instrumentview_vtable_initialized = FALSE;

static void instrumentview_vtable_init(InstrumentView* self)
{
	if (!instrumentview_vtable_initialized) {
		instrumentview_vtable = *(self->component.vtable);		
		instrumentview_vtable.on_destroyed =
			(psy_ui_fp_component)
			instrumentview_on_destroyed;		
		instrumentview_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(instrumentview_base(self),
		&instrumentview_vtable);
}

/* implementation */
void instrumentview_init(InstrumentView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{
	psy_ui_Margin margin;	

	psy_ui_component_init(&self->component, parent, NULL);
	instrumentview_vtable_init(self);
	psy_ui_component_set_id(instrumentview_base(self), VIEW_ID_INSTRUMENTSVIEW);
	psy_ui_component_set_title(instrumentview_base(self), "main.instruments");
	psy_ui_component_set_style_type(&self->component, STYLE_CLIENT_VIEW);	
	psy_ui_component_init(&self->viewtabbar, tabbarparent, NULL);
	psy_ui_component_set_id(&self->viewtabbar, VIEW_ID_INSTRUMENTSVIEW);
	self->statusbar = NULL;
	self->player = &workspace->player;
	self->workspace = workspace;
	psy_ui_margin_init_em(&margin, 0.0, 2.0, 0.0, 0.0);			
	/* header */
	instrumentheaderview_init(&self->header, &self->component,
		&workspace->song->instruments, self, workspace);	
	psy_ui_component_set_align(&self->header.component, psy_ui_ALIGN_TOP);
	/* left */
	psy_ui_component_init(&self->left, &self->component, NULL);
	psy_ui_component_set_align(&self->left, psy_ui_ALIGN_LEFT);	
	instrumentsbox_init(&self->instrumentsbox, &self->left,
		&workspace->song->instruments, workspace);
	psy_ui_component_set_align(&self->instrumentsbox.component,
		psy_ui_ALIGN_CLIENT);	
	instrumentviewbuttons_init(&self->buttons,
		&self->instrumentsbox.groupheader, workspace);
	psy_ui_component_set_align(&self->buttons.component, psy_ui_ALIGN_TOP);	
	/* notebook */
	psy_ui_notebook_init(&self->clientnotebook, &self->component);
	psy_ui_component_set_align(psy_ui_notebook_base(&self->clientnotebook),
		psy_ui_ALIGN_CLIENT);
	{
		psy_ui_Margin margin;

		psy_ui_margin_init_em(&margin, 0.0, 0.0, 0.0, 2.0);
		psy_ui_component_set_margin(psy_ui_notebook_base(&self->clientnotebook),
			margin);
	}
	/* empty */
	psy_ui_label_init_text(&self->empty, 
		psy_ui_notebook_base(&self->clientnotebook), "instrumentview.empty");
	psy_ui_label_set_text_alignment(&self->empty,
		(psy_ui_Alignment)(psy_ui_ALIGNMENT_CENTER_HORIZONTAL |
		 psy_ui_ALIGNMENT_CENTER_VERTICAL));	
	/* client */
	psy_ui_component_init(&self->client, psy_ui_notebook_base(
		&self->clientnotebook), NULL);
	psy_ui_margin_init_em(&margin, 0.0, 0.0, 0.0, 2.0);
	psy_ui_component_set_margin(&self->client, margin);
	psy_ui_component_set_align(&self->client, psy_ui_ALIGN_CLIENT);
	psy_ui_tabbar_init(&self->tabbar, &self->client);
	psy_ui_component_set_align(psy_ui_tabbar_base(&self->tabbar),
		psy_ui_ALIGN_TOP);	
	psy_ui_tabbar_append_tabs(&self->tabbar, "instrumentview.general",
		"instrumentview.volume", "instrumentview.pan", "instrumentview.filter",
		"instrumentview.pitch", NULL);
	psy_ui_notebook_init(&self->notebook, &self->client);
	psy_ui_component_set_align(psy_ui_notebook_base(&self->notebook),
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
		&workspace->song->instruments);	
	instrumentpitchview_init(&self->pitch,
		psy_ui_notebook_base(&self->notebook),
		&workspace->song->instruments, workspace);		
	instrumentsviewbar_init(&self->instrumentsviewbar, &self->component,
		self->workspace);
	psy_ui_component_set_align(&self->instrumentsviewbar.component,
		psy_ui_ALIGN_BOTTOM);
	instrumentsview_setstatusbar(self, &self->instrumentsviewbar);
	/* connect signals */
	psy_ui_notebook_connect_controller(&self->notebook,
		&self->tabbar.signal_change);
	psy_signal_connect(
		&workspace_song(self->workspace)->instruments.signal_insert, self,
		instrumentview_on_instrument_insert);
	psy_signal_connect(
		&workspace_song(self->workspace)->instruments.signal_removed, self,
		instrumentview_on_instrument_removed);
	psy_signal_connect(&workspace_song(
		self->workspace)->instruments.signal_slotchange, self,
		instrumentview_on_instrument_slot_changed);
	psy_signal_connect(
		&workspace_song(self->workspace)->machines.signal_insert, self,
		instrumentview_on_machines_insert);
	psy_signal_connect(
		&workspace_song(self->workspace)->machines.signal_removed, self,
		instrumentview_on_machines_removed);
	psy_ui_notebook_select(&self->notebook, 0);
	psy_signal_connect(&workspace->player.signal_song_changed, self,
		instrumentview_on_song_changed);
	samplesbox_setsamples(
		&self->general.notemapview.samplesbox, &workspace->song->samples);	
	psy_signal_connect(&self->buttons.create.signal_clicked, self,
		instrumentview_on_create_instrument);
	psy_signal_connect(&self->buttons.load.signal_clicked, self,
		instrumentview_on_load_instrument_button);
	psy_signal_connect(&self->buttons.save.signal_clicked, self,
		instrumentview_on_save_instrument_button);
	psy_signal_connect(&self->buttons.del.signal_clicked, self,
		instrumentview_on_delete_instrument);	
	psy_signal_connect(&self->volume.signal_status, self,
		instrumentview_on_status_changed);
	psy_signal_connect(&self->pan.signal_status, self,
		instrumentview_on_status_changed);
	psy_signal_connect(&self->pitch.signal_status, self,
		instrumentview_on_status_changed);
	psy_signal_connect(&self->filter.signal_status, self,
		instrumentview_on_status_changed);
	psy_ui_tabbar_select(&self->tabbar, 0);
	instrumentview_set_instrument(self,
		psy_audio_instrumentindex_make_invalid());
	psy_property_init_type(&self->instrument_load, "load",
		PSY_PROPERTY_TYPE_STRING);
	psy_property_connect(&self->instrument_load, self,
		instrumentview_on_load_instrument);
	psy_property_init_type(&self->instrument_save, "save",
		PSY_PROPERTY_TYPE_STRING);
	psy_property_connect(&self->instrument_save, self,
		instrumentview_on_save_instrument);
}

void instrumentview_on_destroyed(InstrumentView* self)
{
	psy_property_dispose(&self->instrument_load);
	psy_property_dispose(&self->instrument_save);
}

void instrumentview_on_instrument_insert(InstrumentView* self,
	psy_audio_Instruments* sender, const psy_audio_InstrumentIndex* index)
{
	if (index) {
		instrumentview_set_instrument(self, *index);
	}
}

void instrumentview_on_instrument_removed(InstrumentView* self,
	psy_audio_Instruments* sender, const psy_audio_InstrumentIndex* index)
{
	if (index) {
		instrumentview_set_instrument(self, *index);
	}
}

void instrumentview_on_instrument_slot_changed(InstrumentView* self,
	psy_audio_Instrument* sender, const psy_audio_InstrumentIndex* index)
{
	if (index) {
		instrumentview_set_instrument(self, *index);
	}
}

void instrumentview_on_machines_insert(InstrumentView* self,
	psy_audio_Machines* sender, int slot)
{
	virtualgeneratorbox_updatesamplers(&self->header.virtualgenerators);
}

void instrumentview_on_machines_removed(InstrumentView* self,
	psy_audio_Machines* sender, int slot)
{
	virtualgeneratorbox_updatesamplers(&self->header.virtualgenerators);
	virtualgeneratorbox_update(&self->header.virtualgenerators);
}

void instrumentview_set_instrument(InstrumentView* self,
	psy_audio_InstrumentIndex index)
{
	psy_audio_Instrument* instrument;

	instrument = psy_audio_instruments_at(
		&workspace_song(self->workspace)->instruments, index);
	instrumentheaderview_setinstrument(&self->header, instrument);
	instrumentgeneralview_setinstrument(&self->general, instrument);
	instrumentvolumeview_setinstrument(&self->volume, instrument);
	instrumentpanview_setinstrument(&self->pan, instrument);
	instrumentfilterview_set_instrument(&self->filter, instrument);	
	instrumentpitchview_setinstrument(&self->pitch, instrument);
	virtualgeneratorbox_update(&self->header.virtualgenerators);
	if (instrument) {
		psy_ui_notebook_select(&self->clientnotebook, 1);
	} else {		
		psy_ui_notebook_select(&self->clientnotebook, 0);
	}
}

void instrumentview_on_song_changed(InstrumentView* self,
	psy_audio_Player* sender)
{
	psy_audio_Song* song;
	
	song = psy_audio_player_song(sender);
	if (song) {
		self->header.instruments = &song->instruments;
		self->general.instruments = &song->instruments;
		self->volume.instruments = &song->instruments;
		self->pan.instruments = &song->instruments;
		self->filter.instruments = &song->instruments;
		psy_signal_connect(&song->instruments.signal_slotchange, self,
			instrumentview_on_instrument_slot_changed);
		psy_signal_connect(&song->instruments.signal_insert, self,
			instrumentview_on_instrument_insert);
		psy_signal_connect(&song->instruments.signal_removed, self,
			instrumentview_on_instrument_removed);
		psy_signal_connect(&song->machines.signal_insert,
			self, instrumentview_on_machines_insert);
		psy_signal_connect(&song->machines.signal_removed,
			self, instrumentview_on_machines_removed);
		instrumentsbox_setinstruments(&self->instrumentsbox,
			&song->instruments);
		samplesbox_setsamples(&self->general.notemapview.samplesbox,
			&song->samples);
	} else {
		instrumentsbox_setinstruments(&self->instrumentsbox, 0);
		samplesbox_setsamples(&self->general.notemapview.samplesbox, NULL);
	}
	virtualgeneratorbox_updatesamplers(&self->header.virtualgenerators);
	instrumentview_set_instrument(self, psy_audio_instrumentindex_make(0, 0));
}

void instrumentview_on_create_instrument(InstrumentView* self,
	psy_ui_Component* sender)
{
	if (workspace_song(self->workspace)) {
		psy_audio_Instrument* instrument;
		psy_audio_InstrumentIndex selected;

		selected = instrumentsbox_selected(&self->instrumentsbox);
		instrument = psy_audio_instrument_allocinit();
		psy_audio_instrument_setname(instrument, "Untitled");
		psy_audio_instrument_setindex(instrument, selected.subslot);
		psy_audio_exclusivelock_enter();
		psy_audio_instruments_insert(
			&workspace_song(self->workspace)->instruments, instrument,
			selected);
		psy_audio_instruments_select(
			&workspace_song(self->workspace)->instruments, selected);
		psy_audio_exclusivelock_leave();
	}
}

void instrumentview_on_delete_instrument(InstrumentView* self,
	psy_ui_Component* sender)
{
	psy_audio_exclusivelock_enter();
	psy_audio_instruments_remove(&workspace_song(self->workspace)->instruments,
		instrumentsbox_selected(&self->instrumentsbox));
	psy_audio_exclusivelock_leave();
}

void instrumentview_on_load_instrument_button(InstrumentView* self,
	psy_ui_Component* sender)
{
	if (!workspace_song(self->workspace)) {
		return;
	}		
	if (keyboardmiscconfig_ft2_file_view(psycleconfig_misc(
			workspace_conf(self->workspace)))) {
		fileview_set_callbacks(self->workspace->fileview,
			&self->instrument_load, NULL);		
		workspace_select_view(self->workspace, viewindex_make_all(
			VIEW_ID_FILEVIEW, 0, 0, psy_INDEX_INVALID));
	} else {				
		psy_ui_OpenDialog dialog;				

		psy_ui_opendialog_init_all(&dialog, 0, "Load Instrument",
			psy_audio_songfile_instloadfilter(),
			psy_audio_songfile_standardinstloadfilter(),
			dirconfig_samples(&self->workspace->config.directories));
		if (psy_ui_opendialog_execute(&dialog)) {
			instrumentview_load_instrument(self,  psy_path_full(
				psy_ui_opendialog_path(&dialog)));			
		}
		psy_ui_opendialog_dispose(&dialog);
	}
}

void instrumentview_on_load_instrument(InstrumentView* self,
	psy_Property* sender)
{
	instrumentview_load_instrument(self, psy_property_item_str(sender));	
	workspace_select_view(self->workspace, viewindex_make_all(
		VIEW_ID_INSTRUMENTSVIEW, 0, 0, psy_INDEX_INVALID));
}

void instrumentview_load_instrument(InstrumentView* self, const char* path)
{	
	PsyFile file;
	
	if (psyfile_open(&file, path)) {
		psy_audio_SongFile songfile;
	
		psy_audio_songfile_init(&songfile);		
		songfile.song = workspace_song(self->workspace);
		songfile.file = &file;
		songfile.path = psy_strdup(path);		
		psy_audio_songfile_loadinstrument(&songfile, path,
			psy_audio_instruments_selected(
				&workspace_song(self->workspace)->instruments));
		psyfile_close(&file);
		psy_audio_songfile_dispose(&songfile);
	}	
	instrumentsbox_rebuild(&self->instrumentsbox);
}

void instrumentview_on_save_instrument_button(InstrumentView* self,
	psy_ui_Component* sender)
{
	psy_audio_Song* song;
	psy_audio_Instrument* instrument;

	song = workspace_song(self->workspace);		
	if (!song) {
		return;
	}
	instrument = psy_audio_instruments_at(&song->instruments,
		psy_audio_instruments_selected(&song->instruments));
	if (!instrument) {
		return;
	}
	if (keyboardmiscconfig_ft2_file_view(psycleconfig_misc(
			workspace_conf(self->workspace)))) {
		fileview_set_callbacks(self->workspace->fileview,
			NULL, &self->instrument_save);		
		workspace_select_view(self->workspace, viewindex_make_all(
			VIEW_ID_FILEVIEW, 0, 0, psy_INDEX_INVALID));
	} else {	
		psy_ui_SaveDialog dialog;		
		
		if (instrument) {
			psy_ui_savedialog_init_all(&dialog, 0, "Save Instrument",
				psy_audio_songfile_instsavefilter(),
				psy_audio_songfile_standardinstsavefilter(),
				dirconfig_samples(&self->workspace->config.directories));
			if (psy_ui_savedialog_execute(&dialog)) {	
				instrumentview_save_instrument(self, psy_path_full(
					psy_ui_savedialog_path(&dialog)),
					instrument);					
			}
			psy_ui_savedialog_dispose(&dialog);
		}
	}
}

void instrumentview_on_save_instrument(InstrumentView* self,
	psy_Property* sender)
{
	psy_audio_Song* song;	

	song = workspace_song(self->workspace);		
	if (song) {
		psy_audio_Instrument* instrument;
		
		instrument = psy_audio_instruments_at(&song->instruments,
			psy_audio_instruments_selected(&song->instruments));
		if (instrument) {			
			instrumentview_save_instrument(self, psy_property_item_str(sender),
			instrument);	
		}
	}
	workspace_select_view(self->workspace, viewindex_make_all(
		VIEW_ID_INSTRUMENTSVIEW, 0, 0, psy_INDEX_INVALID));
}

void instrumentview_save_instrument(InstrumentView* self, const char* path,
	psy_audio_Instrument* instrument)
{
	if (workspace_song(self->workspace)) {
		psy_audio_SongFile songfile;
		PsyFile file;

		psy_audio_songfile_init(&songfile);
		songfile.song = workspace_song(self->workspace);		;
		songfile.file = &file;
		psy_audio_songfile_saveinstrument(&songfile, path, instrument);
		psy_audio_songfile_dispose(&songfile);
	}
}

void instrumentview_on_status_changed(InstrumentView* self,
	psy_ui_Component* sender, char* text)
{
	if (self->statusbar) {
		instrumentsviewbar_settext(self->statusbar, text);
	}
}
