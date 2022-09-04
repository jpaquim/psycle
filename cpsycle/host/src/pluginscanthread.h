/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PLUGINSCANTHREAD_H)
#define PLUGINSCANTHREAD_H


/* audio */
#include <plugincatcher.h>
/* thread */
#include <lock.h>
#include <thread.h>

/*
** PluginScanThread
**
** Scans parallel to the ui thread plugins.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PluginScanThread {
	/* signals */
	psy_Signal signal_plugincachechanged;
	psy_Signal signal_scanprogress;
	psy_Signal signal_scanstart;
	psy_Signal signal_scanend;
	psy_Signal signal_scanfile;
	psy_Signal signal_scantaskstart;
	/* internal */
	psy_Thread pluginscanthread;
	psy_Lock pluginscanlock;
	psy_audio_PluginCatcher* plugincatcher;
	int filescanned;
	char* scanfilename;
	int scanstart;
	int scanend;
	int scantaskstart;
	int plugincachechanged;
	int scanprogress;
	int scanprogresschanged;
	psy_audio_PluginScanTask lastscantask;
	int scanplugintype;	
} PluginScanThread;

void pluginscanthread_init(PluginScanThread*, psy_audio_PluginCatcher*);
void pluginscanthread_dispose(PluginScanThread*);

void pluginscanthread_start(PluginScanThread*);
void pluginscanthread_idle(PluginScanThread*);

struct Workspace;

typedef void (*fp_workspace_output)(void* context,
	struct Workspace* sender, const char* text);
typedef void (*fp_workspace_songloadprogress)(void* context,
	struct Workspace* sender, intptr_t progress);

#ifdef __cplusplus
}
#endif

#endif /* PLUGINSCANTHREAD_H */
