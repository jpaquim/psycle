// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "cpuview.h"

static void cpuview_initcoreinfo(CPUView*);
static void cpuview_inittitle(CPUView*);
static void cpuview_initresources(CPUView*);
static void cpuview_initperformance(CPUView*);
static void cpuview_initperformancelist(CPUView*);

void cpuview_init(CPUView* self, psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_margin_init(&self->topmargin, psy_ui_value_makepx(0), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0));
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	psy_ui_component_init(&self->top, &self->component);
	psy_ui_component_enablealign(&self->top);
	psy_ui_component_setalign(&self->top, psy_ui_ALIGN_TOP);
	self->workspace = workspace;	
	cpuview_inittitle(self);
	cpuview_initcoreinfo(self);	
	cpuview_initresources(self);
	cpuview_initperformance(self);
	cpuview_initperformancelist(self);
}

void cpuview_inittitle(CPUView* self)
{
	psy_ui_label_init(&self->title, &self->top);
	psy_ui_label_settext(&self->title, "Psycle DSP/CPU Performance Monitor");
	psy_ui_component_setalign(&self->title.component, psy_ui_ALIGN_TOP);
}

void cpuview_initcoreinfo(CPUView* self)
{
	psy_ui_label_init(&self->coreinfo, &self->top);
	psy_ui_label_settext(&self->coreinfo, "Core Info");
	psy_ui_component_setalign(&self->coreinfo.component, psy_ui_ALIGN_TOP);
}

void cpuview_initresources(CPUView* self)
{	
	psy_ui_component_init(&self->resources, &self->top);
	psy_ui_component_enablealign(&self->resources);
	psy_ui_component_setalign(&self->resources, psy_ui_ALIGN_LEFT);
	psy_ui_label_init(&self->resourcestitle, &self->resources);
	psy_ui_label_settext(&self->resourcestitle, "Available Resources");		
	labelpair_init(&self->resources_win, &self->resources, "Windows Resources");
	labelpair_init(&self->resources_mem, &self->resources, "Physical Memory(RAM)");
	labelpair_init(&self->resources_swap, &self->resources, "Page File (Swap)");
	labelpair_init(&self->resources_vmem, &self->resources, "Virtual Memory");
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->resources, 0),
		psy_ui_ALIGN_TOP, &self->topmargin));
}

void cpuview_initperformance(CPUView* self)
{	
	psy_ui_component_init(&self->performance, &self->top);
	psy_ui_component_enablealign(&self->performance);
	psy_ui_component_setalign(&self->performance, psy_ui_ALIGN_LEFT);
	psy_ui_checkbox_init(&self->cpucheck, &self->performance);
	psy_ui_checkbox_settext(&self->cpucheck, "CPU Performance");	
	labelpair_init(&self->audiothreads, &self->performance, "Audio threads");
	labelpair_init(&self->totaltime, &self->performance, "Total (time)");
	labelpair_init(&self->machines, &self->performance, "Machines");
	labelpair_init(&self->routing, &self->performance, "Routing");
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->performance, 0),
		psy_ui_ALIGN_TOP, &self->topmargin));
}

void cpuview_initperformancelist(CPUView* self)
{
	psy_ui_component_init(&self->performancelist, &self->component);
	psy_ui_component_enablealign(&self->performancelist);
	psy_ui_component_setalign(&self->performancelist, psy_ui_ALIGN_CLIENT);
}
