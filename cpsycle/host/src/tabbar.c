#include "tabbar.h"
#include <string.h>

static void OnDraw(TabBar* self, ui_component* sender, ui_graphics* g);
static void OnSize(TabBar* self, ui_component* sender, int width, int height);
static void OnDestroy(TabBar* self, ui_component* component);
static void OnMouseDown(TabBar* self, ui_component* sender, int x, int y, int button);
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
	self->tabs = 0;
	self->selected = 0;
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
	ui_size size;

	size = ui_component_size(&self->component);	
	for (tabs = self->tabs; tabs != 0; tabs = tabs->next, ++c) {
		Tab* tab;

		tab = (Tab*)tabs->entry;
		if (self->selected == c) {
			ui_settextcolor(g, 0xFFFF0000);
		} else {
			ui_settextcolor(g, 0x00000000);
		}
		ui_textout(g, cpx, 0, tab->text, strlen(tab->text));
		cpx += tab->size.width;
		cpx += self->tabmargin.right + tab->margin.right;		
	}
	ui_drawline(g, 0, 19, cpx, 19);
}

void OnSize(TabBar* self, ui_component* sender, int width, int height)
{

}

void OnMouseDown(TabBar* self, ui_component* sender, int x, int y, int button)
{
	List* tabs;
	int c = 0;
	int cpx = 0;	
	
	tabs = self->tabs;
	while (tabs) {
		Tab* tab;

		tab = (Tab*)tabs->entry;
		if (x >= cpx && x < cpx + tab->size.width + tab->margin.right + self->tabmargin.right) {
			self->selected = c;
			break;
		}
		cpx += tab->size.width + tab->margin.right + self->tabmargin.right;
		tabs = tabs->next;
		++c;
	}
	self->selected = c;
	ui_invalidate(&self->component);
	signal_emit(&self->signal_change, self, 1, self->selected);
}

void tabbar_select(TabBar* self, int tab)
{
	self->selected = tab;
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
