#include "nameview.h"

static void Draw(nameView* self, ui_graphics* g);

void InitnameView(nameView* patternview, psy_ui_Component* parent)
{			
	psy_ui_component_init(&nameview->component, parent);
	nameview->component.events.draw = Draw;
	psy_ui_component_move(&nameview->component, 250, 0);
}

void Draw(nameView* self, ui_graphics* g)
{	   	
    ui_size size = psy_ui_component_size(&self->component);    
	psy_ui_Rectangle r;
    psy_ui_setrectangle(&r, 0, 0, size.width, size.height);
    psy_ui_drawrectangle(g, r);
}
