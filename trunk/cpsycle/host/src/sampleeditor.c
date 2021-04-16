// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sampleeditor.h"

#include <exclusivelock.h>
#include <operations.h>
#include <psyclescript.h>

#include <songio.h>

#include <assert.h>
#include <math.h>
#include <string.h>

#include "../../detail/portable.h"

#define LUAPROCESSOR_DEFAULT \
"local frames = {}""\n"\
"\n"\
"function work()""\n"\
"\n"\
"  for i = 0, 1000 do""\n"\
"    frames[i] = 0.5""\n"\
"  end""\n"\
"  return frames""\n"\
"end""\n"\
"\n"

static void sampleeditorbar_ondoublecontloop(SampleEditorBar*, psy_ui_Component* sender);
static void sampleeditorbar_ondoublesustainloop(SampleEditorBar*, psy_ui_Component* sender);
static void sampleeditorbar_ondrawlines(SampleEditorBar*, psy_ui_Component* sender);

void sampleeditorbar_init(SampleEditorBar* self, psy_ui_Component* parent,
	SampleEditor* editor,
	Workspace* workspace)
{
	psy_ui_Margin margin;

	self->workspace = workspace;
	self->editor = editor;
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_checkbox_init(&self->selecttogether, &self->component);
	psy_ui_checkbox_settext(&self->selecttogether, "Select Channels Together");
	psy_ui_checkbox_check(&self->selecttogether);
	psy_ui_label_init_text(&self->selstartlabel, &self->component, NULL,
		"Selection Start");
	psy_ui_edit_init(&self->selstartedit, &self->component);
	psy_ui_edit_setcharnumber(&self->selstartedit, 10);			
	psy_ui_label_init_text(&self->selendlabel, &self->component, NULL,
		"Selection End");
	psy_ui_edit_init(&self->selendedit, &self->component);
	psy_ui_edit_setcharnumber(&self->selendedit, 10);
	psy_ui_label_init_text(&self->visualrepresentationdesc, &self->component,
		NULL, "Visual");
	psy_ui_checkbox_init(&self->doublecontloop, &self->component);
	psy_ui_checkbox_settext(&self->doublecontloop, "Double Cont Loop");		
	psy_signal_connect(&self->doublecontloop.signal_clicked, self,
		sampleeditorbar_ondoublecontloop);
	psy_ui_checkbox_init(&self->doublesustainloop, &self->component);
	psy_ui_checkbox_settext(&self->doublesustainloop, "Double Sus Loop");
	psy_signal_connect(&self->doublesustainloop.signal_clicked, self,
		sampleeditorbar_ondoublesustainloop);	
	psy_ui_checkbox_init(&self->drawlines, &self->component);
	psy_ui_checkbox_settext(&self->drawlines, "Lines");
	psy_signal_connect(&self->drawlines.signal_clicked, self,
		sampleeditorbar_ondrawlines);
	{	// resampling methods
		int quality;
		
		psy_ui_combobox_init(&self->visualrepresentation, &self->component, NULL);
		psy_ui_combobox_setcharnumber(&self->visualrepresentation, 12);
		for (quality = 0; quality < psy_dsp_RESAMPLERQUALITY_NUMRESAMPLERS;
				++quality) {
			psy_ui_combobox_addtext(&self->visualrepresentation,
				psy_dsp_multiresampler_name(
					(psy_dsp_ResamplerQuality)quality));
		}
	}
	// set to default in wavebox, change it there, too
	psy_ui_combobox_setcursel(&self->visualrepresentation,
		psy_dsp_RESAMPLERQUALITY_SPLINE);	
	sampleeditorbar_clearselection(self);
	psy_ui_margin_init_all_em(&margin, 0.0, 2.0, 0.0, 0.0);
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
		margin));
}

void sampleeditorbar_setselection(SampleEditorBar* self, uintptr_t selectionstart,
	uintptr_t selectionend)
{
	char text[128];

	psy_ui_edit_enableedit(&self->selstartedit);
	psy_ui_edit_enableedit(&self->selendedit);
	psy_snprintf(text, 128, "%u", (unsigned int)selectionstart);
	psy_ui_edit_settext(&self->selstartedit, text);
	psy_snprintf(text, 128, "%u", (unsigned int)selectionend);
	psy_ui_edit_settext(&self->selendedit, text);
}

void sampleeditorbar_clearselection(SampleEditorBar* self)
{
	psy_ui_edit_preventedit(&self->selstartedit);
	psy_ui_edit_preventedit(&self->selendedit);
	psy_ui_edit_settext(&self->selstartedit, "");
	psy_ui_edit_settext(&self->selendedit, "");
}

void sampleeditorbar_ondoublecontloop(SampleEditorBar* self, psy_ui_Component* sender)
{
	if (psy_ui_checkbox_checked(&self->doublecontloop)) {
		sampleeditor_showdoublecontloop(self->editor);
	} else {
		sampleeditor_showsinglecontloop(self->editor);
	}
	psy_ui_checkbox_disablecheck(&self->doublesustainloop);
}

void sampleeditorbar_ondoublesustainloop(SampleEditorBar* self, psy_ui_Component* sender)
{
	if (psy_ui_checkbox_checked(&self->doublesustainloop)) {
		sampleeditor_showdoublesustainloop(self->editor);
	} else {
		sampleeditor_showsinglesustainloop(self->editor);		
	}
	psy_ui_checkbox_disablecheck(&self->doublecontloop);
}

void sampleeditorbar_ondrawlines(SampleEditorBar* self, psy_ui_Component* sender)
{
	if (psy_ui_checkbox_checked(&self->drawlines)) {
		sampleeditor_drawlines(self->editor);
	} else {
		sampleeditor_drawbars(self->editor);
	}	
}

static void sampleeditoroperations_initalign(SampleEditorOperations*);

void sampleeditoroperations_init(SampleEditorOperations* self,
	psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_button_init(&self->cut, &self->component, NULL);
	psy_ui_button_settext(&self->cut, "edit.cut");
	psy_ui_button_init(&self->crop, &self->component, NULL);
	psy_ui_button_settext(&self->crop, "edit.crop");
	psy_ui_button_init(&self->copy, &self->component, NULL);
	psy_ui_button_settext(&self->copy, "edit.copy");
	psy_ui_button_init(&self->paste, &self->component, NULL);
	psy_ui_button_settext(&self->paste, "edit.paste");
	psy_ui_button_init(&self->del, &self->component, NULL);
	psy_ui_button_settext(&self->del, "edit.delete");
	sampleeditoroperations_initalign(self);
}

void sampleeditoroperations_initalign(SampleEditorOperations* self)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_all_em(&margin, 0.0, 0.5, 0.0, 0.5);		
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT, margin));
}

static void sampleeditoramplify_ontweak(SampleEditorAmplify*, psy_ui_Slider*, float value);
static void sampleeditoramplify_onvalue(SampleEditorAmplify*, psy_ui_Slider*, float* value);
static void sampleeditoramplify_ondescribe(SampleEditorAmplify*, psy_ui_Slider*, char* text);

void sampleeditoramplify_init(SampleEditorAmplify* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	char text[128];

	self->workspace = workspace;
	self->gainvalue = (psy_dsp_amp_t) 2/3.f;
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_label_init_text(&self->header, &self->component, NULL,
		"Adjust Volume");
	psy_ui_component_setalign(&self->header.component, psy_ui_ALIGN_TOP);
	psy_ui_slider_init(&self->gain, &self->component, NULL);
	psy_ui_slider_showvertical(&self->gain);
	psy_ui_slider_setcharnumber(&self->gain, 4);
	psy_ui_slider_connect(&self->gain, self,
		(ui_slider_fpdescribe)sampleeditoramplify_ondescribe,
		(ui_slider_fptweak)sampleeditoramplify_ontweak,
		(ui_slider_fpvalue)sampleeditoramplify_onvalue);
	psy_ui_component_setalign(&self->gain.component, psy_ui_ALIGN_LEFT);
	psy_ui_label_init(&self->dbdisplay, &self->component, NULL);
	psy_ui_label_preventtranslation(&self->dbdisplay);
	sampleeditoramplify_ondescribe(self, 0, text);
	psy_ui_label_settext(&self->dbdisplay, text);
	psy_ui_component_setalign(&self->dbdisplay.component, psy_ui_ALIGN_BOTTOM);
}

void sampleeditoramplify_ontweak(SampleEditorAmplify* self, psy_ui_Slider* slider,
	float value)
{
	char text[128];

	self->gainvalue = (intptr_t)(value * 288) / 288.f;
	sampleeditoramplify_ondescribe(self, 0, text);
	psy_ui_label_settext(&self->dbdisplay, text);
}

void sampleeditoramplify_onvalue(SampleEditorAmplify* self, psy_ui_Slider* slider,
	float* value)
{
		*value = self->gainvalue;
}

void sampleeditoramplify_ondescribe(SampleEditorAmplify* self, psy_ui_Slider* slider,
	char* text)
{			
	if (self->gainvalue == 0.f) {
		psy_snprintf(text, 10, "-inf. dB");
	} else {
		float db = (self->gainvalue - 2/3.f) * 144.f;
		psy_snprintf(text, 10, "%.2f dB", db);
	}	
}

void sampleeditluaprocessor_init(SampleEditLuaProcessor* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_label_init_text(&self->header, &self->component, NULL,
		"Lua Processor");
	psy_ui_component_setalign(&self->header.component, psy_ui_ALIGN_TOP);
	psy_ui_editor_init(&self->editor, &self->component);
	psy_ui_editor_settext(&self->editor, LUAPROCESSOR_DEFAULT);
	psy_ui_component_setalign(&self->editor.component, psy_ui_ALIGN_CLIENT);
	psy_ui_editor_init(&self->console, &self->component);	
	psy_ui_component_setalign(&self->console.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_setpreferredsize(&self->console.component,
		psy_ui_size_make_em(0.0, 10.0));
}

static void sampleprocessview_buildprocessorlist(SampleEditorProcessView*);
static void sampleeditorprocessview_onprocessorselected(SampleEditorProcessView*,
	psy_ui_Component* sender, int index);
static void sampleeditor_selectionbound(SampleEditor*, uintptr_t* framestart,
	uintptr_t* frameend, bool* hasselection);

void sampleprocessview_init(SampleEditorProcessView* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_Margin margin;
	
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;
	psy_ui_component_init(&self->client, &self->component, NULL);
	psy_ui_component_setalign(&self->client, psy_ui_ALIGN_CLIENT);
	psy_ui_margin_init_all_em(&margin, 0.0, 0.0, 0.0, 1.0);		
	psy_ui_component_setmargin(&self->client, margin);
	sampleeditoroperations_init(&self->copypaste, &self->client, workspace);
	psy_ui_component_setalign(&self->copypaste.component, psy_ui_ALIGN_TOP);
	psy_ui_button_init(&self->process, &self->client, NULL);
	psy_ui_button_settext(&self->process, "samplesview.process");
	psy_ui_component_setalign(&self->process.component, psy_ui_ALIGN_TOP);
	psy_ui_margin_init_all_em(&margin, 1.5, 0.0, 0.5, 0.0);		
	psy_ui_component_setmargin(&self->process.component, margin);
	psy_ui_listbox_init(&self->processors, &self->client);
	psy_ui_component_setalign(&self->processors.component, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->processors.component, margin);	
	psy_ui_notebook_init(&self->notebook, &self->client);
	psy_ui_component_setalign(&self->notebook.component, psy_ui_ALIGN_CLIENT);
	sampleeditoramplify_init(&self->amplify, &self->notebook.component, workspace);	
	sampleprocessview_buildprocessorlist(self);
	psy_ui_component_init(&self->emptypage1,
		psy_ui_notebook_base(&self->notebook), NULL);
	psy_ui_component_init(&self->emptypage2,
		psy_ui_notebook_base(&self->notebook), NULL);
	psy_ui_component_init(&self->emptypage3,
		psy_ui_notebook_base(&self->notebook), NULL);
	psy_ui_component_init(&self->emptypage4,
		psy_ui_notebook_base(&self->notebook), NULL);
	psy_ui_component_init(&self->emptypage5,
		psy_ui_notebook_base(&self->notebook), NULL);
	psy_ui_component_init(&self->emptypage6,
		psy_ui_notebook_base(&self->notebook), NULL);
	sampleeditluaprocessor_init(&self->luaprocessor,
		psy_ui_notebook_base(&self->notebook),
		workspace);	
	psy_ui_listbox_setcursel(&self->processors, 0);
	psy_ui_notebook_select(&self->notebook, 0);
	psy_signal_connect(&self->processors.signal_selchanged, self,
		sampleeditorprocessview_onprocessorselected);	
}

void sampleprocessview_buildprocessorlist(SampleEditorProcessView* self)
{
	psy_ui_listbox_clear(&self->processors);
	psy_ui_listbox_addtext(&self->processors, "Amplify");
	psy_ui_listbox_addtext(&self->processors, "Fade In");
	psy_ui_listbox_addtext(&self->processors, "Fade Out");
	psy_ui_listbox_addtext(&self->processors, "Insert Silence");
	psy_ui_listbox_addtext(&self->processors, "Normalize");
	psy_ui_listbox_addtext(&self->processors, "Remove DC");
	psy_ui_listbox_addtext(&self->processors, "Reverse");
	psy_ui_listbox_addtext(&self->processors, "Lua");
}

void sampleeditorprocessview_onprocessorselected(SampleEditorProcessView* self,
	psy_ui_Component* sender, int index)
{
	psy_ui_notebook_select(&self->notebook, index);
}

static void sampleeditorplaybar_initalign(SampleEditorPlayBar*);
static void sampleeditor_initsampler(SampleEditor*);
static void sampleeditor_ondestroy(SampleEditor*, psy_ui_Component* sender);
static void sampleeditor_onzoom(SampleEditor*, psy_ui_Component* sender);
static void sampleeditor_onsongchanged(SampleEditor*, Workspace*, int flag,
	psy_audio_Song*);
static void sampleeditor_connectmachinessignals(SampleEditor*, Workspace*);
static void sampleeditor_onplay(SampleEditor*, psy_ui_Component* sender);
static void sampleeditor_onstop(SampleEditor*, psy_ui_Component* sender);
static void sampleeditor_onprocess(SampleEditor*, psy_ui_Component* sender);
static void sampleeditor_oncrop(SampleEditor*, psy_ui_Component* sender);
static void sampleeditor_ondelete(SampleEditor*, psy_ui_Component* sender);
static void sampleeditor_onmasterworked(SampleEditor*, psy_audio_Machine*,
	uintptr_t slot, psy_audio_BufferContext*);
static void sampleeditor_onselectionchanged(SampleEditor*, SampleBox*, WaveBox* wavebox);
static void sampleeditor_onscrollzoom_customdraw(SampleEditor*,
	ScrollZoom* sender, psy_ui_Graphics*);
static void sampleeditor_setloopviewmode(SampleEditor*,
	WaveBoxLoopViewMode mode);
static void sampleeditor_amplify(SampleEditor*, uintptr_t channel,
	uintptr_t framestart, uintptr_t frameend,
	psy_dsp_amp_t gain);
static void sampleeditor_removeDC(SampleEditor*, uintptr_t channel,
	uintptr_t framestart, uintptr_t frameend);
static void sampleeditor_processlua(SampleEditor*, uintptr_t channel,
	uintptr_t framestart, uintptr_t frameend);

enum {
	SAMPLEEDITOR_DRAG_NONE,
	SAMPLEEDITOR_DRAG_LEFT,
	SAMPLEEDITOR_DRAG_RIGHT,
	SAMPLEEDITOR_DRAG_MOVE
};

void sampleeditorplaybar_init(SampleEditorPlayBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent, NULL);
	// psy_ui_button_init(&self->loop, &self->component);
	// psy_ui_button_settext(&self->loop, "Loop");	
	// psy_signal_connect(&self->loop.signal_clicked, self, onloopclicked);	
	psy_ui_button_init(&self->play, &self->component, NULL);
	psy_ui_button_settext(&self->play, "play");	
	psy_ui_button_init(&self->stop, &self->component, NULL);
	psy_ui_button_settext(&self->stop, "stop");
	// psy_signal_connect(&self->stop.signal_clicked, self, onstopclicked);	
	sampleeditorplaybar_initalign(self);	
}

void sampleeditorplaybar_initalign(SampleEditorPlayBar* self)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_all_em(&margin, 0.0, 0.5, 0.0, 0.0);		
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HORIZONTALEXPAND);
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT, margin));
}

// Header
static void sampleeditorheader_ondraw(SampleEditorHeader*, psy_ui_Graphics*);
static void sampleeditorheader_onpreferredsize(SampleEditorHeader*,
	psy_ui_Size* limit, psy_ui_Size* size);
static void sampleeditorheader_drawruler(SampleEditorHeader*, psy_ui_Graphics*);
static void sampleeditorheader_setwaveboxmetric(SampleEditorHeader*,
	WaveBoxContext* metric);

static psy_ui_ComponentVtable sampleeditorheader_vtable;
static int sampleeditorheader_vtable_initialized = 0;

static void sampleeditorheader_vtable_init(SampleEditorHeader* self)
{
	if (!sampleeditorheader_vtable_initialized) {
		sampleeditorheader_vtable = *(self->component.vtable);
		sampleeditorheader_vtable.ondraw = (psy_ui_fp_component_ondraw)
			sampleeditorheader_ondraw;
		sampleeditorheader_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			sampleeditorheader_onpreferredsize;
		sampleeditorheader_vtable_initialized = 1;
	}
}

void sampleeditorheader_init(SampleEditorHeader* self,
	psy_ui_Component* parent)
{
	self->metric = 0;
	psy_ui_component_init(&self->component, parent, NULL);
	sampleeditorheader_vtable_init(self);
	self->component.vtable = &sampleeditorheader_vtable;
	psy_ui_component_doublebuffer(&self->component);	
}

void sampleeditorheader_setwaveboxmetric(SampleEditorHeader* self,
	WaveBoxContext* metric)
{
	self->metric = metric;
}

void sampleeditorheader_ondraw(SampleEditorHeader* self, psy_ui_Graphics* g)
{		
	sampleeditorheader_drawruler(self, g);	
}

void sampleeditorheader_onpreferredsize(SampleEditorHeader* self,
	psy_ui_Size* limit, psy_ui_Size* rv)
{
	if (rv) {
		rv->width = limit->width;
		rv->height = psy_ui_value_makeeh(1.5);
	}
}

void sampleeditorheader_drawruler(SampleEditorHeader* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;	
	double baseline;
	const psy_ui_TextMetric* tm;
	uintptr_t frame;
	uintptr_t step;

	size = psy_ui_component_innersize_px(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	baseline = size.height - 1;
	psy_ui_setcolour(g, psy_ui_colour_make(0x00666666));
	psy_ui_drawline(g, psy_ui_realpoint_make(0, baseline),
		psy_ui_realpoint_make(size.width, baseline));
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	psy_ui_settextcolour(g, psy_ui_colour_make(0x00999999));
	if (self->metric) {
		step = waveboxcontext_numframes(self->metric) / 10;
		step = (uintptr_t)((double)step * (self->metric->zoomright - self->metric->zoomleft));
		if (step == 0) {
			step = 1;
		}
		for (frame = 0; frame < waveboxcontext_numframes(self->metric); frame += step) {
			double cpx;

			cpx = waveboxcontext_frametoscreen(self->metric, frame);
			if (cpx >= 0 && cpx < size.width) {
				char txt[40];

				psy_ui_drawline(g, psy_ui_realpoint_make(cpx, baseline),
					psy_ui_realpoint_make(cpx, baseline - tm->tmHeight / 3));
				psy_snprintf(txt, 40, "%d", (int)waveboxcontext_realframe(self->metric, frame));
				psy_ui_textout(g, (int)cpx + (int)(tm->tmAveCharWidth * 0.75),
					baseline - tm->tmHeight - tm->tmHeight / 6, txt, strlen(txt));
			}
		}
	}
}

void sampleeditor_onscrollzoom_customdraw(SampleEditor* self, ScrollZoom* sender,
	psy_ui_Graphics* g)
{
	if (self->sample) {
		psy_ui_RealRectangle r;
		const psy_ui_TextMetric* tm;
		psy_ui_RealSize size;
		
		size = psy_ui_component_innersize_px(&sender->component);
		tm = psy_ui_component_textmetric(&sender->component);
		psy_ui_setrectangle(&r, 0, 0, size.width, size.height);
		psy_ui_setcolour(g, psy_ui_colour_make(0x00B1C8B0));
		if (!self->sample) {			
			static const char* txt = "No wave loaded";
			
			psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
			psy_ui_settextcolour(g, psy_ui_colour_make(0x00D1C5B6));
			psy_ui_textout(g, (size.width - tm->tmAveCharWidth * strlen(txt)) / 2,
				(size.height - tm->tmHeight) / 2, txt, strlen(txt));
		} else {
			double x;
			double centery;
			double offsetstep;
			double scaley;

			centery = size.height / 2;
			scaley = size.height / 2 / 32768.0;
			offsetstep = (double)self->sample->numframes / size.width;
			psy_ui_setcolour(g, psy_ui_colour_make(0x00B1C8B0));
			for (x = 0; x < size.width; ++x) {
				uintptr_t frame;
				double framevalue;
				
				frame = (uintptr_t)(offsetstep * x);
				if (frame >= self->sample->numframes) {
					break;
				}
				framevalue = self->sample->channels.samples[0][frame];							
				psy_ui_drawline(g, psy_ui_realpoint_make(x, centery),
					psy_ui_realpoint_make(x, centery + framevalue * scaley));
			}
		}
	}
}

static void samplebox_ondestroy(SampleBox*, psy_ui_Component* sender);
static void samplebox_ondestroyed(SampleBox*, psy_ui_Component* sender);
static void samplebox_clearwaveboxes(SampleBox*);
static void samplebox_buildwaveboxes(SampleBox*, psy_audio_Sample*,
	WaveBoxLoopViewMode);
static void samplebox_onselectionchanged(SampleBox*, WaveBox* sender);

void samplebox_init(SampleBox* self, psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);
	psy_table_init(&self->waveboxes);
	psy_signal_connect(&self->component.signal_destroy, self,
		samplebox_ondestroy);
	psy_signal_connect(&self->component.signal_destroyed, self,
		samplebox_ondestroyed);
	psy_signal_init(&self->signal_selectionchanged);
}

void samplebox_ondestroy(SampleBox* self, psy_ui_Component* sender)
{		
	psy_signal_dispose(&self->signal_selectionchanged);
}

void samplebox_ondestroyed(SampleBox* self, psy_ui_Component* sender)
{
	psy_table_disposeall(&self->waveboxes, (psy_fp_disposefunc)NULL);	
}

void samplebox_clearwaveboxes(SampleBox* self)
{
	psy_TableIterator it;
	
	for (it = psy_table_begin(&self->waveboxes);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		WaveBox* wavebox;

		wavebox = (WaveBox*)psy_tableiterator_value(&it);
		psy_ui_component_destroy(&wavebox->component);
		free(wavebox);
	}
	psy_table_clear(&self->waveboxes);
	psy_ui_component_align(&self->component);
}

void samplebox_buildwaveboxes(SampleBox* self, psy_audio_Sample* sample,
	WaveBoxLoopViewMode loopviewmode)
{
	samplebox_clearwaveboxes(self);
	if (sample) {
		uintptr_t channel;

		for (channel = 0; channel < psy_audio_buffer_numchannels(
			&sample->channels); ++channel) {
			WaveBox* wavebox;

			wavebox = wavebox_allocinit(&self->component, self->workspace);			
			wavebox_setloopviewmode(wavebox, loopviewmode);
			wavebox->preventdrawonselect = TRUE;
			wavebox_setsample(wavebox, sample, channel);
			psy_ui_component_setalign(&wavebox->component, psy_ui_ALIGN_CLIENT);
			psy_signal_connect(&wavebox->selectionchanged, self,
				samplebox_onselectionchanged);
			psy_table_insert(&self->waveboxes, channel, (void*)wavebox);
		}
	}
	psy_ui_component_align(&self->component);
}

void samplebox_setzoom(SampleBox* self, double zoomleft, double zoomright)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->waveboxes);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		WaveBox* wavebox;

		wavebox = (WaveBox*)psy_tableiterator_value(&it);
		wavebox_setzoom(wavebox, zoomleft, zoomright);
	}	
}

void samplebox_setloopviewmode(SampleBox* self, WaveBoxLoopViewMode mode)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->waveboxes);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		WaveBox* wavebox;

		wavebox = (WaveBox*)psy_tableiterator_value(&it);
		wavebox_setloopviewmode(wavebox, mode);
	}
	psy_ui_component_invalidate(&self->component);
}

void samplebox_drawlines(SampleBox* self)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->waveboxes);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		WaveBox* wavebox;

		wavebox = (WaveBox*)psy_tableiterator_value(&it);
		wavebox_drawlines(wavebox);
	}
	psy_ui_component_invalidate(&self->component);
}

void samplebox_setquality(SampleBox* self, psy_dsp_ResamplerQuality quality)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->waveboxes);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		WaveBox* wavebox;

		wavebox = (WaveBox*)psy_tableiterator_value(&it);
		wavebox_setquality(wavebox, quality);
	}	
}

void samplebox_drawbars(SampleBox* self)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->waveboxes);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		WaveBox* wavebox;

		wavebox = (WaveBox*)psy_tableiterator_value(&it);
		wavebox_drawbars(wavebox);
	}
	psy_ui_component_invalidate(&self->component);
}


void samplebox_onselectionchanged(SampleBox* self, WaveBox* sender)
{
	psy_signal_emit(&self->signal_selectionchanged, self, 1, sender);
}

void sampleeditor_init(SampleEditor* self, psy_ui_Component* parent,
	Workspace* workspace)
{						
	psy_ui_Margin margin;

	self->sample = 0;
	self->samplerevents = 0;
	self->workspace = workspace;
	self->loopviewmode = WAVEBOX_LOOPVIEW_CONT_SINGLE;
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_margin_init_all_em(&margin, 0.0, 0.0, 0.0, 2.0);		
	psy_signal_connect(&self->component.signal_destroy, self,
		sampleeditor_ondestroy);	
	sampleprocessview_init(&self->processview, &self->component, workspace);
	psy_ui_component_setalign(&self->processview.component, psy_ui_ALIGN_RIGHT);
	//psy_ui_component_setmargin(&self->processview.component, &margin);
	psy_signal_connect(&self->processview.process.signal_clicked, self,
		sampleeditor_onprocess);
	psy_signal_connect(&self->processview.copypaste.crop.signal_clicked, self,
		sampleeditor_oncrop);
	psy_signal_connect(&self->processview.copypaste.del.signal_clicked, self,
		sampleeditor_ondelete);
	psy_ui_splitbar_init(&self->splitbar, &self->component);
	psy_ui_component_setalign(&self->splitbar.component, psy_ui_ALIGN_RIGHT);
	sampleeditorplaybar_init(&self->playbar, &self->component, workspace);
	psy_signal_connect(&self->playbar.play.signal_clicked, self,
		sampleeditor_onplay);
	psy_signal_connect(&self->playbar.stop.signal_clicked, self,
		sampleeditor_onstop);
	psy_ui_component_setalign(&self->playbar.component, psy_ui_ALIGN_TOP);	
	sampleeditorheader_init(&self->header, &self->component);
	psy_ui_component_setalign(&self->header.component, psy_ui_ALIGN_TOP);		
	samplebox_init(&self->samplebox, &self->component, workspace);	
	psy_ui_component_setalign(&self->samplebox.component, psy_ui_ALIGN_CLIENT);	
	psy_signal_connect(&self->samplebox.signal_selectionchanged, self,
		sampleeditor_onselectionchanged);
	scrollzoom_init(&self->zoom, &self->component);
	psy_signal_connect(&self->zoom.signal_customdraw, self,
		sampleeditor_onscrollzoom_customdraw);
	psy_ui_component_setalign(&self->zoom.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_setpreferredsize(&self->zoom.component,
		psy_ui_size_make_em(0.0, 2.0));	
	psy_signal_connect(&self->zoom.signal_zoom, self, sampleeditor_onzoom);
	psy_signal_connect(&workspace->signal_songchanged, self,
		sampleeditor_onsongchanged);	
	sampleeditor_initsampler(self);
	sampleeditor_connectmachinessignals(self, workspace);	
	psy_signal_init(&self->signal_samplemodified);	
}

void sampleeditor_initsampler(SampleEditor* self)
{	
	psy_audio_xmsampler_init(&self->sampler,
		self->workspace->machinefactory.machinecallback);
	psy_audio_buffer_init(&self->samplerbuffer, 2);
	psy_audio_buffer_allocsamples(&self->samplerbuffer, psy_audio_MAX_STREAM_SIZE);	
	psy_audio_buffer_clearsamples(&self->samplerbuffer, psy_audio_MAX_STREAM_SIZE);
}

void sampleeditor_ondestroy(SampleEditor* self, psy_ui_Component* sender)
{		
	psy_audio_machine_dispose(&self->sampler.custommachine.machine);	
	psy_audio_buffer_dispose(&self->samplerbuffer);	
	psy_signal_dispose(&self->signal_samplemodified);
	psy_signal_disconnect(&self->workspace->signal_songchanged, self,
		sampleeditor_onsongchanged);
	if (workspace_song(self->workspace)) {
		psy_signal_disconnect(&psy_audio_machines_master(
			&workspace_song(self->workspace)->machines)->signal_worked, self,
			sampleeditor_onmasterworked);
	}
}

void sampleeditor_setsample(SampleEditor* self, psy_audio_Sample* sample)
{
	if (self->sample != NULL || sample != NULL) {
		self->sample = sample;
		sampleeditorheader_setwaveboxmetric(&self->header, 0);
		samplebox_buildwaveboxes(&self->samplebox, sample, self->loopviewmode);
		if (!sample) {
			psy_ui_component_invalidate(&self->component);
		}
		if (psy_table_size(&self->samplebox.waveboxes) > 0) {
			WaveBox* wavebox;
			
			wavebox = (WaveBox*)psy_table_at(&self->samplebox.waveboxes, 0);
			if (wavebox) {
				sampleeditorheader_setwaveboxmetric(&self->header,
					wavebox_metric(wavebox));
			}
		}
		psy_ui_component_invalidate(&self->header.component);
		psy_ui_component_invalidate(&self->samplebox.component);
		psy_ui_component_invalidate(&self->zoom.component);
	}
}

void sampleeditor_onzoom(SampleEditor* self, psy_ui_Component* sender)
{
	samplebox_setzoom(&self->samplebox, scrollzoom_start(&self->zoom),
		scrollzoom_end(&self->zoom));
	psy_ui_component_invalidate(&self->header.component);
}

void sampleeditor_onsongchanged(SampleEditor* self, Workspace* workspace,
	int flag, psy_audio_Song* song)
{
	sampleeditor_connectmachinessignals(self, workspace);
	sampleeditorbar_clearselection(&self->sampleeditortbar);
}

void sampleeditor_connectmachinessignals(SampleEditor* self,
	Workspace* workspace)
{
	if (workspace && workspace->song &&
			psy_audio_machines_master(&workspace->song->machines)) {
		psy_signal_connect(
			&psy_audio_machines_master(&workspace->song->machines)->signal_worked, self,
			sampleeditor_onmasterworked);
	}
}

void sampleeditor_onplay(SampleEditor* self, psy_ui_Component* sender)
{	
	if (workspace_song(self->workspace) && self->sample) {
		psy_audio_PatternEvent event;
		psy_audio_exclusivelock_enter();
		psy_list_free(self->samplerevents);
		psy_audio_patternevent_init_all(&event,
			(unsigned char) 48,
			(unsigned char)psy_audio_instruments_selected(&workspace_song(self->workspace)->instruments).subslot,
			psy_audio_NOTECOMMANDS_psy_audio_EMPTY,
			psy_audio_NOTECOMMANDS_VOL_EMPTY,
			0, 0);	
		psy_audio_patternentry_init_all(&self->samplerentry, &event, 0, 0, 120.f, 0);
		self->samplerevents = psy_list_create(&self->samplerentry);
		psy_audio_exclusivelock_leave();
	}
}

void sampleeditor_onstop(SampleEditor* self, psy_ui_Component* sender)
{	
	psy_audio_PatternEvent event;

	psy_audio_exclusivelock_enter();
	psy_list_free(self->samplerevents);
	psy_audio_patternevent_init_all(&event,
		psy_audio_NOTECOMMANDS_RELEASE,
		0,		
		psy_audio_NOTECOMMANDS_psy_audio_EMPTY,
		psy_audio_NOTECOMMANDS_VOL_EMPTY,
		0, 0);	
	self->samplerevents = psy_list_create(&self->samplerentry);
	psy_audio_patternentry_init_all(&self->samplerentry, &event, 0, 0, 120.f, 0);
	psy_audio_exclusivelock_leave();
}

void sampleeditor_onmasterworked(SampleEditor* self, psy_audio_Machine* machine,
	uintptr_t slot, psy_audio_BufferContext* bc)
{
	/*psy_audio_BufferContext samplerbc;
		
	psy_audio_buffercontext_init(&samplerbc, self->samplerevents, 0,
		&self->samplerbuffer, bc->numsamples, 16);
	psy_audio_buffer_clearsamples(&self->samplerbuffer, bc->numsamples);
	psy_audio_machine_work(&self->sampler.custommachine.machine, &samplerbc);
	psy_audio_buffer_addsamples(bc->output, &self->samplerbuffer, bc->numsamples, 
		(psy_dsp_amp_t) 1.f);
	if (self->samplerevents) {
		psy_audio_patternentry_dispose(&self->samplerentry);
	}
	psy_list_free(self->samplerevents);
	self->samplerevents = 0;*/
	
}

void sampleeditor_onprocess(SampleEditor* self, psy_ui_Component* sender)
{
	psy_TableIterator it;

	intptr_t selected;

	selected = psy_ui_listbox_cursel(&self->processview.processors);
	for (it = psy_table_begin(&self->samplebox.waveboxes);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		WaveBox* wavebox;

		wavebox = (WaveBox*)psy_tableiterator_value(&it);
		if (wavebox_hasselection(wavebox)) {
			uintptr_t channel;

			channel = psy_tableiterator_key(&it);
			switch (selected) {
				case 0: { // Amplify			
					double ratio;

					ratio = pow(10.0, (double)((double)(self->processview.amplify.gainvalue) - 2 / (double)3) / (1 / (double)7));
					sampleeditor_amplify(self, channel, wavebox->context.selection.start, wavebox->context.selection.end,
						(psy_dsp_amp_t)ratio);
					break;
				}				
				case 1:
					// FadeIn
					psy_dsp_fade(&dsp, self->sample->channels.samples[channel] +
						wavebox->context.selection.start,
						waveboxselection_numframes(&wavebox->context.selection), 0.f, 1.f);				
					break;
				case 2:
					// FadeOut
					psy_dsp_fade(&dsp, self->sample->channels.samples[channel] +
						wavebox->context.selection.start,
						waveboxselection_numframes(&wavebox->context.selection), 1.f, 0.f);				
					break;
				case 4:
					psy_dsp_normalize(&dsp, self->sample->channels.samples[channel] +
						wavebox->context.selection.start,
						waveboxselection_numframes(&wavebox->context.selection));
					break;
				case 5:
					sampleeditor_removeDC(self, channel, wavebox->context.selection.start, wavebox->context.selection.end);
					break;
				case 6:
					dsp.reverse(self->sample->channels.samples[channel] +
						wavebox->context.selection.start,
						waveboxselection_numframes(&wavebox->context.selection));				
					break;
				case 7:
					sampleeditor_processlua(self, channel, wavebox->context.selection.start, wavebox->context.selection.end);
					break;
				default:
					break;
			}
		} else			
		if (selected == 7) {
			if (self->sample) {
				psy_ui_editor_settext(&self->processview.luaprocessor.console,
					"No Selection, first mark samples to process");
			} 			
		}		
	}
	if (psy_table_size(&self->samplebox.waveboxes) == 0) {
		if (selected == 7) {
			if (!self->sample) {
				psy_ui_editor_settext(&self->processview.luaprocessor.console,
					"No wave data, first load sample");
			}
		}
	}
	psy_ui_component_invalidate(&self->samplebox.component);
}

void sampleeditor_oncrop(SampleEditor* self, psy_ui_Component* sender)
{
	if (self->sample && self->sample->numframes > 0) {
		psy_TableIterator it;
		uintptr_t framestart;
		uintptr_t frameend;
		bool hasselection;

		sampleeditor_selectionbound(self, &framestart, &frameend,
			&hasselection);
		if (hasselection && frameend <= self->sample->numframes) {
			self->sample->numframes = frameend - framestart + 1;			
			for (it = psy_table_begin(&self->samplebox.waveboxes);
					!psy_tableiterator_equal(&it, psy_table_end());
					psy_tableiterator_inc(&it)) {
				WaveBox* wavebox;

				wavebox = (WaveBox*)psy_tableiterator_value(&it);
				self->sample->channels.samples[psy_tableiterator_key(&it)] = 
					dsp.crop(self->sample->channels.samples[psy_tableiterator_key(&it)]
						, framestart, self->sample->numframes);
				wavebox_clearselection(wavebox);
				wavebox_refresh(wavebox);
			}			
		}
	}
	psy_ui_component_invalidate(&self->samplebox.component);
	psy_ui_component_invalidate(&self->header.component);
	psy_signal_emit(&self->signal_samplemodified, self, 1, self->sample);
}

void sampleeditor_ondelete(SampleEditor* self, psy_ui_Component* sender)
{
	if (self->sample && self->sample->numframes > 0) {
		psy_TableIterator it;
		uintptr_t framestart;
		uintptr_t frameend;		
		bool hasselection = FALSE;		
		sampleeditor_selectionbound(self, &framestart, &frameend,
			&hasselection);
		psy_audio_exclusivelock_enter();		
		if (hasselection && frameend <= self->sample->numframes) {
			uintptr_t range;
			uintptr_t newnumframes;

			range = frameend - framestart + 1;
			newnumframes = self->sample->numframes - range;
			for (it = psy_table_begin(&self->samplebox.waveboxes);
					!psy_tableiterator_equal(&it, psy_table_end());
					psy_tableiterator_inc(&it)) {
				WaveBox* wavebox;
				psy_dsp_amp_t* oldwave;
				psy_dsp_amp_t* newwave;				

				wavebox = (WaveBox*)psy_tableiterator_value(&it);
				oldwave = self->sample->channels.samples[psy_tableiterator_key(&it)];	
				if (newnumframes > 0) {
					newwave = dsp.memory_alloc(newnumframes, sizeof(psy_dsp_amp_t));
					dsp.clear(newwave, newnumframes);
					if (framestart > 0) {
						dsp.add(oldwave, newwave, framestart, 1.f);
					}
					if (newnumframes - range > 0) {
						dsp.add(oldwave + frameend + 1, newwave + framestart,
							newnumframes - framestart - 1, 1.f);
					}
				} else {
					newwave = NULL;
				}
				self->sample->channels.samples[psy_tableiterator_key(&it)] = newwave;
				dsp.memory_dealloc(oldwave);
				wavebox_clearselection(wavebox);
				wavebox_refresh(wavebox);
			}
			self->sample->numframes = newnumframes;
		}		
		psy_audio_exclusivelock_leave();
	}
	psy_ui_component_invalidate(&self->samplebox.component);
	psy_ui_component_invalidate(&self->header.component);
	psy_signal_emit(&self->signal_samplemodified, self, 1, self->sample);
}

void sampleeditor_selectionbound(SampleEditor* self, uintptr_t* framestart,
	uintptr_t* frameend, bool* hasselection)
{
	psy_TableIterator it;	

	*hasselection = FALSE;
	*framestart = self->sample->numframes;
	*frameend = 0;
	for (it = psy_table_begin(&self->samplebox.waveboxes);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		WaveBox* wavebox;

		wavebox = (WaveBox*)psy_tableiterator_value(&it);
		if (wavebox->context.selection.start < *framestart) {
			*framestart = wavebox->context.selection.start;
		}
		if (wavebox->context.selection.end > *frameend) {
			*frameend = wavebox->context.selection.end;
		}
		if (wavebox->context.selection.hasselection) {
			*hasselection = TRUE;
		}
	}
}

void sampleeditor_amplify(SampleEditor* self, uintptr_t channel, uintptr_t framestart, uintptr_t frameend, psy_dsp_amp_t gain)
{	
	dsp.mul(self->sample->channels.samples[channel] + framestart, frameend - framestart + 1, gain);
}

void sampleeditor_processlua(SampleEditor* self, uintptr_t channel, uintptr_t framestart, uintptr_t frameend)
{
	psy_audio_PsycleScript processor;
	char text[65535];

	psy_ui_editor_clear(&self->processview.luaprocessor.console);
	psyclescript_init(&processor);
	psy_ui_editor_text(&self->processview.luaprocessor.editor, 65534, text);

	if (psyclescript_loadstring(&processor, text) != LUA_OK) {
		const char* msg = lua_tostring(processor.L, -1);
		psy_ui_editor_settext(&self->processview.luaprocessor.console,
			msg);
		psyclescript_dispose(&processor);
		return;
	}
	if (psyclescript_run(&processor) != LUA_OK) {
		const char* msg = lua_tostring(processor.L, -1);
		psy_ui_editor_settext(&self->processview.luaprocessor.console,
			msg);
		psyclescript_dispose(&processor);
		return;
	}
	lua_getglobal(processor.L, "work");
	if (!lua_isnil(processor.L, -1)) { 
		lua_Unsigned len;
		lua_Unsigned i;
		int status = lua_pcall(processor.L, 0, LUA_MULTRET, 0);

		if (status) {
			const char* msg = lua_tostring(processor.L, -1);
			psy_ui_editor_settext(&self->processview.luaprocessor.console,
				msg);
			psyclescript_dispose(&processor);
			return;
		}
		len = lua_rawlen(processor.L, -1);		
		for (i = 1; i <= len; ++i) {
			lua_Unsigned dst;
			lua_rawgeti(processor.L, -1, i);

			dst = i + framestart - 1;
			if (dst <= frameend && dst < self->sample->numframes) {
				self->sample->channels.samples[channel][dst] = (float)luaL_checknumber(processor.L, -1)
					* 32768.f;
				lua_pop(processor.L, 1);
			} else {
				break;
			}
		}
		psyclescript_dispose(&processor);
		return;
	}
	psyclescript_dispose(&processor);
}

void sampleeditor_removeDC(SampleEditor* self, uintptr_t channel, uintptr_t framestart, uintptr_t frameend)
{
	uintptr_t c;
	uintptr_t numframes;
	double meanL = 0.0;
	double meanR = 0.0;
	psy_dsp_amp_t* wdLeft;
	psy_dsp_amp_t* wdRight;
	psy_dsp_amp_t buf;

	wdLeft = self->sample->channels.samples[channel];
	wdRight = NULL;

	numframes = frameend - framestart + 1;
	for (c = framestart; c < framestart + numframes; c++) {
		meanL = meanL + ((double)*(wdLeft + c) / numframes);

		//if (wdStereo) meanR = (double)meanR + ((double)*(wdRight + c) / length);
	}
	for (c = framestart; c < framestart + numframes; c++) {
		buf = *(wdLeft + c);
		if (meanL > 0) {
			if ((double)(buf - meanL) < (-32768))	*(wdLeft + c) = (psy_dsp_amp_t) -32768;
			else	*(wdLeft + c) = (psy_dsp_amp_t)(buf - meanL);
		} else if (meanL < 0) {
			if ((double)(buf - meanL) > 32767) *(wdLeft + c) = (psy_dsp_amp_t) 32767;
			else *(wdLeft + c) = (psy_dsp_amp_t)(buf - meanL);
		}
	}
}

void sampleeditor_onselectionchanged(SampleEditor* self, SampleBox* sender, WaveBox* wavebox)
{
	if (wavebox_hasselection(wavebox)) {
		sampleeditorbar_setselection(&self->sampleeditortbar,
			wavebox->context.selection.start,
			wavebox->context.selection.end);
	} else {
		sampleeditorbar_clearselection(&self->sampleeditortbar);
	}
	if (psy_ui_checkbox_checked(&self->sampleeditortbar.selecttogether)) {
		psy_TableIterator it;
		for (it = psy_table_begin(&self->samplebox.waveboxes);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			WaveBox* currwavebox;

			currwavebox = (WaveBox*)psy_tableiterator_value(&it);
			if (currwavebox != wavebox) {
				if (wavebox_hasselection(wavebox)) {
					wavebox_setselection(currwavebox, wavebox->context.selection.start,
						wavebox->context.selection.end);
				} else {
					wavebox_clearselection(currwavebox);
				}				
			}
		}
		psy_ui_component_invalidate(&self->samplebox.component);
		psy_ui_component_update(&self->samplebox.component);
	} else {		
		psy_ui_component_invalidate(&sender->component);
		psy_ui_component_update(&sender->component);
	}	
}

void sampleeditor_showdoublecontloop(SampleEditor* self)
{
	sampleeditor_setloopviewmode(self, WAVEBOX_LOOPVIEW_CONT_DOUBLE);
}

void sampleeditor_showsinglecontloop(SampleEditor* self)
{
	sampleeditor_setloopviewmode(self, WAVEBOX_LOOPVIEW_CONT_SINGLE);
}

void sampleeditor_showdoublesustainloop(SampleEditor* self)
{
	sampleeditor_setloopviewmode(self, WAVEBOX_LOOPVIEW_SUSTAIN_DOUBLE);
}

void sampleeditor_showsinglesustainloop(SampleEditor* self)
{
	sampleeditor_setloopviewmode(self, WAVEBOX_LOOPVIEW_SUSTAIN_SINGLE);
}

void sampleeditor_setloopviewmode(SampleEditor* self, WaveBoxLoopViewMode mode)
{
	self->loopviewmode = mode;
	samplebox_setloopviewmode(&self->samplebox, mode);		
	psy_ui_component_invalidate(&self->header.component);
}

void sampleeditor_drawlines(SampleEditor* self)
{
	samplebox_drawlines(&self->samplebox);
}

void sampleeditor_drawbars(SampleEditor* self)
{
	samplebox_drawbars(&self->samplebox);
}

void sampleeditor_setquality(SampleEditor* self, psy_dsp_ResamplerQuality quality)
{
	samplebox_setquality(&self->samplebox, quality);
}
