#include "../../detail/prefix.h"

#include "envelopeview.h"

static void OnDraw(EnvelopeView*, psy_ui_Component* sender, psy_ui_Graphics* g);
static void DrawLabel(EnvelopeView*, psy_ui_Graphics*);
static void DrawGrid(EnvelopeView*, psy_ui_Graphics* g);
static void DrawPoints(EnvelopeView*, psy_ui_Graphics* g);
static void DrawLines(EnvelopeView*, psy_ui_Graphics* g);
static void OnSize(EnvelopeView*, psy_ui_Component* sender, psy_ui_Size* size);
static void OnDestroy(EnvelopeView*, psy_ui_Component* component);
static void OnMouseDown(EnvelopeView*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void OnMouseMove(EnvelopeView*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void OnMouseUp(EnvelopeView*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static psy_List* HitTestPoint(EnvelopeView* self, int x, int y);
static void ShiftSuccessors(EnvelopeView* self, double timeshift);
static void CheckAdjustPointRange(psy_List* p);
static int pxvalue(EnvelopeView*, double value);
static int pxtime(EnvelopeView*, double t);
static psy_dsp_seconds_t displaymaxtime(EnvelopeView*);
psy_dsp_EnvelopePoint* allocpoint(psy_dsp_seconds_t time, psy_dsp_amp_t value,
	psy_dsp_seconds_t mintime,
	psy_dsp_seconds_t maxtime, psy_dsp_amp_t minvalue, psy_dsp_amp_t maxvalue);
static void InitPoints(EnvelopeView*);

void adsrpointmapper_init(ADSRPointMapper* self)
{
	psy_dsp_envelopepoint_init(&self->start, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
	psy_dsp_envelopepoint_init(&self->attack, 0.2f, 1.f, 0.f, 5.f, 1.f, 1.f);
	psy_dsp_envelopepoint_init(&self->decay, 0.4f, 0.5f, 0.f, 5.f, 0.f, 1.f);
	psy_dsp_envelopepoint_init(&self->release, 0.6f, 0.f, 0.f, 5.f, 0.f, 0.f);
	self->settings = 0;
}

void adsrpointmapper_updatepoints(ADSRPointMapper* self)
{	
	if (self->settings) {
		self->attack.time = self->settings->attack;
		self->decay.time = self->settings->attack + self->settings->decay;
		self->release.time = self->settings->attack + self->settings->decay + self->settings->release;
		self->decay.value = self->settings->sustain;
	}
}

void adsrpointmapper_updatesettings(ADSRPointMapper* self)
{
	if (self->settings) {
		self->settings->attack = self->attack.time;
		self->settings->decay = self->decay.time - self->attack.time;
		self->settings->sustain = self->decay.value;
		self->settings->release = self->release.time - self->decay.time;
	}
}

void envelopeview_init(EnvelopeView* self, psy_ui_Component* parent)
{				
	ui_component_init(&self->component, parent);	
	psy_ui_margin_init(&self->spacing, 
		psy_ui_value_makepx(30),
		psy_ui_value_makepx(5),
		psy_ui_value_makepx(5),
		psy_ui_value_makepx(5));
	self->text = strdup("");
	ui_component_doublebuffer(&self->component);
	psy_signal_connect(&self->component.signal_draw, self, OnDraw);
	psy_signal_connect(&self->component.signal_destroy, self, OnDestroy);
	psy_signal_connect(&self->component.signal_size, self, OnSize);	
	psy_signal_connect(&self->component.signal_mousedown, self, OnMouseDown);
	psy_signal_connect(&self->component.signal_mousemove, self, OnMouseMove);
	psy_signal_connect(&self->component.signal_mouseup, self, OnMouseUp);	
	self->cx = 0;
	self->cy = 0;
	self->dragpoint = 0;	
	self->points = 0;
	adsr_settings_init(&self->dummysettings, 0.2f, 0.4f, 0.8f, 0.3f);
	adsrpointmapper_init(&self->pointmapper);
	self->points = psy_list_create(&self->pointmapper.start);
	psy_list_append(&self->points, &self->pointmapper.attack);
	psy_list_append(&self->points, &self->pointmapper.decay);
	psy_list_append(&self->points, &self->pointmapper.release);
	self->sustainstage = 2;
	self->dragrelative = 1;
}

void EnvelopeViewSetAdsrEnvelope(EnvelopeView* self,
	psy_dsp_ADSRSettings* adsr_settings)
{		
	self->pointmapper.settings = adsr_settings;
	adsrpointmapper_updatepoints(&self->pointmapper);
}

void OnDestroy(EnvelopeView* self, psy_ui_Component* component)
{	
	psy_list_free(self->points);
	free(self->text);
	self->text = 0;
	self->points = 0;
}

void OnDraw(EnvelopeView* self, psy_ui_Component* sender, psy_ui_Graphics* g)
{		
	DrawLabel(self, g);
	DrawGrid(self, g);
	DrawLines(self, g);
	DrawPoints(self, g);
}

void DrawLabel(EnvelopeView* self, psy_ui_Graphics* g)
{
	ui_setbackgroundmode(g, TRANSPARENT);
	ui_settextcolor(g, 0x00D1C5B6);
	ui_textout(g, 0, 0, self->text, strlen(self->text));
}

void DrawGrid(EnvelopeView* self, psy_ui_Graphics* g)
{
	double i;		

	ui_setcolor(g, 0xFF666666);	
	for (i = 0; i <= 1.0; i += 0.1 ) {
		ui_drawline(g,
			self->spacing.left.quantity.integer,
				pxvalue(self, i),
			self->spacing.left.quantity.integer + self->cx,
				pxvalue(self, i));
	}	
	for (i = 0; i <= displaymaxtime(self); i += 0.5) {
		ui_drawline(g,
				pxtime(self, i),
			self->spacing.top.quantity.integer,
				pxtime(self, i),
			self->spacing.top.quantity.integer + self->cy);
	}
}

void DrawPoints(EnvelopeView* self, psy_ui_Graphics* g)
{
	psy_List* p;
	psy_ui_Size ptsize;
	psy_ui_Size ptsize2;
	psy_dsp_EnvelopePoint* q = 0;

	ptsize.width = 5;
	ptsize.height = 5;
	ptsize2.width = ptsize.width / 2;
	ptsize2.height = ptsize.height / 2;	
	for (p = self->points; p !=0; p = p->next) {
		psy_ui_Rectangle r;
		psy_dsp_EnvelopePoint* pt;

		pt = (psy_dsp_EnvelopePoint*)p->entry;
		psy_ui_setrectangle(&r, 
			pxtime(self, pt->time) - ptsize2.width,
			pxvalue(self, pt->value) - ptsize2.height,
			ptsize.width,
			ptsize.height);
		ui_drawsolidrectangle(g, r, 0x00B1C8B0);		
		q = pt;
	}
}

void DrawLines(EnvelopeView* self, psy_ui_Graphics* g)
{
	psy_List* p;
	psy_dsp_EnvelopePoint* q = 0;
	int count = 0;

	ui_setcolor(g, 0x00B1C8B0);	
	for (p = self->points; p !=0; p = p->next, ++count) {		
		psy_dsp_EnvelopePoint* pt;

		pt = (psy_dsp_EnvelopePoint*)p->entry;			
		if (q) {
			ui_drawline(g,
				pxtime(self, q->time),
				pxvalue(self, q->value),
				pxtime(self, pt->time),
				pxvalue(self, pt->value));
		}
		q = pt;
		if (count == self->sustainstage) {
			ui_drawline(g,
				pxtime(self, q->time),
				self->spacing.top.quantity.integer,
				pxtime(self, q->time),
				self->spacing.top.quantity.integer + self->cy);
		}
	}
}

void OnSize(EnvelopeView* self, psy_ui_Component* sender, psy_ui_Size* size)
{
	self->cx = size->width - self->spacing.left.quantity.integer -
		self->spacing.right.quantity.integer;
	self->cy = size->height - self->spacing.top.quantity.integer -
		self->spacing.bottom.quantity.integer;
}

void OnMouseDown(EnvelopeView* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	self->dragpoint = HitTestPoint(self, ev->x, ev->y);
}

void OnMouseMove(EnvelopeView* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{		
	if (self->dragpoint) {		
		psy_dsp_EnvelopePoint* pt;
		double oldtime;

		pt = (psy_dsp_EnvelopePoint*)self->dragpoint->entry;
		oldtime = pt->time;
		pt->value = 1.f - (ev->y - self->spacing.top.quantity.integer) /
			(float)self->cy;
		pt->time = (ev->x - self->spacing.left.quantity.integer) *
			displaymaxtime(self) /
			(float)self->cx;
		CheckAdjustPointRange(self->dragpoint);
		ShiftSuccessors(self, pt->time - oldtime);
		adsrpointmapper_updatesettings(&self->pointmapper);
		ui_component_invalidate(&self->component);
	}
}

void ShiftSuccessors(EnvelopeView* self, double timeshift)
{	
	if (self->dragrelative) {
		psy_List* p;
		for (p = self->dragpoint->next; p != 0; p = p->next) {		
			psy_dsp_EnvelopePoint* pt;		

			pt = (psy_dsp_EnvelopePoint*)p->entry;
			pt->time += (float)timeshift;
			CheckAdjustPointRange(p);
		}
	}
}

void CheckAdjustPointRange(psy_List* p)
{
	psy_dsp_EnvelopePoint* pt;	

	pt = (psy_dsp_EnvelopePoint*) p->entry;	
	if (p->prev) {
		psy_dsp_EnvelopePoint* ptprev;

		ptprev = (psy_dsp_EnvelopePoint*) p->prev->entry;
		if (pt->time < ptprev->time) {
			pt->time = ptprev->time;
		}
	}
	if (pt->value > pt->maxvalue) {
		pt->value = pt->maxvalue;
	} else
	if (pt->value < pt->minvalue) {
		pt->value = pt->minvalue;
	}
	if (pt->time > pt->maxtime) {
		pt->time = pt->maxtime;
	} else
	if (pt->time < pt->mintime) {
		pt->time = pt->mintime;
	}	
}

void OnMouseUp(EnvelopeView* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{	
	self->dragpoint = 0;
}

psy_List* HitTestPoint(EnvelopeView* self, int x, int y)
{
	psy_List* p = 0;	
	
	for (p = self->points; p != 0; p = p->next) {		
		psy_dsp_EnvelopePoint* pt;		

		pt = (psy_dsp_EnvelopePoint*)p->entry;			
		if (abs(pxtime(self, pt->time) - x) < 5 &&
				abs(pxvalue(self, pt->value) - y) < 5) {			
			break;
		}
	}	
	return p;
}

int pxvalue(EnvelopeView* self, double value)
{
	return (int)(self->cy - value * self->cy) +
		self->spacing.top.quantity.integer;
}

int pxtime(EnvelopeView* self, double t)
{
	return (int)(t * self->cx / displaymaxtime(self)) +
		self->spacing.left.quantity.integer;
}

float displaymaxtime(EnvelopeView* self)
{
	return 5.f;
}

psy_dsp_EnvelopePoint* allocpoint(psy_dsp_seconds_t time, psy_dsp_amp_t value,
	psy_dsp_seconds_t mintime, psy_dsp_seconds_t maxtime,
	psy_dsp_amp_t minvalue, psy_dsp_amp_t maxvalue)
{
	psy_dsp_EnvelopePoint* rv;

	rv = (psy_dsp_EnvelopePoint*) malloc(sizeof(psy_dsp_EnvelopePoint));
	rv->time = time;
	rv->value = value;
	rv->mintime = mintime;
	rv->maxtime = maxtime;
	rv->minvalue = minvalue;
	rv->maxvalue = maxvalue;
	return rv;
}

void EnvelopeViewUpdate(EnvelopeView* self)
{
	if (self->points && !self->dragpoint) {
		adsrpointmapper_updatepoints(&self->pointmapper);
		ui_component_invalidate(&self->component);
	}
}

void envelopeview_settext(EnvelopeView* self, const char* text)
{
	free(self->text);
	self->text = strdup(text);
}
