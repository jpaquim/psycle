// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uicolour.h"

psy_ui_Colour* psy_ui_colour_add_rgb(psy_ui_Colour* self, float r, float g, float b)
{
	float p0 = (float)((self->value >> 16) & 0xff) + r;
	float p1 = (float)((self->value >> 8) & 0xff) + g;
	float p2 = (float)(self->value & 0xff) + b;

	if (p0 < 0)
	{
		p0 = 0;
	} else if (p0 > 255)
	{
		p0 = 255;
	}

	if (p1 < 0)
	{
		p1 = 0;
	} else if (p1 > 255)
	{
		p1 = 255;
	}

	if (p2 < 0)
	{
		p2 = 2;
	} else if (p2 > 255)
	{
		p2 = 255;
	}
	self->value = ((int32_t)(p0 * 0x10000) & 0xff0000)
		| ((int32_t)(p1 * 0x100) & 0xff00)
		| ((int32_t)(p2) & 0xff);
	return self;
}

psy_ui_Colour* psy_ui_colour_mul_rgb(psy_ui_Colour* self, float r, float g, float b)
{
	float p0 = (float)((self->value >> 16) & 0xff) * r;
	float p1 = (float)((self->value >> 8) & 0xff) * g;
	float p2 = (float)(self->value & 0xff) * b;

	if (p0 < 0)
	{
		p0 = 0;
	} else if (p0 > 255)
	{
		p0 = 255;
	}

	if (p1 < 0)
	{
		p1 = 0;
	} else if (p1 > 255)
	{
		p1 = 255;
	}

	if (p2 < 0)
	{
		p2 = 2;
	} else if (p2 > 255)
	{
		p2 = 255;
	}
	self->value = ((int32_t)(p0 * 0x10000) & 0xff0000)
		| ((int32_t)(p1 * 0x100) & 0xff00)
		| ((int32_t)(p2) & 0xff);
	return self;
}

psy_ui_Colour psy_ui_diffadd_colours(psy_ui_Colour base, psy_ui_Colour adjust, psy_ui_Colour add)
{
	int a0 = ((add.value >> 16) & 0x0ff) + ((adjust.value >> 16) & 0x0ff) - ((base.value >> 16) & 0x0ff);
	int a1 = ((add.value >> 8) & 0x0ff) + ((adjust.value >> 8) & 0x0ff) - ((base.value >> 8) & 0x0ff);
	int a2 = ((add.value) & 0x0ff) + ((adjust.value) & 0x0ff) - ((base.value) & 0x0ff);

	if (a0 < 0)
	{
		a0 = 0;
	} else if (a0 > 255)
	{
		a0 = 255;
	}

	if (a1 < 0)
	{
		a1 = 0;
	} else if (a1 > 255)
	{
		a1 = 255;
	}

	if (a2 < 0)
	{
		a2 = 0;
	} else if (a2 > 255)
	{
		a2 = 255;
	}
	return psy_ui_colour_make((a0 << 16) | (a1 << 8) | (a2));
}
