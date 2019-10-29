#include "../../detail/prefix.h"

#include "tabbar.h"
#include <string.h>

static void OnDraw(TabBar* self, ui_component* sender, ui_graphics* g);
static void OnDestroy(TabBar* self, ui_component* component);
static void OnMouseDown(TabBar* self, ui_component* sender, int x, int y, int button);
static void OnMouseMove(TabBar* self, ui_component* sender, int x, int y, int button);
static void onmouseenter(TabBar*, ui_component* sender);
static void onmouseleave(TabBar*, ui_component* sender);
static int tabhittest(TabBar* self, int x, int y);
static void onalign(TabBar*, ui_component* sender);
static void onpreferredsize(TabBar*, ui_component* sender, ui_size* limit, int* width, int* height);

void InitTab(Tab* self, const char* text, ui_size* size)
{
	self->text = _strdup(text);
	if (size) {
		self->size = *size;
	}	
	ui_margin_init(&self->margin, 0, 0, 0, 0);
}

void DisposeTab(Tab* self)
{
	free(self->text);
}

void tabbar_init(TabBar* self, ui_component* parent)
{
	ui_component_init(&self->component, parent);	
	self->component.doublebuffered = 1;	
	ui_margin_init(&self->tabmargin, 0, 10, 0, 0);
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

	size = ui_component_size(&self->component);
	ui_setbackgroundmode(g, TRANSPARENT);		

	if (self->tabalignment == UI_ALIGN_TOP) {
		cpx = 5;
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
			cpxsel = cpx;
			selwidth = tab->size.width;
		}
		if (self->hover && self->hoverindex == c && self->hoverindex != self->selected) {
			ui_settextcolor(g, 0x00FFFFFF);
			cpxhover = cpx;
			hoverwidth = tab->size.width;
		} else
		if (self->selected == c) {
			ui_settextcolor(g, 0x00B1C8B0);		
		} else {
			ui_settextcolor(g, 0x00D1C5B6);
		}
		ui_textout(g, cpx, cpy, tab->text, strlen(tab->text));
		if (self->tabalignment == UI_ALIGN_TOP) {
			cpx += tab->size.width;
			cpx += self->tabmargin.right + tab->margin.right;
		} else 
		if (self->tabalignment == UI_ALIGN_LEFT ||
			self->tabalignment == UI_ALIGN_RIGHT) {
			cpy += tab->size.height;
			cpy += self->tabmargin.bottom + tab->margin.bottom;
		}
	}
	ui_setcolor(g, 0x005F5F5F);		
	if (self->tabalignment == UI_ALIGN_TOP) {		
		ui_setcolor(g, 0x00B1C8B0);		
		ui_drawline(g, cpxsel, size.height - 3, cpxsel + selwidth,
			size.height - 3);		 
		if (self->hover && self->hoverindex != self->selected) {		
			ui_setcolor(g, 0x00FFFFFF);
			ui_drawline(g, cpxhover, size.height - 3, cpxhover + hoverwidth,
				size.height - 3);
		}
	} else
	if (self->tabalignment == UI_ALIGN_LEFT) {
		ui_drawline(g, size.width - 2, 0, size.width - 2, cpy);
	}
	if (self->tabalignment == UI_ALIGN_RIGHT) {
		ui_drawline(g, 0, 0, 0, cpy);
	}
}

void OnMouseDown(TabBar* self, ui_component* sender, int x, int y, int button)
{
	int tabindex;
	
	tabindex = tabhittest(self, x, y);
	if (tabindex != -1 && tabindex != self->selected)  {
		self->selected = tabhittest(self, x, y);
		ui_invalidate(&self->component);
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

	if (self->tabalignment == UI_ALIGN_TOP) {
		cpx = 5;
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
			x < cpx + tab->size.width + tab->margin.right +
				self->tabmargin.right) {
			rv = c;
			break;
		} else
		if ((self->tabalignment == UI_ALIGN_LEFT || self->tabalignment == UI_ALIGN_RIGHT) &&
			y >= cpy && y < cpy + tab->size.height + tab->margin.bottom +
				self->tabmargin.bottom) {
			rv = c;
			break;
		}
		if (self->tabalignment == UI_ALIGN_TOP) {
			cpx += tab->size.width + tab->margin.right + self->tabmargin.right;
		} else
		if (self->tabalignment == UI_ALIGN_LEFT ||
			self->tabalignment == UI_ALIGN_RIGHT) {
			cpy += tab->size.height + tab->margin.bottom + self->tabmargin.bottom;
		}
		tabs = tabs->next;
		++c;
	}
	return rv;
}

void OnMouseMove(TabBar* self, ui_component* sender, int x, int y, int button)
{	
	int tabindex;

	tabindex = tabhittest(self, x, y);	
	if (tabindex != self->hoverindex) {
		self->hoverindex = tabindex;
		ui_invalidate(&self->component);
	}
	
}

void onmouseenter(TabBar* self, ui_component* sender)
{
	self->hover = 1;
	ui_invalidate(&self->component);
}

void onmouseleave(TabBar* self, ui_component* sender)
{		
	self->hover = 0;
	ui_invalidate(&self->component);
}

void tabbar_select(TabBar* self, int tab)
{
	self->selected = tab;
	ui_invalidate(&self->component);
	signal_emit(&self->signal_change, self, 1, self->selected);
}

int tabbar_selected(TabBar* self)
{
	return self->selected;
}

void tabbar_append(TabBar* self, const char* label)
{
	Tab* tab;

	tab = (Tab*)malloc(sizeof(Tab));
	InitTab(tab, label, 0);
	list_append(&self->tabs, tab);	
	tab->size = ui_component_textsize(&self->component, tab->text);
	tab->size.height = 20;
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

void onpreferredsize(TabBar* self, ui_component* sender, ui_size* limit, int* width, int* height)
{
	ui_size size;
	
	List* tabs;	
	int cpx = 0;
	int cpy = 0;
	int cpxmax = 0;
	int cpymax = 0;
	
	size = ui_component_size(&self->component);	

	if (self->tabalignment == UI_ALIGN_TOP) {
		cpx = 5;
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
			cpx += self->tabmargin.right + tab->margin.right;
			if (cpymax < cpy + tabsize.height + tab->margin.top + tab->margin.bottom) {
				cpymax = cpy + tabsize.height + tab->margin.top + tab->margin.bottom;
			}
			if (cpxmax < cpx + tabsize.width + tab->margin.left + tab->margin.right) {
				cpxmax = cpx + tabsize.width + tab->margin.left + tab->margin.right;
			}
		} else 
		if (self->tabalignment == UI_ALIGN_LEFT ||
			self->tabalignment == UI_ALIGN_RIGHT) {
									
			cpy += tabsize.height;
			if (cpxmax < cpx + tabsize.width + tab->margin.left + tab->margin.right) {
				cpxmax = cpx + tabsize.width + tab->margin.left + tab->margin.right;
			}
			if (cpymax < cpy + tabsize.height + tab->margin.top + tab->margin.bottom) {
				cpymax = cpy + tabsize.height + tab->margin.top + tab->margin.bottom;
			}
		}
	}

	*width = cpxmax;
	*height = cpymax;
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