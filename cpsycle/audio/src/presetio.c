// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "presetio.h"

#include <stdlib.h>
#include <stdio.h>

static void presetio_loadversion0(FILE*, int numpresets, int numparameters, psy_audio_Presets*);
static void presetio_loadversion1(FILE*, int numParameters, psy_audio_Presets*);

void psy_audio_presetsio_load(const char* path, psy_audio_Presets* presets)
{
	FILE* fp;	
	
	if ((fp = fopen(path, "rb")) != 0) {
		int numpresets;
		int filenumpars;
		if (fread(&numpresets,sizeof(int), 1, fp) != 1 ||
			fread(&filenumpars,sizeof(int), 1, fp) != 1 ) {
			// ::MessageBox(0,"Couldn't read from file. Operation aborted","Preset File Error",MB_OK);
		} else if (numpresets >= 0) {
			// ok so we still support old file format by checking for a positive numpresets
//			if (filenumpars != numparameters) //  || (dataSizeStruct))
//			{
//				// ::MessageBox(0,"The current preset File is not up-to-date with the plugin.","Preset File Error",MB_OK);
//			} else {
				// presets.clear();
			presetio_loadversion0(fp, numpresets, filenumpars, presets);
				// presets.sort();
			//}
		} else {
			// new preset file format since numpresets was < 0
			// we will use filenumpars already read as version #
			if (filenumpars == 1) {
				// presets.clear();
				presetio_loadversion1(fp, numpresets, presets);
				// presets.sort();
			} else {
				// ::MessageBox(0,"The current preset file is from a newer version of psycle than you are currently running.","Preset File Error",MB_OK);
			}
		}
		fclose(fp);
	} else {
		//::MessageBox(0,"Couldn't open file. Operation aborted","Preset File Error",MB_OK);
	}	
}

void psy_audio_presetsio_save(const char* path, psy_audio_Presets* presets)
{

}

void presetio_loadversion0(FILE* fp, int numpresets, int numparameters, psy_audio_Presets* presets)
{	
	char name[32];
	int* ibuf;
	int i;
	
	ibuf = malloc(sizeof(int) * numparameters);
	for (i = 0; i < numpresets && !feof(fp) && !ferror(fp); ++i) {		
		psy_audio_Preset* preset;
		int j;

		preset = psy_audio_preset_allocinit();
		fread(name, sizeof(name), 1, fp);
		psy_audio_preset_setname(preset, name);
		fread(ibuf, numparameters * sizeof(int), 1, fp);
		for (j = 0; j < numparameters; ++j) {
			psy_audio_preset_setvalue(preset, j, ibuf[j]);
		}		
		psy_audio_presets_append(presets, preset);
	}
	free(ibuf);
}

void presetio_loadversion1(FILE* fp, int numparameters, psy_audio_Presets* presets)
{
	int numpresets;
	int filenumpars;
	int filepresetsize;
	int* ibuf;
	unsigned char* dbuf;
	char name[32];
	int i;

	// new preset format version 1
	fread(&numpresets, sizeof(int), 1, fp);
	fread(&filenumpars, sizeof(int), 1, fp);
	fread(&filepresetsize, sizeof(int), 1, fp);
	// now it is time to check our file for compatability
	if (filenumpars != numparameters) // || (filepresetsize != dataSizeStruct))
	{
		//::MessageBox(0,"The current preset File is not up-to-date with the plugin.","Preset File Error",MB_OK);
		return;
	}
	// ok that works, so we should now load the names of all of the presets
	
	ibuf= malloc(sizeof(int) * numparameters);
	dbuf = 0;
	// if ( dataSizeStruct > 0 ) dbuf = new unsigned char[dataSizeStruct];

	for (i = 0; i < numpresets && !feof(fp) && !ferror(fp); ++i) {
		psy_audio_Preset* preset;
		int j;

		preset = psy_audio_preset_allocinit();
		fread(name, sizeof(name), 1, fp);
		psy_audio_preset_setname(preset, name);
		fread(ibuf, numparameters * sizeof(int), 1, fp);
		for (j = 0; j < numparameters; ++j) {
			psy_audio_preset_setvalue(preset, j, ibuf[j]);
		}
		// if ( dataSizeStruct > 0 )  fread(dbuf, dataSizeStruct, 1, hfile);		
		// preset.Init(numParameters,name,ibuf,dataSizeStruct,dbuf);
		psy_audio_presets_append(presets, preset);
	}
	free(ibuf);
	free(dbuf);
}
