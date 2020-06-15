// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "presetio.h"

#include <stdlib.h>
#include <stdio.h>

#include <dir.h>

#include "../../detail/portable.h"

static int presetio_loadversion0(FILE*, int numpresets, int numparameters,
	psy_audio_Presets*);
static int presetio_loadversion1(FILE*, int numParameters,
	uintptr_t datasizestruct, psy_audio_Presets*);
static int psy_audio_presetsio_saveversion1(FILE*, psy_audio_Presets*);

int psy_audio_presetsio_load(const char* filename, psy_audio_Presets* presets,
	uintptr_t numparameters, uintptr_t datasizestruct, const char* pluginroot)
{	
	int status;
	FILE* fp;
		
	status = psy_audio_PRESETIO_OK;
	fp = fopen(filename, "rb");
	// if not found, check if it is in the root plugin dir
	if (!fp) {
		psy_Path path;

		psy_path_init(&path, filename);
		psy_path_setprefix(&path, pluginroot);		
		fp = fopen(psy_path_path(&path), "rb");
		psy_path_dispose(&path);
	}
	// if not found, check if it finds underscore names
	if (!fp) {
		psy_Path path;
		char* name;

		psy_path_init(&path, filename);
		name = strdup(psy_path_name(&path));
		psy_replacechar(name, '-', '_');
		psy_path_setname(&path, name);				
		free(name);
		fp = fopen(psy_path_path(&path), "rb");
		psy_path_dispose(&path);
	}	
	if (fp != NULL) {
		int numpresets;
		int filenumpars;

		if (fread(&numpresets,sizeof(int), 1, fp) != 1 ||
			fread(&filenumpars,sizeof(int), 1, fp) != 1) {
			status = psy_audio_PRESETIO_ERROR_READ;			
		} else if (numpresets >= 0) {
			// ok so we still support old file format by checking for a
			// positive numpresets
			if ((filenumpars != numparameters)  || (datasizestruct)) {
				status = psy_audio_PRESETIO_ERROR_UPTODATE;
			} else {
				// presets.clear();
				status = presetio_loadversion0(fp, numpresets, filenumpars,
					presets);
				// presets.sort();
			}
		} else {
			// new preset file format since numpresets was < 0
			// we will use filenumpars already read as version #
			if (filenumpars == 1) {
				// presets.clear();
				status = presetio_loadversion1(fp, numparameters,
					datasizestruct, presets);
				// presets.sort();
			} else {
				status = psy_audio_PRESETIO_ERROR_NEWVERSION;				
			}
		}
		fclose(fp);
	} else {
		status = psy_audio_PRESETIO_ERROR_OPEN;		
	}
	return status;
}

int presetio_loadversion0(FILE* fp, int numpresets, int numparameters,
	psy_audio_Presets* presets)
{	
	int status;
	char name[32];
	int* ibuf;
	int i;
	
	status = psy_audio_PRESETIO_OK;
	ibuf = malloc(sizeof(int) * numparameters);
	for (i = 0; i < numpresets && !feof(fp) && !ferror(fp); ++i) {		
		psy_audio_Preset* preset;
		int j;

		preset = psy_audio_preset_allocinit();
		fread(name, sizeof(name), 1, fp);
		psy_audio_preset_setname(preset, name);
		fread(ibuf, numparameters * sizeof(int), 1, fp);
		for (j = 0; j < numparameters; ++j) {
			psy_audio_preset_setvalue(preset, j, (intptr_t) ibuf[j]);
		}		
		psy_audio_presets_insert(presets, i, preset);
	}
	free(ibuf);
	return status;
}

int presetio_loadversion1(FILE* fp, int numparameters,
	uintptr_t datasizestruct, psy_audio_Presets* presets)
{
	int status;
	int numpresets;
	int filenumpars;
	int filepresetsize;
	int* ibuf;
	unsigned char* dbuf;
	char name[32];
	int i;

	status = psy_audio_PRESETIO_OK;
	// new preset format version 1
	fread(&numpresets, sizeof(int), 1, fp);
	fread(&filenumpars, sizeof(int), 1, fp);
	fread(&filepresetsize, sizeof(int), 1, fp);
	// now it is time to check our file for compatability
	if (filenumpars != numparameters || (filepresetsize != datasizestruct))
	{
		status = psy_audio_PRESETIO_ERROR_UPTODATE;
		return status;
	}
	// ok that works, so we should now load the names of all of the presets	
	ibuf= malloc(sizeof(int) * numparameters);
	dbuf = 0;
	if (datasizestruct > 0) {
		dbuf = (unsigned char*)malloc(datasizestruct);
	}
	for (i = 0; i < numpresets && !feof(fp) && !ferror(fp); ++i) {
		psy_audio_Preset* preset;
		int j;

		preset = psy_audio_preset_allocinit();
		fread(name, sizeof(name), 1, fp);
		psy_audio_preset_setname(preset, name);
		fread(ibuf, numparameters * sizeof(int), 1, fp);
		for (j = 0; j < numparameters; ++j) {
			psy_audio_preset_setvalue(preset, j, (intptr_t) ibuf[j]);
		}
		if (datasizestruct > 0) {
			fread(dbuf, datasizestruct, 1, fp);
			psy_audio_preset_setdatastruct(preset, numparameters,name, ibuf,
				datasizestruct,dbuf);
		}
		psy_audio_presets_insert(presets, i, preset);
	}
	free(ibuf);
	free(dbuf);
	return status;
}

int psy_audio_presetsio_save(const char* path, psy_audio_Presets* presets)
{
	int status;
	FILE* fp;

	status = psy_audio_PRESETIO_OK;
	if (presets && !psy_audio_presets_empty(presets)) {
		if (!(fp = fopen(path, "wb")))
		{
			status = psy_audio_PRESETIO_ERROR_WRITEOPEN;			
			return status;
		}		
		psy_audio_presetsio_saveversion1(fp, presets);
		fclose(fp);
	}
	return status;
}

int psy_audio_presetsio_saveversion1(FILE* fp, psy_audio_Presets* presets)
{
	int status;
	int32_t temp1 = -1;
	int32_t fileversion = 1;
	// psy_TableIterator preset_iterator;
	int32_t numpresets;
	int32_t numparameters;
	int32_t datasizestruct;	

	status = psy_audio_PRESETIO_OK;
	if (fwrite(&temp1, sizeof(int32_t), 1, fp) != 1 ||
		fwrite(&fileversion, sizeof(int32_t), 1, fp) != 1)
	{
		status = psy_audio_PRESETIO_ERROR_WRITE;		
		return status;
	}
	// preset_iterator = presets->container;
	numpresets = (psy_table_empty(&presets->container))
		? 0
		: psy_table_maxkey(&presets->container) + 1;

	if (numpresets != 0) {
		psy_audio_Preset* preset;

		preset = (psy_audio_Preset*)psy_audio_presets_at(presets, numpresets - 1);
		numparameters = psy_table_size(&preset->parameters);
		datasizestruct = preset->datasize;
	} else {
		numparameters = 0;
		datasizestruct = 0;
	}	
	fwrite(&numpresets, sizeof(int), 1, fp);
	fwrite(&numparameters, sizeof(int), 1, fp);
	fwrite(&datasizestruct, sizeof(int), 1, fp);

	if (numparameters > 0) {
		char cbuf[32];
		int* ibuf;
		unsigned char* dbuf = 0;
		int32_t i;

		ibuf = malloc(sizeof(int32_t) * numparameters);
		dbuf = NULL;
		if (datasizestruct > 0) {
			dbuf = malloc(datasizestruct);
		}
		for (i = 0; i < numpresets && !feof(fp) && !ferror(fp); i++) {
			psy_audio_Preset* preset;
			int32_t j;
			bool empty;

			preset = (psy_audio_Preset*)psy_audio_presets_at(presets, (uintptr_t)i);
			empty = (preset == NULL);
			if (!preset) {				
				preset = (psy_audio_Preset*)psy_audio_presets_at(presets, numpresets - 1);
			}
			for (j = 0; j < numparameters; ++j) {
				ibuf[j] = (int32_t)psy_audio_preset_value(preset, j);
			}
			memset(cbuf, 0, sizeof(cbuf));
			if (empty) {
				psy_snprintf(cbuf, 32, "%s", "");
			} else {
				psy_snprintf(cbuf, 32, "%s", psy_audio_preset_name(preset));
			}
			fwrite(cbuf, sizeof(cbuf), 1, fp);
			fwrite(ibuf, numparameters * sizeof(int32_t), 1, fp);
			if (datasizestruct > 0) {
				fwrite(dbuf, datasizestruct, 1, fp);
			}
		}
		free(ibuf);
		free(dbuf);
	}
	return status;
}

const char* psy_audio_presetsio_statusstr(int status)
{	
	switch (status) {
		case psy_audio_PRESETIO_OK:
			return "Preset File OK";
			break;
		case psy_audio_PRESETIO_ERROR_OPEN:
			return "Couldn't open file. Operation aborted";
			break;
		case psy_audio_PRESETIO_ERROR_READ:
			return "Couldn't read from file. Operation aborted";
			break;
		case psy_audio_PRESETIO_ERROR_UPTODATE:
			return "The current preset File is not up-to-date with the plugin.";
			break;
		case psy_audio_PRESETIO_ERROR_NEWVERSION:
			return "The current preset file is from a newer version of psycle than you are currently running.";
			break;
		case psy_audio_PRESETIO_ERROR_WRITEOPEN:
			return "The File couldn't be opened for Writing. Operation Aborted";
			break;
		case psy_audio_PRESETIO_ERROR_WRITE:
			return "Couldn't write to File. Operation Aborted";
			break;
		default:
			break;
	}
	return "Preset File Error";
}
