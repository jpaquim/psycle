// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "cpuview.h"

#define TIMERID_CPUVIEW 250

#if defined(DIVERSALIS__OS__MICROSOFT)
#include <windows.h>
#endif

#include "../../detail/portable.h"
#include "../../detail/os.h"

static void cpuview_initcoreinfo(CPUView*);
static void cpuview_inittitle(CPUView*);
static void cpuview_initresources(CPUView*);
static void cpuview_initperformance(CPUView*);
static void cpuview_initperformancelist(CPUView*);
static void cpuview_ontimer(CPUView*, psy_ui_Component* sender,
	int timerid);

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
	psy_signal_connect(&self->component.signal_timer, self,
		cpuview_ontimer);
	psy_ui_component_starttimer(&self->component, TIMERID_CPUVIEW, 200);
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

void cpuview_ontimer(CPUView* self, psy_ui_Component* sender,
	int timerid)
{
#if defined(DIVERSALIS__OS__MICROSOFT)
	MEMORYSTATUSEX lpBuffer;
	char buffer[128];

	lpBuffer.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&lpBuffer);

	psy_snprintf(buffer, 128, "%d%%", 100 - lpBuffer.dwMemoryLoad);
	psy_ui_label_settext(&self->resources_win.value, buffer);

	psy_snprintf(buffer, 128, "%.0fM (of %.0fM)",
		lpBuffer.ullAvailPhys / (float)(1 << 20),
		lpBuffer.ullTotalPhys / (float)(1 << 20));
	psy_ui_label_settext(&self->resources_mem.value, buffer);

	psy_snprintf(buffer, 128, "%.0fM (of %.0fM)",
		(lpBuffer.ullAvailPageFile / (float)(1 << 20)),
		(lpBuffer.ullTotalPageFile / (float)(1 << 20)));
	psy_ui_label_settext(&self->resources_swap.value, buffer);
#if defined _WIN64
	psy_snsprintf(buffer, 128, "%.0fG (of %.0fG)",
		(lpBuffer.ullAvailVirtual / (float)(1 << 30)),
		(lpBuffer.ullTotalVirtual / (float)(1 << 30)));
#elif defined _WIN32
	psy_snprintf(buffer, 128, "%.0fM (of %.0fM)",
		(lpBuffer.ullAvailVirtual / (float)(1 << 20)),
		(lpBuffer.ullTotalVirtual / (float)(1 << 20)));
#endif
	psy_ui_label_settext(&self->resources_vmem.value, buffer);
#endif
}
