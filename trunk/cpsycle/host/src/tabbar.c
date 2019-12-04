#include "../../detail/prefix.h"

#include "tabbar.h"
#include <string.h>

static void OnDraw(TabBar* self, ui_component* sender, ui_graphics* g);
static void OnDestroy(TabBar* self, ui_component* component);
static void OnMouseDown(TabBar* self, ui_component* sender, MouseEvent*);
static void OnMouseMove(TabBar* self, ui_component* sender, MouseEvent*);
static void onmouseenter(TabBar*, ui_component* sender);
static void onmouseleave(TabBar*, ui_component* sender);
static int tabhittest(TabBar* self, int x, int y);
static void onalign(TabBar*, ui_component* sender);
static void onpreferredsize(TabBar*, ui_component* sender, ui_size* limit, ui_size* rv);

void InitTab(Tab* self, const char* text, ui_size* size)
{
	self->text = _strdup(text);
	if (size) {
		self->size = *size;
	}	
	ui_margin_init(&self->margin,
		ui_value_makepx(0),
		ui_value_makeew(0),
		ui_value_makepx(0),
		ui_value_makepx(0));
}

void DisposeTab(Tab* self)
{
	free(self->text);
}

void tabbar_init(TabBar* self, ui_component* parent)
{
	ui_component_init(&self->component, parent);	
	self->component.doublebuffered = 1;	
	signal_init(&self->signal_change);
	signal_connect(&self->component.signal_draw, self, OnDraw);
	signal_connect(&self->component.signal_destroy, self, OnDestroy);
	signal_connect(&self->component.signal_align, self, onalign);
	signal_connect(&self->component.signal_preferredsize, self, onpreferredsize);
	signal_connect(&self->component.signal_mousedown, self, OnMouseDown);
	signal_connect(&self->component.signal_mousemove, self, OnMouseMove);
	signal_connect(&self->component.signal_mouseenter, self, onmouseenter);
	signal_connect(&self->component.signal_mouseleave, self, onmouseleave);
	ui_component_resize(&self->component,0, 20);
	self->tabs = 0;
	self->selected = 0;
	self->hover = 0;
	self->hoverindex = -1;
	self->tabalignment = UI_ALIGN_TOP;
}

void OnDestroy(TabBar* self, ui_component* component)
{	
	List* p;	

	for (p = self->tabs; p != 0; p = p->next) {	
		DisposeTab((Tab*)(p->entry));
		free(p->entry);
	}
	list_free(self->tabs);
	signal_dispose(&self->signal_change);	
}

void OnDraw(TabBar* self, ui_component* sender, ui_graphics* g)
{	
	List* tabs;
	int c = 0;
	int cpx = 0;
	int cpy = 0;
	int cpxsel = 0;
	int selwidth = 0;	
	int cpxhover = 0;
	int hoverwidth = 0;
	ui_size size;
	ui_textmetric tm;
	
	size = ui_component_size(&self->component);
	tm = ui_component_textmetric(&self->component);
	ui_setbackgroundmode(g, TRANSPARENT);	
	if (self->tabalignment == UI_ALIGN_TOP) {
		cpx = 0;
	} else 
	if (self->tabalignment == UI_ALIGN_RIGHT) {
		cpy = 5;
		cpx = 10;
	} else
	if (self->tabalignment == UI_ALIGN_LEFT) {		
		cpy = 5;
	}
	for (tabs = self->tabs; tabs != 0; tabs = tabs->next, ++c) {
		Tab* tab;

		tab = (Tab*)tabs->entry;
		if (self->selected == c) {
			ui_settextcolor(g, 0x00B1C8B0);
			cpxsel = cpx + ui_value_px(&tab->margin.left, &tm);
			selwidth = tab->size.width;
		}
		if (self->hover && self->hoverindex == c && self->hoverindex != self->selected) {
			ui_settextcolor(g, 0x00FFFFFF);
			cpxhover = cpx + ui_value_px(&tab->margin.left, &tm);
			hoverwidth = tab->size.width;
		} else
		if (self->selected == c) {
			ui_settextcolor(g, 0x00B1C8B0);		
		} else {
			ui_settextcolor(g, 0x00D1C5B6);
		}		
		if (self->tabalignment == UI_ALIGN_TOP) {
			cpx += ui_value_px(&tab->margin.left, &tm);
			ui_textout(g, cpx, cpy, tab->text, strlen(tab->text));
			cpx += tab->size.width + ui_value_px(&tab->margin.right, &tm);
		} else
		if (self->tabalignment == UI_ALIGN_LEFT ||
			self->tabalignment == UI_ALIGN_RIGHT) {
			cpy += ui_value_px(&tab->margin.top, &tm);
			ui_textout(g, cpx, cpy, tab->text, strlen(tab->text));
			cpy += tab->size.height + ui_value_px(&tab->margin.bottom, &tm);
		}
	}
	ui_setcolor(g, 0x005F5F5F);		
	if (self->tabalignment == UI_ALIGN_TOP) {
		int y;

		ui_setcolor(g, 0x00B1C8B0);		
		y = tm.tmHeight + 2;
		ui_drawline(g, cpxsel, y, cpxsel + selwidth, y);		 
		if (self->hover && self->hoverindex != self->selected) {		
			ui_setcolor(g, 0x00FFFFFF);
			ui_drawline(g, cpxhover, y, cpxhover + hoverwidth, y);
		}
	} else
	if (self->tabalignment == UI_ALIGN_LEFT) {
		ui_drawline(g, size.width - 2, 0, size.width - 2, cpy);
	}
	if (self->tabalignment == UI_ALIGN_RIGHT) {
		ui_drawline(g, 0, 0, 0, cpy);
	}
}

void OnMouseDown(TabBar* self, ui_component* sender, MouseEvent* mouseevent)
{
	int tabindex;
	
	tabindex = tabhittest(self, mouseevent->x, mouseevent->y);
	if (tabindex != -1 && tabindex != self->selected)  {
		self->selected = tabhittest(self, mouseevent->x, mouseevent->y);
		ui_component_invalidate(&self->component);
		signal_emit(&self->signal_change, self, 1, self->selected);
	}		
}

int tabhittest(TabBar* self, int x, int y) 
{
	List* tabs;	
	int rv = -1;
	int c = 0;
	int cpx = 0;
	int cpy = 0;
	ui_textmetric tm;

	tm = ui_component_textmetric(&self->component);
	if (self->tabalignment == UI_ALIGN_TOP) {
		cpx = 0;
	} else 
	if (self->tabalignment == UI_ALIGN_RIGHT) {
		cpy = 5;
		cpx = 10;
	} else
	if (self->tabalignment == UI_ALIGN_LEFT) {		
		cpy = 5;
	}
	
	tabs = self->tabs;
	while (tabs) {
		Tab* tab;

		tab = (Tab*)tabs->entry;
		if (self->tabalignment == UI_ALIGN_TOP && x >= cpx &&
			x < cpx + tab->size.width +
				ui_value_px(&tab->margin.left, &tm) +
				ui_value_px(&tab->margin.right, &tm)) {
			rv = c;
			break;
		} else
		if ((self->tabalignment == UI_ALIGN_LEFT ||
				self->tabalignment == UI_ALIGN_RIGHT) &&
			y >= cpy && y < cpy + tab->size.height +
				ui_value_px(&tab->margin.top, &tm) +
				ui_value_px(&tab->margin.bottom, &tm)) {
			rv = c;
			break;
		}
		if (self->tabalignment == UI_ALIGN_TOP) {
			cpx += tab->size.width +
				ui_value_px(&tab->margin.left, &tm) +
				ui_value_px(&tab->margin.right, &tm);
		} else
		if (self->tabalignment == UI_ALIGN_LEFT ||
			self->tabalignment == UI_ALIGN_RIGHT) {
			cpy += tab->size.height +
				ui_value_px(&tab->margin.top, &tm) +
				ui_value_px(&tab->margin.bottom, &tm);
		}
		tabs = tabs->next;
		++c;
	}
	return rv;
}

void OnMouseMove(TabBar* self, ui_component* sender, MouseEvent* ev)
{	
	int tabindex;

	tabindex = tabhittest(self, ev->x, ev->y);	
	if (tabindex != self->hoverindex) {
		self->hoverindex = tabindex;
		ui_component_invalidate(&self->component);
	}
	
}

void onmouseenter(TabBar* self, ui_component* sender)
{
	self->hover = 1;
	ui_component_invalidate(&self->component);
}

void onmouseleave(TabBar* self, ui_component* sender)
{		
	self->hover = 0;
	ui_component_invalidate(&self->component);
}

void tabbar_select(TabBar* self, int tab)
{
	self->selected = tab;
	ui_component_invalidate(&self->component);
	signal_emit(&self->signal_change, self, 1, self->selected);
}

int tabbar_selected(TabBar* self)
{
	return self->selected;
}

Tab* tabbar_append(TabBar* self, const char* label)
{
	Tab* tab;

	tab = (Tab*)malloc(sizeof(Tab));
	InitTab(tab, label, 0);
	if (self->tabs != 0) {
		tab->margin.left = ui_value_makeew(1.5);
	} 
	list_append(&self->tabs, tab);	
	tab->size = ui_component_textsize(&self->component, tab->text);
	tab->size.height = 20;

	return tab;
}

void tabbar_settabmargin(TabBar* self, int tabindex, const ui_margin* margin)
{
	List* tabs;
	int c = 0;
	
	if (!margin) {
		return;
	}

	tabs = self->tabs;
	while (tabs) {
		Tab* tab;

		tab = (Tab*)tabs->entry;		
		if (c == tabindex) {
			tab->margin = *margin;
			break;
		}
		tabs = tabs->next;
		++c;
	}		
}

void onalign(TabBar* self, ui_component* sender)
{
	List* p;
	
	for (p = self->tabs; p != 0; p = p->next) {
		Tab* tab;

		tab = (Tab*)p->entry;
		tab->size = ui_component_textsize(&self->component, tab->text);
	}		
}

void onpreferredsize(TabBar* self, ui_component* sender, ui_size* limit, ui_size* rv)
{
	if (rv) {
		ui_size size;
		ui_textmetric tm;
		List* tabs;	
		int cpx = 0;
		int cpy = 0;
		int cpxmax = 0;
		int cpymax = 0;
		
		size = ui_component_size(&self->component);
		tm = ui_component_textmetric(&self->component);
		if (self->tabalignment == UI_ALIGN_TOP) {
			cpx = 0;
		} else 
		if (self->tabalignment == UI_ALIGN_RIGHT) {
			cpy = 5;
			cpx = 10;
		} else
		if (self->tabalignment == UI_ALIGN_LEFT) {		
			cpy = 5;
		}
		for (tabs = self->tabs; tabs != 0; tabs = tabs->next) {
			Tab* tab;
			ui_size tabsize;

			tab = (Tab*)tabs->entry;
			tabsize = ui_component_textsize(&self->component, tab->text);
			tabsize.height = (int) (tabsize.height * 1.5);
			tabsize.width = (int) tabsize.width;
			if (self->tabalignment == UI_ALIGN_TOP) {								
				cpx += tabsize.width;				
				cpx += ui_value_px(&tab->margin.left, &tm) +
						ui_value_px(&tab->margin.right, &tm);
				if (cpymax < cpy + tabsize.height +
						ui_value_px(&tab->margin.top, &tm) +
						ui_value_px(&tab->margin.bottom, &tm)) {
					cpymax = cpy + tabsize.height +
						ui_value_px(&tab->margin.top, &tm) +
						ui_value_px(&tab->margin.bottom, &tm);
				}
				if (cpxmax < cpx) {
					cpxmax = cpx;
				}
			} else 
			if (self->tabalignment == UI_ALIGN_LEFT ||
				self->tabalignment == UI_ALIGN_RIGHT) {
										
				cpy += tabsize.height;
				if (cpxmax < cpx + tabsize.width +
						ui_value_px(&tab->margin.left, &tm) +
						ui_value_px(&tab->margin.right, &tm)) {
					cpxmax = cpx + tabsize.width +
						ui_value_px(&tab->margin.left, &tm) +
						ui_value_px(&tab->margin.right, &tm);
				}
				if (cpymax < cpy + tabsize.height +
						ui_value_px(&tab->margin.top, &tm) +
						ui_value_px(&tab->margin.bottom, &tm)) {
					cpymax = cpy + tabsize.height +
						ui_value_px(&tab->margin.top, &tm) +
						ui_value_px(&tab->margin.bottom, &tm);
				}
			}
		}

		rv->width = cpxmax;
		rv->height = cpymax;
	}
}

Tab* tabbar_tab(TabBar* self, int tabindex)
{
	Tab* rv = 0;
	List* p;
	int c = 0;
	
	for (p = self->tabs; p != 0 && c < tabindex; p = p->next, ++c);
	if (p) {		
		rv = (Tab*)p->entry;		
	}
	return rv;
}
