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

static void cpumoduleview_ondraw(CPUModuleView*, psy_ui_Graphics*);
static void cpumoduleview_onsize(CPUModuleView*, psy_ui_Size*);
static void cpumoduleview_adjustscroll(CPUModuleView*);
static void cpumoduleview_onscroll(CPUModuleView*, psy_ui_Component* sender,
	int stepx, int stepy);

static psy_ui_ComponentVtable cpumoduleview_vtable;
static int cpumoduleview_vtable_initialized = 0;

static void cpumoduleview_vtable_init(CPUModuleView* self)
{
	if (!cpumoduleview_vtable_initialized) {
		cpumoduleview_vtable = *(self->component.vtable);
		cpumoduleview_vtable.ondraw = (psy_ui_fp_ondraw)cpumoduleview_ondraw;
		cpumoduleview_vtable.onsize = (psy_ui_fp_onsize)cpumoduleview_onsize;
		cpumoduleview_vtable_initialized = 1;
	}
}

void cpumoduleview_init(CPUModuleView* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	cpumoduleview_vtable_init(self);
	self->component.vtable = &cpumoduleview_vtable;
	self->workspace = workspace;
	self->dy = 0;
	psy_ui_component_showverticalscrollbar(&self->component);
	psy_ui_component_setwheelscroll(&self->component, 4);
	psy_signal_connect(&self->component.signal_scroll, self,
		cpumoduleview_onscroll);
}

void cpumoduleview_ondraw(CPUModuleView* self, psy_ui_Graphics* g)
{
	if (self->workspace->song) {
		uintptr_t slot;
		int cpy = 0;
		psy_ui_TextMetric tm;
		psy_ui_IntSize size;

		tm = psy_ui_component_textmetric(&self->component);
		size = psy_ui_intsize_init_size(
			psy_ui_component_size(&self->component), &tm);
		psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
		psy_ui_settextcolor(g, 0x00D1C5B6);
		for (slot = 0; slot <= psy_audio_machines_maxindex(
				&self->workspace->song->machines); ++slot) {
			psy_audio_Machine* machine;

			machine = psy_audio_machines_at(&self->workspace->song->machines,
				slot);
			if (machine) {
				if (cpy + self->dy >= 0) {
					char text[20];
					const psy_audio_MachineInfo* info;

					info = psy_audio_machine_info(machine);
					psy_snprintf(text, 20, "%d", (int)slot);
					psy_ui_textout(g, 0, cpy + self->dy, text, strlen(text));
					psy_ui_textout(g, tm.tmAveCharWidth * 5, cpy + self->dy,
						psy_audio_machine_editname(machine),
						min(strlen(psy_audio_machine_editname(machine)), 14));
					psy_ui_textout(g, tm.tmAveCharWidth * 21, cpy + self->dy,
						info->Name, strlen(info->Name));					
					if (cpy + self->dy > size.height) {
						//break;
					}
				}
				cpy += tm.tmHeight;
			}
		}
	}
}

void cpumoduleview_onsize(CPUModuleView* self, psy_ui_Size* size)
{
	cpumoduleview_adjustscroll(self);
}

void cpumoduleview_adjustscroll(CPUModuleView* self)
{
	psy_ui_Size size;
	psy_ui_TextMetric tm;
	int visilines;
	int currlines;

	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	visilines = psy_ui_value_px(&size.height, &tm) / tm.tmHeight;
	if (self->workspace->song) {
		currlines = psy_audio_machines_size(&self->workspace->song->machines);
	} else {
		currlines = 0;
	}		
	self->component.scrollstepy = tm.tmHeight;
	if (currlines > visilines) {
		psy_ui_component_setverticalscrollrange(&self->component,
			0, currlines - visilines);
	} else {
		self->dy = 0;
		psy_ui_component_setverticalscrollrange(&self->component,
			0, 0);
	}
}

void cpumoduleview_onscroll(CPUModuleView* self, psy_ui_Component* sender,
	int stepx, int stepy)
{
	self->dy += sender->scrollstepy * stepy;	
}

static void cpuview_initcoreinfo(CPUView*);
static void cpuview_inittitle(CPUView*);
static void cpuview_initresources(CPUView*);
static void cpuview_initperformance(CPUView*);
static void cpuview_initmodules(CPUView*, Workspace* workspace);
static void cpuview_ontimer(CPUView*, psy_ui_Component* sender,
	uintptr_t timerid);

void cpuview_init(CPUView* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent);	
	self->workspace = workspace;
	psy_ui_margin_init(&self->topmargin);
	psy_ui_component_enablealign(&self->component);
	psy_ui_component_init(&self->top, &self->component);
	psy_ui_component_enablealign(&self->top);
	psy_ui_component_setalign(&self->top, psy_ui_ALIGN_BOTTOM);	
	cpuview_inittitle(self);
	cpuview_initcoreinfo(self);	
	cpuview_initresources(self);
	cpuview_initperformance(self);
	cpuview_initmodules(self, workspace);
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
		psy_ui_component_children(&self->resources, psy_ui_NONRECURSIVE),
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
		psy_ui_component_children(&self->performance, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_TOP,
		&self->topmargin));
}

void cpuview_initmodules(CPUView* self, Workspace* workspace)
{
	psy_ui_Margin margin;

	cpumoduleview_init(&self->modules, &self->component, workspace);
	psy_ui_component_setalign(&self->modules.component, psy_ui_ALIGN_CLIENT);
	psy_ui_margin_init_all(&margin, psy_ui_value_makeeh(1),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_ui_component_setmargin(&self->modules.component, &margin);
}

void cpuview_ontimer(CPUView* self, psy_ui_Component* sender,
	uintptr_t timerid)
{
	uintptr_t nummachines;
#if defined(DIVERSALIS__OS__MICROSOFT) && WINVER >= 0x600
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
	if (self->workspace->song) {
		nummachines = psy_audio_machines_size(&self->workspace->song->machines);
	} else {
		nummachines = 0;
	}
	if (nummachines != self->lastnummachines) {
		self->lastnummachines = nummachines;
		cpumoduleview_adjustscroll(&self->modules);
		psy_ui_component_invalidate(&self->modules.component);
	}
}
