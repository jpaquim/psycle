/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "pianoroll.h"
/* local */
#include "trackergridstate.h"
#include "patterncmds.h"
#include "cmdsnotes.h"
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

#define PIANOROLL_REFRESHRATE 50
#define CMD_ENTER 1100

static void setcmdall(psy_Property* cmds, uintptr_t cmd, uint32_t keycode,
	bool shift, bool ctrl, const char* key, const char* shorttext);

/* PianoBar */
/* implenentation */
void pianobar_init(PianoBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_Margin margin;

	assert(self);
		
	psy_ui_component_init(&self->component, parent, NULL);
	margin = psy_ui_defaults_hmargin(psy_ui_defaults());
	psy_ui_margin_setleft(&margin, psy_ui_value_make_ew(1.0));
	psy_ui_margin_setbottom(&margin, psy_ui_value_make_ew(0.5));
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		margin);
	self->workspace = workspace;
	psy_ui_label_init_text(&self->keys, pianobar_base(self), NULL,
		"Keyboard");
	zoombox_init(&self->zoombox_keyheight, &self->component, NULL);	
	psy_ui_combobox_init(&self->keytype, pianobar_base(self), NULL);
	psy_ui_combobox_setcharnumber(&self->keytype, 8);	
	psy_ui_combobox_addtext(&self->keytype, "Keys");	
	psy_ui_combobox_addtext(&self->keytype, "Notes");
	psy_ui_combobox_addtext(&self->keytype, "Drums");
	psy_ui_combobox_setcursel(&self->keytype, 0);
	psy_ui_label_init_text(&self->tracks, pianobar_base(self), NULL,
		"Show Tracks");
	psy_ui_button_init_text(&self->tracks_all, pianobar_base(self), NULL, "All");
	psy_ui_button_highlight(&self->tracks_all);
	psy_ui_button_init_text(&self->track_curr, pianobar_base(self), NULL, "Current");
	psy_ui_button_init_text(&self->tracks_active, pianobar_base(self), NULL, "Active");		
}

/* Pianoroll */
/* protoypes */
static void pianoroll_ontimer(Pianoroll*, uintptr_t timerid);
static void pianoroll_onplaylinechanged(Pianoroll*, Workspace* sender);
static void pianoroll_onlpbchanged(Pianoroll*, psy_audio_Player*,
	uintptr_t lpb);
static void pianoroll_onalign(Pianoroll*);
static void pianoroll_onmousedown(Pianoroll*, psy_ui_MouseEvent*);
static void pianoroll_ongridscroll(Pianoroll*, psy_ui_Component* sender);
static void pianoroll_onbeatwidthchanged(Pianoroll*, ZoomBox* sender);
static void pianoroll_onkeyheightchanged(Pianoroll*, ZoomBox* sender);
static void pianoroll_onkeytypeselchange(Pianoroll*, psy_ui_Component* sender, int sel);
static void pianoroll_ondisplayalltracks(Pianoroll*, psy_ui_Button* sender);
static void pianoroll_ondisplaycurrenttrack(Pianoroll*, psy_ui_Button* sender);
static void pianoroll_ondisplayactivetracks(Pianoroll*, psy_ui_Button* sender);
static void pianoroll_updatetrackdisplaybuttons(Pianoroll*);
static void pianoroll_onthemechanged(Pianoroll*, PatternViewConfig*, psy_Property* theme);
static void pianoroll_updatetheme(Pianoroll*);
static void pianoroll_oncursorchanged(Pianoroll*, psy_audio_Sequence* sender);
static void pianoroll_onsongchanged(Pianoroll*, Workspace* sender, int flag);
static bool pianoroll_onrollcmds(Pianoroll*, InputHandler* sender);
static bool pianoroll_onnotecmds(Pianoroll*, InputHandler* sender);
static void pianoroll_onconfigure(Pianoroll*, PatternViewConfig* config,
	psy_Property*);
/* vtable */
static psy_ui_ComponentVtable pianoroll_vtable;
static bool pianoroll_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* pianoroll_vtable_init(Pianoroll* self)
{
	assert(self);

	if (!pianoroll_vtable_initialized) {
		pianoroll_vtable = *(self->component.vtable);
		pianoroll_vtable.onalign =
			(psy_ui_fp_component_event)
			pianoroll_onalign;
		pianoroll_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			pianoroll_onmousedown;		
		pianoroll_vtable.ontimer =
			(psy_ui_fp_component_ontimer)
			pianoroll_ontimer;
		pianoroll_vtable_initialized = TRUE;
	}
	return &pianoroll_vtable;
}
/* implenentation */
void pianoroll_init(Pianoroll* self, psy_ui_Component* parent,
	PatternViewState* pvstate, Workspace* workspace)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setvtable(&self->component, pianoroll_vtable_init(self));
	self->workspace = workspace;	
	self->opcount = 0;
	self->syncpattern = 1;
	self->chordbegin = 0;
	psy_ui_component_setbackgroundmode(&self->component, psy_ui_NOBACKGROUND);
	/* shared states */
	keyboardstate_init(&self->keyboardstate, patternviewstate_skin(pvstate));
	pianogridstate_init(&self->gridstate, pvstate);	
	/* left area (keyboardheader, keyboard) */
	psy_ui_component_init(&self->left, &self->component, NULL);
	psy_ui_component_setalign(&self->left, psy_ui_ALIGN_LEFT);
	zoombox_init(&self->zoombox_beatwidth, &self->left, NULL);
	psy_ui_component_setpreferredheight(&self->zoombox_beatwidth.component,
		psy_ui_value_make_eh(1.0));
	psy_signal_connect(&self->zoombox_beatwidth.signal_changed, self,
		pianoroll_onbeatwidthchanged);
	psy_ui_component_setalign(zoombox_base(&self->zoombox_beatwidth),
		psy_ui_ALIGN_TOP);
	/* Keyboard */
	psy_ui_component_init(&self->keyboardpane, &self->left, NULL);
	psy_ui_component_setalign(&self->keyboardpane, psy_ui_ALIGN_CLIENT);
	pianokeyboard_init(&self->keyboard, &self->keyboardpane, &self->keyboardstate);
	psy_ui_component_setalign(&self->keyboard.component,
		psy_ui_ALIGN_FIXED);	
	/* top area (beatruler) */
	psy_ui_component_init(&self->top, &self->component, NULL);
	psy_ui_component_setalign(&self->top, psy_ui_ALIGN_TOP);
	pianoruler_init(&self->header, &self->top, &self->gridstate);
	psy_ui_component_setalign(pianoruler_base(&self->header),
		psy_ui_ALIGN_FIXED);
	/* client area (event grid) */
	pianogrid_init(&self->grid, &self->component, &self->keyboardstate,
		&self->gridstate, self->workspace);
	psy_ui_scroller_init(&self->scroller, pianogrid_base(&self->grid),
		&self->component, NULL);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);	
	psy_ui_component_setalign(&self->grid.component, psy_ui_ALIGN_FIXED);
	/* bar */
	pianobar_init(&self->bar, &self->scroller.component, self->workspace);	
	psy_ui_component_setalign(&self->bar.component, psy_ui_ALIGN_BOTTOM);
	psy_signal_connect(&self->bar.zoombox_keyheight.signal_changed, self,
		pianoroll_onkeyheightchanged);
	psy_ui_component_setalign(&self->bar.zoombox_keyheight.component,
		psy_ui_ALIGN_LEFT);
	psy_signal_connect(&workspace->player.signal_lpbchanged, self,
		pianoroll_onlpbchanged);
	psy_signal_connect(&pianogrid_base(&self->grid)->signal_scroll, self,
		pianoroll_ongridscroll);
	psy_signal_connect(&self->bar.keytype.signal_selchanged, self,
		pianoroll_onkeytypeselchange);
	psy_signal_connect(&self->bar.tracks_all.signal_clicked, self,
		pianoroll_ondisplayalltracks);
	psy_signal_connect(&self->bar.track_curr.signal_clicked, self,
		pianoroll_ondisplaycurrenttrack);
	psy_signal_connect(&self->bar.tracks_active.signal_clicked, self,
		pianoroll_ondisplayactivetracks);	
	psy_signal_connect(
		&psycleconfig_patview(workspace_conf(workspace))->signal_themechanged,
		self, pianoroll_onthemechanged);
	psy_signal_connect(&self->workspace->song->sequence.signal_cursorchanged,
		self, pianoroll_oncursorchanged);
	psy_signal_connect(&self->workspace->signal_playlinechanged, self,
		pianoroll_onplaylinechanged);
	psy_signal_connect(&workspace->signal_songchanged, self,
		pianoroll_onsongchanged);
	pianoroll_updatetheme(self);
	inputhandler_connect(&workspace->inputhandler, INPUTHANDLER_FOCUS,
		psy_EVENTDRIVER_CMD, "pianoroll", psy_INDEX_INVALID, 
		self, (fp_inputhandler_input)pianoroll_onrollcmds);
	inputhandler_connect(&workspace->inputhandler, INPUTHANDLER_FOCUS,
		psy_EVENTDRIVER_CMD, "notes", psy_INDEX_INVALID, 
		self, (fp_inputhandler_input)pianoroll_onnotecmds);
	psy_ui_component_starttimer(&self->component, 0, PIANOROLL_REFRESHRATE);
	/* configuration */
	psy_signal_connect(&workspace->config.patview.signal_changed, self,
		pianoroll_onconfigure);
}

void pianoroll_setpattern(Pianoroll* self, psy_audio_Pattern* pattern)
{
	assert(self);
		
	pianogrid_onpatternchange(&self->grid, pattern);
	psy_ui_component_setscroll(pianogrid_base(&self->grid),
		psy_ui_point_make(psy_ui_value_zero(),
			psy_ui_component_scrolltop(pianogrid_base(&self->grid))));
	psy_ui_component_setscroll(pianoruler_base(&self->header),
		psy_ui_point_zero());
	pianoroll_updatescroll(self);
}

void pianoroll_onplaylinechanged(Pianoroll* self, Workspace* sender)
{
	assert(self);

	if (self->gridstate.pv->display == PATTERN_DISPLAYMODE_TRACKER) {
		return;
	}
	if (patternviewstate_pattern(self->gridstate.pv)) {
		if (sender->lastplayline != psy_INDEX_INVALID) {
			pianogrid_invalidateline(&self->grid,
				self->grid.lastplayposition - self->gridstate.pv->cursor.seqoffset);
			self->grid.lastplayposition =
				psy_audio_player_position(workspace_player(self->workspace));
			pianogrid_invalidateline(&self->grid,
				self->grid.lastplayposition - self->gridstate.pv->cursor.seqoffset);
		} else {
			pianogrid_invalidateline(&self->grid,
				self->grid.lastplayposition - self->gridstate.pv->cursor.seqoffset);
			self->grid.lastplayposition = -1;
		}
	}
}

void pianoroll_ontimer(Pianoroll* self, uintptr_t timerid)
{
	assert(self);

	if (patternviewstate_pattern(self->gridstate.pv)) {
		if (patternviewstate_pattern(self->gridstate.pv) &&
				patternviewstate_pattern(self->gridstate.pv)->opcount != self->opcount &&
				self->syncpattern) {
			psy_ui_component_invalidate(pianogrid_base(&self->grid));
		}
		self->opcount = patternviewstate_pattern(self->gridstate.pv)
			? patternviewstate_pattern(self->gridstate.pv)->opcount
			: 0;
	}
}

void pianoroll_onalign(Pianoroll* self)
{
	assert(self);

	pianoroll_updatescroll(self);
}

void pianoroll_onmousedown(Pianoroll* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	psy_ui_component_setfocus(pianoroll_base(self));
}

void pianoroll_updatescroll(Pianoroll* self)
{	
	assert(self);	
	
	pianogrid_updatekeystate(&self->grid);
	psy_ui_component_setscrollstep(pianogrid_base(&self->grid),
		psy_ui_size_make_px(
			pianogridstate_steppx(&self->gridstate),
			self->keyboardstate.keyheightpx));
	psy_ui_component_setscrollstep_height(pianokeyboard_base(&self->keyboard),
		psy_ui_value_make_px(self->keyboardstate.keyheightpx));
	psy_ui_component_setscrollstep_width(pianoruler_base(&self->header),
		psy_ui_value_make_px(pianogridstate_steppx(&self->gridstate)));
	psy_ui_component_updateoverflow(pianogrid_base(&self->grid));
	psy_ui_component_invalidate(pianogrid_base(&self->grid));
	psy_ui_component_invalidate(pianoruler_base(&self->header));
	psy_ui_component_invalidate(pianokeyboard_base(&self->keyboard));
}

void pianoroll_onlpbchanged(Pianoroll* self, psy_audio_Player* sender,
	uintptr_t lpb)
{
	assert(self);
	
	psy_ui_component_setscrollstep_width(pianogrid_base(&self->grid),
		psy_ui_value_make_px(pianogridstate_steppx(&self->gridstate)));
	psy_ui_component_updateoverflow(pianogrid_base(&self->grid));
	psy_ui_component_setscrollleft(pianoruler_base(&self->header),
		psy_ui_component_scrollleft(pianogrid_base(&self->grid)));
	psy_ui_component_invalidate(pianoruler_base(&self->header));
	psy_ui_component_update(pianoruler_base(&self->header));
	psy_ui_component_invalidate(pianogrid_base(&self->grid));
}

void pianoroll_oncursorchanged(Pianoroll* self, psy_audio_Sequence* sender)
{	
	assert(self);
	
	pianogrid_setcursor(&self->grid, sender->cursor);	
}

void pianoroll_onsongchanged(Pianoroll* self, Workspace* workspace, int flag)
{	
	if (workspace->song) {
		psy_signal_connect(&self->workspace->song->sequence.signal_cursorchanged, self,
			pianoroll_oncursorchanged);
	}	
}

void pianoroll_ongridscroll(Pianoroll* self, psy_ui_Component* sender)
{
	assert(self);

	if (psy_ui_component_scrollleftpx(pianogrid_base(&self->grid)) !=
			psy_ui_component_scrollleftpx(pianoruler_base(&self->header))) {
		psy_ui_component_setscrollleft(pianoruler_base(&self->header),
			psy_ui_component_scrollleft(pianogrid_base(&self->grid)));		
	}
	if (psy_ui_component_scrolltop_px(pianogrid_base(&self->grid)) !=
			psy_ui_component_scrolltop_px(&self->keyboard.component)) {
		psy_ui_component_setscrolltop(&self->keyboard.component,
			psy_ui_component_scrolltop(pianogrid_base(&self->grid)));		
	}
}

void pianoroll_onbeatwidthchanged(Pianoroll* self, ZoomBox* sender)
{
	assert(self);

	pianogridstate_setzoom(&self->gridstate, zoombox_rate(sender));
	psy_ui_component_setscrollstep_width(pianogrid_base(&self->grid),
		psy_ui_value_make_px(pianogridstate_steppx(&self->gridstate)));
	psy_ui_component_updateoverflow(pianogrid_base(&self->grid));
	psy_ui_component_setscrollleft(pianoruler_base(&self->header),
		psy_ui_component_scrollleft(pianogrid_base(&self->grid)));
	psy_ui_component_align(&self->top);
	psy_ui_component_invalidate(pianoruler_base(&self->header));
	psy_ui_component_update(pianoruler_base(&self->header));
	psy_ui_component_align(&self->scroller.pane);
	psy_ui_component_invalidate(&self->scroller.pane);
}

void pianoroll_onkeyheightchanged(Pianoroll* self, ZoomBox* sender)
{
	assert(self);

	self->keyboardstate.keyheight = psy_ui_mul_value_real(
		self->keyboardstate.defaultkeyheight, zoombox_rate(sender));
	pianogrid_updatekeystate(&self->grid);
	psy_ui_component_setscrollstep_height(pianogrid_base(&self->grid),
		psy_ui_value_make_px(self->keyboardstate.keyheightpx));
	psy_ui_component_setscrollstep_height(pianokeyboard_base(&self->keyboard),
		psy_ui_value_make_px(self->keyboardstate.keyheightpx));
	psy_ui_component_updateoverflow(pianogrid_base(&self->grid));
	psy_ui_component_setscrolltop(&self->keyboard.component,
		psy_ui_component_scrolltop(pianogrid_base(&self->grid)));
	psy_ui_component_invalidate(&self->keyboard.component);
	psy_ui_component_update(&self->keyboard.component);	
	psy_ui_component_align(&self->scroller.pane);
	psy_ui_component_invalidate(&self->scroller.pane);
}

void pianoroll_onkeytypeselchange(Pianoroll* self, psy_ui_Component* sender, int sel)
{
	assert(self);

	if (sel >= 0 && sel < KEYBOARDTYPE_NUM) {
		pianokeyboard_setkeyboardtype(&self->keyboard, (KeyboardType)sel);
		psy_ui_component_align(pianoroll_base(self));
	}	
}

void pianoroll_ondisplayalltracks(Pianoroll* self, psy_ui_Button* sender)
{
	assert(self);

	pianogrid_settrackdisplay(&self->grid, PIANOROLL_TRACK_DISPLAY_ALL);
	pianoroll_updatetrackdisplaybuttons(self);
}

void pianoroll_ondisplaycurrenttrack(Pianoroll* self, psy_ui_Button* sender)
{
	assert(self);

	pianogrid_settrackdisplay(&self->grid, PIANOROLL_TRACK_DISPLAY_CURRENT);
	pianoroll_updatetrackdisplaybuttons(self);
}

void pianoroll_ondisplayactivetracks(Pianoroll* self, psy_ui_Button* sender)
{
	assert(self);

	pianogrid_settrackdisplay(&self->grid, PIANOROLL_TRACK_DISPLAY_ACTIVE);
	pianoroll_updatetrackdisplaybuttons(self);
}

void pianoroll_updatetrackdisplaybuttons(Pianoroll* self)
{
	assert(self);

	psy_ui_button_disablehighlight(&self->bar.tracks_all);
	psy_ui_button_disablehighlight(&self->bar.track_curr);
	psy_ui_button_disablehighlight(&self->bar.tracks_active);
	switch (pianogrid_trackdisplay(&self->grid)) {
		case PIANOROLL_TRACK_DISPLAY_ALL:
			psy_ui_button_highlight(&self->bar.tracks_all);			
			break;
		case PIANOROLL_TRACK_DISPLAY_CURRENT:
			psy_ui_button_highlight(&self->bar.track_curr);
			break;
		case PIANOROLL_TRACK_DISPLAY_ACTIVE:			
			psy_ui_button_highlight(&self->bar.tracks_active);
			break;
		default:			
			break;
	}	
}

void pianoroll_makecmds(psy_Property* parent)
{
	psy_Property* cmds;

	assert(parent);

	cmds = psy_property_settext(psy_property_append_section(parent,
		"pianoroll"), "Pianoroll");
	setcmdall(cmds, CMD_NAVUP,
		psy_ui_KEY_UP, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navup", "up");	
	setcmdall(cmds, CMD_NAVDOWN,
		psy_ui_KEY_DOWN, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navdown", "down");
	setcmdall(cmds, CMD_NAVLEFT,
		psy_ui_KEY_LEFT, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navleft", "left");
	setcmdall(cmds, CMD_NAVRIGHT,
		psy_ui_KEY_RIGHT, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navright", "right");
	setcmdall(cmds, CMD_NAVPAGEUP,
		psy_ui_KEY_PRIOR, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navpageup", "pageup");
	setcmdall(cmds, CMD_NAVPAGEDOWN,
		psy_ui_KEY_NEXT, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navpagedown", "pagedown");
	setcmdall(cmds, CMD_NAVPAGEUPKEYBOARD,
		psy_ui_KEY_PRIOR, psy_SHIFT_ON, psy_CTRL_OFF,
		"navpageupkbd", "pageup kbd");
	setcmdall(cmds, CMD_NAVPAGEDOWNKEYBOARD,
		psy_ui_KEY_NEXT, psy_SHIFT_ON, psy_CTRL_OFF,
		"navpagedownkbd", "pagedown kbd");
	setcmdall(cmds, CMD_ENTER,
		psy_ui_KEY_SPACE, psy_SHIFT_OFF, psy_CTRL_OFF,
		"enter", "enter");
	setcmdall(cmds, CMD_ROWCLEAR,
		psy_ui_KEY_DELETE, psy_SHIFT_OFF, psy_CTRL_OFF,
		"rowclear", "clr row");
	setcmdall(cmds, CMD_BLOCKSTART,
		psy_ui_KEY_B, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockstart", "sel start");
	setcmdall(cmds, CMD_BLOCKEND,
		psy_ui_KEY_E, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockend", "sel end");
	setcmdall(cmds, CMD_BLOCKUNMARK,
		psy_ui_KEY_U, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockunmark", "unmark");
	setcmdall(cmds, CMD_BLOCKCUT,
		psy_ui_KEY_X, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockcut", "cut");
	setcmdall(cmds, CMD_BLOCKCOPY,
		psy_ui_KEY_C, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockcopy", "copy");
	setcmdall(cmds, CMD_BLOCKPASTE,
		psy_ui_KEY_V, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockpaste", "paste");
	setcmdall(cmds, CMD_BLOCKMIX,
		psy_ui_KEY_M, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockmix", "mix");
	setcmdall(cmds, CMD_BLOCKDELETE,
		0, 0, 0,
		"blockdelete", "blkdel");

	setcmdall(cmds, CMD_SELECTALL,
		psy_ui_KEY_A, psy_SHIFT_OFF, psy_CTRL_ON,
		"selectall", "sel all");
	setcmdall(cmds, CMD_SELECTBAR,
		psy_ui_KEY_K, psy_SHIFT_OFF, psy_CTRL_ON,
		"selectbar", "sel bar");

	setcmdall(cmds, CMD_BLOCKUNMARK,
		psy_ui_KEY_U, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockunmark", "unmark");
	setcmdall(cmds, CMD_BLOCKSTART,
		psy_ui_KEY_B, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockstart", "sel start");
	setcmdall(cmds, CMD_BLOCKEND,
		psy_ui_KEY_E, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockend", "sel end");
}

bool pianoroll_onrollcmds(Pianoroll* self, InputHandler* sender)
{
	psy_EventDriverCmd cmd;

	cmd = inputhandler_cmd(sender);
	return pianoroll_handlecommand(self, cmd.id);
}

bool pianoroll_handlecommand(Pianoroll* self, uintptr_t cmd)
{
	bool handled;

	assert(self);
	
	handled = TRUE;
	switch (cmd) {
		case CMD_NAVUP:
			pianoroll_navup(self);
			break;
		case CMD_NAVPAGEUP:
			pianogrid_prevlines(&self->grid, self->gridstate.pv->pgupdownstep, 0);
			break;
		case CMD_NAVPAGEUPKEYBOARD:
			pianogrid_advancekeys(&self->grid, 12, 0);
			break;
		case CMD_NAVDOWN:
			pianoroll_navdown(self);
			break;
		case CMD_NAVPAGEDOWN:
			pianogrid_advancelines(&self->grid, self->gridstate.pv->pgupdownstep, 0);
			break;
		case CMD_NAVPAGEDOWNKEYBOARD:
			pianogrid_prevkeys(&self->grid, 12, 0);			
			break;
		case CMD_NAVLEFT:
			pianogrid_prevline(&self->grid);		
			break;
		case CMD_NAVRIGHT:
			pianogrid_advanceline(&self->grid);
			break;				
		case CMD_BLOCKSTART:
			pianoroll_blockstart(self);
			break;
		case CMD_BLOCKEND:
			pianoroll_blockend(self);
			break;
		case CMD_ENTER: {
			pianoroll_enter(self);
			break; }
		case CMD_ROWCLEAR:		
			pianoroll_rowclear(self);
			break;
		case CMD_BLOCKPASTE:			
			patternviewstate_blockpaste(self->gridstate.pv);
			break;
		case CMD_BLOCKCOPY:			
			patternviewstate_blockcopy(self->gridstate.pv);
			break;
		case CMD_BLOCKCUT:			
			patternviewstate_blockcut(self->gridstate.pv);
			break;
		case CMD_BLOCKDELETE:
			patternviewstate_blockdelete(self->gridstate.pv);
			break;
		case CMD_SELECTALL:			
			patternviewstate_selectall(self->gridstate.pv);
			break;
		case CMD_SELECTBAR:
			patternviewstate_selectbar(self->gridstate.pv);
			break;
		case CMD_BLOCKUNMARK:
			patternviewstate_blockunmark(self->gridstate.pv);
			break;			
		default:
			handled = FALSE;
			break;
	}
	return handled;
}

bool pianoroll_onnotecmds(Pianoroll* self, InputHandler* sender)
{
	psy_EventDriverCmd cmd;

	assert(self);

	cmd = inputhandler_cmd(sender);
	if (cmd.id != -1) {		
		psy_audio_PatternEvent ev;
		bool chord;
		
		chord = FALSE;
		if (cmd.id == CMD_NOTE_CHORD_END) {
			self->gridstate.pv->cursor.track = self->chordbegin;
			return 1;
		} else if (cmd.id >= CMD_NOTE_CHORD_C_0 && cmd.id < CMD_NOTE_STOP) {
			chord = TRUE;
			ev = psy_audio_player_patternevent(&self->workspace->player,
				(uint8_t)cmd.id - (uint8_t)CMD_NOTE_CHORD_C_0);
		} else if (cmd.id < 256) {
			chord = FALSE;
			ev = psy_audio_player_patternevent(&self->workspace->player, (uint8_t)cmd.id);
		}
		psy_undoredo_execute(&self->workspace->undoredo,
			&insertcommand_alloc(patternviewstate_pattern(self->gridstate.pv),
				1.0 / (double)self->gridstate.pv->cursor.lpb,
				self->gridstate.pv->cursor, ev,
				self->workspace)->command);
		if (chord != FALSE) {
			++self->gridstate.pv->cursor.track;
		} else {
			self->gridstate.pv->cursor.track = self->chordbegin;
		}
		if (ev.note < psy_audio_NOTECOMMANDS_RELEASE) {
			self->gridstate.pv->cursor.key = ev.note;
		}
		if (self->workspace && workspace_song(self->workspace)) {
			psy_audio_sequence_setcursor(
				psy_audio_song_sequence(workspace_song(self->workspace)),
				pianogridstate_cursor(&self->gridstate));
		}
		return 1;
	}
	return 0;
}

/*
** Defines a property with shortcut defaults for the keyboard driver
** key		: cmd id used by the trackerview
** text		: "cmds.key" language dictionary key used by the translator
** shorttext: short description for the keyboard help view
** value	: encoded key shortcut (keycode/shift/ctrl)
*/
void setcmdall(psy_Property* cmds, uintptr_t cmd, uint32_t keycode, bool shift,
	bool ctrl, const char* key, const char* shorttext)
{
	char text[256];

	assert(cmds);

	psy_snprintf(text, 256, "cmds.%s", key);
	psy_property_sethint(psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(cmds, key,
			psy_audio_encodeinput(keycode, shift, ctrl, 0, 0), 0, 0),
			cmd), shorttext), text), PSY_PROPERTY_HINT_SHORTCUT);
}

void pianoroll_navup(Pianoroll* self)
{
	if (self->gridstate.pv->cursor.key < self->keyboardstate.keymax - 1) {
		pianogrid_storecursor(&self->grid);
		++self->gridstate.pv->cursor.key;
		pianogrid_scrollup(&self->grid, self->gridstate.pv->cursor);
		if (self->workspace && workspace_song(self->workspace)) {
			psy_audio_sequence_setcursor(
				psy_audio_song_sequence(workspace_song(self->workspace)),
				pianogridstate_cursor(&self->gridstate));
		}
		pianogrid_invalidatecursor(&self->grid);
	} 
}

void pianoroll_navdown(Pianoroll* self)
{
	if (self->gridstate.pv->cursor.key > self->keyboardstate.keymin) {
		pianogrid_storecursor(&self->grid);
		--self->gridstate.pv->cursor.key;
		pianogrid_scrolldown(&self->grid, self->gridstate.pv->cursor);
		if (self->workspace && workspace_song(self->workspace)) {
			psy_audio_sequence_setcursor(
				psy_audio_song_sequence(workspace_song(self->workspace)),
				pianogridstate_cursor(&self->gridstate));
		}
		pianogrid_invalidatecursor(&self->grid);
	}
}

void pianoroll_enter(Pianoroll* self)
{
	psy_audio_PatternEvent patternevent;

	psy_audio_patternevent_clear(&patternevent);
	patternevent.note = self->gridstate.pv->cursor.key;
	psy_undoredo_execute(&self->workspace->undoredo,
		&insertcommand_alloc(patternviewstate_pattern(self->gridstate.pv),
			pianogridstate_step(&self->gridstate),
			self->gridstate.pv->cursor, patternevent,
			self->workspace)->command);
	pianogrid_advanceline(&self->grid);
}

void pianoroll_rowclear(Pianoroll* self)
{
	psy_undoredo_execute(&self->workspace->undoredo,
		&removecommand_alloc(self->gridstate.pv->pattern,
			pianogridstate_step(&self->gridstate),
			self->gridstate.pv->cursor, self->workspace)->command);
	pianogrid_advanceline(&self->grid);
}

void pianoroll_blockstart(Pianoroll* self)
{
	self->grid.gridstate->pv->dragselectionbase = self->gridstate.pv->cursor;
	pianogrid_startdragselection(&self->grid, self->gridstate.pv->cursor);
	psy_ui_component_invalidate(&self->component);
}

void pianoroll_blockend(Pianoroll* self)
{
	pianogrid_dragselection(&self->grid, self->gridstate.pv->cursor);
	psy_ui_component_invalidate(&self->component);
}

void pianoroll_onthemechanged(Pianoroll* self, PatternViewConfig* config,
	psy_Property* theme)
{
	pianoroll_updatetheme(self);
}

void pianoroll_updatetheme(Pianoroll* self)
{
	assert(self);
	
	psy_ui_component_setbackgroundcolour(pianoruler_base(&self->header),
		patternviewskin_backgroundcolour(patternviewstate_skin(
			self->gridstate.pv), 0, 0));
}

void pianoroll_onconfigure(Pianoroll* self, PatternViewConfig* config,
	psy_Property* property)
{
	if (patternviewconfig_issmoothscrolling(config)) {
		psy_ui_scroller_scrollsmooth(&self->scroller);
	} else {
		psy_ui_scroller_scrollfast(&self->scroller);
	}
}
