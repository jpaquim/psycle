// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "midiview.h"

#include <songio.h>
#include <patterns.h>

#include "../../detail/portable.h"

// MidiActiveChannelBox
// prototypes
static void midiactivechannelbox_ondraw(MidiActiveChannelBox*, psy_ui_Graphics*);
static void midiactivechannelbox_onpreferredsize(MidiActiveChannelBox*,
	psy_ui_Size* limit, psy_ui_Size* rv);
// vtable
static psy_ui_ComponentVtable midiactivechannelbox_vtable;
static bool midiactivechannelbox_vtable_initialized = FALSE;

static void midiactivechannelbox_vtable_init(MidiActiveChannelBox* self)
{
	if (!midiactivechannelbox_vtable_initialized) {
		midiactivechannelbox_vtable = *(self->component.vtable);
		midiactivechannelbox_vtable.ondraw = (psy_ui_fp_ondraw)
			midiactivechannelbox_ondraw;
		midiactivechannelbox_vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			midiactivechannelbox_onpreferredsize;
		midiactivechannelbox_vtable_initialized = TRUE;
	}
}
// implementation
void midiactivechannelbox_init(MidiActiveChannelBox* self,
	psy_ui_Component* parent, int* channelmap)
{
	psy_ui_component_init(&self->component, parent);
	midiactivechannelbox_vtable_init(self);
	self->component.vtable = &midiactivechannelbox_vtable;
	self->channelmap = channelmap;
}

void midiactivechannelbox_ondraw(MidiActiveChannelBox* self,
	psy_ui_Graphics* g)
{
	int ch;
	int cpx;
	psy_ui_TextMetric tm;
	psy_ui_Value colew;
	int headercolw_px;
	int colw_px;
	int lineheight;
	
	tm = psy_ui_component_textmetric(&self->component);
	colew = psy_ui_value_makeew(4);
	headercolw_px = psy_ui_value_px(&colew, &tm);
	lineheight = (int)(tm.tmHeight * 1.2);
	psy_ui_textout(g, 0, 0, "Ch:", strlen("Ch:"));
	colew = psy_ui_value_makeew(3.5);
	colw_px = psy_ui_value_px(&colew, &tm);
	for (ch = 0, cpx = headercolw_px; ch < psy_audio_MAX_MIDI_CHANNELS; ++ch,
			cpx += colw_px) {
		char text[256];
		bool active;

		psy_snprintf(text, 256, "%d", (ch + 1));
		psy_ui_textout(g, cpx, 0, text, strlen(text));
		active = (*self->channelmap) & (0x01 << ch);
		if (active) {
			psy_ui_textout(g, cpx, lineheight, ".", strlen("."));
		}
	}
}

void midiactivechannelbox_onpreferredsize(MidiActiveChannelBox* self,
	psy_ui_Size* limit, psy_ui_Size* rv)
{
	rv->width = psy_ui_value_makeew(3.5 * 16 + 4);
	rv->height = psy_ui_value_makeeh(2.4);
}

// MidiFlagsView
// implementation
void midiflagsview_init(MidiFlagsView* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	self->workspace = workspace;
	midiactivechannelbox_init(&self->channelmap, &self->component,
		&self->workspace->player.midiinput.stats.channelmap);
	psy_ui_component_setalign(&self->channelmap.component, psy_ui_ALIGN_TOP);
}

// MidiChannelMappingView
// prototypes
static void midichannelmappingview_ondraw(MidiChannelMappingView*, psy_ui_Graphics*);
static void midichannelmappingview_drawheader(MidiChannelMappingView*,
	psy_ui_Graphics*, int colx_px[4], int y);
static void midichannelmappingview_onpreferredsize(MidiChannelMappingView*,
	psy_ui_Size* limit, psy_ui_Size* rv);
// vtable
static psy_ui_ComponentVtable midichannelmappingview_vtable;
static bool midichannelmappingview_vtable_initialized = 0;
// implementation
static void vtable_init(MidiChannelMappingView* self)
{
	if (!midichannelmappingview_vtable_initialized) {
		midichannelmappingview_vtable = *(self->component.vtable);
		midichannelmappingview_vtable.ondraw = (psy_ui_fp_ondraw)
			midichannelmappingview_ondraw;
		midichannelmappingview_vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			midichannelmappingview_onpreferredsize;
		midichannelmappingview_vtable_initialized = TRUE;
	}
}

void midichannelmappingview_init(MidiChannelMappingView* self,
	psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &midichannelmappingview_vtable;
	self->workspace = workspace;
	self->colx[0] = psy_ui_value_makeew(0.0);
	self->colx[1] = psy_ui_value_makeew(10.0);
	self->colx[2] = psy_ui_value_makeew(30.0);
	self->colx[3] = psy_ui_value_makeew(50.0);
}

void midichannelmappingview_ondraw(MidiChannelMappingView* self, psy_ui_Graphics* g)
{
	psy_audio_MidiInput* midiinput;
	int ch;
	int cpy;
	int colx_px[4];
	psy_ui_TextMetric tm;
	int i;
	int lineheight;

	tm = psy_ui_component_textmetric(&self->component);
	for (i = 0; i < 4; ++i) {
		colx_px[i] = psy_ui_value_px(&self->colx[i], &tm);
	}
	lineheight = (int)(tm.tmHeight * 1.2);
	midichannelmappingview_drawheader(self, g, colx_px, 0);
	midiinput = &self->workspace->player.midiinput;
	for (ch = 0, cpy = lineheight; ch < psy_audio_MAX_MIDI_CHANNELS; ++ch,
			cpy += lineheight) {
		char text[256];
		uintptr_t selidx;
		int inst;
		psy_audio_Machine* machine;
				
		machine = NULL;		
		psy_snprintf(text, 256, "Ch %d", (ch + 1));
		psy_ui_textout(g, colx_px[0], cpy, text, strlen(text));
		//Generator/effect selector
		selidx = UINTPTR_MAX;
		switch (midiinput->midiconfig.gen_select_with) {
			case psy_audio_MIDICONFIG_MS_USE_SELECTED:
				if (self->workspace->song) {
					selidx = psy_audio_machines_slot(&self->workspace->song->machines);
				}
				break;
			case psy_audio_MIDICONFIG_MS_BANK:
			case psy_audio_MIDICONFIG_MS_PROGRAM:
				selidx = psy_audio_midiinput_genmap(midiinput, ch);
				break;
			case psy_audio_MIDICONFIG_MS_MIDI_CHAN:
				selidx = ch;
				break;
			default:
				selidx = UINTPTR_MAX;
				break;
		}
		if (self->workspace->song && selidx != UINTPTR_MAX) {
			machine = psy_audio_machines_at(&self->workspace->song->machines,
				selidx);
			if (machine) {
				psy_ui_textout(g, colx_px[1], cpy,
					psy_audio_machine_editname(machine),
					strlen(psy_audio_machine_editname(machine)));
			} else {
				psy_ui_textout(g, colx_px[1], cpy, "-", strlen("-"));
			}
		} else {
			psy_ui_textout(g, colx_px[1], cpy, "-", strlen("-"));
		}
		//instrument selection
		inst = -1;
		if (inst == -1) {
			switch (midiinput->midiconfig.inst_select_with)
			{
			case psy_audio_MIDICONFIG_MS_USE_SELECTED:
				if (self->workspace->song) {
					psy_audio_InstrumentIndex instidx;

					instidx = psy_audio_instruments_selected(&self->workspace->song->instruments);
					selidx = instidx.subslot;
				}
				break;
			case psy_audio_MIDICONFIG_MS_BANK:
			case psy_audio_MIDICONFIG_MS_PROGRAM:
				selidx = psy_audio_midiinput_instmap(midiinput, ch);
				break;
			case psy_audio_MIDICONFIG_MS_MIDI_CHAN:
				selidx = ch;
				break;
			}
		} else {
			selidx = inst;
		}
		if (machine && machine_supports(machine, MACH_SUPPORTS_INSTRUMENTS) &&
				selidx >= 0 && selidx < MAX_INSTRUMENTS) { // pMachine->NumAuxColumnIndexes())		
			psy_snprintf(text, 256, "%02X", selidx);
		} else { psy_snprintf(text, 256, "-"); }
		psy_ui_textout(g, colx_px[2], cpy, text, strlen(text));
		psy_ui_textout(g, colx_px[3], cpy, "Yes", strlen("Yes"));
	}
}

void midichannelmappingview_drawheader(MidiChannelMappingView* self,
	psy_ui_Graphics* g, int colx_px[4], int y)
{
	psy_ui_textout(g, colx_px[0], y, "Channel", strlen("Channel"));
	psy_ui_textout(g, colx_px[1], y, "Generator/Effect", strlen("Generator/Effect"));
	psy_ui_textout(g, colx_px[2], y, "Instrument", strlen("Instrument"));
	psy_ui_textout(g, colx_px[3], y, "Note Off", strlen("Note Off"));
}

void midichannelmappingview_onpreferredsize(MidiChannelMappingView* self,
	psy_ui_Size* limit, psy_ui_Size* rv)
{
	rv->width = psy_ui_value_makeew(62.0);
	rv->height = psy_ui_value_makeeh(20.0);
}

// MidiMonitor
// prototypes
static void midimonitor_initcorestatus(MidiMonitor*);
static void midimonitor_inittitle(MidiMonitor*);
static void midimonitor_initcorestatus(MidiMonitor*);
static void midimonitor_initcorestatusleft(MidiMonitor*);
static void midimonitor_initcorestatusright(MidiMonitor*);
static void midimonitor_initflags(MidiMonitor*);
static void midimonitor_initchannelmapping(MidiMonitor*);
static void midimonitor_onsongchanged(MidiMonitor*, Workspace*, int flag,
	psy_audio_SongFile* songfile);
static void midimonitor_onmachineslotchange(MidiMonitor* self, psy_audio_Machines* sender,
	uintptr_t slot);
static void midimonitor_ontimer(MidiMonitor*, uintptr_t timerid);
static void midimonitor_updatechannelmap(MidiMonitor*);
static void midimonitor_onhide(MidiMonitor*);

// vtable
static psy_ui_ComponentVtable midimonitor_vtable;
static bool midimonitor_vtable_initialized = FALSE;

static void midimonitor_vtable_init(MidiMonitor* self)
{
	if (!midimonitor_vtable_initialized) {
		midimonitor_vtable = *(self->component.vtable);		
		midimonitor_vtable.ontimer = (psy_ui_fp_ontimer)midimonitor_ontimer;
		midimonitor_vtable_initialized = TRUE;
	}
}
// implementation
void midimonitor_init(MidiMonitor* self, psy_ui_Component* parent, Workspace*
	workspace)
{	
	psy_ui_component_init(&self->component, parent);
	midimonitor_vtable_init(self);
	self->component.vtable = &midimonitor_vtable;
	midimonitor_inittitle(self);
	psy_ui_component_init(&self->top, &self->component);
	psy_ui_margin_init(&self->topmargin);
	psy_ui_component_setalign(&self->top, psy_ui_ALIGN_TOP);
	self->workspace = workspace;
	self->channelstatcounter = 0;
	//midimonitor_initcorestatus(self);
	//midimonitor_initcorestatusleft(self);
	//midimonitor_initcorestatusright(self);
	midimonitor_initflags(self);
	midimonitor_initchannelmapping(self);
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void midimonitor_inittitle(MidiMonitor* self)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(0.5),
		psy_ui_value_makepx(0));
	// titlebar
	psy_ui_component_init(&self->titlebar, &self->component);
	psy_ui_component_setalign(&self->titlebar, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->titlebar, &margin);
	psy_ui_label_init(&self->title, &self->titlebar);
	psy_ui_label_settext(&self->title, "Psycle MIDI Monitor");
	psy_ui_component_setalign(&self->title.component, psy_ui_ALIGN_CLIENT);
	psy_ui_button_init(&self->hide, &self->titlebar);
	psy_ui_button_settext(&self->hide, "X");
	psy_signal_connect(&self->hide.signal_clicked, self, midimonitor_onhide);
	psy_ui_component_setalign(&self->hide.component, psy_ui_ALIGN_RIGHT);	
}

void midimonitor_initcorestatus(MidiMonitor* self)
{
	psy_ui_label_init(&self->coretitle, &self->top);
	psy_ui_label_settext(&self->coretitle, "Core Status");
	psy_ui_component_setminimumsize(&self->coretitle.component,
		psy_ui_size_make(psy_ui_value_makepx(0), psy_ui_value_makeeh(2.0)));
	psy_ui_component_setalign(&self->coretitle.component,
		psy_ui_ALIGN_TOP);
}

void midimonitor_initcorestatusleft(MidiMonitor* self)
{	
	psy_ui_component_init(&self->resources, &self->top);
	psy_ui_component_enablealign(&self->resources);
	psy_ui_component_setalign(&self->resources, psy_ui_ALIGN_LEFT);
	psy_ui_label_init(&self->resourcestitle, &self->resources);
	psy_ui_label_settext(&self->resourcestitle, "Core Status");		
	labelpair_init(&self->resources_win, &self->resources, "Buffer Used (events)");
	labelpair_init(&self->resources_mem, &self->resources, "Buffer capacity (events)");
	labelpair_init(&self->resources_swap, &self->resources, "Events lost");
	labelpair_init(&self->resources_vmem, &self->resources, "MIDI headroom (ms)");
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->resources, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_TOP,
		&self->topmargin));
}

void midimonitor_initcorestatusright(MidiMonitor* self)
{	
	psy_ui_component_init(&self->performance, &self->top);
	psy_ui_component_enablealign(&self->performance);
	psy_ui_component_setalign(&self->performance, psy_ui_ALIGN_LEFT);
	psy_ui_checkbox_init(&self->cpucheck, &self->performance);
	psy_ui_checkbox_settext(&self->cpucheck, "");	
	labelpair_init(&self->audiothreads, &self->performance, "Internal MIDI Version");
	labelpair_init(&self->totaltime, &self->performance, "MIDI clock deviation (ms)");
	labelpair_init(&self->machines, &self->performance, "Audio latency (sampl.)");
	labelpair_init(&self->routing, &self->performance, "Sync Offset (ms)");
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->performance, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_TOP,
		&self->topmargin));
}

void midimonitor_initflags(MidiMonitor* self)
{	
	self->lastchannelmap = 0;
	self->channelstatcounter = 0;
	psy_ui_label_init(&self->flagtitle, &self->component);
	psy_ui_label_settext(&self->flagtitle, "Flags");
	psy_ui_component_setminimumsize(&self->flagtitle.component,
		psy_ui_size_make(psy_ui_value_makepx(0), psy_ui_value_makeeh(2.0)));	
	psy_ui_component_setalign(&self->flagtitle.component,
		psy_ui_ALIGN_TOP);
	midiflagsview_init(&self->flags, &self->component, self->workspace);
	psy_ui_component_setalign(&self->flags.component, psy_ui_ALIGN_TOP);	
}

void midimonitor_initchannelmapping(MidiMonitor* self)
{	
	psy_ui_label_init(&self->channelmappingtitle, &self->component);
	psy_ui_label_settext(&self->channelmappingtitle, "Channel Mapping");	
	psy_ui_component_setminimumsize(&self->channelmappingtitle.component,
		psy_ui_size_make(psy_ui_value_makepx(0), psy_ui_value_makeeh(2.0)));
	psy_ui_component_setalign(&self->channelmappingtitle.component,
		psy_ui_ALIGN_TOP);	
	midichannelmappingview_init(&self->channelmapping, &self->component,
		self->workspace);
	psy_ui_component_setoverflow(&self->channelmapping.component,
		psy_ui_OVERFLOW_VSCROLL);
	psy_ui_scroller_init(&self->scroller, &self->channelmapping.component,
		&self->component);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->workspace->signal_songchanged, self,
		midimonitor_onsongchanged);	
	if (self->workspace->song) {
		psy_signal_connect(&self->workspace->song->machines.signal_slotchange,
			self, midimonitor_onmachineslotchange);
	}
}

void midimonitor_onsongchanged(MidiMonitor* self, Workspace* sender, int flag,
	psy_audio_SongFile* songfile)
{
	self->channelmapupdate =
		self->workspace->player.midiinput.stats.channelmapupdate - 1;
	if (sender->song) {
		psy_signal_connect(&sender->song->machines.signal_slotchange, self,
			midimonitor_onmachineslotchange);
	}
}

void midimonitor_ontimer(MidiMonitor* self, uintptr_t timerid)
{
	if (psy_ui_component_visible(&self->component)) {
		if (self->channelstatcounter > 0) {
			--self->channelstatcounter;
			if (self->channelstatcounter == 0) {
				self->workspace->player.midiinput.stats.channelmap = 0;
			}
		}
		if (self->channelmapupdate !=
				self->workspace->player.midiinput.stats.channelmapupdate) {
			psy_ui_component_invalidate(&self->channelmapping.component);
			self->channelmapupdate =
				self->workspace->player.midiinput.stats.channelmapupdate;			
		}
		if (self->lastchannelmap != self->workspace->player.midiinput.stats.channelmap) {
			self->channelstatcounter = 5;			
			self->lastchannelmap = self->workspace->player.midiinput.stats.channelmap;
			psy_ui_component_invalidate(&self->flags.channelmap.component);
		}
	}
}

void midimonitor_onmachineslotchange(MidiMonitor* self, psy_audio_Machines* sender,
	uintptr_t slot)
{	
	midimonitor_updatechannelmap(self);	
}

void midimonitor_updatechannelmap(MidiMonitor* self)
{
	self->channelmapupdate =
		self->workspace->player.midiinput.stats.channelmapupdate - 1;
}

void midimonitor_onhide(MidiMonitor* self)
{
	psy_ui_component_hide(&self->component);
	psy_ui_component_align(psy_ui_component_parent(&self->component));
}
