// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "midimonitor.h"
// host
#include "resources/resource.h"
#include "styles.h"
// audio
#include <patterns.h>
#include <songio.h>
// platform
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
		midiactivechannelbox_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			midiactivechannelbox_ondraw;
		midiactivechannelbox_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			midiactivechannelbox_onpreferredsize;
		midiactivechannelbox_vtable_initialized = TRUE;
	}
}
// implementation
void midiactivechannelbox_init(MidiActiveChannelBox* self,
	psy_ui_Component* parent, uint32_t* channelmap)
{
	psy_ui_component_init(&self->component, parent, NULL);
	midiactivechannelbox_vtable_init(self);
	self->component.vtable = &midiactivechannelbox_vtable;
	self->channelmap = channelmap;
}

void midiactivechannelbox_ondraw(MidiActiveChannelBox* self,
	psy_ui_Graphics* g)
{
	uintptr_t ch;
	double cpx;
	const psy_ui_TextMetric* tm;
	psy_ui_Value colew;
	double headercolw_px;
	double colw_px;
	double lineheight;
	
	tm = psy_ui_component_textmetric(&self->component);
	colew = psy_ui_value_make_ew(4);
	headercolw_px = psy_ui_value_px(&colew, tm, NULL);
	lineheight = (int)(tm->tmHeight * 1.2);
	psy_ui_textout(g, 0, 0, "Ch:", psy_strlen("Ch:"));
	colew = psy_ui_value_make_ew(3.5);
	colw_px = psy_ui_value_px(&colew, tm, NULL);
	for (ch = 0, cpx = headercolw_px; ch < psy_audio_MAX_MIDI_CHANNELS; ++ch,
			cpx += colw_px) {
		char text[256];
		bool active;

		psy_snprintf(text, 256, "%d", (ch + 1));
		psy_ui_textout(g, cpx, 0, text, psy_strlen(text));
		active = (*self->channelmap) & (0x01 << ch);
		if (active) {
			psy_ui_textout(g, cpx, lineheight, ".", psy_strlen("."));
		}
	}
}

void midiactivechannelbox_onpreferredsize(MidiActiveChannelBox* self,
	psy_ui_Size* limit, psy_ui_Size* rv)
{
	rv->width = psy_ui_value_make_ew(3.5 * 16 + 4);
	rv->height = psy_ui_value_make_eh(2.4);
}

// MidiActiveClockBox
// prototypes
static void midiactiveclockbox_ondraw(MidiActiveClockBox*, psy_ui_Graphics*);
static void midiactiveclockbox_onpreferredsize(MidiActiveClockBox*,
	psy_ui_Size* limit, psy_ui_Size* rv);
// vtable
static psy_ui_ComponentVtable midiactiveclockbox_vtable;
static bool midiactiveclockbox_vtable_initialized = FALSE;

static void midiactiveclockbox_vtable_init(MidiActiveClockBox* self)
{
	if (!midiactiveclockbox_vtable_initialized) {
		midiactiveclockbox_vtable = *(self->component.vtable);
		midiactiveclockbox_vtable.ondraw = (psy_ui_fp_component_ondraw)
			midiactiveclockbox_ondraw;
		midiactiveclockbox_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			midiactiveclockbox_onpreferredsize;
		midiactiveclockbox_vtable_initialized = TRUE;
	}
}
// implementation
void midiactiveclockbox_init(MidiActiveClockBox* self,
	psy_ui_Component* parent, uint32_t* flags)
{
	psy_ui_component_init(&self->component, parent, NULL);
	midiactiveclockbox_vtable_init(self);
	self->component.vtable = &midiactiveclockbox_vtable;
	self->flags = flags;
}

void midiactiveclockbox_ondraw(MidiActiveClockBox* self,
	psy_ui_Graphics* g)
{	
	const psy_ui_TextMetric* tm;
	psy_ui_Value colew;
	double headercolw_px;
	double colw_px;
	double lineheight;

	tm = psy_ui_component_textmetric(&self->component);
	colew = psy_ui_value_make_ew(20);
	headercolw_px = psy_ui_value_px(&colew, tm, NULL);
	colew = psy_ui_value_make_ew(3.5);
	colw_px = psy_ui_value_px(&colew, tm, NULL);
	lineheight = (int)(tm->tmHeight * 1.2);
	psy_ui_textout(g, 0, 0,     "MIDI Sync: START", psy_strlen("MIDI Sync: START"));
	psy_ui_textout(g, 0, lineheight, "MIDI Sync: CLOCK", psy_strlen("MIDI Sync: CLOCK"));
	psy_ui_textout(g, 0, lineheight * 2, "MIDI Sync: STOP", psy_strlen("MIDI Sync: STOP"));
	if ((*self->flags & FSTAT_FASTART) == FSTAT_FASTART) {
		psy_ui_textout(g, headercolw_px, 0, ".", psy_strlen("."));
	}
	if ((*self->flags & FSTAT_F8CLOCK) == FSTAT_F8CLOCK) {
		psy_ui_textout(g, headercolw_px, lineheight, ".", psy_strlen("."));
	}
	if ((*self->flags & FSTAT_FCSTOP) == FSTAT_FCSTOP) {
		psy_ui_textout(g, headercolw_px, lineheight * 2, ".", psy_strlen("."));
	}
}

void midiactiveclockbox_onpreferredsize(MidiActiveClockBox* self,
	psy_ui_Size* limit, psy_ui_Size* rv)
{
	rv->width = psy_ui_value_make_ew(22);
	rv->height = psy_ui_value_make_eh(3 * 1.2);
}

// MidiFlagsView
// implementation
void midiflagsview_init(MidiFlagsView* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;
	midiactiveclockbox_init(&self->clock, &self->component,
		&workspace_player(self->workspace)->midiinput.stats.flags);
	psy_ui_component_setalign(&self->clock.component, psy_ui_ALIGN_TOP);
	midiactivechannelbox_init(&self->channelmap, &self->component,
		&workspace_player(self->workspace)->midiinput.stats.channelmap);
	psy_ui_component_setalign(&self->channelmap.component, psy_ui_ALIGN_TOP);
	
}

// MidiChannelMappingView
// prototypes
static void midichannelmappingview_ondraw(MidiChannelMappingView*, psy_ui_Graphics*);
static void midichannelmappingview_drawheader(MidiChannelMappingView*,
	psy_ui_Graphics*, double colx_px[4], double y);
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
		midichannelmappingview_vtable.ondraw = (psy_ui_fp_component_ondraw)
			midichannelmappingview_ondraw;
		midichannelmappingview_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			midichannelmappingview_onpreferredsize;
		midichannelmappingview_vtable_initialized = TRUE;
	}
}

void midichannelmappingview_init(MidiChannelMappingView* self,
	psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	self->component.vtable = &midichannelmappingview_vtable;
	self->workspace = workspace;
	self->colx[0] = psy_ui_value_make_ew(0.0);
	self->colx[1] = psy_ui_value_make_ew(10.0);
	self->colx[2] = psy_ui_value_make_ew(30.0);
	self->colx[3] = psy_ui_value_make_ew(50.0);
}

void midichannelmappingview_ondraw(MidiChannelMappingView* self, psy_ui_Graphics* g)
{
	psy_audio_MidiInput* midiinput;
	intptr_t ch;
	double cpy;
	double colx_px[4];
	const psy_ui_TextMetric* tm;
	intptr_t i;
	double lineheight;

	tm = psy_ui_component_textmetric(&self->component);
	for (i = 0; i < 4; ++i) {
		colx_px[i] = psy_ui_value_px(&self->colx[i], tm, NULL);
	}
	lineheight = (int)(tm->tmHeight * 1.2);
	midichannelmappingview_drawheader(self, g, colx_px, 0);
	midiinput = &workspace_player(self->workspace)->midiinput;
	for (ch = 0, cpy = lineheight; ch < psy_audio_MAX_MIDI_CHANNELS; ++ch,
			cpy += lineheight) {
		char text[256];
		uintptr_t selidx;
		int inst;
		psy_audio_Machine* machine;
				
		machine = NULL;	
		if (psy_audio_midiinput_genmap(midiinput, ch) != psy_INDEX_INVALID) {
			psy_ui_settextcolour(g, psy_ui_component_colour(&self->component));
		} else {
			psy_ui_settextcolour(g, psy_ui_colour_make(0x00444444));
		}
		psy_snprintf(text, 256, "Ch %d", (ch + 1));
		psy_ui_textout(g, colx_px[0], cpy, text, psy_strlen(text));
		//Generator/effect selector
		selidx = psy_INDEX_INVALID;
		switch (midiinput->midiconfig.gen_select_with) {
			case psy_audio_MIDICONFIG_MS_USE_SELECTED:
				if (workspace_song(self->workspace)) {
					selidx = psy_audio_machines_selected(&workspace_song(self->workspace)->machines);
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
				selidx = psy_INDEX_INVALID;
				break;
		}
		if (workspace_song(self->workspace) && selidx != psy_INDEX_INVALID) {
			machine = psy_audio_machines_at(&workspace_song(self->workspace)->machines,
				selidx);
			if (machine) {
				psy_ui_textout(g, colx_px[1], cpy,
					psy_audio_machine_editname(machine),
					psy_strlen(psy_audio_machine_editname(machine)));
			} else {
				psy_ui_textout(g, colx_px[1], cpy, "-", psy_strlen("-"));
			}
		} else {
			psy_ui_textout(g, colx_px[1], cpy, "-", psy_strlen("-"));
		}
		//instrument selection
		inst = -1;
		if (inst == -1) {
			switch (midiinput->midiconfig.inst_select_with)
			{
			case psy_audio_MIDICONFIG_MS_USE_SELECTED:
				if (workspace_song(self->workspace)) {
					psy_audio_InstrumentIndex instidx;

					instidx = psy_audio_instruments_selected(
						&workspace_song(self->workspace)->instruments);
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
		if (machine && machine_supports(machine, psy_audio_SUPPORTS_INSTRUMENTS) &&
				selidx >= 0 && selidx < MAX_INSTRUMENTS) { // pMachine->NumAuxColumnIndexes())		
			psy_snprintf(text, 256, "%02X", selidx);
		} else { psy_snprintf(text, 256, "-"); }
		psy_ui_textout(g, colx_px[2], cpy, text, psy_strlen(text));
		psy_ui_textout(g, colx_px[3], cpy, "Yes", psy_strlen("Yes"));
	}
}

void midichannelmappingview_drawheader(MidiChannelMappingView* self,
	psy_ui_Graphics* g, double colx_px[4], double y)
{
	psy_ui_textout(g, colx_px[0], y, "Channel", psy_strlen("Channel"));
	psy_ui_textout(g, colx_px[1], y, "Generator/Effect", psy_strlen("Generator/Effect"));
	psy_ui_textout(g, colx_px[2], y, "Instrument", psy_strlen("Instrument"));
	psy_ui_textout(g, colx_px[3], y, "Note Off", psy_strlen("Note Off"));
}

void midichannelmappingview_onpreferredsize(MidiChannelMappingView* self,
	psy_ui_Size* limit, psy_ui_Size* rv)
{
	rv->width = psy_ui_value_make_ew(62.0);
	rv->height = psy_ui_value_make_eh(20.0);
}

// MidiMonitor
// prototypes
static void midimonitor_initcorestatus(MidiMonitor*);
static void midimonitor_inittitlebar(MidiMonitor*);
static void midimonitor_initcorestatus(MidiMonitor*);
static void midimonitor_initcorestatusleft(MidiMonitor*);
static void midimonitor_initcorestatusright(MidiMonitor*);
static void midimonitor_initflags(MidiMonitor*);
static void midimonitor_initchannelmapping(MidiMonitor*);
static void midimonitor_onsongchanged(MidiMonitor*, Workspace*, int flag,
	psy_audio_Song* song);
static void midimonitor_onmachineslotchange(MidiMonitor* self, psy_audio_Machines* sender,
	uintptr_t slot);
static void midimonitor_ontimer(MidiMonitor*, uintptr_t timerid);
static void midimonitor_updatechannelmap(MidiMonitor*);
static void midimonitor_onhide(MidiMonitor*);
static void midimonitor_onconfigure(MidiMonitor*);
static void midimonitor_onmapconfigure(MidiMonitor*);
// vtable
static psy_ui_ComponentVtable midimonitor_vtable;
static bool midimonitor_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* midimonitor_vtable_init(MidiMonitor* self)
{
	if (!midimonitor_vtable_initialized) {
		midimonitor_vtable = *(self->component.vtable);		
		midimonitor_vtable.ontimer = (psy_ui_fp_component_ontimer)
			midimonitor_ontimer;	
		midimonitor_vtable_initialized = TRUE;
	}
	return &midimonitor_vtable;
}
// implementation
void midimonitor_init(MidiMonitor* self, psy_ui_Component* parent, Workspace*
	workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	midimonitor_vtable_init(self);
	psy_ui_component_setvtable(midimonitor_base(self), midimonitor_vtable_init(self));
	psy_ui_component_setstyletype(&self->component,
		STYLE_RECENTVIEW_MAINSECTION);
	midimonitor_inittitlebar(self);
	psy_ui_component_init(&self->client, midimonitor_base(self), NULL);
	psy_ui_component_setalign(&self->client, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setmargin(&self->client,
		psy_ui_defaults_cmargin(psy_ui_defaults()));	
	psy_ui_component_init(&self->top, &self->client, NULL);
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

void midimonitor_inittitlebar(MidiMonitor* self)
{		
	titlebar_init(&self->titlebar, &self->component, NULL,
		"Psycle MIDI Monitor");		
	titlebar_hideonclose(&self->titlebar);
	psy_ui_button_init_text(&self->configure, &self->titlebar.client, NULL,
		"Devices");
	psy_ui_button_loadresource(&self->configure, IDB_SETTINGS_DARK,
		psy_ui_colour_white());
	psy_ui_component_setalign(&self->configure.component, psy_ui_ALIGN_LEFT);
	psy_signal_connect(&self->configure.signal_clicked, self, midimonitor_onconfigure);			
}

void midimonitor_initcorestatus(MidiMonitor* self)
{
	psy_ui_label_init_text(&self->coretitle, &self->top, NULL,
		"Core Status");
	psy_ui_component_setminimumsize(&self->coretitle.component,
		psy_ui_size_make_em(0.0, 2.0));
	psy_ui_component_setalign(&self->coretitle.component,
		psy_ui_ALIGN_TOP);
}

void midimonitor_initcorestatusleft(MidiMonitor* self)
{	
	psy_ui_component_init(&self->resources, &self->top, NULL);
	psy_ui_component_setalign(&self->resources, psy_ui_ALIGN_LEFT);
	psy_ui_label_init_text(&self->resourcestitle, &self->resources, NULL,
		"Core Status");		
	labelpair_init(&self->resources_win, &self->resources, "Buffer Used (events)", 25.0);
	labelpair_init(&self->resources_mem, &self->resources, "Buffer capacity (events)", 25.0);
	labelpair_init(&self->resources_swap, &self->resources, "Events lost", 25.0);
	labelpair_init(&self->resources_vmem, &self->resources, "MIDI headroom (ms)", 25.0);
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->resources, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_TOP,
		self->topmargin));
}

void midimonitor_initcorestatusright(MidiMonitor* self)
{	
	psy_ui_component_init(&self->performance, &self->top, NULL);
	psy_ui_component_setalign(&self->performance, psy_ui_ALIGN_LEFT);
	psy_ui_checkbox_init(&self->cpucheck, &self->performance);
	psy_ui_checkbox_settext(&self->cpucheck, "");	
	labelpair_init(&self->audiothreads, &self->performance, "Internal MIDI Version", 25.0);
	labelpair_init(&self->totaltime, &self->performance, "MIDI clock deviation (ms)", 25.0);
	labelpair_init(&self->machines, &self->performance, "Audio latency (sampl.)", 25.0);
	labelpair_init(&self->routing, &self->performance, "Sync Offset (ms)", 25.0);
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->performance, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_TOP,
		self->topmargin));
}

void midimonitor_initflags(MidiMonitor* self)
{	
	self->lastchannelmap = 0;
	self->channelstatcounter = 0;
	psy_ui_label_init_text(&self->flagtitle, &self->client, NULL, "Flags");
	psy_ui_component_setminimumsize(&self->flagtitle.component,
		psy_ui_size_make_em(0.0, 2.0));	
	psy_ui_component_setalign(&self->flagtitle.component,
		psy_ui_ALIGN_TOP);
	midiflagsview_init(&self->flags, &self->client, self->workspace);
	psy_ui_component_setalign(&self->flags.component, psy_ui_ALIGN_TOP);	
}

void midimonitor_initchannelmapping(MidiMonitor* self)
{	
	psy_ui_component_init(&self->topchannelmapping, &self->client, NULL);
	psy_ui_component_setalign(&self->topchannelmapping, psy_ui_ALIGN_TOP);
	psy_ui_component_setminimumsize(&self->topchannelmapping,
		psy_ui_size_make_em(0.0, 2.0));
	psy_ui_label_init_text(&self->channelmappingtitle,
		&self->topchannelmapping, NULL, "Channel Mapping");
	psy_ui_component_setalign(&self->channelmappingtitle.component,
		psy_ui_ALIGN_LEFT);
	psy_ui_button_init(&self->mapconfigure, &self->topchannelmapping, NULL);
	psy_ui_bitmap_loadresource(&self->mapconfigure.bitmapicon, IDB_SETTINGS_DARK);
	psy_ui_bitmap_settransparency(&self->mapconfigure.bitmapicon, psy_ui_colour_make(0x00FFFFFF));
	psy_signal_connect(&self->mapconfigure.signal_clicked, self, midimonitor_onmapconfigure);
	psy_ui_component_setalign(&self->mapconfigure.component, psy_ui_ALIGN_LEFT);
	midichannelmappingview_init(&self->channelmapping, &self->client,
		self->workspace);
	psy_ui_component_setoverflow(&self->channelmapping.component,
		psy_ui_OVERFLOW_VSCROLL);
	psy_ui_scroller_init(&self->scroller, &self->channelmapping.component,
		&self->client, NULL);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->workspace->signal_songchanged, self,
		midimonitor_onsongchanged);	
	if (workspace_song(self->workspace)) {
		psy_signal_connect(&workspace_song(self->workspace)->machines.signal_slotchange,
			self, midimonitor_onmachineslotchange);
	}
}

void midimonitor_onsongchanged(MidiMonitor* self, Workspace* sender, int flag,
	psy_audio_Song* song)
{
	self->channelmapupdate =
		workspace_player(self->workspace)->midiinput.stats.channelmapupdate - 1;
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
				workspace_player(self->workspace)->midiinput.stats.channelmap = 0;
			}
		}
		if (self->flagstatcounter > 0) {
			--self->flagstatcounter;
			if (self->flagstatcounter == 0) {
				workspace_player(self->workspace)->midiinput.stats.flags = 0;
			}
		}
		if (self->channelmapupdate !=
				workspace_player(self->workspace)->midiinput.stats.channelmapupdate) {
			psy_ui_component_invalidate(&self->channelmapping.component);			
			self->channelmapupdate =
				workspace_player(self->workspace)->midiinput.stats.channelmapupdate;
		}
		if (self->lastchannelmap != workspace_player(self->workspace)->midiinput.stats.channelmap) {
			self->channelstatcounter = 5;			
			self->lastchannelmap = workspace_player(self->workspace)->midiinput.stats.channelmap;
			psy_ui_component_invalidate(&self->flags.channelmap.component);
		}
		if (self->lastflags != workspace_player(self->workspace)->midiinput.stats.flags) {
			self->flagstatcounter = 5;
			self->lastflags = workspace_player(self->workspace)->midiinput.stats.flags;
			psy_ui_component_invalidate(&self->flags.clock.component);
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
		workspace_player(self->workspace)->midiinput.stats.channelmapupdate - 1;
}

void midimonitor_onhide(MidiMonitor* self)
{
	psy_ui_component_hide(&self->component);
	psy_ui_component_align(psy_ui_component_parent(&self->component));
}

void midimonitor_onconfigure(MidiMonitor* self)
{	
	workspace_selectview(self->workspace, VIEW_ID_SETTINGSVIEW, 6, 0);
}

void midimonitor_onmapconfigure(MidiMonitor* self)
{
	workspace_selectview(self->workspace, VIEW_ID_SETTINGSVIEW, 7, 0);
}
