#include "../../detail/prefix.h"

#include "envelopeview.h"

static void OnDraw(EnvelopeView*, ui_component* sender, ui_graphics* g);
static void DrawGrid(EnvelopeView*, ui_graphics* g);
static void DrawPoints(EnvelopeView*, ui_graphics* g);
static void DrawLines(EnvelopeView*, ui_graphics* g);
static void OnSize(EnvelopeView*, ui_component* sender, ui_size* size);
static void OnDestroy(EnvelopeView*, ui_component* component);
static void OnMouseDown(EnvelopeView*, ui_component* sender, int x, int y, int button);
static void OnMouseMove(EnvelopeView*, ui_component* sender, int x, int y, int button);
static void OnMouseUp(EnvelopeView*, ui_component* sender, int x, int y, int button);
static List* HitTestPoint(EnvelopeView* self, int x, int y);
static void ShiftSuccessors(EnvelopeView* self, double timeshift);
static void CheckAdjustPointRange(List* p);
static int pxvalue(EnvelopeView*, double value);
static int pxtime(EnvelopeView*, double t);
static seconds_t displaymaxtime(EnvelopeView*);
EnvelopePoint* allocpoint(seconds_t time, amp_t value, seconds_t mintime,
	seconds_t maxtime, amp_t minvalue, amp_t maxvalue);
static void InitPoints(EnvelopeView*);

void adsrpointmapper_init(ADSRPointMapper* self)
{
	envelopepoint_init(&self->start, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
	envelopepoint_init(&self->attack, 0.2f, 1.f, 0.f, 5.f, 1.f, 1.f);
	envelopepoint_init(&self->decay, 0.4f, 0.5f, 0.f, 5.f, 0.f, 1.f);
	envelopepoint_init(&self->release, 0.6f, 0.f, 0.f, 5.f, 0.f, 0.f);
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

void InitEnvelopeView(EnvelopeView* self, ui_component* parent)
{				
	ui_component_init(&self->component, parent);	
	ui_margin_init(&self->spacing, 5, 5, 5, 5);
	self->component.doublebuffered = 1;
	signal_connect(&self->component.signal_draw, self, OnDraw);
	signal_connect(&self->component.signal_destroy, self, OnDestroy);
	signal_connect(&self->component.signal_size, self, OnSize);	
	signal_connect(&self->component.signal_mousedown, self, OnMouseDown);
	signal_connect(&self->component.signal_mousemove, self, OnMouseMove);
	signal_connect(&self->component.signal_mouseup, self, OnMouseUp);	
	self->cx = 0;
	self->cy = 0;
	self->dragpoint = 0;	
	self->points = 0;
	adsr_settings_init(&self->dummysettings, 0.2f, 0.4f, 0.8f, 0.3f);
	adsrpointmapper_init(&self->pointmapper);
	self->points = list_create(&self->pointmapper.start);
	list_append(&self->points, &self->pointmapper.attack);
	list_append(&self->points, &self->pointmapper.decay);
	list_append(&self->points, &self->pointmapper.release);
	self->sustainstage = 2;
	self->dragrelative = 1;
}

void EnvelopeViewSetAdsrEnvelope(EnvelopeView* self, ADSRSettings* adsr_settings)
{		
	self->pointmapper.settings = adsr_settings;
	adsrpointmapper_updatepoints(&self->pointmapper);
}

void OnDestroy(EnvelopeView* self, ui_component* component)
{	
	list_free(self->points);
	self->points = 0;
}

void OnDraw(EnvelopeView* self, ui_component* sender, ui_graphics* g)
{		
	DrawGrid(self, g);
	DrawLines(self, g);
	DrawPoints(self, g);
}

void DrawGrid(EnvelopeView* self, ui_graphics* g)
{
	double i;		

	ui_setcolor(g, 0xFF666666);	
	for (i = 0; i <= 1.0; i += 0.1 ) {
		ui_drawline(g,
			self->spacing.left,
			pxvalue(self, i),
			self->spacing.left + self->cx,
			pxvalue(self, i));
	}	
	for (i = 0; i <= displaymaxtime(self); i += 0.5) {
		ui_drawline(g,
			pxtime(self, i),
			self->spacing.top,
			pxtime(self, i),
			self->spacing.top + self->cy);
	}
}

void DrawPoints(EnvelopeView* self, ui_graphics* g)
{
	List* p;
	ui_size ptsize;
	ui_size ptsize2;
	EnvelopePoint* q = 0;

	ptsize.width = 5;
	ptsize.height = 5;
	ptsize2.width = ptsize.width / 2;
	ptsize2.height = ptsize.height / 2;	
	for (p = self->points; p !=0; p = p->next) {
		ui_rectangle r;
		EnvelopePoint* pt;

		pt = (EnvelopePoint*)p->entry;
		ui_setrectangle(&r, 
			pxtime(self, pt->time) - ptsize2.width,
			pxvalue(self, pt->value) - ptsize2.height,
			ptsize.width,
			ptsize.height);
		ui_drawsolidrectangle(g, r, 0x00B1C8B0);		
		q = pt;
	}
}

void DrawLines(EnvelopeView* self, ui_graphics* g)
{
	List* p;
	EnvelopePoint* q = 0;
	int count = 0;

	ui_setcolor(g, 0x00B1C8B0);	
	for (p = self->points; p !=0; p = p->next, ++count) {		
		EnvelopePoint* pt;

		pt = (EnvelopePoint*)p->entry;			
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
				self->spacing.top,
				pxtime(self, q->time),
				self->spacing.top + self->cy);
		}
	}
}

void OnSize(EnvelopeView* self, ui_component* sender, ui_size* size)
{
	self->cx = size->width - self->spacing.left - self->spacing.right;
	self->cy = size->height - self->spacing.top - self->spacing.bottom;
}

void OnMouseDown(EnvelopeView* self, ui_component* sender, int x, int y, int button)
{
	self->dragpoint = HitTestPoint(self, x, y);
}

void OnMouseMove(EnvelopeView* self, ui_component* sender, int x, int y, int button)
{		
	if (self->dragpoint) {		
		EnvelopePoint* pt;
		double oldtime;

		pt = (EnvelopePoint*)self->dragpoint->entry;
		oldtime = pt->time;
		pt->value = 1.f - (y - self->spacing.top)/(float)self->cy;
		pt->time = (x - self->spacing.left) * displaymaxtime(self)/(float)self->cx;				
		CheckAdjustPointRange(self->dragpoint);
		ShiftSuccessors(self, pt->time - oldtime);
		adsrpointmapper_updatesettings(&self->pointmapper);
		ui_invalidate(&self->component);
	}
}

void ShiftSuccessors(EnvelopeView* self, double timeshift)
{	
	if (self->dragrelative) {
		List* p;
		for (p = self->dragpoint->next; p != 0; p = p->next) {		
			EnvelopePoint* pt;		

			pt = (EnvelopePoint*)p->entry;
			pt->time += (float)timeshift;
			CheckAdjustPointRange(p);
		}
	}
}

void CheckAdjustPointRange(List* p)
{
	EnvelopePoint* pt;	

	pt = (EnvelopePoint*) p->entry;	
	if (p->prev) {
		EnvelopePoint* ptprev;

		ptprev = (EnvelopePoint*) p->prev->entry;
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

void OnMouseUp(EnvelopeView* self, ui_component* sender, int x, int y, int button)
{	
	self->dragpoint = 0;
}

List* HitTestPoint(EnvelopeView* self, int x, int y)
{
	List* p = 0;	
	
	for (p = self->points; p != 0; p = p->next) {		
		EnvelopePoint* pt;		

		pt = (EnvelopePoint*)p->entry;			
		if (abs(pxtime(self, pt->time) - x) < 5 &&
				abs(pxvalue(self, pt->value) - y) < 5) {			
			break;
		}
	}	
	return p;
}

int pxvalue(EnvelopeView* self, double value)
{
	return (int)(self->cy - value*self->cy) + self->spacing.left;
}

int pxtime(EnvelopeView* self, double t)
{
	return (int)(t * self->cx / displaymaxtime(self)) + self->spacing.top;
}

float displaymaxtime(EnvelopeView* self)
{
	return 5.f;
}

EnvelopePoint* allocpoint(seconds_t time, amp_t value, seconds_t mintime, seconds_t maxtime, amp_t minvalue, amp_t maxvalue)
{
	EnvelopePoint* rv;

	rv = (EnvelopePoint*) malloc(sizeof(EnvelopePoint));
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
		ui_invalidate(&self->component);
	}
}