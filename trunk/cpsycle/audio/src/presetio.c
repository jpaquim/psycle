/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "presetio.h"
/* file */
#include <dir.h>
/* std */
#include <stdio.h>
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static int presetio_loadversion0(FILE*, uintptr_t numpresets, uintptr_t numparameters,
	psy_audio_Presets*);
static int presetio_loadversion1(FILE*, uintptr_t numParameters,
	uintptr_t datasizestruct, psy_audio_Presets*);
static int psy_audio_presetsio_saveversion1(FILE*, psy_audio_Presets*);
/* implementation */
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

int psy_audio_presetsio_load(const psy_Path* path, psy_audio_Presets* presets,
	uintptr_t numparameters, uintptr_t datasizestruct, const char* pluginroot)
{	
	int status;
	FILE* fp;
		
	status = psy_audio_PRESETIO_OK;
	fp = fopen(psy_path_full(path), "rb");
	/* if not found, check if it is in the root plugin dir */
	if (!fp) {
		psy_Path rootpath;

		psy_path_init(&rootpath, psy_path_full(path));
		psy_path_set_prefix(&rootpath, pluginroot);
		fp = fopen(psy_path_full(&rootpath), "rb");
		psy_path_dispose(&rootpath);
	}
	/* if not found, check if it finds underscore names */
	if (!fp) {
		psy_Path underscorepath;
		char* name;

		psy_path_init(&underscorepath, psy_path_full(path));
		name = strdup(psy_path_name(&underscorepath));
		psy_replacechar(name, '-', '_');
		psy_path_set_name(&underscorepath, name);
		free(name);
		fp = fopen(psy_path_full(&underscorepath), "rb");
		psy_path_dispose(&underscorepath);
	}	
	if (fp != NULL) {
		int numpresets;
		int filenumpars;

		if (fread(&numpresets,sizeof(int), 1, fp) != 1 ||
			fread(&filenumpars,sizeof(int), 1, fp) != 1) {
			status = psy_audio_PRESETIO_ERROR_READ;			
		} else if (numpresets >= 0) {
			/* ok so we still support old file format by checking for a */ 
			/* positive numpresets */
			if ((filenumpars != numparameters)  || (datasizestruct)) {
				status = psy_audio_PRESETIO_ERROR_UPTODATE;
			} else {
				/* presets.clear(); */
				status = presetio_loadversion0(fp, numpresets, filenumpars,
					presets);
				/* presets.sort(); */
			}
		} else {
			/* new preset file format since numpresets was < 0 */
			/* we will use filenumpars already read as version # */
			if (filenumpars == 1) {
				/* presets.clear(); */
				status = presetio_loadversion1(fp, numparameters,
					datasizestruct, presets);
				/* presets.sort(); */
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

int presetio_loadversion0(FILE* fp, uintptr_t numpresets, uintptr_t numparameters,
	psy_audio_Presets* presets)
{	
	int status;
	char name[32];
	int* ibuf;
	uintptr_t i;
	
	status = psy_audio_PRESETIO_OK;
	ibuf = malloc(sizeof(int) * numparameters);
	for (i = 0; i < numpresets && !feof(fp) && !ferror(fp); ++i) {		
		psy_audio_Preset* preset;
		uintptr_t j;

		preset = psy_audio_preset_allocinit();
		fread(name, sizeof(name), 1, fp);
		psy_audio_preset_setname(preset, name);
		fread(ibuf, numparameters * sizeof(int), 1, fp);
		for (j = 0; j < numparameters; ++j) {
			psy_audio_preset_setvalue(preset, j, (intptr_t)ibuf[j]);
		}		
		psy_audio_presets_insert(presets, i, preset);
	}
	free(ibuf);
	return status;
}

int presetio_loadversion1(FILE* fp, uintptr_t numparameters,
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
	/* new preset format version 1 */
	fread(&numpresets, sizeof(int), 1, fp);
	fread(&filenumpars, sizeof(int), 1, fp);
	fread(&filepresetsize, sizeof(int), 1, fp);
	/* now it is time to check our file for compatability */
	if (filenumpars != numparameters || (filepresetsize != datasizestruct))
	{
		status = psy_audio_PRESETIO_ERROR_UPTODATE;
		return status;
	}
	/* ok that works, so we should now load the names of all of the presets	*/
	ibuf= malloc(sizeof(int) * numparameters);
	dbuf = 0;
	if (datasizestruct > 0) {
		dbuf = (unsigned char*)malloc(datasizestruct);
	}
	for (i = 0; i < numpresets && !feof(fp) && !ferror(fp); ++i) {
		psy_audio_Preset* preset;
		uintptr_t j;

		preset = psy_audio_preset_allocinit();
		fread(name, sizeof(name), 1, fp);
		psy_audio_preset_setname(preset, name);
		fread(ibuf, numparameters * sizeof(int), 1, fp);
		for (j = 0; j < numparameters; ++j) {
			psy_audio_preset_setvalue(preset, j, (intptr_t)ibuf[j]);
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

int psy_audio_presetsio_save(const psy_Path* path, psy_audio_Presets* presets)
{
	int status;
	FILE* fp;

	status = psy_audio_PRESETIO_OK;
	if (presets && !psy_audio_presets_empty(presets)) {
		if (!(fp = fopen(psy_path_full(path), "wb")))
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
	/* psy_TableIterator preset_iterator; */
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
	/* preset_iterator = presets->container; */
	numpresets = (int32_t)((psy_table_empty(&presets->container))
		? 0
		: psy_table_maxkey(&presets->container) + 1);

	if (numpresets != 0) {
		psy_audio_Preset* preset;

		preset = (psy_audio_Preset*)psy_audio_presets_at(presets, numpresets - 1);
		numparameters = (int32_t)psy_table_size(&preset->parameters);
		datasizestruct = (int32_t)preset->datasize;
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

/* preset io */
static int psy_audio_presetio_savefxpversion0(FILE*, psy_audio_Preset*);

int psy_audio_presetio_savefxp(const psy_Path* path, psy_audio_Preset* preset)
{
	int status;
	FILE* fp;

	status = psy_audio_PRESETIO_OK;
	if (preset) {
		if (!(fp = fopen(psy_path_full(path), "wb")))
		{
			status = psy_audio_PRESETIO_ERROR_WRITEOPEN;
			return status;
		}
		psy_audio_presetio_savefxpversion0(fp, preset);
		fclose(fp);
	}
	return status;
}

int psy_audio_presetio_savefxpversion0(FILE* fp, psy_audio_Preset* preset)
{		
	int32_t temp32;
	char name[28];	
	const char chnk[] = "CcnK";
	intptr_t sizepos;
	intptr_t currpos;
	
	fwrite(chnk, 4, 1, fp);
	sizepos = ftell(fp);
	/* sizepos is filled at save end */
	temp32 = 0;
	fwrite(&temp32, sizeof(temp32), 1, fp);
	temp32 = preset->magic;
	fwrite(&temp32, sizeof(temp32), 1, fp);
	/* version */
	temp32 = 1;
	fwrite(&temp32, sizeof(temp32), 1, fp);
	temp32 = preset->id;
	fwrite(&temp32, sizeof(temp32), 1, fp);
	temp32 = preset->version;
	fwrite(&temp32, sizeof(temp32), 1, fp);
	if (preset->datasize > 0) {
		temp32 = 0;
	} else {
		temp32 = (int32_t)psy_audio_preset_numparameters(preset);
	}
	fwrite(&temp32, sizeof(temp32), 1, fp);
	memset(&name, 0, sizeof(name));
	psy_snprintf(name, 28, psy_audio_preset_name(preset));
	if (fwrite(&name, sizeof(char), 28, fp) != 28) {
		return psy_audio_PRESETIO_ERROR_WRITE;
	}	
	if (preset->datasize > 0) {
		temp32 = (int32_t)preset->datasize;
		fwrite(&temp32, sizeof(temp32), 1, fp);		
		if (fwrite(&preset->data, 1, preset->datasize, fp) != preset->datasize) {
			return psy_audio_PRESETIO_ERROR_WRITE;
		}				
	} else {
		uintptr_t param;

		for (param = 0; param < psy_audio_preset_numparameters(preset); ++param) {
			float value;

			value = psy_audio_preset_value(preset, param) / (float)0xFFFF;
			if (fwrite(&value, sizeof(float), 1, fp) != sizeof(float)) {
				return psy_audio_PRESETIO_ERROR_WRITE;
			}
		}
	}
	currpos = ftell(fp);
	fseek(fp, (int32_t)sizepos, SEEK_SET);
	temp32 = (int32_t)sizepos - (int32_t)currpos;
	fwrite(&temp32, sizeof(int32_t), 1, fp);	
	fseek(fp, (int32_t)currpos, SEEK_SET);
	return psy_audio_PRESETIO_OK;
}
