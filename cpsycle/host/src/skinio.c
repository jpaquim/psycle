/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "skinio.h"
/* file */
#include <propertiesio.h>
/* std */
#include <assert.h>
/* platform */
#include "../../detail/portable.h"


static void skin_define_psm(psy_Property* psm);
static void skin_define_psh(psy_Property* psm);
static void skin_define_psv(psy_Property* psv);
static void skin_define_psc(psy_Property* psc);

int _httoi(const char *value)
{  
	return (int)strtol(value, 0, 16);
}

int skin_load_machine(psy_Property* psm, const char* path)
{	
	int rv;	
	psy_PropertyReader propertyreader;

	assert(psm);

	skin_define_psm(psm);
	psy_propertyreader_init(&propertyreader, psm, path);	
	psy_propertyreader_allow_cpp_comments(&propertyreader);
	psy_propertyreader_parse_types(&propertyreader);
	rv = psy_propertyreader_load(&propertyreader);
	psy_propertyreader_dispose(&propertyreader);	
	return rv;
}

void skin_define_psm(psy_Property* psm)
{
	// source coords in bitmap
	// x, y, width, height

	// numbers must be fixed height and width - ie
	// each number has the same dimensions - and they are
	// laid out in the order 0123456789

	// on indicators also designate clickable area
	psy_property_append_str(psm, "master_source", "0, 54, 118, 53");
	psy_property_append_str(psm, "generator_source", "0, 107, 118, 54");
	psy_property_append_str(psm, "generator_vu0_source", "0, 170, 6, 2"); // should be the width of one chunk
	psy_property_append_str(psm, "generator_vu_peak_source", "108, 170, 6, 2");
	psy_property_append_str(psm, "generator_pan_source", "0, 161, 24, 8");
	psy_property_append_str(psm, "generator_mute_source", "25, 161, 9, 7");
	psy_property_append_str(psm, "generator_solo_source", "34, 161, 9, 7");

	psy_property_append_str(psm, "effect_source", "0, 0, 118, 54");
	psy_property_append_str(psm, "effect_vu0_source", "0, 170, 6, 2");
	psy_property_append_str(psm, "effect_vu_peak_source", "108, 170, 6, 2");
	psy_property_append_str(psm, "effect_pan_source", "0, 161, 24, 8");
	psy_property_append_str(psm, "effect_mute_source", "25, 161, 9, 7");
	psy_property_append_str(psm, "effect_bypass_source", "43, 161, 9, 7");
	// destination coords to be rendered to
	// destinations use 0,0 as top left of background
	// x,y,width
	psy_property_append_str(psm, "generator_vu_dest", "16, 15, 81");
	psy_property_append_str(psm, "generator_pan_dest", "10, 40, 72");
	psy_property_append_str(psm, "generator_mute_dest", "104, 30");
	psy_property_append_str(psm, "generator_solo_dest", "104, 19");
	psy_property_append_str(psm, "generator_name_dest", "18, 22");

	psy_property_append_str(psm, "effect_vu_dest", "16, 15, 81");
	psy_property_append_str(psm, "effect_pan_dest", "10, 40, 72");
	psy_property_append_str(psm, "effect_mute_dest", "104, 30");
	psy_property_append_str(psm, "effect_bypass_dest", "104, 19");
	psy_property_append_str(psm, "effect_name_dest", "18, 22");

	psy_property_append_str(psm, "transparency", "0000ff00");
}

int skin_load_pattern_header(psy_Property* psh, const char* path)
{
	int rv;
	char* p;
	psy_PropertyReader propertyreader;

	assert(psh);

	skin_define_psh(psh);
	psy_propertyreader_init(&propertyreader, psh, path);
	psy_propertyreader_allow_cpp_comments(&propertyreader);
	rv = psy_propertyreader_load(&propertyreader);
	psy_propertyreader_dispose(&propertyreader);
	/* skip the "dword:"  and "hex:" keywords */
	p = strrchr(psy_property_at_str(psh, "transparency", ""), ':');
	if (p) {
		psy_property_set_str(psh, "transparency", p + 1);
	}
	return rv;
}

void skin_define_psh(psy_Property* psh)
{
	// source coords in bitmap
	// x, y, width, height

	// numbers must be fixed height and width - ie
	// each number has the same dimensions - and they are
	// laid out in the order 0123456789

	// on indicators also designate clickable area
	psy_property_append_str(psh, "background_source", "0, 0, 109, 18");
	psy_property_append_str(psh, "number_0_source", "0, 18, 7, 12");
	psy_property_append_str(psh, "record_on_source" ,"70, 18, 11, 11");
	psy_property_append_str(psh, "mute_on_source", "81, 18, 11, 11");
	psy_property_append_str(psh, "solo_on_source", "92, 18, 11, 11");
	psy_property_append_str(psh, "playing_on_source", "4, 24, 18, 18");
	// destination coords to be rendered to
	// destinations use 0,0 as top left of background
	psy_property_append_str(psh, "digit_x0_dest" , "24, 3");
	psy_property_append_str(psh, "digit_0x_dest" , "31, 3");
	psy_property_append_str(psh, "record_on_dest" , "52, 3");
	psy_property_append_str(psh, "mute_on_dest" , "75, 3");
	psy_property_append_str(psh, "solo_on_dest" , "97, 3");
	psy_property_append_str(psh, "playing_on_dest", "4, 2");

	psy_property_append_int(psh, "transparency", 0x0000FF00, 0, 0);	
}

int skin_load(psy_Property* psv, const char* path)
{
	int rv;	
	psy_PropertyReader propertyreader;
	const char* machine_background;
	intptr_t font_pt;

	assert(psv);

	skin_define_psv(psv);
	psy_propertyreader_init(&propertyreader, psv, path);
	psy_propertyreader_allow_cpp_comments(&propertyreader);
	psy_propertyreader_parse_types(&propertyreader);
	psy_propertyreader_ignore_sections(&propertyreader);
	psy_propertyreader_skip_double_quotes(&propertyreader);
	rv = psy_propertyreader_load(&propertyreader);
	psy_propertyreader_dispose(&propertyreader);	
	machine_background = psy_property_at_str(psv, "machine_background", "");
	if (psy_strlen(machine_background) > 0) {
		psy_Path path;

		psy_path_init(&path, machine_background);
		psy_property_set_str(psv, "machine_background",
			psy_path_filename(&path));
		psy_path_dispose(&path);
	}
	font_pt = psy_property_at_int(psv, "generator_font_point", 80);
	psy_property_set_int(psv, "generator_font_point", font_pt / 7);
	font_pt = psy_property_at_int(psv, "effect_font_point", 80);
	psy_property_set_int(psv, "effect_font_point", font_pt / 7);	
	return rv;
}

void skin_define_psv(psy_Property* psv)
{
//	[Psycle Display Presets v1.0]

	psy_property_append_str(psv, "pattern_fontface", "Verdana");
	psy_property_append_int(psv, "pattern_font_point", 0x00000050, 0, 0);
	psy_property_append_int(psv, "pattern_font_flags", 0x00000000, 0, 0);
	psy_property_append_int(psv, "pattern_font_x", 0x0000000C, 0, 0);
	psy_property_append_int(psv, "pattern_font_y", 0x0000000C, 0, 0);
	psy_property_append_str(psv, "pattern_header_skin", "ilkke header");
	psy_property_append_int(psv, "pvc_separator", 0x002C2E2E, 0, 0);
	psy_property_append_int(psv, "pvc_separator2", 0x002C2E2E, 0, 0);
	psy_property_append_int(psv, "pvc_background", 0x00807E720, 0, 0);
	psy_property_append_int(psv, "pvc_background2", 0x00807E720, 0, 0);
	psy_property_append_int(psv, "pvc_font", 0x00000000, 0, 0);
	psy_property_append_int(psv, "pvc_font2", 0x00000000, 0, 0);
	psy_property_append_int(psv, "pvc_fontCur", 0x00FFFFFF, 0, 0);
	psy_property_append_int(psv, "pvc_fontCur2", 0x00FFFFFF, 0, 0);
	psy_property_append_int(psv, "pvc_fontSel", 0x00FFFFFF, 0, 0);
	psy_property_append_int(psv, "pvc_fontSel2", 0x00FFFFFF, 0, 0);
	psy_property_append_int(psv, "pvc_fontPlay", 0x00FFFFFF, 0, 0);
	psy_property_append_int(psv, "pvc_fontPlay2", 0x00FFFFFF, 0, 0);
	psy_property_append_int(psv, "pvc_row", 0x00949286, 0, 0);
	psy_property_append_int(psv, "pvc_row2", 0x00949286, 0, 0);
	psy_property_append_int(psv, "pvc_rowbeat", 0x008A887C, 0, 0);
	psy_property_append_int(psv, "pvc_rowbeat2", 0x008A887C, 0, 0);
	psy_property_append_int(psv, "pvc_row4beat", 0x009E9C90, 0, 0);
	psy_property_append_int(psv, "pvc_row4beat2", 0x009E9C90, 0, 0);
	psy_property_append_int(psv, "pvc_selection", 0x003C3C3C, 0, 0);
	psy_property_append_int(psv, "pvc_selection2", 0x003C3C3C, 0, 0);
	psy_property_append_int(psv, "pvc_playbar", 0x00807E72, 0, 0);
	psy_property_append_int(psv, "pvc_playbar2", 0x00787D7F, 0, 0);
	psy_property_append_int(psv, "pvc_cursor", 0x00000000, 0, 0);
	psy_property_append_int(psv, "pvc_cursor2", 0x00000000, 0, 0);
	psy_property_append_int(psv, "vu1", 0x0040FF00, 0, 0);
	psy_property_append_int(psv, "vu2", 0x00C0C0C0, 0, 0);
	psy_property_append_int(psv, "vu3", 0x000000FF, 0, 0);
	psy_property_append_str(psv, "generator_fontface", "Verdana");
	psy_property_append_int(psv, "generator_font_point", 0x00000046, 0, 0);
	psy_property_append_int(psv, "generator_font_flags", 0x00000000, 0, 0);
	psy_property_append_str(psv, "effect_fontface", "Verdana");
	psy_property_append_int(psv, "effect_font_point", 0x00000046, 0, 0);
	psy_property_append_int(psv, "effect_font_flags", 0x00000000, 0, 0);
	psy_property_append_str(psv, "machine_skin", "ilkke machines");
	psy_property_append_int(psv, "mv_colour", 0x00787D7F, 0, 0);
	psy_property_append_int(psv, "mv_wirecolour", 0x003B3B3B, 0, 0);
	psy_property_append_int(psv, "mv_polycolour", 0x00BBC1C1, 0, 0);
	psy_property_append_int(psv, "mv_generator_fontcolour", 0x00595F62, 0, 0);
	psy_property_append_int(psv, "mv_effect_fontcolour", 0x004A4A4A, 0, 0);
	psy_property_append_int(psv, "mv_wirewidth", 0x00000001, 0, 0);
	psy_property_append_int(psv, "mv_wireaa", 0x0, 0, 0);
	psy_property_append_str(psv, "machine_background", "");
	psy_property_append_str(psv, "machine_GUI_bitmap", "");
	psy_property_append_int(psv, "mv_triangle_size", 0x0E, 0, 0);
	psy_property_append_int(psv, "machineGUITopColor", 0x00788D93, 0, 0);
	psy_property_append_int(psv, "machineGUIFontTopColor", 0x00CCDDEE, 0, 0);
	psy_property_append_int(psv, "machineGUIBottomColor", 0x00687D83, 0, 0);
	psy_property_append_int(psv, "machineGUIFontBottomColor", 0x0044EEFF, 0, 0);
	psy_property_append_int(psv, "machineGUIHTopColor", 0x00BCAF93, 0, 0);
	psy_property_append_int(psv, "machineGUIHFontTopColor", 0x00CCDDEE, 0, 0);
	psy_property_append_int(psv, "machineGUIHBottomColor", 0x00AC9F83, 0, 0);
	psy_property_append_int(psv, "machineGUIHFontBottomColor", 0x0044EEFF, 0, 0);
	psy_property_append_int(psv, "machineGUITitleColor", 0x0088A8B4, 0, 0);
	psy_property_append_int(psv, "machineGUITitleFontColor", 0x00FFFFFF, 0, 0);
}

void skin_psh_values(const char* str, intptr_t maxcount, intptr_t* values)
{
	char s[512];
	char* token;
	int c = 0;

	strcpy(s, str);
	token = strtok(s, ",");
	while (token != 0 && (c < maxcount)) {
		values[c] = atoi(token);
		token = strtok(NULL, ",");
		++c;
	}

}

int skin_load_psc(psy_Property* psc, const char* path)
{
	int rv;
	psy_PropertyReader propertyreader;	

	assert(psc);

	skin_define_psc(psc);
	psy_propertyreader_init(&propertyreader, psc, path);
	psy_propertyreader_allow_cpp_comments(&propertyreader);
	psy_propertyreader_parse_types(&propertyreader);
	psy_propertyreader_ignore_sections(&propertyreader);
	psy_propertyreader_skip_double_quotes(&propertyreader);
	rv = psy_propertyreader_load(&propertyreader);
	psy_propertyreader_dispose(&propertyreader);	
	return rv;
}

void skin_define_psc(psy_Property* psc)
{
	// source coords in bitmap
	// x, y, width, height
	// numbers must be fixed height and width

	// WARNING!!! Use the same name than the .psc file for one of the three bmp files! The loader requires this.
	psy_property_append_str(psc, "machinedial_bmp", "Psycle_110_knobs.bmp");
	psy_property_append_str(psc, "send_return_bmp", "Psycle_110_mixer_skin.bmp");
	psy_property_append_str(psc, "master_bmp", "Psycle_110_master_skin.bmp");

	//Font used for the parameters and mixer windows.
	psy_property_append_str(psc, "params_text_font_name", "Tahoma");
	psy_property_append_int(psc, "params_text_font_point", 0x00000050, 0, 0);
	psy_property_append_int(psc, "params_text_font_flags", 0x00000000, 0, 0);
	psy_property_append_str(psc, "params_text_font_bold_name", "Tahoma");
	psy_property_append_int(psc, "params_text_font_bold_point", 0x00000050, 0, 0);
	psy_property_append_int(psc, "params_text_font_bold_flags", 0x00000001, 0, 0);

	// Note: Colours are written in the form "00BGR" (i.e. opposite of RGB)
	psy_property_append_int(psc, "master_text_backcolour", 0x00000000, 0, 0);
	psy_property_append_int(psc, "master_text_forecolour", 0x00FFFFFF, 0, 0);
	psy_property_append_str(psc, "master_text_font_name", "Tahoma");
	psy_property_append_int(psc, "master_text_font_point", 0x00000050, 0, 0);
	psy_property_append_int(psc, "master_text_font_flags", 0x00000000, 0, 0);
	// names are rendered on the y axis 15 pixels apart. 
	psy_property_append_str(psc, "master_text_names_dest", "427, 32, 75, 12");
	psy_property_append_str(psc, "master_text_numbers_master_dest", "22, 186");
	// slider dBs are rendered on the x axis 24 pixels apart. height is master_text_names_dest height.
	psy_property_append_str(psc, "master_text_numbers_channels_dest", "118, 186");

	// These are the constants used for the graphics. They are not user-modifiable because
	// it affects some internal objects and calculations.
	// rotating knob graphic
	// knob (x,y,w,h) 0,0,28,28
	// number of knobs :  64  (middle position is knob number 32)

	// Mixer graphic positions and widths are:
	// mixer slider (x,y,w,h) 0,0,30,182
	// mixer slider knob (x,y,w,h) 0,182,22,10
	// mixer vu off (x,y,w,h) 30,0,16,90
	// mixer vu on (x,y) 46,0
	// mixer switch off (x,y,w,h) 30,90,28,28
	// mixer switch on (x,y) 30,118
	// mixer check off (x,y,w,h) 30,146,13,13
	// mixer check on (x,y) 30,159

	// Master graphic positions and widths are:
	// background (x,y,w,h) 0,0,516,225
	// Vu Left off (x,y,w,h) 516,0,18,159
	// Vu Left on (x,y) 534,0
	// Vu Right off (x,y) 552,0
	// Vu Right on (x,y) 570,0
	// slider knob (x,y,w,h) 516,159,22,10
}

/* locate psycle skins */
static int locate_pattern_skin_enum_dir(psy_Property*, const char* path, int flag);

void skin_locate_pattern_skins(psy_Property* skins, const char* path)
{
	psy_dir_enumerate_recursive(skins, path, "*.psh", 0,
		(psy_fp_findfile)locate_pattern_skin_enum_dir);
}

int locate_pattern_skin_enum_dir(psy_Property* self, const char* path, int type)
{
	psy_Path skinpath;

	psy_path_init(&skinpath, path);
	psy_property_append_str(self,
		psy_path_name(&skinpath), path);
	psy_path_dispose(&skinpath);
	return 1;
}

/* locate machine skins */
static int locate_machine_skin_enum_dir(psy_Property*, const char* path, int flag);

void skin_locate_machine_skins(psy_Property* skins, const char* path)
{
	psy_dir_enumerate_recursive(skins, path, "*.psm", 0,
		(psy_fp_findfile)locate_machine_skin_enum_dir);
}

int locate_machine_skin_enum_dir(psy_Property* self, const char* path, int type)
{		
	psy_Path skinpath;

	psy_path_init(&skinpath, path);
	psy_property_append_str(self,
		psy_path_name(&skinpath), path);
	psy_path_dispose(&skinpath);
	return 1;
}
