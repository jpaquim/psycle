// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "plugincategorylist.h"
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

static void psy_audio_plugincategorylist_addnatives(psy_audio_PluginCategoryList*);
static void psy_audio_plugincategorylist_add(psy_audio_PluginCategoryList*,
	const char* dllname, const char* category);

void psy_audio_plugincategorylist_init(psy_audio_PluginCategoryList* self)
{
	psy_table_init(&self->container);
	psy_audio_plugincategorylist_addnatives(self);
}

void psy_audio_plugincategorylist_dispose(psy_audio_PluginCategoryList* self)
{
	psy_table_dispose_all(&self->container, NULL);
}

const char* psy_audio_plugincategorylist_category(psy_audio_PluginCategoryList* self,
	const char* plugincatchername)
{
	return (char*)psy_table_at_strhash(&self->container, plugincatchername);
}

void psy_audio_plugincategorylist_addnatives(psy_audio_PluginCategoryList* self)
{
	psy_audio_plugincategorylist_add(self, "alk-muter:0", "Mixer");
	psy_audio_plugincategorylist_add(self, "arguru-compressor:0", "Compressor");
	psy_audio_plugincategorylist_add(self, "arguru-distortion:0", "Distortion");
	psy_audio_plugincategorylist_add(self, "arguru-freeverb:0", "Reverb");
	psy_audio_plugincategorylist_add(self, "arguru-goaslicer:0", "Effect");
	psy_audio_plugincategorylist_add(self, "arguru-reverb:0", "Reverb");
	psy_audio_plugincategorylist_add(self, "arguru-synth-2f:0", "Synth");
	psy_audio_plugincategorylist_add(self, "arguru-xfilter:0", "Filter");
	psy_audio_plugincategorylist_add(self, "audacity-compressor:0", "Compressor");	
	psy_audio_plugincategorylist_add(self, "audacity-phaser:0", "Phaser");
	psy_audio_plugincategorylist_add(self, "bexphase:0", "Phaser");
	psy_audio_plugincategorylist_add(self, "blitz12:0", "Synth");
	psy_audio_plugincategorylist_add(self, "blitzn:0", "Synth");
	psy_audio_plugincategorylist_add(self, "crasher:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "delay:0", "Delay");
	psy_audio_plugincategorylist_add(self, "distortion:0", "Distortion");
	psy_audio_plugincategorylist_add(self, "dw-eq:0", "EQ");
	psy_audio_plugincategorylist_add(self, "dw-granulizer:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "dw-iopan:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "dw-tremolo:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "eq3:0", "EQ");
	psy_audio_plugincategorylist_add(self, "feedme:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "filter-2-poles:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "flanger:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "gainer:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "gamefx13:0", "Drum");
	psy_audio_plugincategorylist_add(self, "gamefxn:0", "Drum");
	psy_audio_plugincategorylist_add(self, "haas:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "jmdrum:0", "Drum");
	psy_audio_plugincategorylist_add(self, "karlkox-surround:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "koruz:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "ladspa-gverb:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "legasynth-303:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "m3:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "maeq:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "negative:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "nrs-7900-fractal:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "phantom:0", "Synth");
	psy_audio_plugincategorylist_add(self, "pluckedstring:0", "Synth");
	psy_audio_plugincategorylist_add(self, "pooplog-autopan:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "pooplog-delay-light:0", "Delay");
	psy_audio_plugincategorylist_add(self, "pooplog-delay:0", "Delay");
	psy_audio_plugincategorylist_add(self, "pooplog-filter:0", "Filter");
	psy_audio_plugincategorylist_add(self, "pooplog-fm-laboratory:0", "Synth");
	psy_audio_plugincategorylist_add(self, "pooplog-fm-light:0", "Synth");
	psy_audio_plugincategorylist_add(self, "pooplog-fm-ultralight:0", "Synth");
	psy_audio_plugincategorylist_add(self, "pooplog-lofi-processor:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "pooplog-scratch-master-2:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "pooplog-scratch-master:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "ring-modulator:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "schorus:0", "Chorus");
	psy_audio_plugincategorylist_add(self, "slicit:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "stk-plucked:0", "Synth");
	psy_audio_plugincategorylist_add(self, "stk-rev:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "stk-shakers:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "sublime:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "thunderpalace-softsat:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "vdallpass:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "vdecho:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "vdnoisegate:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "vsfastverb:0", "Reverb");
	psy_audio_plugincategorylist_add(self, "wahwah:0", "Effect");
	psy_audio_plugincategorylist_add(self, "ymidi:0", "MIDI");
	psy_audio_plugincategorylist_add(self, "zephod-superfm:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "singlefrequency:0", "Unknown");
	psy_audio_plugincategorylist_add(self, "s-filter:0", "Effect");
	psy_audio_plugincategorylist_add(self, "s-reverb:0", "Reverb");
}

void psy_audio_plugincategorylist_add(psy_audio_PluginCategoryList* self,
	const char* plugincatchername, const char* category)
{
	psy_table_insert_strhash(&self->container, plugincatchername, psy_strdup(category));
}
