#include "tabbar.h"
#include <string.h>

static void OnDraw(TabBar* self, ui_component* sender, ui_graphics* g);
static void OnSize(TabBar* self, ui_component* sender, int width, int height);
static void OnDestroy(TabBar* self, ui_component* component);
static void OnMouseDown(TabBar* self, ui_component* sender, int x, int y, int button);
static void OnMouseMove(TabBar* self, ui_component* sender, int x, int y, int button);
static void onmouseenter(TabBar*, ui_component* sender);
static void onmouseleave(TabBar*, ui_component* sender);
static int tabhittest(TabBar* self, int x, int y);
static void AlignTabs(TabBar* self);

void InitTab(Tab* self, const char* text, ui_size* size)
{
	self->text = strdup(text);
	if (size) {
		self->size = *size;
	}	
	ui_setmargin(&self->margin, 0, 0, 0, 0);
}

void DisposeTab(Tab* self)
{
	free(self->text);
}

void InitTabBar(TabBar* self, ui_component* parent)
{	
	ui_fontinfo fontinfo;		

	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	ui_fontinfo_init(&fontinfo, "Tahoma", 80);
	ui_font_init(&self->font, &fontinfo);
	ui_component_setfont(&self->component, &self->font);
	ui_setmargin(&self->tabmargin, 0, 10, 0, 0);
	signal_init(&self->signal_change);
	signal_connect(&self->component.signal_draw, self, OnDraw);
	signal_connect(&self->component.signal_destroy, self, OnDestroy);
	signal_connect(&self->component.signal_size, self, OnSize);	
	signal_connect(&self->component.signal_mousedown, self, OnMouseDown);
	signal_connect(&self->component.signal_mousemove, self, OnMouseMove);
	signal_connect(&self->component.signal_mouseenter, self, onmouseenter);
	signal_connect(&self->component.signal_mouseleave, self, onmouseleave);
	ui_component_resize(&self->component,0, 20);
	self->tabs = 0;
	self->selected = 0;
	self->hover = 0;
	self->hoverindex = -1;
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
	int cpx = 5;	
	ui_size size;

	size = ui_component_size(&self->component);
	ui_setbackgroundmode(g, TRANSPARENT);		
	for (tabs = self->tabs; tabs != 0; tabs = tabs->next, ++c) {
		Tab* tab;

		tab = (Tab*)tabs->entry;
		if (self->hover && self->hoverindex == c) {
			ui_settextcolor(g, 0x00FFFFFF);
		} else
		if (self->selected == c) {
			ui_settextcolor(g, 0x00B1C8B0);
		} else {
			ui_settextcolor(g, 0x00D1C5B6);
		}
		ui_textout(g, cpx, 0, tab->text, strlen(tab->text));
		cpx += tab->size.width;
		cpx += self->tabmargin.right + tab->margin.right;		
	}
	ui_setcolor(g, 0x005F5F5F);		
	ui_drawline(g, 0, size.height - 2, cpx, size.height - 2);
}

void OnSize(TabBar* self, ui_component* sender, int width, int height)
{

}

void OnMouseDown(TabBar* self, ui_component* sender, int x, int y, int button)
{
	int tabindex;
	
	tabindex = tabhittest(self, x, y);
	if (tabindex != -1 && tabindex != self->selected)  {
		self->selected = tabhittest(self, x, y);
		signal_emit(&self->signal_change, self, 1, self->selected);
	}		
}

int tabhittest(TabBar* self, int x, int y) 
{
	List* tabs;
	int rv = -1;
	int c = 0;
	int cpx = 0;	
	
	tabs = self->tabs;
	while (tabs) {
		Tab* tab;

		tab = (Tab*)tabs->entry;
		if (x >= cpx &&
			x < cpx + tab->size.width + tab->margin.right +
				self->tabmargin.right) {
			rv = c;
			break;
		}
		cpx += tab->size.width + tab->margin.right + self->tabmargin.right;
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

void tabbar_append(TabBar* self, const char* label)
{
	Tab* tab;

	tab = (Tab*)malloc(sizeof(Tab));
	InitTab(tab, label, 0);
	if (self->tabs) {
		list_append(self->tabs, tab);		
	} else {
		self->tabs = list_create(tab);
	}	
	tab->size = ui_component_textsize(&self->component, tab->text);
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
