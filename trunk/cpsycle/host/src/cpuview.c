// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "cpuview.h"

#if defined(DIVERSALIS__OS__MICROSOFT)
#include <windows.h>
#endif

#include "../../detail/portable.h"
#include "../../detail/os.h"

static void cpumoduleview_ondraw(CPUModuleView*, psy_ui_Graphics*);
static void cpumoduleview_onpreferredsize(CPUModuleView*, const psy_ui_Size* limit,
	psy_ui_Size* rv);

static psy_ui_ComponentVtable cpumoduleview_vtable;
static bool cpumoduleview_vtable_initialized = FALSE;

static void cpumoduleview_vtable_init(CPUModuleView* self)
{
	if (!cpumoduleview_vtable_initialized) {
		cpumoduleview_vtable = *(self->component.vtable);
		cpumoduleview_vtable.ondraw = (psy_ui_fp_component_ondraw)cpumoduleview_ondraw;
		cpumoduleview_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			cpumoduleview_onpreferredsize;
		cpumoduleview_vtable_initialized = TRUE;
	}
}

void cpumoduleview_init(CPUModuleView* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	cpumoduleview_vtable_init(self);	
	self->component.vtable = &cpumoduleview_vtable;
	self->workspace = workspace;
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setwheelscroll(&self->component, 4);
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_VSCROLL);
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
		psy_ui_settextcolour(g, psy_ui_colour_make(0x00D1C5B6));
		for (slot = 0; slot <= psy_audio_machines_maxindex(
				&self->workspace->song->machines); ++slot) {
			psy_audio_Machine* machine;

			machine = psy_audio_machines_at(&self->workspace->song->machines,
				slot);
			if (machine) {
				if ((cpy - psy_ui_component_scrolltop(&self->component)) >= 0) {
					char text[40];
					const psy_audio_MachineInfo* info;					

					info = psy_audio_machine_info(machine);
					if (info) {						
						psy_snprintf(text, 20, "%d", (int)slot);
						psy_ui_textout(g, 0, cpy, text, strlen(text));
						psy_ui_textout(g, tm.tmAveCharWidth * 5, cpy,
							psy_audio_machine_editname(machine),
							psy_min(strlen(psy_audio_machine_editname(machine)), 14));
						psy_ui_textout(g, tm.tmAveCharWidth * 21, cpy,
							info->Name, strlen(info->Name));
						if (psy_audio_player_measuring_cpu_usage(&self->workspace->player)) {
							psy_snprintf(text, 40, "%.1f%%", 100.0f * psy_audio_machine_cpu_time(machine).perc);						
						} else {
							psy_snprintf(text, 40, "N/A");
						}
						psy_ui_textout(g, tm.tmAveCharWidth * 60, cpy, text,
							strlen(text));
					}
					if ((cpy - psy_ui_component_scrolltop(&self->component)) > size.height) {
						break;
					}
				}
				cpy += tm.tmHeight;
			}
		}
	}
}

void cpumoduleview_onpreferredsize(CPUModuleView* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	uintptr_t currlines;
	psy_ui_Size size;
	
	if (self->workspace->song) {
		currlines = psy_audio_machines_size(&self->workspace->song->machines);
	} else {
		currlines = 0;
	}
	size = psy_ui_component_size(&self->component);
	rv->height = psy_ui_value_makepx(self->component.scrollstepy * currlines);
	rv->width = size.width;	
}

static void cpuview_initcoreinfo(CPUView*);
static void cpuview_inittitle(CPUView*);
static void cpuview_initresources(CPUView*);
static void cpuview_initperformance(CPUView*);
static void cpuview_initmodules(CPUView*, Workspace* workspace);
static void cpuview_ontimer(CPUView*, psy_ui_Component* sender,
	uintptr_t timerid);
static void cpuview_onhide(CPUView*);
static void cpuview_oncpuperf(CPUView*, psy_ui_CheckBox* sender);

void cpuview_init(CPUView* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent);	
	self->workspace = workspace;
	psy_ui_margin_init(&self->topmargin);		
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
	psy_ui_component_starttimer(&self->component, 0, 200);
}

void cpuview_inittitle(CPUView* self)
{
	psy_ui_Margin margin;
	
	// titlebar
	psy_ui_component_init(&self->titlebar, &self->component);
	psy_ui_component_setalign(&self->titlebar, psy_ui_ALIGN_TOP);
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(0.5),
		psy_ui_value_makepx(0));
	psy_ui_component_setmargin(&self->titlebar, &margin);
	psy_ui_label_init(&self->title, &self->titlebar);
	psy_ui_label_settext(&self->title, "Psycle DSP/CPU Performance Monitor");
	psy_ui_component_setcolour(&self->title.component, psy_ui_colour_make(0x00B1C8B0));
	psy_ui_component_setalign(&self->title.component, psy_ui_ALIGN_CLIENT);
	psy_ui_button_init(&self->hide, &self->titlebar);
	psy_ui_button_settext(&self->hide, "X");
	psy_signal_connect(&self->hide.signal_clicked, self, cpuview_onhide);
	psy_ui_component_setalign(&self->hide.component, psy_ui_ALIGN_RIGHT);
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(2.0), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_ui_component_setmargin(&self->hide.component, &margin);
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
	psy_signal_connect(&self->cpucheck.signal_clicked, self,
		cpuview_oncpuperf);
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
	psy_ui_scroller_init(&self->scroller, &self->modules.component,
		&self->component);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
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

	if (!psy_ui_component_visible(&self->component)) {
		return;
	}
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
		psy_ui_component_updateoverflow(&self->modules.component);
	}
	psy_ui_component_invalidate(&self->modules.component);
}

void cpuview_onhide(CPUView* self)
{
	psy_ui_component_hide(&self->component);
	psy_ui_component_align(psy_ui_component_parent(&self->component));
}

void cpuview_oncpuperf(CPUView* self, psy_ui_CheckBox* sender)
{
	if (psy_ui_checkbox_checked(sender) != 0) {
		psy_audio_player_measure_cpu_usage(&self->workspace->player);
	} else {
		psy_audio_player_stop_measure_cpu_usage(&self->workspace->player);
	}
}
