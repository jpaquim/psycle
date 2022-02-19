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

static int skinio_loadproperties(FILE* hfile, psy_Property* props);
void skin_define_psm(psy_Property* psm);

int _httoi(const char *value)
{  
	return (int)strtol(value, 0, 16);
}

int skin_load(psy_Property* properties, const psy_Path* path)
{
	FILE* fp;
	char buf[512];

	if ((fp = fopen(psy_path_full(path), "rb")) == NULL )
	{
//		MessageBox("Couldn't open File for Reading. Operation Aborted","File Open Error",MB_OK);
		return PSY_ERRFILE;
	}
	while (fgets(buf, 512, fp))
	{
		if (strstr(buf, "\"pattern_fontface\"=\""))
		{
			char *q = strchr(buf,61); // =
			if (q)
			{
				char *p;
				q+=2;
				p = strrchr(q,34); // "
				if (p)
				{
					p[0]=0;					
					psy_property_append_str(properties, "pattern_fontface", q);
				}
			}
		} else
		if (strstr(buf, "\"machine_GUI_bitmap\"=\""))
		{
			char* q = strchr(buf, 61); // =
			if (q)
			{
				char* p;
				q += 2;
				p = strrchr(q, 34); // "
				if (p)
				{
					p[0] = 0;
					psy_property_append_str(properties, "machineguibitmap", q);
				}
			}
		}
		else if (strstr(buf,"\"pattern_font_point\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "pattern_font_point", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pattern_font_flags\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "pattern_font_flags", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pattern_font_x\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "pattern_font_x", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pattern_font_y\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "pattern_font_y", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pattern_header_skin\"=\""))
		{
			char *q = strchr(buf,61); // =
			if (q)
			{
				char *p;
				q+=2;
				p = strrchr(q,34); // "
				if (p)
				{
					p[0]=0;					
					psy_property_append_str(properties, "pattern_header_skin", q);
				}
			}
		}
		else if (strstr(buf,"\"generator_fontface\"=\""))
		{
			char *q = strchr(buf,61); // =
			if (q)
			{
				char *p;
				q+=2;
				p = strrchr(q,34); // "
				if (p)
				{
					p[0]=0;					
					psy_property_append_str(properties, "generator_fontface", q);
				}
			}
		}
		else if (strstr(buf,"\"generator_font_point\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "generator_font_point", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"generator_font_flags\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "generator_font_flags", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"effect_fontface\"=\""))
		{
			char *q = strchr(buf,61); // =
			if (q)
			{
				char *p;
				q+=2;
				p = strrchr(q,34); // "
				if (p)
				{
					p[0]=0;					
					psy_property_append_str(properties, "effect_fontface", q);
				}
			}
		}
		else if (strstr(buf,"\"effect_font_point\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "effect_font_point", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"effect_font_flags\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "effect_font_flags", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"machine_skin\"=\""))
		{
			char *q = strchr(buf,61); // =
			if (q)
			{
				char *p;
				q+=2;
				p = strrchr(q,34); // "
				if (p)
				{
					p[0]=0;					
					psy_property_append_str(properties, "machine_skin", q);
				}
			}
		}
		else if (strstr(buf,"\"machine_background\"=\""))
		{
			char *q = strchr(buf,61); // =
			if (q)
			{
				char *p;
				q+=2;
				p = strrchr(q,34); // "
				if (p)
				{					
					p[0]=0;					

					if (psy_strlen(q) > 1) {
						psy_Path path;

						psy_path_init(&path, q);
						psy_property_append_str(properties, "machine_background",
							psy_path_filename(&path));
						psy_path_dispose(&path);
					} else {
						psy_property_append_str(properties, "machine_background", "");
					}

					// check for no \ in which case search for it?
					//bBmpBkg = TRUE;
				}
			}
		}
		/*
		else if (strstr(buf,"\"DisplayLineNumbers\"=hex:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				_linenumbers=_httoi(q+1)?1:0;
			}
		}
		else if (strstr(buf,"\"DisplayLineNumbersHex\"=hex:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				_linenumbersHex=_httoi(q+1)?1:0;
			}
		}
		*/
		else if (strstr(buf,"\"pvc_separator\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				psy_property_append_int(properties, "pvc_separator", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_separator2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "pvc_separator2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_background\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "pvc_background", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_background2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "pvc_background2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_font\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "pvc_font", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_font2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "pvc_font2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_fontCur\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "pvc_fontcur", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_fontCur2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "pvc_fontcur2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_fontSel\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "pvc_fontsel", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_fontSel2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "pvc_fontsel2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_fontPlay\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "pvc_fontplay", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_fontPlay2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				psy_property_append_int(properties, "pvc_fontplay2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_row\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				psy_property_append_int(properties, "pvc_row", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_row2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				psy_property_append_int(properties, "pvc_row2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_rowbeat\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				psy_property_append_int(properties, "pvc_rowbeat", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_rowbeat2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				psy_property_append_int(properties, "pvc_rowbeat2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_row4beat\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				psy_property_append_int(properties, "pvc_row4beat", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_row4beat2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				psy_property_append_int(properties, "pvc_row4beat2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_selection\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "pvc_selection", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_selection2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "pvc_selection2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_playbar\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "pvc_playbar", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_playbar2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "pvc_playbar2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_cursor\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "pvc_cursor", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_cursor2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "pvc_cursor2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"vu1\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "vu", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"vu2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "vu2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"vu3\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "vu3", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"mv_colour\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "mv_colour", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"mv_wirecolour\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "mv_wirecolour", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"mv_polycolour\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "mv_polycolour", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"mv_wirewidth\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "mv_wirewidth", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"mv_wireaa\"=hex:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "mv_wireaa", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"mv_generator_fontcolour\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				psy_property_append_int(properties, "mv_generator_fontcolour", _httoi(q+1), 0, 0);				
			}
		}
		else if (strstr(buf,"\"mv_effect_fontcolour\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "mv_effect_fontcolour", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"mv_triangle_size\"=hex:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "mv_triangle_size", _httoi(q+1), 0, 0);
			}
		}

		//
		//
		//
		// legacy...
		//
		//
		//
		else if (strstr(buf,"\"mv_fontcolour\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				psy_property_append_int(properties, "mv_fontcolour_generator", _httoi(q+1), 0, 0);	
				psy_property_append_int(properties, "mv_fontcolour_effect", _httoi(q+1), 0, 0);				
			}
		}
		else if (strstr(buf,"\"machine_fontface\"=\""))
		{
			char *q = strchr(buf,61); // =
			if (q)
			{
				char *p;
				q+=2;
				p = strrchr(q,34); // "
				if (p)
				{
					p[0]=0;					
					psy_property_append_str(properties, "generator_fontface", q);	
					psy_property_append_str(properties, "effect_fontface", q);
				}
			}
		}
		else if (strstr(buf,"\"machine_font_point\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_property_append_int(properties, "generator_font_point", _httoi(q+1), 0, 0);
				psy_property_append_int(properties, "effect_font_point", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf, "\"machineGUITopColor\"=dword:"))
		{
			char* q = strchr(buf, 58); // :
			if (q)
			{
				psy_property_append_int(properties, "machineguitopcolour", _httoi(q + 1), 0, 0);			
			}
		}
		else if (strstr(buf, "\"machineGUIFontTopColor\"=dword:"))
		{
			char* q = strchr(buf, 58); // :
			if (q)
			{
				psy_property_append_int(properties, "machineguifonttopcolour", _httoi(q + 1), 0, 0);
			}
		}
		else if (strstr(buf, "\"machineGUIBottomColor\"=dword:"))
		{
			char* q = strchr(buf, 58); // :
			if (q)
			{
				psy_property_append_int(properties, "machineguibottomcolour", _httoi(q + 1), 0, 0);
			}
		}
		else if (strstr(buf, "\"machineGUIFontBottomColor\"=dword:"))
		{
			char* q = strchr(buf, 58); // :
			if (q)
			{
				psy_property_append_int(properties, "machineguifontbottomcolour", _httoi(q + 1), 0, 0);
			}
		}
		else if (strstr(buf, "\"machineGUIHTopColor\"=dword:"))
		{
			char* q = strchr(buf, 58); // :
			if (q)
			{
				psy_property_append_int(properties, "machineguihtopcolour", _httoi(q + 1), 0, 0);
			}
		}
		else if (strstr(buf, "\"machineGUIHFontTopColor\"=dword:"))
		{
			char* q = strchr(buf, 58); // :
			if (q)
			{
				psy_property_append_int(properties, "machineguihfonttopcolour", _httoi(q + 1), 0, 0);
			}
		}
		else if (strstr(buf, "\"machineGUIHBottomColor\"=dword:"))
		{
			char* q = strchr(buf, 58); // :
			if (q)
			{
				psy_property_append_int(properties, "machineguihbottomcolour", _httoi(q + 1), 0, 0);
			}
		}
		else if (strstr(buf, "\"machineGUIHFontBottomColor\"=dword:"))
		{
			char* q = strchr(buf, 58); // :
			if (q)
			{
				psy_property_append_int(properties, "machineguihfontbottomcolour", _httoi(q + 1), 0, 0);
			}
		}
		else if (strstr(buf, "\"machineGUITitleColor\"=dword:"))
		{
			char* q = strchr(buf, 58); // :
			if (q)
			{
				psy_property_append_int(properties, "machineguititlecolour", _httoi(q + 1), 0, 0);
			}
		}
		else if (strstr(buf, "\"machineGUITitleFontColor\"=dword:"))
		{
		char* q = strchr(buf, 58); // :
		if (q)
		{
			psy_property_append_int(properties, "machineguititlefontcolour", _httoi(q + 1), 0, 0);
		}
		}
	}
	fclose(fp);
	return PSY_OK;
}

int skin_load_machine_skin(psy_Property* psm, const char* path)
{	
	int rv;	
	char* p;
	psy_PropertyReader propertyreader;

	assert(psm);

	skin_define_psm(psm);
	psy_propertyreader_init(&propertyreader, psm, path);	
	psy_propertyreader_allow_cpp_comments(&propertyreader);
	rv = psy_propertyreader_load(&propertyreader);
	psy_propertyreader_dispose(&propertyreader);
	/* skip the "dword:"  and "hex:" keywords */
	p = strrchr(psy_property_at_str(psm, "transparency", ""), ':');
	if (p) {
		psy_property_set_str(psm, "transparency", p + 1);
	}
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

int skin_loadpsh(psy_Property* properties, const char* path)
{
	char buf[1 << 10];
	int loaded = 0;
	FILE* fp;	

	if ((fp = fopen(path, "r")) == NULL )
	{
//		MessageBox("Couldn't open File for Reading. Operation Aborted","File Open Error",MB_OK);
		return PSY_ERRFILE;
	}
	while(fgets(buf, sizeof buf, fp))
	{
		char* equal;

		if(buf[0] == '#' || (buf[0] == '/' && buf[1] == '/') || buf[0] == '\n')
		{ 
			// Skip comments
			continue;
		} 
		equal = strchr(buf,'=');
		if (equal != NULL)
		{
			char key[512];
			char* value;
			uintptr_t length;

			equal[0]='\0';
			//Skip the double quotes containing strings			
			if(buf[0] == '"')
			{
				char *tmp = equal-1;
				tmp[0]='\0';
				tmp = buf+1;
				strcpy(key, tmp);
			} else { strcpy(key, buf);	}

			value = &equal[1];
			length = psy_strlen(value);
			if(value[0] == '"')
			{
				length-=2;
				value++;
				value[length-1]='\0';
			}
			else {
				//skip the "dword:"  and "hex:" keywords
				char* twodots = strchr(value,':');
				if(twodots != NULL) {
					value =  &twodots[1];
				}
			}			
			psy_property_append_str(properties, key, value);			
			loaded = 1;
		}
	}
	if(!loaded)
	{
		//MessageBox(0, "No settings found in the specified file",
		//	"File Load Error", MB_ICONERROR | MB_OK);		
	}
	fclose(fp);
	return PSY_OK;
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

int skin_loadpsc(psy_Property* self, const char* path)
{
	FILE* hfile;
	psy_Property props;
	const char* strvalue;
	int success;

	assert(self);

	if (!(hfile = fopen(path, "r")))
	{
		//::MessageBox(0, "Couldn't open File for Reading. Operation Aborted",
			//"File Open Error", MB_ICONERROR | MB_OK);
		return PSY_ERRFILE;
	}	
	psy_property_init(&props);
	success = skinio_loadproperties(hfile, &props);
	strvalue = psy_property_at_str(&props, "machinedial_bmp", "Psycle_110_knobs.bmp");
	if (strvalue) {
		psy_property_append_str(self, "machinedialbmp", strvalue);
	}
	psy_property_dispose(&props);
	if (hfile) {
		fclose(hfile);
	}
	return success;
}

int skinio_loadproperties(FILE* hfile, psy_Property* props)
{
	char buf[1 << 10];
	bool loaded = FALSE;
	while (fgets(buf, sizeof buf, hfile))
	{
		char* equal;

		if (buf[0] == '#' || (buf[0] == '/' && buf[1] == '/') || buf[0] == '\n')
		{
			// Skip comments
			continue;
		}
		equal = strchr(buf, '=');
		if (equal != NULL)
		{
			char* value;
			char* strvalue;
			char* key;
			uintptr_t length;

			equal[0] = '\0';
			//Skip the double quotes containing strings			
			if (buf[0] == '"')
			{
				char* tmp = equal - 1;
				tmp[0] = '\0';
				tmp = buf + 1;
				key = tmp;
			} else { key = buf; }

			value = &equal[1];
			length = psy_strlen(value);
			if (value[0] == '"')
			{
				length -= 2;
				value++;
				value[length - 1] = '\0';
			} else {
				//skip the "dword:"  and "hex:" keywords
				char* twodots = strchr(value, ':');
				if (twodots != NULL) {
					value = &twodots[1];
				}
			}
			strvalue = value;
			psy_property_set_str(props, key, strvalue);			
			loaded = TRUE;
		}
	}
	if (!loaded)
	{
		//::MessageBox(0, "No settings found in the specified file",
			//"File Load Error", MB_ICONERROR | MB_OK);
		return PSY_ERRFILE;
	}
	return PSY_OK;
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