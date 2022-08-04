/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQUENCERBAR_H)
#define SEQUENCERBAR_H

/* host */
#include "generalconfig.h"
#include "keyboardmiscconfig.h"
/* ui */
#include <uibutton.h>
#include <uicheckbox.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** SequencerBar
**
** Changes settings of the sequencer/view/player
** - Follow song – Make the cursor follow the song progress during playback. 
** - Record note-off – Record Noteoffs (when the key is lifted) during
**   real-time recording.
** - Record tweak – Record any mouse tweaks (altering of parameters)
**   during real-time recording
** - Multitrack playback on keypress – Allows multiple notes to be played at
**   the same time while playing with a PC keyboard.
** - Allow notes to effect machines – Allows sending notes to effects machines
**   while using the PC keyboard. This can be used with certain VSTs.
**   Otherwise, notes will be sent to the selected generator.
** Show/hides the seqeditor and stepsequencer
*/

typedef struct SequencerBar {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_CheckBox follow_song;
	psy_ui_CheckBox shownames;	
	psy_ui_CheckBox recordnoteoff;
	psy_ui_CheckBox recordtweak;
	psy_ui_CheckBox multichannelaudition;
	psy_ui_CheckBox allownotestoeffect;
	psy_ui_Component view_buttons;
	psy_ui_Button toggleseqedit;
	psy_ui_Button togglestepseq;
	psy_ui_Button togglekbd;
} SequencerBar;

void sequencerbar_init(SequencerBar* self, psy_ui_Component* parent,
	KeyboardMiscConfig*, GeneralConfig*);

INLINE psy_ui_Component* sequencerbar_base(SequencerBar* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* SEQUENCERBAR_H */
