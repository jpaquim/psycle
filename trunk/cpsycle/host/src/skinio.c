// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "skinio.h"
#include <string.h>
#include <stdio.h>

#include <windows.h>


#define HexMapL 16

struct CHexMap
  {
    TCHAR chr;
    int value;
  };


struct CHexMap HexMap[HexMapL] =
  {
    {'0', 0}, {'1', 1},
    {'2', 2}, {'3', 3},
    {'4', 4}, {'5', 5},
    {'6', 6}, {'7', 7},
    {'8', 8}, {'9', 9},
    {'A', 10}, {'B', 11},
    {'C', 12}, {'D', 13},
    {'E', 14}, {'F', 15}
  };

int _httoi(const TCHAR *value)
{  
  TCHAR *mstr = _strupr(_strdup(value));
  TCHAR *s = mstr;
  BOOL firsttime;
  int result = 0;
  if (*s == '0' && *(s + 1) == 'X') s += 2;
  firsttime = TRUE;
  while (*s != '\0')
  {
    BOOL found = FALSE;
	int i;
    for (i = 0; i < HexMapL; i++)
    {
      if (*s == HexMap[i].chr)
      {
        if (!firsttime) result <<= 4;
        result |= HexMap[i].value;
        found = TRUE;
        break;
      }
    }
    if (!found) break;
    s++;
    firsttime = FALSE;
  }
  free(mstr);
  return result;
}


void skin_load(Properties* properties, const char* path)
{
	FILE* hfile;
	char buf[512];

	if ((hfile=fopen(path,"rw")) == NULL )
	{
//		MessageBox("Couldn't open File for Reading. Operation Aborted","File Open Error",MB_OK);
		return;
	}
	while (fgets(buf, 512, hfile))
	{
		if (strstr(buf,"\"pattern_fontface\"=\""))
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
					properties_append_string(properties, "pattern_fontface", q);
				}
			}
		}
		else if (strstr(buf,"\"pattern_font_point\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "pattern_font_point", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pattern_font_flags\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "pattern_font_flags", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pattern_font_x\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "pattern_font_x", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pattern_font_y\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "pattern_font_y", _httoi(q+1), 0, 0);
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
					properties_append_string(properties, "pattern_header_skin", q);
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
					properties_append_string(properties, "generator_fontface", q);
				}
			}
		}
		else if (strstr(buf,"\"generator_font_point\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "generator_font_point", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"generator_font_flags\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "generator_font_flags", _httoi(q+1), 0, 0);
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
					properties_append_string(properties, "effect_fontface", q);
				}
			}
		}
		else if (strstr(buf,"\"effect_font_point\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "effect_font_point", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"effect_font_flags\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "effect_font_flags", _httoi(q+1), 0, 0);
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
					properties_append_string(properties, "machine_skin", q);
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
					properties_append_string(properties, "machine_background", q);
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
				properties_append_int(properties, "pvc_separator", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_separator2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "pvc_separator2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_background\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "pvc_background", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_background2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "pvc_background2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_font\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "pvc_font", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_font2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "pvc_font2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_fontCur\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "pvc_fontcur", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_fontCur2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "pvc_fontcur2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_fontSel\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "pvc_fontsel", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_fontSel2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "pvc_fontsel2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_fontPlay\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "pvc_fontplay", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_fontPlay2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				properties_append_int(properties, "pvc_fontplay2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_row\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				properties_append_int(properties, "pvc_row", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_row2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				properties_append_int(properties, "pvc_row2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_rowbeat\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				properties_append_int(properties, "pvc_rowbeat", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_rowbeat2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				properties_append_int(properties, "pvc_rowbeat2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_row4beat\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				properties_append_int(properties, "pvc_row4beat", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_row4beat2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				properties_append_int(properties, "pvc_row4beat2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_selection\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "pvc_selection", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_selection2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "pvc_selection2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_playbar\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "pvc_playbar", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_playbar2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "pvc_playbar2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_cursor\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "pvc_cursor", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"pvc_cursor2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "pvc_cursor2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"vu1\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "vu", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"vu2\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "vu2", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"vu3\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "vu3", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"mv_colour\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "mv_colour", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"mv_wirecolour\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "mv_wirecolour", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"mv_polycolour\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "mv_polycolour", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"mv_wirewidth\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "mv_wirewidth", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"mv_wireaa\"=hex:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "mv_wireaa", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"mv_generator_fontcolour\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{
				properties_append_int(properties, "mv_generator_fontcolour", _httoi(q+1), 0, 0);				
			}
		}
		else if (strstr(buf,"\"mv_effect_fontcolour\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "mv_effect_fontcolour", _httoi(q+1), 0, 0);
			}
		}
		else if (strstr(buf,"\"mv_triangle_size\"=hex:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "mv_triangle_size", _httoi(q+1), 0, 0);
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
				properties_append_int(properties, "mv_fontcolour_generator", _httoi(q+1), 0, 0);	
				properties_append_int(properties, "mv_fontcolour_effect", _httoi(q+1), 0, 0);				
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
					properties_append_string(properties, "generator_fontface", q);	
					properties_append_string(properties, "effect_fontface", q);
				}
			}
		}
		else if (strstr(buf,"\"machine_font_point\"=dword:"))
		{
			char *q = strchr(buf,58); // :
			if (q)
			{				
				properties_append_int(properties, "generator_font_point", _httoi(q+1), 0, 0);	
				properties_append_int(properties, "effect_font_point", _httoi(q+1), 0, 0);
			}
		}
	}
	fclose(hfile);
}
