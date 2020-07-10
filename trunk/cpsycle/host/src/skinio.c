// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "../../detail/psydef.h"
#include "skinio.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int _httoi(const char *value)
{  
	return (int)strtol(value, 0, 16);
}

int skin_load(psy_Properties* properties, const char* path)
{
	FILE* fp;
	char buf[512];

	if ((fp = fopen(path, "rb")) == NULL )
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
					psy_properties_append_string(properties, "pattern_fontface", q);
				}
			}
		}
		else if (strstr(buf,"\"pattern_font_point\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "pattern_font_point", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pattern_font_flags\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "pattern_font_flags", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pattern_font_x\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "pattern_font_x", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pattern_font_y\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "pattern_font_y", _httoi(q+1), 0, 0);
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
					psy_properties_append_string(properties, "pattern_header_skin", q);
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
					psy_properties_append_string(properties, "generator_fontface", q);
				}
			}
		}
		else if (strstr(buf,"\"generator_font_point\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "generator_font_point", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"generator_font_flags\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "generator_font_flags", _httoi(q+1), 0, 0);
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
					psy_properties_append_string(properties, "effect_fontface", q);
				}
			}
		}
		else if (strstr(buf,"\"effect_font_point\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "effect_font_point", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"effect_font_flags\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "effect_font_flags", _httoi(q+1), 0, 0);
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
					psy_properties_append_string(properties, "machine_skin", q);
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
					psy_properties_append_string(properties, "machine_background", q);
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
				psy_properties_append_int(properties, "pvc_separator", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_separator2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "pvc_separator2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_background\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "pvc_background", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_background2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "pvc_background2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_font\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "pvc_font", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_font2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "pvc_font2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_fontCur\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "pvc_fontcur", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_fontCur2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "pvc_fontcur2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_fontSel\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "pvc_fontsel", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_fontSel2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "pvc_fontsel2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_fontPlay\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "pvc_fontplay", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_fontPlay2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				psy_properties_append_int(properties, "pvc_fontplay2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_row\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				psy_properties_append_int(properties, "pvc_row", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_row2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				psy_properties_append_int(properties, "pvc_row2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_rowbeat\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				psy_properties_append_int(properties, "pvc_rowbeat", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_rowbeat2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				psy_properties_append_int(properties, "pvc_rowbeat2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_row4beat\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				psy_properties_append_int(properties, "pvc_row4beat", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_row4beat2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				psy_properties_append_int(properties, "pvc_row4beat2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_selection\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "pvc_selection", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_selection2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "pvc_selection2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_playbar\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "pvc_playbar", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_playbar2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "pvc_playbar2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_cursor\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "pvc_cursor", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_cursor2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "pvc_cursor2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"vu1\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "vu", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"vu2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "vu2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"vu3\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "vu3", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"mv_colour\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "mv_colour", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"mv_wirecolour\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "mv_wirecolour", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"mv_polycolour\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "mv_polycolour", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"mv_wirewidth\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "mv_wirewidth", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"mv_wireaa\"=hex:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "mv_wireaa", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"mv_generator_fontcolour\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				psy_properties_append_int(properties, "mv_generator_fontcolour", _httoi(q+1), 0, 0);				
			}
		}
		else if (strstr(buf,"\"mv_effect_fontcolour\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "mv_effect_fontcolour", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"mv_triangle_size\"=hex:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "mv_triangle_size", _httoi(q+1), 0, 0);
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
				psy_properties_append_int(properties, "mv_fontcolour_generator", _httoi(q+1), 0, 0);	
				psy_properties_append_int(properties, "mv_fontcolour_effect", _httoi(q+1), 0, 0);				
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
					psy_properties_append_string(properties, "generator_fontface", q);	
					psy_properties_append_string(properties, "effect_fontface", q);
				}
			}
		}
		else if (strstr(buf,"\"machine_font_point\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				psy_properties_append_int(properties, "generator_font_point", _httoi(q+1), 0, 0);
				psy_properties_append_int(properties, "effect_font_point", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf, "\"machineGUITopColor\"=dword:"))
		{
			char* q = strchr(buf, 58); // :
			if (q)
			{
				psy_properties_append_int(properties, "machineguitopcolor", _httoi(q + 1), 0, 0);			
			}
		}
		else if (strstr(buf, "\"machineGUIFontTopColor\"=dword:"))
		{
			char* q = strchr(buf, 58); // :
			if (q)
			{
				psy_properties_append_int(properties, "machineguifonttopcolor", _httoi(q + 1), 0, 0);
			}
		}
		else if (strstr(buf, "\"machineGUIBottomColor\"=dword:"))
		{
			char* q = strchr(buf, 58); // :
			if (q)
			{
				psy_properties_append_int(properties, "machineguibottomcolor", _httoi(q + 1), 0, 0);
			}
		}
		else if (strstr(buf, "\"machineGUIFontBottomColor\"=dword:"))
		{
			char* q = strchr(buf, 58); // :
			if (q)
			{
				psy_properties_append_int(properties, "machineguifontbottomcolor", _httoi(q + 1), 0, 0);
			}
		}
		else if (strstr(buf, "\"machineGUIHTopColor\"=dword:"))
		{
			char* q = strchr(buf, 58); // :
			if (q)
			{
				psy_properties_append_int(properties, "machineguihtopcolor", _httoi(q + 1), 0, 0);
			}
		}
		else if (strstr(buf, "\"machineGUIHFontTopColor\"=dword:"))
		{
			char* q = strchr(buf, 58); // :
			if (q)
			{
				psy_properties_append_int(properties, "machineguihfonttopcolor", _httoi(q + 1), 0, 0);
			}
		}
		else if (strstr(buf, "\"machineGUIHBottomColor\"=dword:"))
		{
			char* q = strchr(buf, 58); // :
			if (q)
			{
				psy_properties_append_int(properties, "machineguihbottomcolor", _httoi(q + 1), 0, 0);
			}
		}
		else if (strstr(buf, "\"machineGUIHFontBottomColor\"=dword:"))
		{
			char* q = strchr(buf, 58); // :
			if (q)
			{
				psy_properties_append_int(properties, "machineguihfontbottomcolor", _httoi(q + 1), 0, 0);
			}
		}
		else if (strstr(buf, "\"machineGUITitleColor\"=dword:"))
		{
			char* q = strchr(buf, 58); // :
			if (q)
			{
				psy_properties_append_int(properties, "machineguititlecolor", _httoi(q + 1), 0, 0);
			}
		}
		else if (strstr(buf, "\"machineGUITitleFontColor\"=dword:"))
		{
		char* q = strchr(buf, 58); // :
		if (q)
		{
			psy_properties_append_int(properties, "machineguititlefontcolor", _httoi(q + 1), 0, 0);
		}
		}
	}
	fclose(fp);
	return PSY_OK;
}

int skin_loadpsh(psy_Properties* properties, const char* path)
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
			int length;

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
			length = strlen(value);
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
			psy_properties_append_string(properties, key, value);			
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

void skin_psh_values(const char* str, int maxcount, int* values)
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
