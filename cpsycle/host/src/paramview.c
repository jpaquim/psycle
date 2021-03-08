// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

// host
#include "paramview.h"
#include "resources/resource.h"
#include "skingraphics.h"
// std
#include <assert.h>
// platform
#include "../../detail/portable.h"


// KnobDraw
// implementation
void knobdraw_init(KnobDraw* self, ParamSkin* skin,
	psy_audio_Machine* machine, psy_audio_MachineParam* param,
	psy_ui_RealSize size, const psy_ui_TextMetric* tm, bool tweaking)
{
	self->machine = machine;
	self->skin = skin;
	self->param = param;
	self->size = size;
	self->tweaking = tweaking;
	self->tm = tm;	
}

void knobdraw_draw(KnobDraw* self, psy_ui_Graphics* g)
{
	char label[128];
	char str[128];	
	psy_ui_RealRectangle r_top;
	psy_ui_RealRectangle r_bottom;	
		
	if (self->machine && self->param) {
		psy_ui_setrectangle(&r_top,
			psy_ui_realrectangle_width(&self->skin->knob.dest), 0,
			self->size.width - psy_ui_realrectangle_width(&self->skin->knob.dest),
			self->size.height / 2);
	} else {
		psy_ui_setrectangle(&r_top, 0, 0, self->size.width,
			self->size.height / 2);
	}
	psy_ui_drawsolidrectangle(g, r_top, self->skin->topcolour);		
	r_bottom = r_top;
	psy_ui_realrectangle_settopleft(&r_bottom,
		psy_ui_realpoint_make(r_top.left,
			r_top.top + psy_ui_realrectangle_height(&r_top)));		
	psy_ui_drawsolidrectangle(g, r_bottom, self->skin->bottomcolour);
	if (self->machine && self->param) {
		if (!psy_audio_machine_parameter_name(self->machine, self->param, label)) {
			if (!psy_audio_machine_parameter_label(self->machine, self->param, label)) {
				psy_snprintf(label, 128, "%s", "");
			}
		}
		if (!psy_audio_machine_parameter_describe(self->machine, self->param, str)) {
			psy_snprintf(str, 128, "%d",
				(int)psy_audio_machineparam_scaledvalue(self->param));
		}
		if (self->tweaking) {
			psy_ui_setbackgroundcolour(g, self->skin->htopcolour);
			psy_ui_settextcolour(g, self->skin->fonthtopcolour);
		} else {
			psy_ui_setbackgroundcolour(g, self->skin->topcolour);
			psy_ui_settextcolour(g, self->skin->fonttopcolour);
		}
		psy_ui_textoutrectangle(g, psy_ui_realrectangle_topleft(&r_top),
			psy_ui_ETO_OPAQUE, r_top, label, strlen(label));
		psy_ui_setbackgroundcolour(g, (self->tweaking)
			? self->skin->hbottomcolour : self->skin->bottomcolour);
		psy_ui_settextcolour(g, (self->tweaking)
			? self->skin->fonthbottomcolour : self->skin->fontbottomcolour);
		psy_ui_textoutrectangle(g, psy_ui_realrectangle_topleft(&r_bottom),
			psy_ui_ETO_OPAQUE, r_bottom, str, strlen(str));
		if (!psy_ui_bitmap_empty(&self->skin->knobbitmap)) {
			intptr_t knob_frame;

			knob_frame = (intptr_t)(
				(psy_audio_machine_parameter_normvalue(self->machine, self->param) * 63.f));
			if (self->size.height < psy_ui_realrectangle_height(&self->skin->knob.dest)) {
				double ratio;
				double w;

				ratio = self->size.height / (double)psy_ui_realrectangle_height(&self->skin->knob.dest);
				w = ratio * psy_ui_realrectangle_width(&self->skin->knob.dest);
				psy_ui_drawstretchedbitmap(g, &self->skin->knobbitmap,
					psy_ui_realrectangle_make(psy_ui_realpoint_zero(),
						psy_ui_realsize_make(w, self->size.height)),
					psy_ui_realpoint_make(knob_frame *
						psy_ui_realrectangle_height(&self->skin->knob.dest), 0.0),
					psy_ui_realrectangle_size(&self->skin->knob.dest));
			} else {
				psy_ui_drawbitmap(g, &self->skin->knobbitmap,
					self->skin->knob.dest, psy_ui_realpoint_make(knob_frame *
						psy_ui_realrectangle_width(&self->skin->knob.dest), 0));
			}
		}
	}
	psy_ui_setcolour(g, psy_ui_colour_make(0x00232323));	
	psy_ui_drawline(g,
		psy_ui_realpoint_make(0, self->size.height - 1),
		psy_ui_realpoint_make(self->size.width, self->size.height - 1));
	psy_ui_drawline(g,
		psy_ui_realpoint_make(self->size.width - 1, 0),
		psy_ui_realpoint_make(self->size.width - 1,
			self->size.height - 1));
}

// SliderDraw
// implementation
void sliderdraw_init(SliderDraw* self, ParamSkin* skin,
	psy_audio_Machine* machine, psy_audio_MachineParam* param,
	psy_ui_RealSize size, const psy_ui_TextMetric* tm, bool tweaking,
	bool drawlabel)
{
	self->machine = machine;
	self->skin = skin;
	self->param = param;
	self->size = size;
	self->tweaking = tweaking;
	self->tm = tm;
	self->drawlabel = drawlabel;
}

void sliderdraw_draw(SliderDraw* self, psy_ui_Graphics* g)
{
	double xoffset;
	double yoffset;
	double value;
	psy_ui_RealRectangle r;
	char str[128];
	bool drawparamname;

	// todo: make the slider scalable
	str[0] = '\0';	
	psy_ui_setrectangle(&r, 0, 0, self->size.width, self->size.height);
	psy_ui_drawsolidrectangle(g, r, self->skin->bottomcolour);
	skin_blitcoord(g, &self->skin->mixerbitmap,
		psy_ui_realpoint_zero(), &self->skin->slider);
	xoffset = (psy_ui_realrectangle_width(&self->skin->slider.dest) -
		psy_ui_realrectangle_width(&self->skin->knob.dest)) / 2;
	if (self->param) {
		if (self->machine) {
			value = psy_audio_machine_parameter_normvalue(self->machine,
				self->param);
		} else {
			value = psy_audio_machineparam_normvalue(self->param);
		}
	} else {
		value = 0.f;
	}
	yoffset = ((1.0 - value) *
		(psy_ui_realrectangle_height(&self->skin->slider.dest) -
			psy_ui_realrectangle_height(&self->skin->sliderknob.dest)));
	skin_blitcoord(g, &self->skin->mixerbitmap,
		psy_ui_realpoint_make(xoffset, yoffset),
		&self->skin->sliderknob);
	drawparamname = FALSE;
	if (self->param && self->drawlabel) {
		if (self->machine) {
			// call with proxy protection via machine
			if (psy_audio_machine_parameter_name(self->machine, self->param, str) != FALSE) {
				drawparamname = TRUE;
			}
		} else {
			if (psy_audio_machineparam_name(self->param, str) != FALSE) {
				drawparamname = TRUE;
			}
		}
	}
	if (drawparamname != FALSE) {
		psy_ui_setbackgroundcolour(g, self->skin->topcolour);
		psy_ui_settextcolour(g, self->skin->fonttopcolour);
		psy_ui_setrectangle(&r,
			32, self->skin->slider.dest.bottom -
			self->skin->slider.dest.top - 48,
			self->size.width - 32, 24);
		psy_ui_textoutrectangle(g, psy_ui_realrectangle_topleft(&r),
			psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
			r, str, strlen(str));
	}
	psy_ui_setrectangle(&r,
		32, psy_ui_realrectangle_height(&self->skin->slider.dest) - 24,
		self->size.width - 32, 24);
	str[0] = '\0';
	if (self->drawlabel && self->param) {
		if (self->machine) {
			// call with proxy protection via machine
			if (psy_audio_machine_parameter_describe(self->machine, self->param, str) == FALSE) {
				psy_snprintf(str, 128, "%d",
					psy_audio_machine_parameter_normvalue(self->machine, self->param));
			}
		} else {			
			if (psy_audio_machineparam_describe(self->param, str) == FALSE) {
				psy_snprintf(str, 128, "%d",
					psy_audio_machineparam_normvalue(self->param));
			}
		}
	}
	if (self->drawlabel) {
		psy_ui_setbackgroundcolour(g, self->skin->bottomcolour);
		psy_ui_settextcolour(g, self->skin->fontbottomcolour);
		psy_ui_textoutrectangle(g,
			psy_ui_realpoint_make(
				32,
				psy_ui_realrectangle_height(&self->skin->slider.dest) - 24),
			psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
			r, str, strlen(str));
	}
}

// ParamTweak
static psy_audio_MachineParam* paramtweak_tweakparam(ParamTweak* self);

void paramtweak_init(ParamTweak* self)
{
	assert(self);

	self->machine = NULL;
	self->paramindex = psy_INDEX_INVALID;
	self->param = NULL;
}

void paramtweak_begin(ParamTweak* self, psy_audio_Machine* machine,
	uintptr_t paramindex)
{
	assert(self);

	self->machine = machine;
	self->paramindex = paramindex;
	self->param = NULL;
}

void paramtweak_end(ParamTweak* self)
{
	assert(self);

	self->machine = NULL;
	self->paramindex = psy_INDEX_INVALID;
	self->param = NULL;
}

void paramtweak_onmousedown(ParamTweak* self, psy_ui_MouseEvent* ev)
{	
	psy_audio_MachineParam* param;

	assert(self);

	param = paramtweak_tweakparam(self);
	if (ev->button == 1 && param) {
		uintptr_t paramtype;

		self->tweakbase = (float)ev->pt.y;
		if (self->machine) {
			self->tweakval = psy_audio_machine_parameter_normvalue(self->machine, param);
			paramtype = psy_audio_machine_parameter_type(self->machine, param) & ~MPF_SMALL;
		} else {
			self->tweakval = psy_audio_machineparam_normvalue(param);
			paramtype = psy_audio_machineparam_type(param) & ~MPF_SMALL;
		}		
		
		if (paramtype == MPF_SLIDERCHECK || paramtype == MPF_SWITCH) {
			if (self->tweakval == 0.f) {
				if (self->machine) {
					psy_audio_machine_parameter_tweak(self->machine, param, 1.f);
				} else {
					psy_audio_machineparam_tweak(param, 1.f);
				}
			} else {
				if (self->machine) {
					psy_audio_machine_parameter_tweak(self->machine, param, 0.f);
				} else {
					psy_audio_machineparam_tweak(param, 0.f);
				}
			}
		}		
	}
}

void paramtweak_onmousemove(ParamTweak* self, psy_ui_MouseEvent* ev)
{	
	psy_audio_MachineParam* param;	

	assert(self);

	param = paramtweak_tweakparam(self);	
	if (param) {
		uintptr_t paramtype;
		float val;		

		if (self->machine) {
			paramtype = psy_audio_machine_parameter_type(self->machine, param) & ~MPF_SMALL;
		} else {
			paramtype = psy_audio_machineparam_type(param) & ~MPF_SMALL;
		}
		if ((paramtype != MPF_SLIDERCHECK) && (paramtype != MPF_SWITCH)) {
			val = self->tweakval + (self->tweakbase - (float)ev->pt.y) / 200.f;
			if (val > 1.f) {
				val = 1.f;
			} else if (val < 0.f) {
				val = 0.f;
			}
			if (self->machine) {
				psy_audio_machine_parameter_tweak(self->machine, param, val);
			} else {
				psy_audio_machineparam_tweak(param, val);
			}
		}
	}
}

psy_audio_MachineParam* paramtweak_tweakparam(ParamTweak* self)
{
	assert(self);

	if (self->param) {
		return self->param;
	}
	if (self->machine && self->paramindex != psy_INDEX_INVALID) {
		return psy_audio_machine_parameter(self->machine, self->paramindex);
	}
	return NULL;
}

// ParamKnob
// prototypes
static void paramknob_ondestroy(ParamView*);
static void paramknob_ondraw(ParamKnob*, psy_ui_Graphics*);
static void paramknob_onmousedown(ParamKnob*, psy_ui_MouseEvent*);
static void paramknob_onmouseup(ParamKnob*, psy_ui_MouseEvent*);
static void paramknob_onmousemove(ParamKnob*, psy_ui_MouseEvent*);
// static void paramknob_onmousedoubleclick(ParamKnob*, psy_ui_MouseEvent*);
static void paramknob_onpreferredsize(ParamKnob*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static psy_audio_MachineParam* paramknob_tweakparam(ParamKnob*);
// vtable
static psy_ui_ComponentVtable paramknob_vtable;
static bool paramknob_vtable_initialized = FALSE;

static void paramknob_vtable_init(ParamKnob* self)
{
	if (!paramknob_vtable_initialized) {
		paramknob_vtable = *(self->component.vtable);
		paramknob_vtable.ondestroy = (psy_ui_fp_component_ondestroy)
			paramknob_ondestroy;
		paramknob_vtable.ondraw = (psy_ui_fp_component_ondraw)
			paramknob_ondraw;
		paramknob_vtable.onmousedown = (psy_ui_fp_component_onmouseevent)
			paramknob_onmousedown;
		paramknob_vtable.onmousemove = (psy_ui_fp_component_onmouseevent)
			paramknob_onmousemove;
		paramknob_vtable.onmouseup = (psy_ui_fp_component_onmouseevent)
			paramknob_onmouseup;
		// paramknob_vtable.onmousedoubleclick =
		//	(psy_ui_fp_component_onmousedoubleclick)
		//	paramknob_onmousedoubleclick;
		paramknob_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)paramknob_onpreferredsize;
		paramknob_vtable_initialized = TRUE;
	}
}
// implementation
void paramknob_init(ParamKnob* self, psy_ui_Component* parent,
	psy_audio_Machine* machine, uintptr_t paramindex,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	paramknob_vtable_init(self);
	self->component.vtable = &paramknob_vtable;
	psy_ui_component_doublebuffer(&self->component);	
	self->skin = machineparamconfig_skin(
		psycleconfig_macparam(workspace_conf(workspace)));
	psy_ui_component_setbackgroundcolour(&self->component,
		self->skin->bottomcolour);
	paramtweak_init(&self->paramtweak);	
	self->machine = machine;
	self->paramindex = paramindex;
}

void paramknob_ondestroy(ParamView* self)
{	
}

void paramknob_ondraw(ParamKnob* self, psy_ui_Graphics* g)
{	
	psy_ui_RealSize size;
	KnobDraw knobdraw;
	psy_audio_MachineParam* param;
		
	param = paramknob_tweakparam(self);
	size = mpfsize(self->skin,
		psy_ui_component_textmetric(&self->component), MPF_STATE, FALSE);	
	knobdraw_init(&knobdraw, self->skin, self->machine, param,
		size, psy_ui_component_textmetric(&self->component),
		self->paramtweak.paramindex != psy_INDEX_INVALID);
	knobdraw_draw(&knobdraw, g);
}

void paramknob_onmousedown(ParamKnob* self, psy_ui_MouseEvent* ev)
{		
	if (ev->button == 1 && self->paramindex != psy_INDEX_INVALID) {
		paramtweak_begin(&self->paramtweak, self->machine, self->paramindex);
		paramtweak_onmousedown(&self->paramtweak, ev);
		psy_ui_component_capture(&self->component);		
	}
}

void paramknob_onmousemove(ParamKnob* self, psy_ui_MouseEvent* ev)
{		
	if (self->paramtweak.paramindex != psy_INDEX_INVALID) {		
		paramtweak_onmousemove(&self->paramtweak, ev);
		psy_ui_component_invalidate(&self->component);
	}
}

void paramknob_onmouseup(ParamKnob* self, psy_ui_MouseEvent* ev)
{
	paramtweak_end(&self->paramtweak);
	psy_ui_component_releasecapture(&self->component);
	psy_ui_component_invalidate(&self->component);
}

void paramknob_onpreferredsize(ParamKnob* self, const psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_ui_RealSize size;

	size = mpfsize(self->skin, psy_ui_component_textmetric(&self->component),
		MPF_STATE, FALSE);
	psy_ui_size_setpx(rv, size.width, size.height);
}

psy_audio_MachineParam* paramknob_tweakparam(ParamKnob* self)
{
	if (self->machine && self->paramindex != psy_INDEX_INVALID) {
		return psy_audio_machine_parameter(self->machine, self->paramindex);
	}
	return NULL;
}


// ParamView
// Prototypes
static void onpreferredsize(ParamView*, const psy_ui_Size* limit, psy_ui_Size* rv);
static void ondestroy(ParamView*, psy_ui_Component* sender);
static void ondraw(ParamView*, psy_ui_Graphics*);
static void drawparameter(ParamView*, psy_ui_Graphics*, psy_audio_MachineParam*,
	uintptr_t paramindex, uintptr_t row, uintptr_t col);
static void drawslider(ParamView*, psy_ui_Graphics*, psy_audio_MachineParam*,
	uintptr_t paramnum, psy_ui_RealSize);
static void drawsliderlevel(ParamView*, psy_ui_Graphics*,
	psy_audio_MachineParam*, psy_ui_RealSize);
static void drawslidercheck(ParamView*, psy_ui_Graphics*,
	psy_audio_MachineParam*, psy_ui_RealSize);
static void drawswitch(ParamView*, psy_ui_Graphics*, psy_audio_MachineParam*,
	psy_ui_RealSize);
static void drawheader(ParamView*, psy_ui_Graphics*, psy_audio_MachineParam*,
	psy_ui_RealSize);
static void drawinfolabel(ParamView*, psy_ui_Graphics*,
	psy_audio_MachineParam*, psy_ui_RealSize);
static void drawknob(ParamView*, psy_ui_Graphics*, psy_audio_MachineParam*,
	uintptr_t paramindex, psy_ui_RealSize);
static void drawblank(ParamView*, psy_ui_Graphics*, psy_audio_MachineParam*,
	psy_ui_RealSize);
static const psy_ui_RealRectangle* cellposition(const ParamView*,
	uintptr_t row, uintptr_t col);
static void onmousedown(ParamView*, psy_ui_MouseEvent*);
static void onmouseup(ParamView*, psy_ui_MouseEvent*);
static void onmousemove(ParamView*, psy_ui_MouseEvent*);
static void onmousewheel(ParamView*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void onmousedoubleclick(ParamView*, psy_ui_MouseEvent*);
static uintptr_t hittest(ParamView*, psy_ui_RealPoint);
static void ontimer(ParamView*, psy_ui_Component* sender, uintptr_t timerid);
static uintptr_t paramview_numrows(const ParamView*);
static void paramview_computepositions(ParamView*);
static void paramview_clearpositions(ParamView*);
static void mixer_vumeterdraw(ParamView*, psy_ui_Graphics*,
	psy_ui_RealPoint topleft, double value);
static psy_audio_MachineParam* tweakparam(ParamView*);
// vtable
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(ParamView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondraw = (psy_ui_fp_component_ondraw)ondraw;
		vtable.onmousedown = (psy_ui_fp_component_onmouseevent)onmousedown;
		vtable.onmousemove = (psy_ui_fp_component_onmouseevent)onmousemove;
		vtable.onmouseup = (psy_ui_fp_component_onmouseevent)onmouseup;
		vtable.onmousedoubleclick = (psy_ui_fp_component_onmouseevent)
			onmousedoubleclick;
		vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			onpreferredsize;
		vtable_initialized = TRUE;
	}
}
// implementation
void paramview_init(ParamView* self, psy_ui_Component* parent,
	psy_audio_Machine* machine, Workspace* workspace)
{
	psy_Property* pv;

	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_ui_component_doublebuffer(&self->component);	
	self->workspace = workspace;
	self->machine = machine;
	self->numparams = 0;
	paramtweak_init(&self->paramtweak);	
	pv = psy_property_findsection(&self->workspace->config.config, "visual.paramview");
	if (pv) {
		psy_ui_Font font;

		psy_ui_fontinfo_init_string(&self->fontinfo,
			psy_property_at_str(pv, "font", "tahoma;-16"));
		psy_ui_font_init(&font, &self->fontinfo);
		psy_ui_component_setfont(&self->component, &font);
		psy_ui_font_dispose(&font);
	} else {
		psy_ui_fontinfo_init_string(&self->fontinfo, "tahoma;-16");
	}	
	self->skin = machineparamconfig_skin(
		psycleconfig_macparam(workspace_conf(workspace)));
	psy_ui_component_setbackgroundcolour(&self->component,
		self->skin->bottomcolour);
	psy_table_init(&self->positions);
	self->tweak = psy_INDEX_INVALID;
	self->lasttweak = psy_INDEX_INVALID;
	self->sizechanged = 1;
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);		
	psy_signal_connect(&self->component.signal_timer, self, ontimer);
	psy_signal_connect(&self->component.signal_mousewheel, self,
		onmousewheel);
	psy_ui_component_starttimer(&self->component, 0, 50);
}

ParamView* paramview_alloc(void)
{
	return (ParamView*) malloc(sizeof(ParamView));
}

ParamView* paramview_allocinit(psy_ui_Component* parent,
	psy_audio_Machine* machine, Workspace* workspace)
{
	ParamView* rv;

	rv = paramview_alloc();
	if (rv) {
		paramview_init(rv, parent, machine, workspace);
	}
	return rv;
}

void ondestroy(ParamView* self, psy_ui_Component* sender)
{	
	paramview_clearpositions(self);
	psy_table_dispose(&self->positions);
}

void ondraw(ParamView* self, psy_ui_Graphics* g)
{				
	if (self->machine && psy_audio_machine_numparameters(self->machine) > 0
			&& psy_audio_machine_numparametercols(self->machine) > 0) {
		uintptr_t row = 0;
		uintptr_t col = 0;
		uintptr_t numrows = 0;
		uintptr_t paramnum;		
						
		if (self->numparams != psy_audio_machine_numparameters(self->machine)) {
				self->numparams = psy_audio_machine_numparameters(self->machine);
			paramview_computepositions(self);
			self->sizechanged = 1;
			psy_signal_emit(&self->component.signal_preferredsizechanged, self, 0);
		}
		numrows = paramview_numrows(self);
		for (paramnum = 0; paramnum < psy_audio_machine_numparameters(self->machine);
				++paramnum) {			
			psy_audio_MachineParam* machineparam;

			machineparam = psy_audio_machine_parameter(self->machine, paramnum);
			if (machineparam) {
				drawparameter(self, g, machineparam, paramnum, row, col);
			}
			++row;
			if (row >= numrows) {
			  row = 0;
			  ++col;			
			}
		}
	}			
}

void drawparameter(ParamView* self, psy_ui_Graphics* g, psy_audio_MachineParam* param,
	uintptr_t paramindex, uintptr_t row, uintptr_t col)
{			
	const psy_ui_RealRectangle* position;	

	position = cellposition(self, row, col);	
	if (position) {
		psy_ui_RealSize size;
		
		psy_ui_setorigin(g, psy_ui_realpoint_make(-position->left,
			-position->top));
		size = psy_ui_realrectangle_size(position);
		switch (psy_audio_machine_parameter_type(self->machine, param) &
			~MPF_SMALL) {
		case MPF_HEADER:
			drawheader(self, g, param, size);
			break;
		case MPF_INFOLABEL:
			drawinfolabel(self, g, param, size);
			break;
		case MPF_STATE:
			drawknob(self, g, param, paramindex, size);
			break;
		case MPF_SLIDER:
			drawslider(self, g, param, paramindex, size);
			break;
		case MPF_SLIDERCHECK:
			drawslidercheck(self, g, param, size);
			break;
		case MPF_SWITCH:
			drawswitch(self, g, param, size);
			break;
		case MPF_SLIDERLEVEL:
			drawsliderlevel(self, g, param, size);
			break;
		case MPF_NULL:
			drawblank(self, g, param, size);
			break;
		default:
			break;
		}
		psy_ui_resetorigin(g);
	}
}

void drawknob(ParamView* self, psy_ui_Graphics* g, psy_audio_MachineParam* param,
	uintptr_t paramnum, psy_ui_RealSize size)
{		
	KnobDraw knobdraw;
	
	knobdraw_init(&knobdraw, self->skin, self->machine, param,
		size, psy_ui_component_textmetric(&self->component),
		self->tweak == paramnum);	
	knobdraw_draw(&knobdraw, g);	
}

void drawheader(ParamView* self, psy_ui_Graphics* g,
	psy_audio_MachineParam* param, psy_ui_RealSize size)
{	
	double half;
	double quarter;
	psy_ui_RealRectangle r;		
	char str[128];
		
	half = size.height / 2;
	quarter = half / 2;
	psy_ui_realrectangle_init_all(&r, psy_ui_realpoint_zero(), size);
	psy_ui_drawsolidrectangle(g, r, self->skin->topcolour);
	psy_ui_setrectangle(&r, 0, half + quarter, size.width, quarter);
	psy_ui_drawsolidrectangle(g, r, self->skin->bottomcolour);
	if (!psy_audio_machine_parameter_name(self->machine, param, str)) {
		if (!psy_audio_machine_parameter_label(self->machine, param, str)) {
			psy_snprintf(str, 128, "%s", "");
		}
	}	
	psy_ui_setrectangle(&r, 0, quarter, size.width, half);
	psy_ui_setbackgroundcolour(g, self->skin->titlecolour);
	psy_ui_settextcolour(g, self->skin->fonttitlecolour);
	// todo font_bold
	psy_ui_textoutrectangle(g, psy_ui_realpoint_make(0, quarter),
		psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED, r, str,
		strlen(str));	
}

void drawinfolabel(ParamView* self, psy_ui_Graphics* g,
	psy_audio_MachineParam* param, psy_ui_RealSize size)
{	
	double half;
	psy_ui_RealRectangle r;
	char str[128];
			
	str[0] = '\0';		
	half = size.height / 2;
	psy_ui_setrectangle(&r, 0.0, 0.0, size.width, half);
	psy_ui_setbackgroundcolour(g, self->skin->titlecolour);
	psy_ui_settextcolour(g, self->skin->fonttitlecolour);
	if (!psy_audio_machine_parameter_name(self->machine, param, str)) {
		if (!psy_audio_machine_parameter_label(self->machine, param, str)) {
			psy_snprintf(str, 128, "%s", "");
		}
	}
	psy_ui_textoutrectangle(g, 
		psy_ui_realpoint_zero(), psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		r, str, strlen(str));	
	if (psy_audio_machine_parameter_describe(self->machine, param, str) == FALSE) {
		psy_snprintf(str, 128, "%s", "");
	}
	psy_ui_setbackgroundcolour(g, self->skin->bottomcolour);
	psy_ui_settextcolour(g, self->skin->fontbottomcolour);
	psy_ui_setrectangle(&r, 0, half, size.width, half);
	psy_ui_textoutrectangle(g, psy_ui_realpoint_make(0, half),
		psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		r, str, strlen(str));	
}

void drawslider(ParamView* self, psy_ui_Graphics* g, psy_audio_MachineParam* param,
	uintptr_t paramnum, psy_ui_RealSize size)
{	
	SliderDraw draw;

	sliderdraw_init(&draw, self->skin, self->machine, param,
		size, psy_ui_component_textmetric(&self->component),
		self->tweak == paramnum, TRUE);
	sliderdraw_draw(&draw, g);	
}

void drawsliderlevel(ParamView* self, psy_ui_Graphics* g, psy_audio_MachineParam* param,
	psy_ui_RealSize size)
{	
	mixer_vumeterdraw(self, g, psy_ui_realpoint_zero(),
		(double)psy_audio_machineparam_normvalue(param));	
}

void mixer_vumeterdraw(ParamView* self, psy_ui_Graphics* g, psy_ui_RealPoint topleft, double value)
{	
	double ypos;

	if (value < 0.0) value = 0.0;
	if (value > 1.0) value = 1.0;
	ypos = (1.0 - value) * psy_ui_realrectangle_height(&self->skin->vuoff.dest);
	psy_ui_drawbitmap(g, &self->skin->mixerbitmap,
		psy_ui_realrectangle_make(
			topleft,
			psy_ui_realsize_make(
				psy_ui_realrectangle_width(&self->skin->vuoff.dest),
				ypos)),
		psy_ui_realrectangle_topleft(&self->skin->vuoff.src));
	psy_ui_drawbitmap(g, &self->skin->mixerbitmap,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(topleft.x, topleft.y + ypos),
			psy_ui_realsize_make(
				psy_ui_realrectangle_width(&self->skin->vuoff.dest),
				psy_ui_realrectangle_height(&self->skin->vuoff.dest) - ypos)),
		psy_ui_realpoint_make(
			self->skin->vuon.src.left,
			self->skin->vuon.src.top + ypos));
}

void drawslidercheck(ParamView* self, psy_ui_Graphics* g, psy_audio_MachineParam* param,
	psy_ui_RealSize size)
{	
	double centery;
	psy_ui_RealRectangle r;
	char label[128];
	const psy_ui_TextMetric* tm;
	
	centery = (size.height - psy_ui_realrectangle_height(&self->skin->checkoff.dest)) / 2;
	if (psy_audio_machine_parameter_normvalue(self->machine, param) == 0.f) {
		skin_blitcoord(g, &self->skin->mixerbitmap,
			psy_ui_realpoint_make(0, centery), &self->skin->checkoff);
	} else {
		skin_blitcoord(g, &self->skin->mixerbitmap,
			psy_ui_realpoint_make(0, centery), & self->skin->checkon);
	}
	psy_ui_setrectangle(&r, 20, 0, size.width, size.height);
	if (!psy_audio_machine_parameter_name(self->machine, param, label)) {
		if (!psy_audio_machineparam_label(param, label)) {
			psy_snprintf(label, 128, "%s", "");
		}
	}
	tm = psy_ui_component_textmetric(&self->component);
	centery = (size.height - tm->tmHeight) / 2;
	psy_ui_textoutrectangle(g, psy_ui_realpoint_make(20, centery),
		psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		r, label, strlen(label));	
}

void drawswitch(ParamView* self, psy_ui_Graphics* g,
	psy_audio_MachineParam* param, psy_ui_RealSize size)
{	
	psy_ui_RealRectangle r;
	char label[128];
	
	psy_ui_setrectangle(&r, 0, 0, size.width, size.height);
	psy_ui_drawsolidrectangle(g, r, self->skin->bottomcolour);
	if (psy_audio_machine_parameter_normvalue(self->machine, param) == 0.f) {
		skin_blitcoord(g, &self->skin->mixerbitmap,
			psy_ui_realpoint_zero(), &self->skin->switchoff);
	}
	else {
		skin_blitcoord(g, &self->skin->mixerbitmap,
			psy_ui_realpoint_zero(), &self->skin->switchon);
	}	
	if (!psy_audio_machine_parameter_name(self->machine, param, label)) {
		if (!psy_audio_machine_parameter_label(self->machine, param, label)) {
			psy_snprintf(label, 128, "%s", "");
		}
	}
	psy_ui_setbackgroundcolour(g, self->skin->topcolour);
	psy_ui_settextcolour(g, self->skin->fonttopcolour);
	psy_ui_setrectangle(&r,
		psy_ui_realrectangle_width(&self->skin->switchon.dest), 0,
		size.width, size.height / 2);
	psy_ui_textoutrectangle(g,
		psy_ui_realpoint_make(
			psy_ui_realrectangle_width(&self->skin->switchon.dest), 0.0),
		psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		r, label, strlen(label));	
}

void drawblank(ParamView* self, psy_ui_Graphics* g, psy_audio_MachineParam* param,
	psy_ui_RealSize size)
{	
	psy_ui_RealRectangle r;
	
	psy_ui_setrectangle(&r, 0, 0, size.width, size.height);
	psy_ui_drawsolidrectangle(g, r, self->skin->bottomcolour);	
}

const psy_ui_RealRectangle* cellposition(const ParamView* self, uintptr_t row,
	uintptr_t col)
{		
	return (const psy_ui_RealRectangle*)psy_table_at_const(&self->positions,
		paramview_numrows(self) * col + row);	
}

void onmousedown(ParamView* self, psy_ui_MouseEvent* ev)
{
	self->lasttweak = hittest(self, ev->pt);
	if (ev->button == 1) {		
		self->tweak = self->lasttweak;		
		if (self->tweak != psy_INDEX_INVALID) {
			paramtweak_begin(&self->paramtweak, self->machine, self->tweak);
			psy_ui_component_capture(&self->component);
			paramtweak_onmousedown(&self->paramtweak, ev);			
		}
	}
	psy_ui_mouseevent_stoppropagation(ev);
}

psy_audio_MachineParam* tweakparam(ParamView* self)
{
	psy_audio_MachineParam* rv = NULL;
	
	if (self->machine && self->tweak != psy_INDEX_INVALID) {
		rv = psy_audio_machine_parameter(self->machine, self->tweak);
	}
	return rv;
}

uintptr_t hittest(ParamView* self, psy_ui_RealPoint pt)
{
	uintptr_t rv = psy_INDEX_INVALID;

	if (self->machine) {
		uintptr_t paramnum;

		for (paramnum = 0;	paramnum < psy_table_size(&self->positions);
				++paramnum) {			
			psy_audio_MachineParam* param;
			
			param = psy_audio_machine_parameter(self->machine, paramnum);
			if (param && ((psy_audio_machine_parameter_type(self->machine, param) & MPF_IGNORE)
					!= MPF_IGNORE)) {
				psy_ui_RealRectangle* position;

				position = (psy_ui_RealRectangle*)psy_table_at(&self->positions,
					paramnum);
				if (position && psy_ui_realrectangle_intersect(position, pt)) {
					rv = paramnum;
				}
				// break;
			}
		}
	}	
	return rv;
}

void onmousemove(ParamView* self, psy_ui_MouseEvent* ev)
{		
	if (self->paramtweak.paramindex != psy_INDEX_INVALID) {		
		paramtweak_onmousemove(&self->paramtweak, ev);		
		psy_ui_component_invalidate(&self->component);
	}
	psy_ui_mouseevent_stoppropagation(ev);
}

void onmouseup(ParamView* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_component_releasecapture(&self->component);
	paramtweak_end(&self->paramtweak);	
	self->tweak = psy_INDEX_INVALID;
	psy_ui_mouseevent_stoppropagation(ev);
}

void onmousewheel(ParamView* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	psy_audio_MachineParam* param;

	self->tweak = hittest(self, ev->pt);
	param = tweakparam(self);
	if (param != NULL) {
		self->tweakval = psy_audio_machine_parameter_normvalue(self->machine, param);
		if (ev->delta > 0) {
			float val;
			
			val = self->tweakval + 1 / 200.f;
			if (val > 1.f) {
				val = 1.f;
			}
			else
				if (val < 0.f) {
					val = 0.f;
				}
			psy_audio_machine_parameter_tweak(self->machine, param, val);			
			psy_ui_component_invalidate(&self->component);
		} else
		if (ev->delta < 0) {
			float val;

			val = self->tweakval - 1 / 200.f;
			if (val > 1.f) {
				val = 1.f;
			} else
			if (val < 0.f) {
				val = 0.f;
			}
			psy_audio_machine_parameter_tweak(self->machine, param, val);			
			psy_ui_component_invalidate(&self->component);
		}
	}
	ev->preventdefault = 1;
	self->tweak = psy_INDEX_INVALID;
}

void onmousedoubleclick(ParamView* self, psy_ui_MouseEvent* ev)
{
	if (self->machine) {
		psy_audio_MachineParam* tweakpar;
		uintptr_t paramindex;

		paramindex = hittest(self, ev->pt);
		if (paramindex != psy_INDEX_INVALID) {
			tweakpar = psy_audio_machine_parameter(self->machine, paramindex);
			if (tweakpar) {
				// psy_audio_machine_parameter_reset(self->machine, tweakpar);
			}
		}
	}
}

void ontimer(ParamView* self, psy_ui_Component* sender, uintptr_t timerid)
{
	psy_ui_component_invalidate(&self->component);
}

void onpreferredsize(ParamView* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	if (self->machine && psy_audio_machine_numparameters(self->machine) > 0) {				
		paramview_computepositions(self);
		*rv = self->cpmax;
		self->sizechanged = 0;
	} else {
		*rv = psy_ui_component_size(&self->component);
		self->sizechanged = 0;
	}
}

void paramview_computepositions(ParamView* self)
{
	paramview_clearpositions(self);
	if (self->machine) {
		uintptr_t paramindex;
		uintptr_t numrows;
		uintptr_t row = 0;
		uintptr_t col = 0;
		uintptr_t paramrowbegin = 0;
		double colmax = 0;
		double cpx = 0;
		double cpy = 0;
		double cpy_slidercheck;
		const psy_ui_TextMetric* tm;
		bool haslevel;

		tm = psy_ui_component_textmetric(&self->component);
		numrows = paramview_numrows(self);
		self->cpmax.width = psy_ui_value_makepx(0);
		haslevel = FALSE;
		for (paramindex = 0;
				paramindex < psy_audio_machine_numparameters(self->machine);
				++paramindex) {
			psy_ui_RealRectangle* position;
			psy_ui_RealRectangle* firstrow;
			uintptr_t paramtype;
			bool issmall;
			psy_ui_RealSize size;
			psy_audio_MachineParam* param;

			param = psy_audio_machine_parameter(self->machine, paramindex);
			if (param) {
				paramtype = psy_audio_machine_parameter_type(self->machine, param) & ~MPF_SMALL;
				issmall = (psy_audio_machine_parameter_type(self->machine, param) & MPF_SMALL) == MPF_SMALL;
			} else {
				paramtype = MPF_IGNORE;
				issmall = TRUE;
			}
			position = (psy_ui_RealRectangle*)malloc(sizeof(psy_ui_RealRectangle));
			psy_ui_setrectangle(position, cpx, 0, 0, 0);
			if (paramtype == MPF_SLIDERLEVEL) {
				haslevel = TRUE;
				position->left += psy_ui_realrectangle_width(&self->skin->slider.dest);
			} else if (paramtype == MPF_SLIDERCHECK) {
				if (param->isslidergroup) {
					if (haslevel) {
						position->left += 50;
					} else {
						position->left += 30;
					}
				}
			}
			psy_table_insert(&self->positions, paramindex, position);
			size = mpfsize(self->skin, psy_ui_component_textmetric(&self->component),
				paramtype, issmall);
			if (colmax < size.width) {
				colmax = size.width;
			}
			firstrow = psy_table_at(&self->positions, row);
			if (paramtype != MPF_IGNORE && paramtype != MPF_SLIDERLEVEL &&				
					paramtype != MPF_SLIDERCHECK) {
				if (size.height > (firstrow->bottom - firstrow->top)) {
					firstrow->bottom = firstrow->top + size.height;
				}
			}
			++row;
			if (row >= numrows ||
					paramindex == psy_audio_machine_numparameters(self->machine) - 1) {
				uintptr_t i;

				for (i = paramrowbegin; i <= paramindex; ++i) {
					psy_audio_MachineParam* param;

					param = psy_audio_machine_parameter(self->machine, i);
					position = psy_table_at(&self->positions, i);
					if (param) {
						if (psy_audio_machine_parameter_type(self->machine, param)
									== MPF_SLIDERCHECK ||
								psy_audio_machine_parameter_type(self->machine, param)
									== MPF_SLIDERLEVEL) {
							const psy_ui_RealRectangle* r;

							r = cellposition(self, i, col);							
							position->right = position->left +
								((r) ? psy_ui_realrectangle_width(r) : 0.0);
						} else {
							position->right = cpx + colmax;
						}
					}
				}
				cpx += colmax;
				if (psy_ui_value_px(&self->cpmax.width, tm) < cpx) {
					self->cpmax.width = psy_ui_value_makepx(cpx);
				}
				++col;
				row = 0;
				haslevel = FALSE;
				paramrowbegin = paramindex + 1;
			}
		}
		cpy_slidercheck = 0;	
		row = 0;
		self->cpmax.height = psy_ui_value_makepx(0);
		for (paramindex = 0; paramindex < psy_audio_machine_numparameters(self->machine);
				++paramindex) {
			psy_ui_RealRectangle* position;
			psy_ui_RealRectangle* firstrow;
			psy_ui_RealSize size;
			double height;
			uintptr_t paramtype;
			bool issmall;
			psy_audio_MachineParam* param;
			
			param = psy_audio_machine_parameter(self->machine, paramindex);
			position = psy_table_at(&self->positions, paramindex);
			firstrow = psy_table_at(&self->positions, row);
			if (param) {
				paramtype = psy_audio_machine_parameter_type(self->machine, param) & ~MPF_SMALL;
				issmall = (psy_audio_machine_parameter_type(self->machine, param) & MPF_SMALL) == MPF_SMALL;
			} else {
				paramtype = MPF_IGNORE;
				issmall = TRUE;
			}
			size = mpfsize(self->skin, psy_ui_component_textmetric(&self->component),
				paramtype, issmall);
			height = firstrow->bottom - firstrow->top;
			position->top = cpy;
			if (paramtype == MPF_SLIDER) {
				position->bottom = cpy + size.height;
			} else if (paramtype == MPF_SLIDERCHECK) {								
				position->top += cpy_slidercheck;
				position->bottom = position->top + size.height;
			} else {
				position->bottom = cpy + height;
			}
			if (psy_ui_value_px(&self->cpmax.height, tm) < cpy + height) {
				self->cpmax.height = psy_ui_value_makepx(cpy + height);
			}
			if (paramtype != MPF_IGNORE && paramtype != MPF_SLIDER &&
					paramtype != MPF_SLIDERCHECK) {
				cpy += height;
			}
			if (paramtype == MPF_SLIDERCHECK) {
				cpy_slidercheck += size.height;
			}
			++row;					
			if (row >= numrows) {
				++col;
				row = 0;
				cpy = 0;	
				cpy_slidercheck = 0;				
			}
		}
	}	
}

void paramview_clearpositions(ParamView* self)
{
	psy_table_disposeall(&self->positions, (psy_fp_disposefunc)NULL);	
	psy_table_init(&self->positions);
}

uintptr_t paramview_numrows(const ParamView* self)
{
	if (self->machine &&
			psy_audio_machine_numparametercols(self->machine) > 0) {
		return (psy_audio_machine_numparameters(self->machine) - 1) /
			psy_audio_machine_numparametercols(self->machine) + 1;
	}
	return 0;
}

void paramview_changecontrolskin(const char* path)
{
		
}

void paramview_setzoom(ParamView* self, double zoomrate)
{
	psy_ui_Font font;	
	psy_ui_FontInfo fontinfo;

	fontinfo = self->fontinfo;
	fontinfo.lfHeight = (int32_t)(self->fontinfo.lfHeight * zoomrate);
	psy_ui_font_init(&font, &fontinfo);
	psy_ui_component_setfont(&self->component, &font);
	psy_ui_font_dispose(&font);
	paramview_computepositions(self);
}
