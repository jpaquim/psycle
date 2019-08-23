#include "nameview.h"

static void Draw(nameView* self, ui_graphics* g);

void InitnameView(nameView* patternview, ui_component* parent)
{			
	ui_component_init(&nameview->component, parent);
	nameview->component.events.draw = Draw;
	ui_component_move(&nameview->component, 250, 0);
}

void Draw(nameView* self, ui_graphics* g)
{	   	
    ui_size size = ui_component_size(&self->component);    
	ui_rectangle r;
    ui_setrectangle(&r, 0, 0, size.width, size.height);
    ui_drawrectangle(g, r);
}
