/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "pluginscanthread.h"
/* audio */
#include <exclusivelock.h>
/* platform */
#include "../../detail/os.h"
#include "../../detail/portable.h"


#if defined DIVERSALIS__OS__MICROSOFT
static unsigned int __stdcall pluginscanthread(void* context)
#else
static unsigned int pluginscanthread(void* context)
#endif
{
	PluginScanThread* self;

	assert(context);

	self = (PluginScanThread*)context;
	psy_audio_plugincatcher_scan(self->plugincatcher);
	self->filescanned = 0;
	self->scantaskstart = 0;
	free(self->scanfilename);
	self->scanfilename = NULL;
	return 0;
}

/* prototypes */
static void pluginscanthread_connect_plugincatcher(PluginScanThread*);
static void pluginscanthread_on_scan_end(PluginScanThread*, psy_audio_PluginCatcher* sender);
static void pluginscanthread_on_scan_file(PluginScanThread*, psy_audio_PluginCatcher* sender,
	const char* path, int type);
static void pluginscanthread_on_scan_progress(PluginScanThread*, psy_audio_PluginCatcher* sender,
	int progress);
void pluginscanthread_on_scan_start(PluginScanThread*, psy_audio_PluginCatcher* sender);
void pluginscanthread_on_scan_end(PluginScanThread*, psy_audio_PluginCatcher* sender);
static void pluginscanthread_on_scan_task_start(PluginScanThread*,
	psy_audio_PluginCatcher* sender, psy_audio_PluginScanTask*);
static void pluginscanthread_on_plugin_cache_changed(PluginScanThread*,
	psy_audio_PluginCatcher* sender);

void pluginscanthread_init(PluginScanThread* self, psy_audio_PluginCatcher* plugincatcher)
{
	assert(self);
	assert(plugincatcher);

	psy_thread_init(&self->pluginscanthread);
	psy_lock_init(&self->pluginscanlock);
	self->plugincatcher = plugincatcher;
	self->filescanned = 0;
	self->scanstart = 0;
	self->scanend = 0;
	self->scantaskstart = 0;
	self->plugincachechanged = 0;
	self->scanprogresschanged = 0;
	self->scanfilename = NULL;
	self->scanplugintype = psy_audio_UNDEFINED;
	pluginscanthread_connect_plugincatcher(self);
}

void pluginscanthread_dispose(PluginScanThread* self)
{
	psy_lock_dispose(&self->pluginscanlock);
	psy_thread_dispose(&self->pluginscanthread);	
	free(self->scanfilename);
	self->scanfilename = NULL;
}

void pluginscanthread_connect_plugincatcher(PluginScanThread* self)
{
	assert(self);

	psy_signal_connect(&self->plugincatcher->signal_changed, self,
		pluginscanthread_on_plugin_cache_changed);
	psy_signal_connect(&self->plugincatcher->signal_scanprogress, self,
		pluginscanthread_on_scan_progress);
	psy_signal_connect(&self->plugincatcher->signal_scanfile, self,
		pluginscanthread_on_scan_file);
	psy_signal_connect(&self->plugincatcher->signal_scanstart, self,
		pluginscanthread_on_scan_start);
	psy_signal_connect(&self->plugincatcher->signal_scanend, self,
		pluginscanthread_on_scan_end);
	psy_signal_connect(&self->plugincatcher->signal_taskstart, self,
		pluginscanthread_on_scan_task_start);
}

void pluginscanthread_start(PluginScanThread* self)
{
	assert(self);
	
	if (!psy_audio_plugincatcher_scanning(self->plugincatcher)) {
		free(self->scanfilename);
		self->scanfilename = NULL;
		self->filescanned = 0;
		self->plugincachechanged = 0;
		self->scanstart = 0;
		self->scantaskstart = 0;
		free(self->scanfilename);
		self->scanplugintype = psy_audio_UNDEFINED;
		psy_thread_start(&self->pluginscanthread, self,
			pluginscanthread);
	}
}

void pluginscanthread_on_scan_progress(PluginScanThread* self, psy_audio_PluginCatcher* sender,
	int progress)
{
	psy_lock_enter(&self->pluginscanlock);
	self->scanprogresschanged = 1;
	self->scanprogress = progress;
	psy_lock_leave(&self->pluginscanlock);
}

void pluginscanthread_on_scan_file(PluginScanThread* self, psy_audio_PluginCatcher* sender,
	const char* path, int type)
{
	assert(self);

	psy_lock_enter(&self->pluginscanlock);
	self->filescanned = 1;
	psy_strreset(&self->scanfilename, path);
	self->scanplugintype = type;
	psy_lock_leave(&self->pluginscanlock);
}

void pluginscanthread_on_scan_start(PluginScanThread* self, psy_audio_PluginCatcher* sender)
{
	assert(self);

	psy_lock_enter(&self->pluginscanlock);
	self->scanstart = 1;
	psy_lock_leave(&self->pluginscanlock);
}

void pluginscanthread_on_scan_end(PluginScanThread* self, psy_audio_PluginCatcher* sender)
{
	assert(self);

	psy_lock_enter(&self->pluginscanlock);
	self->scanend = 1;
	psy_lock_leave(&self->pluginscanlock);
}

void pluginscanthread_on_scan_task_start(PluginScanThread* self, psy_audio_PluginCatcher* sender,
	psy_audio_PluginScanTask* task)
{
	assert(self);
	assert(task);

	psy_lock_enter(&self->pluginscanlock);
	self->scantaskstart = 1;
	self->lastscantask = *task;
	psy_lock_leave(&self->pluginscanlock);
}

void pluginscanthread_on_plugin_cache_changed(PluginScanThread* self,
	psy_audio_PluginCatcher* sender)
{
	assert(self);

	psy_lock_enter(&self->pluginscanlock);
	self->plugincachechanged = 1;
	psy_lock_leave(&self->pluginscanlock);
}
