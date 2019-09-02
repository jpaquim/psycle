#include "playbar.h"
#include <string.h>

static void OnDraw(PlayBar* self, ui_component* sender, ui_graphics* g);
static void OnSize(PlayBar* self, ui_component* sender, int width, int height);
static void OnDestroy(PlayBar* self, ui_component* component);
static void OnMouseDown(PlayBar* self, ui_component* sender, int x, int y, int button);

void InitPlayBar(PlayBar* self, ui_component* parent)
{			
	ui_component_init(&self->component, parent);	
	signal_init(&self->signal_play);
	signal_init(&self->signal_stop);
	signal_connect(&self->component.signal_draw, self, OnDraw);
	signal_connect(&self->component.signal_destroy, self, OnDestroy);
	signal_connect(&self->component.signal_size, self, OnSize);	
	signal_connect(&self->component.signal_mousedown, self, OnMouseDown);
	self->tabs = 0;
	self->selected = -1;
	playbar_append(self, "Play");
	playbar_append(self, "Stop");
}

void OnDestroy(PlayBar* self, ui_component* component)
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
	signal_dispose(&self->signal_play);
	signal_dispose(&self->signal_stop);
}

void OnDraw(PlayBar* self, ui_component* sender, ui_graphics* g)
{	
	List* ptr;
	int cpx = 0;
	int c = 0;

	ptr = self->tabs;
	while (ptr) {
		const char* str = (const char*)ptr->entry;
		if (self->selected == c) {
			ui_settextcolor(g, 0xFFFF0000);
		} else {
			ui_settextcolor(g, 0x00000000);
		}
		ui_textout(g, cpx, 0, str, strlen(str));
		cpx += 50;
		ptr = ptr->next;
		++c;
	}
	ui_drawline(g, 0, 19, 600, 19);
}

void OnSize(PlayBar* self, ui_component* sender, int width, int height)
{
}

void OnMouseDown(PlayBar* self, ui_component* sender, int x, int y, int button)
{
	self->selected = x / 50;	
	if (self->selected == 0) {
		signal_emit(&self->signal_play, self, 0);
	} else
	if (self->selected == 1) {
		signal_emit(&self->signal_stop, self, 0);
		self->selected = -1;
	}
	ui_invalidate(&self->component);
}

void playbar_select(PlayBar* self, int tab)
{
	self->selected = tab;
}

void playbar_append(PlayBar* self, const char* label)
{
	if (self->tabs) {
		list_append(self->tabs, strdup(label));		
	} else {
		self->tabs = list_create(strdup(label));
	}
}
