/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


/* local */
#include "uicomponent.h"


#include "uicomponentbackground.h"

static void psy_ui_componentbackground_draw_image(psy_ui_ComponentBackground*,
	psy_ui_Graphics*, const psy_ui_Style*);

void psy_ui_componentbackground_init(psy_ui_ComponentBackground* self,
	psy_ui_Component* component)
{	
	self->component = component;
	self->bgframetimer = FALSE;
	self->currbgframe = 0;
}

void psy_ui_componentbackground_dispose(psy_ui_ComponentBackground* self)
{
}

void psy_ui_componentbackground_draw_image(psy_ui_ComponentBackground* self,
	psy_ui_Graphics* g, const psy_ui_Style* style)
{
	if (!psy_ui_bitmap_empty(&style->background.bitmap)) {
		psy_ui_RealSize size;
		psy_ui_RealPoint cp;
		psy_ui_RealSize bmpsize;
		psy_ui_RealRectangle position;
		psy_ui_RealPoint src;

		size = psy_ui_component_size_px(self->component);
		psy_ui_realpoint_init(&cp);
		bmpsize = psy_ui_bitmap_size(&style->background.bitmap);
		if (bmpsize.width < 1.0 || bmpsize.height < 1.0) {
			return;
		}
		if (style->background.animation.enabled) {
			src = psy_ui_realpoint_make(
				self->currbgframe *
				style->background.animation.framesize.width,
				0);
			if (src.x >= bmpsize.width) {
				src.x = 0;
				self->currbgframe = 0;
			}
			bmpsize.width = style->background.animation.framesize.width;
			if (bmpsize.width < 1.0 || bmpsize.height < 1.0) {
				return;
			}
		} else {
			src = psy_ui_realpoint_zero();
		}
		if (style->background.position_set) {
			psy_ui_realpoint_sub(&src, style->background.position);
		}
		if (style->background.size_set) {
			bmpsize = style->background.size;
			if (bmpsize.width < 1.0 || bmpsize.height < 1.0) {
				return;
			}
		}
		if (style->background.repeat == psy_ui_REPEAT) {
			while (cp.y < size.height) {
				position = psy_ui_realrectangle_make(cp, bmpsize);
				psy_ui_drawbitmap(g, &((psy_ui_Style*)style)->background.bitmap,
					position, src);
				cp.x += bmpsize.width;
				if (cp.x >= size.width) {
					cp.x = 0.0;
					cp.y += bmpsize.height;
				}
			}
		} else {
			if (((style->background.align & psy_ui_ALIGNMENT_RIGHT) ==
					psy_ui_ALIGNMENT_RIGHT) &&
				!((style->background.align & psy_ui_ALIGNMENT_LEFT) ==
					psy_ui_ALIGNMENT_LEFT)) {
				cp.x = size.width - bmpsize.width;
			} else if (style->background.align & psy_ui_ALIGNMENT_CENTER_HORIZONTAL) {
				cp.x = (size.width - bmpsize.width) / 2.0;
			}
			if (style->background.align & psy_ui_ALIGNMENT_CENTER_VERTICAL) {
				cp.y = (size.height - bmpsize.height) / 2.0;
			}
			position = psy_ui_realrectangle_make(cp, bmpsize);
			psy_ui_drawbitmap(g, &((psy_ui_Style*)style)->background.bitmap,
				position, src);
		}
	}
}

void psy_ui_componentbackground_draw(psy_ui_ComponentBackground* self,
	psy_ui_Graphics* g)
{			
	const psy_ui_Border* b;

	b = psy_ui_component_border(self->component);
	if (psy_ui_border_isround(b)) {
		psy_ui_RealRectangle r;
		const psy_ui_TextMetric* tm;

		r = psy_ui_realrectangle_make(
			psy_ui_realpoint_zero(),
			psy_ui_component_scroll_size_px(self->component));
		tm = psy_ui_component_textmetric(self->component);
		psy_ui_drawsolidrectangle(g, r,
			psy_ui_component_background_colour(
				psy_ui_component_parent(self->component)));
		psy_ui_drawsolidroundrectangle(g, r,
			psy_ui_realsize_make(
				psy_ui_value_px(&b->left.radius, tm, NULL),
				psy_ui_value_px(&b->left.radius, tm, NULL)),
			psy_ui_component_background_colour(self->component));
	} else {
		const psy_ui_Style* style;

		style = psy_ui_componentstyle_currstyle_const(&self->component->style);
		if (psy_ui_bitmap_empty(&style->background.bitmap)) {								
			psy_ui_drawsolidrectangle(g, psy_ui_graphics_cliprect(g),
				psy_ui_component_background_colour(self->component));
		} else {
			if (&style->background.repeat == psy_ui_NOREPEAT) {
				psy_ui_drawsolidrectangle(g, psy_ui_graphics_cliprect(g),
					psy_ui_component_background_colour(self->component));
			}
			psy_ui_componentbackground_draw_image(self, g, style);
		}
	}	
}

void psy_ui_componentbackground_idle(psy_ui_ComponentBackground* self)
{
	const psy_ui_Style* style;

	style = psy_ui_componentstyle_currstyle_const(&self->component->style);
	if (style->background.image_id && style->background.animation.enabled) {
		self->bgframetimer++;
		if (self->bgframetimer == style->background.animation.interval) {
#ifndef PSYCLE_DEBUG_PREVENT_TIMER_DRAW
			psy_ui_component_invalidate(self->component);
#endif
			self->currbgframe++;
			self->bgframetimer = 0;
		}
	}
}
