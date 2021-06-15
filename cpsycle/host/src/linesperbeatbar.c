/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "linesperbeatbar.h"
/* host */
#include "styles.h"
/* platform */
#include "../../detail/portable.h"

#define LPB_REFRESHRATE 200

/* prototypes */
static void linesperbeatbar_onlessclicked(LinesPerBeatBar*,
	psy_ui_Component* sender);
static void linesperbeatbar_onmoreclicked(LinesPerBeatBar*,
	psy_ui_Component* sender);
static void linesperbeatbar_ontimer(LinesPerBeatBar*, uintptr_t timerid);
/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(LinesPerBeatBar* self)
{
	if (!vtable_initialized) {		
		vtable = *(linesperbeatbar_base(self)->vtable);		
		vtable.ontimer =
			(psy_ui_fp_component_ontimer)
			linesperbeatbar_ontimer;		
		vtable_initialized = TRUE;
	}	
	psy_ui_component_setvtable(linesperbeatbar_base(self), &vtable);
}
/* implementation */
void linesperbeatbar_init(LinesPerBeatBar* self, psy_ui_Component* parent,
	psy_audio_Player* player)
{		
	assert(self);	
	assert(player);

	psy_ui_component_init(linesperbeatbar_base(self), parent, NULL);
	vtable_init(self);
	self->lpb = 0;
	self->player = player;
	/* prevent line wrap */
	psy_ui_component_setalignexpand(linesperbeatbar_base(self),
		psy_ui_HEXPAND);
	psy_ui_component_setdefaultalign(linesperbeatbar_base(self),
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));
	/* lpb description label */
	psy_ui_label_init_text(&self->desc, linesperbeatbar_base(self),
		NULL, "lpb.lines-per-beat");	
	/* less button */
	psy_ui_button_init_connect(&self->less, linesperbeatbar_base(self),
		NULL, self, linesperbeatbar_onlessclicked);
	psy_ui_button_seticon(&self->less, psy_ui_ICON_LESS);
	/* lpb number label */
	psy_ui_label_init(&self->number, linesperbeatbar_base(self), NULL);
	psy_ui_component_setstyletype(psy_ui_label_base(&self->number),
		STYLE_LPB_NUMLABEL);
	psy_ui_label_preventtranslation(&self->number);
	psy_ui_label_setcharnumber(&self->number, 6.0);
	psy_ui_label_settextalignment(&self->number, psy_ui_ALIGNMENT_CENTER);
	/* more button */
	psy_ui_button_init_connect(&self->more, linesperbeatbar_base(self),
		NULL, self, linesperbeatbar_onmoreclicked);
	psy_ui_button_seticon(&self->more, psy_ui_ICON_MORE);
	/* start lpb poll timer */
	psy_ui_component_starttimer(linesperbeatbar_base(self), 0,
		LPB_REFRESHRATE);
}

void linesperbeatbar_onlessclicked(LinesPerBeatBar* self, psy_ui_Component* sender)
{
	assert(self);

	if (psy_audio_player_lpb(self->player) > 1) {
		psy_audio_player_setlpb(self->player,
			psy_audio_player_lpb(self->player) - 1);
	}
}

void linesperbeatbar_onmoreclicked(LinesPerBeatBar* self,
	psy_ui_Component* sender)
{
	assert(self);

	psy_audio_player_setlpb(self->player,
		psy_audio_player_lpb(self->player) + 1);
}

void linesperbeatbar_ontimer(LinesPerBeatBar* self, uintptr_t timerid)
{
	assert(self);

	if (self->lpb != psy_audio_player_lpb(self->player)) {
		char text[64];

		self->lpb = psy_audio_player_lpb(self->player);
		psy_snprintf(text, 64, "%d", (int)self->lpb);
		psy_ui_label_settext(&self->number, text);		
	}
}
