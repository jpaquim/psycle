#include "tabbar.h"
#include <string.h>

static void OnDraw(TabBar* self, ui_component* sender, ui_graphics* g);
static void OnSize(TabBar* self, ui_component* sender, int width, int height);
static void OnDestroy(TabBar* self, ui_component* component);
static void OnMouseDown(TabBar* self, ui_component* sender, int x, int y, int button);

void InitTabBar(TabBar* self, ui_component* parent)
{			
	ui_component_init(&self->component, parent);	
	signal_init(&self->signal_change);
	signal_connect(&self->component.signal_draw, self, OnDraw);
	signal_connect(&self->component.signal_destroy, self, OnDestroy);
	signal_connect(&self->component.signal_size, self, OnSize);	
	signal_connect(&self->component.signal_mousedown, self, OnMouseDown);
	self->tabs = 0;
	self->tabwidth = 90;
	self->selected = -1;
}

void OnDestroy(TabBar* self, ui_component* component)
{	
	List* ptr;
	List* next;
	ptr = self->tabs;
	while (ptr) {
		next = ptr->next;
		free(ptr->entry);
		ptr = next;
	}
	list_free(self->tabs);
	signal_dispose(&self->signal_change);
}

void OnDraw(TabBar* self, ui_component* sender, ui_graphics* g)
{	
	List* tab;
	int cpx = 0;
	int c = 0;

	tab = self->tabs;
	while (tab) {
		const char* str = (const char*)tab->entry;
		if (self->selected == c) {
			ui_settextcolor(g, 0xFFFF0000);
		} else {
			ui_settextcolor(g, 0x00000000);
		}
		ui_textout(g, cpx, 0, str, strlen(str));
		cpx += self->tabwidth;
		tab = tab->next;
		++c;
	}
	ui_drawline(g, 0, 19, 600, 19);
}

void OnSize(TabBar* self, ui_component* sender, int width, int height)
{
}

void OnMouseDown(TabBar* self, ui_component* sender, int x, int y, int button)
{
	self->selected = x / self->tabwidth;
	ui_invalidate(&self->component);
	signal_emit(&self->signal_change, self, 1, self->selected);
}

void tabbar_select(TabBar* self, int tab)
{
	self->selected = tab;
}

void tabbar_append(TabBar* self, const char* label)
{
	if (self->tabs) {
		list_append(self->tabs, strdup(label));		
	} else {
		self->tabs = list_create(strdup(label));
	}
}
