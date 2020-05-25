// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "midiview.h"

static void midiview_initcorestatus(MidiView*);
static void midiview_inittitle(MidiView*);
static void midiview_initcorestatus(MidiView*);
static void midiview_initcorestatusleft(MidiView*);
static void midiview_initcorestatusright(MidiView*);
static void midiview_initflags(MidiView*);

void midiview_init(MidiView* self, psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_margin_init(&self->topmargin);
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	psy_ui_component_init(&self->top, &self->component);
	psy_ui_component_enablealign(&self->top);
	psy_ui_component_setalign(&self->top, psy_ui_ALIGN_TOP);
	self->workspace = workspace;	
	midiview_inittitle(self);
	midiview_initcorestatus(self);
	midiview_initcorestatusleft(self);
	midiview_initcorestatusright(self);
	midiview_initflags(self);
}

void midiview_inittitle(MidiView* self)
{
	psy_ui_label_init(&self->title, &self->top);
	psy_ui_label_settext(&self->title, "Psycle MIDI Monitor");
	psy_ui_component_setalign(&self->title.component, psy_ui_ALIGN_TOP);
}

void midiview_initcorestatus(MidiView* self)
{
	psy_ui_label_init(&self->coreinfo, &self->top);
	psy_ui_label_settext(&self->coreinfo, "MIDI Input");
	psy_ui_component_setalign(&self->coreinfo.component, psy_ui_ALIGN_TOP);
}

void midiview_initcorestatusleft(MidiView* self)
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

void midiview_initcorestatusright(MidiView* self)
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

void midiview_initflags(MidiView* self)
{
	psy_ui_component_init(&self->performancelist, &self->component);
	psy_ui_component_enablealign(&self->performancelist);
	psy_ui_component_setalign(&self->performancelist, psy_ui_ALIGN_CLIENT);
}
