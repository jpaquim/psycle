// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instrumentview.h"
#include <xm.h>

#include <uiopendialog.h>
#include <uisavedialog.h>

#include <songio.h>
#include <machinefactory.h>
#include "virtualgenerator.h"

#include <math.h>

#include "../../detail/portable.h"

static void virtualgeneratorbox_updategenerator(VirtualGeneratorsBox*);
static void virtualgeneratorbox_ongeneratorschanged(VirtualGeneratorsBox*, psy_ui_Component* sender, int slot);
static void virtualgeneratorbox_onsamplerschanged(VirtualGeneratorsBox*, psy_ui_Component* sender, int slot);
static void virtualgeneratorbox_onactivechanged(VirtualGeneratorsBox*, psy_ui_Component* sender);
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
		uintptr_t maxkey;

maxkey = psy_table_maxkey(&self->workspace->song->machines.slots);

for (i = 0; i < maxkey; ++i) {
	psy_audio_Machine* machine;

	machine = (psy_audio_Machine*)psy_audio_machines_at(
		&self->workspace->song->machines, i);
	if (machine && machine_supports(machine,
		MACH_SUPPORTS_INSTRUMENTS)) {
		char text[512];

		psy_snprintf(text, 512, "%X: %s", (int)i,
			psy_audio_machine_editname(machine));
		psy_ui_combobox_addtext(&self->samplers, text);
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
			psy_audio_Machine* generator;

			generator = psy_audio_machines_at(&self->workspace->song->machines,
				psy_ui_combobox_cursel(&self->generators) + 0x81);
			if (generator) {
				psy_audio_machines_remove(&self->workspace->song->machines,
					psy_ui_combobox_cursel(&self->generators) + 0x81);
			}
			generator = psy_audio_machinefactory_makemachinefrompath(
				&self->workspace->machinefactory, MACH_VIRTUALGENERATOR,
				NULL, (uintptr_t)psy_ui_combobox_itemdata(&self->generators,
					psy_ui_combobox_cursel(&self->generators)),
				psy_audio_instruments_selected(&self->workspace->song->instruments).subslot);
			if (generator) {
				char editname[256];

				psy_snprintf(editname, 256, "Virtual Generator");
				psy_audio_machine_seteditname(generator, editname);
				psy_audio_machines_insert(&self->workspace->song->machines,
					psy_ui_combobox_cursel(&self->generators) + 0x81, generator);
			}
		}
	} else {
		if (psy_ui_combobox_cursel(&self->generators) != -1) {
			psy_audio_Machine* generator;

			generator = psy_audio_machines_at(&self->workspace->song->machines,
				psy_ui_combobox_cursel(&self->generators) + 0x81);
			if (generator) {
				psy_audio_machines_remove(&self->workspace->song->machines,
					psy_ui_combobox_cursel(&self->generators) + 0x81);
			}
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
						if (index == psy_audio_instruments_selected(&self->workspace->song->instruments).subslot) {
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

// InstrumentHeaderView
// prototypes
static void instrumentheaderview_onprevinstrument(InstrumentHeaderView*, psy_ui_Component* sender);
static void instrumentheaderview_onnextinstrument(InstrumentHeaderView*, psy_ui_Component* sender);
static void instrumentheaderview_ondeleteinstrument(InstrumentHeaderView*, psy_ui_Component* sender);
static void instrumentheaderview_oneditinstrumentname(InstrumentHeaderView*, psy_ui_Edit* sender);
// implementation
void instrumentheaderview_init(InstrumentHeaderView* self, psy_ui_Component* parent,
	psy_audio_Instruments* instruments, InstrumentView* view,
	Workspace* workspace)
{	
	psy_ui_Margin margin;

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
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
			&margin));	
}

void instrumentheaderview_setinstrument(InstrumentHeaderView* self,
	psy_audio_Instrument* instrument)
{
	self->instrument = instrument;
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
		int index;
		
		index = instrumentsbox_selected(&self->view->instrumentsbox);
		psy_audio_instrument_setname(self->instrument, psy_ui_edit_text(sender));
		psy_snprintf(text, 20, "%02X:%s", 
			(int) index, psy_audio_instrument_name(self->instrument));
		psy_ui_listbox_settext(&self->view->instrumentsbox.instrumentlist, text,
			index);		
	}
}

void instrumentheaderview_onprevinstrument(InstrumentHeaderView* self, psy_ui_Component* sender)
{
	/*instruments_changeslot(self->instruments,
		instruments_slot(self->instruments) > 0 ?
		instruments_slot(self->instruments) - 1 : 0);*/
}

void instrumentheaderview_onnextinstrument(InstrumentHeaderView* self, psy_ui_Component* sender)
{
	/*instruments_changeslot(self->instruments,
		instruments_slot(self->instruments) < 255 ?
		instruments_slot(self->instruments) + 1 : 255);*/
}

void instrumentheaderview_ondeleteinstrument(InstrumentHeaderView* self, psy_ui_Component* sender)
{	
}

// InstrumentViewButtons
// implementation
void instrumentviewbuttons_init(InstrumentViewButtons* self,
	psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(0.5), psy_ui_value_makeeh(0.5),
		psy_ui_value_makepx(0));
	psy_ui_component_init(&self->component, parent);	
	psy_ui_component_init_align(&self->row1, &self->component,
		psy_ui_ALIGN_TOP);
	psy_ui_component_setalignexpand(&self->row1, psy_ui_HORIZONTALEXPAND);
	psy_ui_button_init_text(&self->create, &self->row1, "file.new");
	psy_ui_button_init_text(&self->load, &self->row1, "file.load");
	psy_ui_button_init_text(&self->save, &self->row1, "file.save");
	psy_ui_button_init_text(&self->duplicate, &self->row1, "edit.duplicate");
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->row1, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
		&margin));
	psy_ui_component_init_align(&self->row2, &self->component,
		psy_ui_ALIGN_TOP);
	psy_ui_component_setalignexpand(&self->row2, psy_ui_HORIZONTALEXPAND);
	psy_ui_button_init_text(&self->del, &self->row2, "edit.delete");
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->row2, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
		&margin));
}

// InstrumentGeneralView
// prototypes
static void instrumentgeneralview_onfitrow(InstrumentGeneralView*,
	psy_ui_Component* sender);
static void instrumentgeneralview_onnnacut(InstrumentGeneralView*);
static void instrumentgeneralview_onnnarelease(InstrumentGeneralView*);
static void instrumentgeneralview_onnnafadeout(InstrumentGeneralView*);
static void instrumentgeneralview_onnnanone(InstrumentGeneralView*);
static void instrumentgeneralview_nnahighlight(InstrumentGeneralView*,
	psy_ui_Button* highlight);
static void instrumentgeneralview_ongeneralviewdescribe(InstrumentGeneralView*,
	psy_ui_Slider*, char* text);
static void instrumentgeneralview_ongeneralviewtweak(InstrumentGeneralView*,
	psy_ui_Slider*, float value);
static void instrumentgeneralview_ongeneralviewvalue(InstrumentGeneralView*,
	psy_ui_Slider*, float* value);
// implementation
void instrumentgeneralview_init(InstrumentGeneralView* self,
	psy_ui_Component* parent, psy_audio_Instruments* instruments,
	Workspace* workspace)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(2), psy_ui_value_makeeh(1.5),
		psy_ui_value_makepx(0));
	self->instruments = instruments;
	self->instrument = 0;
	psy_ui_component_init(&self->component, parent);	
	// left	
	psy_ui_component_setalign(&self->notemapview.samplesbox.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_setmargin(&self->notemapview.samplesbox.component, &margin);
	// nna
	psy_ui_component_init(&self->nna, &self->component);
	psy_ui_component_setalign(&self->nna, psy_ui_ALIGN_TOP);
	psy_ui_label_init_text(&self->nnaheader, &self->nna,
		"instrumentview.new-note-action");
	psy_ui_button_init_connect(&self->nnacut, &self->nna,
		self, instrumentgeneralview_onnnacut);
	psy_ui_button_settext(&self->nnacut, "instrumentview.note-cut");		
	psy_ui_button_init_connect(&self->nnarelease, &self->nna,
		self, instrumentgeneralview_onnnarelease);
	psy_ui_button_settext(&self->nnarelease, "instrumentview.note-release");
	psy_ui_button_init_connect(&self->nnafadeout, &self->nna,
		self, instrumentgeneralview_onnnafadeout);
	psy_ui_button_settext(&self->nnafadeout, "instrumentview.note-fadeout");
	psy_ui_button_init_connect(&self->nnanone, &self->nna,
		self, instrumentgeneralview_onnnanone);	
	psy_ui_button_settext(&self->nnanone, "instrumentview.none");
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->nna, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
		&margin));
	// fitrow
	psy_ui_component_init(&self->fitrow, &self->component);
	psy_ui_component_setalign(&self->fitrow, psy_ui_ALIGN_TOP);
	psy_ui_checkbox_init(&self->fitrowcheck, &self->fitrow);
	psy_signal_connect(&self->fitrowcheck.signal_clicked, self,
		instrumentgeneralview_onfitrow);
	psy_ui_edit_init(&self->fitrowedit, &self->fitrow);
	psy_ui_edit_setcharnumber(&self->fitrowedit, 4);
	psy_ui_label_init_text(&self->fitrowlabel, &self->fitrow,
		"instrumentview.pattern-rows");
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->fitrow, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
		&margin));
	psy_ui_slider_init(&self->globalvolume, &self->component);
	psy_ui_component_setalign(&self->globalvolume.component, psy_ui_ALIGN_TOP);
	psy_ui_slider_connect(&self->globalvolume, self,
		(ui_slider_fpdescribe)instrumentgeneralview_ongeneralviewdescribe,
		(ui_slider_fptweak)instrumentgeneralview_ongeneralviewtweak,
		(ui_slider_fpvalue)instrumentgeneralview_ongeneralviewvalue);
	instrumentnotemapview_init(&self->notemapview, &self->component, workspace);
	psy_ui_component_setalign(&self->notemapview.component, psy_ui_ALIGN_CLIENT);
	psy_ui_checkbox_settext(&self->fitrowcheck,
		"instrumentview.play-sample-to-fit");
	psy_ui_slider_settext(&self->globalvolume,
		"instrumentview.global-volume");
	
}

void instrumentgeneralview_setinstrument(InstrumentGeneralView* self,
	psy_audio_Instrument* instrument)
{	
	self->instrument = instrument;		
	instrumentnotemapview_setinstrument(&self->notemapview, instrument);
	if (instrument) {
		char text[128];

		if (self->instrument->loop) {
			psy_ui_checkbox_check(&self->fitrowcheck);
		} else {
			psy_ui_checkbox_disablecheck(&self->fitrowcheck);
		}
		psy_snprintf(text, 128, "%d", (int)self->instrument->lines);
		psy_ui_edit_settext(&self->fitrowedit, text);
		switch (self->instrument->nna) {
			case psy_audio_NNA_STOP:
				instrumentgeneralview_nnahighlight(self, &self->nnacut);
			break;
			case psy_audio_NNA_NOTEOFF:
				instrumentgeneralview_nnahighlight(self, &self->nnarelease);
			break;
			case psy_audio_NNA_FADEOUT:
				instrumentgeneralview_nnahighlight(self, &self->nnafadeout);
			break;
			case psy_audio_NNA_CONTINUE:
				instrumentgeneralview_nnahighlight(self, &self->nnanone);
			break;
			default:
				instrumentgeneralview_nnahighlight(self, &self->nnacut);
			break;
		}
	} else {
		instrumentgeneralview_nnahighlight(self, &self->nnacut);
	}
}

void instrumentgeneralview_onfitrow(InstrumentGeneralView* self,
	psy_ui_Component* sender)
{
	if (self->instrument) {
		if (psy_ui_checkbox_checked(&self->fitrowcheck)) {
			self->instrument->loop = TRUE;
		} else {
			self->instrument->loop = FALSE;
		}
	}
}

void instrumentgeneralview_onnnacut(InstrumentGeneralView* self)
{
	if (self->instrument) {
		psy_audio_instrument_setnna(self->instrument, psy_audio_NNA_STOP);
		instrumentgeneralview_nnahighlight(self, &self->nnacut);
	}
}

void instrumentgeneralview_onnnarelease(InstrumentGeneralView* self)
{
	if (self->instrument) {
		psy_audio_instrument_setnna(self->instrument, psy_audio_NNA_NOTEOFF);
		instrumentgeneralview_nnahighlight(self, &self->nnarelease);
	}
}

void instrumentgeneralview_onnnafadeout(InstrumentGeneralView* self)
{
	if (self->instrument) {
		psy_audio_instrument_setnna(self->instrument, psy_audio_NNA_FADEOUT);		
		instrumentgeneralview_nnahighlight(self, &self->nnafadeout);
	}
}

void instrumentgeneralview_onnnanone(InstrumentGeneralView* self)
{
	if (self->instrument) {
		psy_audio_instrument_setnna(self->instrument, psy_audio_NNA_CONTINUE);
		instrumentgeneralview_nnahighlight(self, &self->nnanone);
	}
}

void instrumentgeneralview_nnahighlight(InstrumentGeneralView* self,
	psy_ui_Button* highlight)
{
	psy_ui_button_highlight(highlight);
	if (highlight != &self->nnacut) {
		psy_ui_button_disablehighlight(&self->nnacut);
	}
	if (highlight != &self->nnafadeout) {
		psy_ui_button_disablehighlight(&self->nnafadeout);
	}
	if (highlight != &self->nnarelease) {
		psy_ui_button_disablehighlight(&self->nnarelease);
	}
	if (highlight != &self->nnanone) {
		psy_ui_button_disablehighlight(&self->nnanone);
	}	
}

void instrumentgeneralview_ongeneralviewtweak(InstrumentGeneralView* self,
	psy_ui_Slider* slider, float value)
{
	if (self->instrument) {					
		if (slider == &self->globalvolume) {		
			self->instrument->globalvolume = (value * value);
		}
	}
}

void instrumentgeneralview_ongeneralviewvalue(InstrumentGeneralView* self,
	psy_ui_Slider* slider, float* value)
{		
	if (slider == &self->globalvolume) {
		if (self->instrument) {			
			*value = (float)sqrt(self->instrument->globalvolume);
		} else {
			*value = 1.f;
		}
	}
}

void instrumentgeneralview_ongeneralviewdescribe(InstrumentGeneralView* self,
	psy_ui_Slider* slider, char* text)
{			
	if (slider == &self->globalvolume) {		
		if (!self->instrument) {
			psy_snprintf(text, 10, "0.0dB");
		} else
		if (self->instrument->globalvolume == 0) {
			psy_snprintf(text, 10, "-inf. dB");
		} else {
			float db = (float)(20 * log10(self->instrument->globalvolume));
			psy_snprintf(text, 10, "%.1f dB", db);
		}
	}
}

// InstrumentVolumeView
// prototypes
static void instrumentvolumeview_updatetext(InstrumentVolumeView*);
static void instrumentvolumeview_onlanguagechanged(InstrumentVolumeView*, psy_ui_Component* sender);
static void instrumentvolumeview_onvolumeviewdescribe(InstrumentVolumeView*,
	psy_ui_Slider*, char* txt);
static void instrumentvolumeview_onvolumeviewtweak(InstrumentVolumeView*,
	psy_ui_Slider*, float value);
static void instrumentvolumeview_onvolumeviewvalue(InstrumentVolumeView*,
	psy_ui_Slider*, float* value);
// implementation
void instrumentvolumeview_init(InstrumentVolumeView* self, psy_ui_Component* parent,
	psy_audio_Instruments* instruments, Workspace* workspace)
{
	psy_ui_Margin margin;
	int i;
	psy_ui_Slider* sliders[] = {
		&self->attack,
		&self->decay,
		&self->sustain,
		&self->release
	};

	self->instruments = instruments;	
	self->instrument = 0;
	psy_ui_component_init(&self->component, parent);		
	envelopeview_init(&self->envelopeview, &self->component);
	envelopeview_settext(&self->envelopeview, "Amplitude envelope");
	psy_ui_component_setalign(&self->envelopeview.component, psy_ui_ALIGN_TOP);
	psy_ui_margin_init_all(&margin,
		psy_ui_value_makepx(20),
		psy_ui_value_makepx(5),
		psy_ui_value_makepx(5),
		psy_ui_value_makepx(5));
	psy_ui_component_setmargin(&self->envelopeview.component, &margin);		
	psy_ui_slider_init(&self->attack, &self->component);
	psy_ui_slider_settext(&self->attack, "instrumentview.attack");
	psy_ui_slider_init(&self->decay, &self->component);
	psy_ui_slider_settext(&self->decay, "instrumentview.decay");
	psy_ui_slider_init(&self->sustain, &self->component);
	psy_ui_slider_settext(&self->sustain, "instrumentview.sustain-level");
	psy_ui_slider_init(&self->release, &self->component);
	psy_ui_slider_settext(&self->release, "instrumentview.release");
	psy_ui_margin_init_all(&margin,
		psy_ui_value_makepx(0),
		psy_ui_value_makepx(5),
		psy_ui_value_makepx(5),
		psy_ui_value_makepx(5));
	for (i = 0; i < 4; ++i) {				
		psy_ui_component_setalign(&sliders[i]->component, psy_ui_ALIGN_TOP);
		psy_ui_component_setmargin(&sliders[i]->component, &margin);
		psy_ui_slider_connect(sliders[i], self,
			(ui_slider_fpdescribe)instrumentvolumeview_onvolumeviewdescribe,
			(ui_slider_fptweak)instrumentvolumeview_onvolumeviewtweak,
			(ui_slider_fpvalue)instrumentvolumeview_onvolumeviewvalue);
	}
	instrumentvolumeview_updatetext(self);
	psy_signal_connect(&self->component.signal_languagechanged, self,
		instrumentvolumeview_onlanguagechanged);
}

void instrumentvolumeview_updatetext(InstrumentVolumeView* self)
{
	envelopeview_settext(&self->envelopeview, psy_ui_translate(
		"instrumentview.amplitude-envelope"));	
}

void instrumentvolumeview_onlanguagechanged(InstrumentVolumeView* self, psy_ui_Component* sender)
{
	instrumentvolumeview_updatetext(self);
}

void instrumentvolumeview_setinstrument(InstrumentVolumeView* self,
	psy_audio_Instrument* instrument)
{	
	self->instrument = instrument;
	envelopeview_setadsrenvelope(&self->envelopeview,
		instrument
		? &instrument->volumeenvelope
		: 0);
}

void instrumentvolumeview_onvolumeviewdescribe(InstrumentVolumeView* self,
	psy_ui_Slider* slidergroup, char* txt)
{
	if (slidergroup == &self->attack) {		
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0ms");
		} else {
			psy_snprintf(txt, 20, "%.4fms", 
				adsr_settings_attack(&self->instrument->volumeenvelope) * 1000);				
		}		
	} else
	if (slidergroup == &self->decay) {		
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0ms");
		} else {
			psy_snprintf(txt, 20, "%.4fms",
				adsr_settings_decay(&self->instrument->volumeenvelope) * 1000);
		}		
	} else
	if (slidergroup == &self->sustain) {
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0%%");
		} else {
			psy_snprintf(txt, 20, "%d%%", (int)
				(adsr_settings_sustain(&self->instrument->volumeenvelope) * 100));
		}		
	} else
	if (slidergroup == &self->release) {
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0ms");
		} else {
			psy_snprintf(txt, 20, "%.4fms",
				adsr_settings_release(&self->instrument->volumeenvelope) * 1000);
		}		
	}
}

void instrumentvolumeview_onvolumeviewtweak(InstrumentVolumeView* self,
	psy_ui_Slider* slidergroup, float value)
{
	if (!self->instrument) {
		return;
	}
	if (slidergroup == &self->attack) {
		adsr_settings_setattack(
			&self->instrument->volumeenvelope, value * 1.4f);
	} else
	if (slidergroup == &self->decay) {
		adsr_settings_setdecay(
			&self->instrument->volumeenvelope, value * 1.4f);
	} else
	if (slidergroup == &self->sustain) {
		adsr_settings_setsustain(
			&self->instrument->volumeenvelope, value);
	} else
	if (slidergroup == &self->release) {
		adsr_settings_setrelease(
			&self->instrument->volumeenvelope, value * 1.4f);
	}
	envelopeview_update(&self->envelopeview);
}

void instrumentvolumeview_onvolumeviewvalue(InstrumentVolumeView* self,
	psy_ui_Slider* slidergroup, float* value)
{
	if (slidergroup == &self->attack) {
		*value = self->instrument
			? adsr_settings_attack(&self->instrument->volumeenvelope) / 1.4f
			: 0.f;
	} else 
	if (slidergroup == &self->decay) {
		*value = self->instrument
			? adsr_settings_decay(&self->instrument->volumeenvelope) / 1.4f
			: 0.f;
	} else 	
	if (slidergroup == &self->sustain) {
		*value = self->instrument
			? adsr_settings_sustain(&self->instrument->volumeenvelope)
			: 0.5f;
	} else
	if (slidergroup == &self->release) {
		*value = self->instrument
			? adsr_settings_release(&self->instrument->volumeenvelope) / 1.4f
			: 0.5f;
	}
}

// InstrumentPanView
// prototypes
static void instrumentpanview_onrandompanning(InstrumentPanView* self,
	psy_ui_CheckBox* sender);
// implementation
void instrumentpanview_init(InstrumentPanView* self, psy_ui_Component* parent,
	psy_audio_Instruments* instruments, Workspace* workspace)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(1.5),
		psy_ui_value_makepx(0));
	self->instrument = 0;
	self->instruments = instruments;	
	psy_ui_component_init(&self->component, parent);	
	psy_ui_checkbox_init(&self->randompanning, &self->component);
	psy_ui_checkbox_settext(&self->randompanning,
		"instrumentview.random-panning");
	psy_signal_connect(&self->randompanning.signal_clicked, self,
		instrumentpanview_onrandompanning);
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_TOP,
		&margin));	
}

void instrumentpanview_setinstrument(InstrumentPanView* self,
	psy_audio_Instrument* instrument)
{	
	self->instrument = instrument;
	if (self->instrument && self->instrument->randompan) {
		psy_ui_checkbox_check(&self->randompanning);
	} else {
		psy_ui_checkbox_disablecheck(&self->randompanning);	
	}
}

void instrumentpanview_onrandompanning(InstrumentPanView* self, psy_ui_CheckBox* sender)
{
	if (self->instrument) {
		self->instrument->randompan = psy_ui_checkbox_checked(sender);
	}
}

// InstrumentFilterView
// prototypes
static void instrumentfilterview_updatetext(InstrumentFilterView*);
static void instrumentfilterview_onlanguagechanged(InstrumentFilterView*, psy_ui_Component* sender);
static void instrumentfilterview_fillfiltercombobox(InstrumentFilterView*);
static void instrumentfilterview_ondescribe(InstrumentFilterView*, psy_ui_Slider*, char* txt);
static void instrumentfilterview_ontweak(InstrumentFilterView*, psy_ui_Slider*, float value);
static void instrumentfilterview_onvalue(InstrumentFilterView*, psy_ui_Slider*, float* value);
static void instrumentfilterview_onfiltercomboboxchanged(InstrumentFilterView*, psy_ui_ComboBox* sender, int index);
// implementation
void instrumentfilterview_init(InstrumentFilterView* self, psy_ui_Component* parent,
	psy_audio_Instruments* instruments, Workspace* workspace)
{
	psy_ui_Margin margin;
	int i;
	psy_ui_Slider* sliders[] = {
		&self->attack,
		&self->decay,
		&self->sustain,
		&self->release,
		&self->cutoff,
		&self->res,
		&self->modamount
	};

	self->instruments = instruments;
	self->instrument = 0;
	psy_ui_component_init(&self->component, parent);	
	psy_ui_component_init(&self->filter, &self->component);
	psy_ui_component_setalign(&self->filter, psy_ui_ALIGN_TOP);	
	psy_ui_label_init_text(&self->filtertypeheader, &self->filter,
		"Filter type");
	psy_ui_component_setalign(&self->filtertypeheader.component, psy_ui_ALIGN_LEFT);
	psy_ui_margin_init_all(&margin,
		psy_ui_value_makepx(0), psy_ui_value_makeew(2),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0));
	psy_ui_component_setmargin(&self->filtertypeheader.component, &margin);
	psy_ui_combobox_init(&self->filtertype, &self->filter);
	psy_ui_combobox_setcharnumber(&self->filtertype, 20);
	psy_ui_component_setalign(&self->filtertype.component, psy_ui_ALIGN_LEFT);	
	instrumentfilterview_fillfiltercombobox(self);
	psy_ui_combobox_setcursel(&self->filtertype, (int)F_NONE);
	psy_signal_connect(&self->filtertype.signal_selchanged, self,
		instrumentfilterview_onfiltercomboboxchanged);
	envelopeview_init(&self->envelopeview, &self->component);
	envelopeview_settext(&self->envelopeview, "Filter envelope");
	psy_ui_component_setalign(&self->envelopeview.component, psy_ui_ALIGN_TOP);
	psy_ui_margin_init_all(&margin,
		psy_ui_value_makepx(20),
		psy_ui_value_makepx(5),
		psy_ui_value_makepx(5),
		psy_ui_value_makepx(5));
	psy_ui_component_setmargin(&self->envelopeview.component, &margin);	
	//psy_ui_component_resize(&self->envelopeview.component,
		//psy_ui_value_makepx(0),
		//psy_ui_value_makeeh(20));
	psy_ui_slider_init(&self->attack, &self->component);
	psy_ui_slider_settext(&self->attack, "Attack");
	psy_ui_slider_init(&self->decay, &self->component);
	psy_ui_slider_settext(&self->decay, "Decay");
	psy_ui_slider_init(&self->sustain, &self->component);
	psy_ui_slider_settext(&self->sustain, "Sustain Level");
	psy_ui_slider_init(&self->release, &self->component);
	psy_ui_slider_settext(&self->release, "Release");
	psy_ui_slider_init(&self->cutoff, &self->component);
	psy_ui_slider_settext(&self->cutoff, "Cut-off");
	psy_ui_slider_init(&self->res, &self->component);
	psy_ui_slider_settext(&self->res, "Res/bandw.");
	psy_ui_slider_init(&self->modamount, &self->component);
	psy_ui_slider_settext(&self->modamount, "Mod. Amount");
	
	psy_ui_margin_init_all(&margin,
		psy_ui_value_makepx(0),
		psy_ui_value_makepx(5),
		psy_ui_value_makepx(5),
		psy_ui_value_makepx(5));
	for (i = 0; i < 7; ++i) {				
		psy_ui_component_setalign(&sliders[i]->component, psy_ui_ALIGN_TOP);
		psy_ui_component_setmargin(&sliders[i]->component, &margin);
		psy_ui_slider_connect(sliders[i], self,
			(ui_slider_fpdescribe)instrumentfilterview_ondescribe,
			(ui_slider_fptweak)instrumentfilterview_ontweak,
			(ui_slider_fpvalue)instrumentfilterview_onvalue);
	}
	instrumentfilterview_updatetext(self);
	psy_signal_connect(&self->component.signal_languagechanged, self,
		instrumentfilterview_onlanguagechanged);
	psy_ui_label_settext(&self->filtertypeheader, "instrumentview.filter-type");
	psy_ui_slider_settext(&self->attack, "instrumentview.attack");
	psy_ui_slider_settext(&self->decay, "instrumentview.decay");
	psy_ui_slider_settext(&self->sustain, "instrumentview.sustain-level");
	psy_ui_slider_settext(&self->release, "instrumentview.release");
	psy_ui_slider_settext(&self->cutoff, "instrumentview.cut-off");
	psy_ui_slider_settext(&self->res, "instrumentview.res");
	psy_ui_slider_settext(&self->modamount, "instrumentview.mod");
}

void instrumentfilterview_updatetext(InstrumentFilterView* self)
{	
	envelopeview_settext(&self->envelopeview,
		psy_ui_translate("instrumentview.filter-envelope"));	
}

void instrumentfilterview_onlanguagechanged(InstrumentFilterView* self, psy_ui_Component* sender)
{
	instrumentfilterview_updatetext(self);
}

void instrumentfilterview_fillfiltercombobox(InstrumentFilterView* self)
{
	uintptr_t ft;
	for (ft = 0; ft < filter_numfilters(); ++ft) {
		psy_ui_combobox_addtext(&self->filtertype, filter_name((FilterType)ft));
	}
}

void instrumentfilterview_onfiltercomboboxchanged(InstrumentFilterView* self, psy_ui_ComboBox* sender, int index)
{
	self->instrument->filtertype = (FilterType)index;
}

void instrumentfilterview_setinstrument(InstrumentFilterView* self, psy_audio_Instrument* instrument)
{	
	self->instrument = instrument;
	if (self->instrument) {
		envelopeview_setadsrenvelope(&self->envelopeview,
			&instrument->filterenvelope);
		psy_ui_combobox_setcursel(&self->filtertype, (intptr_t)instrument->filtertype);
	} else {
		envelopeview_setadsrenvelope(&self->envelopeview, NULL);
		psy_ui_combobox_setcursel(&self->filtertype, (intptr_t)F_NONE);
	}	
}

void instrumentfilterview_ondescribe(InstrumentFilterView* self, psy_ui_Slider* slidergroup, char* txt)
{
	if (slidergroup == &self->attack) {		
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0ms");
		} else {
			psy_snprintf(txt, 20, "%.4fms", 
				adsr_settings_attack(
					&self->instrument->filterenvelope) * 1000);				
		}		
	} else
	if (slidergroup == &self->decay) {		
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0ms");
		} else {
			psy_snprintf(txt, 20, "%.4fms",
				adsr_settings_decay(
					&self->instrument->filterenvelope) * 1000);
		}		
	} else
	if (slidergroup == &self->sustain) {
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0%%");
		} else {
			psy_snprintf(txt, 20, "%d%%", (int)(
				adsr_settings_sustain(
					&self->instrument->filterenvelope) * 100));
		}		
	} else
	if (slidergroup == &self->release) {
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0ms");
		} else {
			psy_snprintf(txt, 20, "%.4fms",
				adsr_settings_release(
					&self->instrument->filterenvelope) * 1000);
		}		
	} else
	if (slidergroup == &self->cutoff) {
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0");
		} else {
			psy_snprintf(txt, 20, "%d", (int)(self->instrument->filtercutoff * 11665 + 2333));
		}		
	} else
	if (slidergroup == &self->res) {
		if (!self->instrument) {
			psy_snprintf(txt, 10, "50%%");
		} else {
			psy_snprintf(txt, 20, "%d%%", (int)(self->instrument->filterres * 100));
		}		
	} else
	if (slidergroup == &self->modamount) {
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0ms");
		} else {
			psy_snprintf(txt, 20, "%d%%", (int)(self->instrument->filtermodamount) * 100);
		}		
	}	
}

void instrumentfilterview_ontweak(InstrumentFilterView* self,
	psy_ui_Slider* slidergroup, float value)
{
	if (!self->instrument) {
		return;
	}
	if (slidergroup == &self->attack) {
			adsr_settings_setattack(
				&self->instrument->filterenvelope, value * 1.4f);
	} else
	if (slidergroup == &self->decay) {
			adsr_settings_setdecay(
				&self->instrument->filterenvelope, value * 1.4f);
	} else
	if (slidergroup == &self->sustain) {
			adsr_settings_setsustain(
				&self->instrument->filterenvelope, value);
	} else
	if (slidergroup == &self->release) {
		adsr_settings_setrelease(
			&self->instrument->filterenvelope, value * 1.4f);
	} else
	if (slidergroup == &self->cutoff) {
		self->instrument->filtercutoff = value;
	} else
	if (slidergroup == &self->res) {
		self->instrument->filterres = value;
	} else
	if (slidergroup == &self->modamount) {
		self->instrument->filtermodamount = value - 0.5f;
	}
	envelopeview_update(&self->envelopeview);
}

void instrumentfilterview_onvalue(InstrumentFilterView* self,
	psy_ui_Slider* slidergroup, float* value)
{
	if (slidergroup == &self->attack) {
		*value = self->instrument
			? adsr_settings_attack(&self->instrument->filterenvelope) / 1.4f
			: 0.f;
	} else 
	if (slidergroup == &self->decay) {
		*value = self->instrument
			? adsr_settings_decay(&self->instrument->filterenvelope) / 1.4f
			: 0.f;
	} else 	
	if (slidergroup == &self->sustain) {
		*value = self->instrument
			? adsr_settings_sustain(&self->instrument->filterenvelope)
			: 0.5f;
	} else
	if (slidergroup == &self->release) {
		*value = self->instrument
			? adsr_settings_release(&self->instrument->filterenvelope) / 1.4f
			: 0.5f;
	} else
	if (slidergroup == &self->cutoff) {
		*value = self->instrument
			? self->instrument->filtercutoff
			: 0.5f;
	} else
	if (slidergroup == &self->res) {
		*value = self->instrument
			? self->instrument->filterres
			: 0.5f;
	} else
	if (slidergroup == &self->modamount) {
		*value = self->instrument
			? self->instrument->filtermodamount + 0.5f
			: 0.5f;
	}
}

// InstrumentPitchView
// implementation
void instrumentpitchview_init(InstrumentPitchView* self, psy_ui_Component* parent,
	psy_audio_Instruments* instruments, Workspace* workspace)
{
	self->instruments = instruments;	
	psy_ui_component_init(&self->component, parent);	
}

void instrumentpitchview_setinstrument(InstrumentPitchView* self,
	psy_audio_Instrument* instrument)
{
	self->instrument = instrument;	
}

// InstrumentView
// prototypes
static void instrumentview_oncreateinstrument(InstrumentView*,
	psy_ui_Component* sender);
static void instrumentview_onloadinstrument(InstrumentView*, psy_ui_Component* sender);
static void instrumentview_onaddentry(InstrumentView*, psy_ui_Component* sender);
static void instrumentview_onremoveentry(InstrumentView*,
	psy_ui_Component* sender);
static void instrumentview_oninstrumentinsert(InstrumentView*, psy_audio_Instruments* sender,
	const psy_audio_InstrumentIndex* index);
static void instrumentview_oninstrumentremoved(InstrumentView*, psy_audio_Instruments* sender,
	const psy_audio_InstrumentIndex* index);
static void instrumentview_oninstrumentslotchanged(InstrumentView*,
	psy_audio_Instrument* sender, const psy_audio_InstrumentIndex* slot);
static void instrumentview_setinstrument(InstrumentView*,
	psy_audio_InstrumentIndex index);
static void instrumentview_onmachinesinsert(InstrumentView* self, psy_audio_Machines* sender,
	int slot);
static void instrumentview_onmachinesremoved(InstrumentView* self, psy_audio_Machines* sender,
	int slot);
static void instrumentview_onsongchanged(InstrumentView*, Workspace* workspace, int flag,
	psy_audio_SongFile*);
// implementation
void instrumentview_init(InstrumentView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{
	psy_ui_Margin margin;
	psy_ui_Margin leftmargin;

	psy_ui_component_init(&self->component, parent);	
	psy_ui_component_init(&self->viewtabbar, tabbarparent);
	self->player = &workspace->player;
	self->workspace = workspace;
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(2), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
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
	instrumentviewbuttons_init(&self->buttons, &self->left, workspace);
	psy_ui_component_setalign(&self->buttons.component, psy_ui_ALIGN_TOP);
	instrumentsbox_init(&self->instrumentsbox, &self->left,
		&workspace->song->instruments, workspace);
	psy_ui_component_setalign(&self->instrumentsbox.component,
		psy_ui_ALIGN_CLIENT);
	{
		psy_ui_Margin margin;

		psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
			psy_ui_value_makeew(2.0), psy_ui_value_makepx(0),
			psy_ui_value_makepx(0));
		psy_ui_component_setmargin(&self->left,
			&leftmargin);
	}
	// client
	psy_ui_component_init(&self->client, &self->component);
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
		psy_ui_notebook_base(&self->notebook), &workspace->song->instruments,
		workspace);
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
	samplesbox_setsamples(&self->general.notemapview.samplesbox, &workspace->song->samples
	/* ,&workspace->song->instruments */);
	psy_signal_connect(&self->buttons.create.signal_clicked, self,
		instrumentview_oncreateinstrument);
	psy_signal_connect(&self->buttons.load.signal_clicked, self,
		instrumentview_onloadinstrument);
	psy_signal_connect(&self->general.notemapview.buttons.add.signal_clicked, self,
		instrumentview_onaddentry);
	psy_signal_connect(&self->general.notemapview.buttons.remove.signal_clicked, self,
		instrumentview_onremoveentry);	
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
}

void instrumentview_onsongchanged(InstrumentView* self, Workspace* workspace, int flag, psy_audio_SongFile* songfile)
{
	if (workspace->song) {
		self->header.instruments = &workspace->song->instruments;
		self->general.instruments = &workspace->song->instruments;
		self->volume.instruments = &workspace->song->instruments;
		self->pan.instruments = &workspace->song->instruments;
		self->filter.instruments = &workspace->song->instruments;
		self->pitch.instruments = &workspace->song->instruments;
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
		samplesbox_setsamples(&self->general.notemapview.samplesbox, &workspace->song->samples
		/*,&workspace->song->instruments*/);		
	} else {
		instrumentsbox_setinstruments(&self->instrumentsbox, 0);
		samplesbox_setsamples(&self->general.notemapview.samplesbox,
			&workspace->song->samples);
	}
	virtualgeneratorbox_updatesamplers(&self->header.virtualgenerators);
	instrumentview_setinstrument(self, psy_audio_instrumentindex_make(0, 0));
}

void instrumentview_oncreateinstrument(InstrumentView* self, psy_ui_Component* sender)
{
	if (workspace_song(self->workspace)) {
		psy_audio_Instrument* instrument;
		int selected;

		selected = instrumentsbox_selected(&self->instrumentsbox);
		if (psy_audio_instruments_at(&workspace_song(self->workspace)->instruments, psy_audio_instrumentindex_make(0, selected))) {
			psy_audio_instruments_remove(&workspace_song(self->workspace)->instruments, psy_audio_instrumentindex_make(0, selected));
		}
		instrument = psy_audio_instrument_allocinit();
		psy_audio_instrument_setname(instrument, "Untitled");
		psy_audio_instrument_setindex(instrument, selected);
		psy_audio_instruments_insert(&workspace_song(self->workspace)->instruments, instrument,
			psy_audio_instrumentindex_make(0, selected));
	}
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
			if (psyfile_open(&file, psy_ui_opendialog_filename(&dialog))) {
				psy_audio_xi_load(&songfile, psy_audio_instruments_selected(
					&workspace_song(self->workspace)->instruments).subslot);
			}
			psyfile_close(&file);
			psy_audio_songfile_dispose(&songfile);
			instrumentsbox_rebuild(&self->instrumentsbox);
		}
		psy_ui_opendialog_dispose(&dialog);
	}
}

void instrumentview_onaddentry(InstrumentView* self, psy_ui_Component* sender)
{
	if (self->general.instrument) {
		psy_audio_InstrumentEntry entry;

		psy_audio_instrumententry_init(&entry);
		entry.sampleindex = samplesbox_selected(&self->general.notemapview.samplesbox);
		psy_audio_instrument_addentry(self->general.instrument, &entry);
		instrumentnotemapview_update(&self->general.notemapview);
	}
}

void instrumentview_onremoveentry(InstrumentView* self, psy_ui_Component* sender)
{
	if (self->general.instrument) {
		psy_audio_instrument_removeentry(
			self->general.instrument,
			self->general.notemapview.entryview.selected);
		self->general.notemapview.entryview.selected = UINTPTR_MAX;
		instrumentnotemapview_update(&self->general.notemapview);
	}
}
