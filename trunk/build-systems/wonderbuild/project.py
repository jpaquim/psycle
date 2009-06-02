#! /usr/bin/env python
# This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
# copyright 2008-2009 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

import sys, os, gc, cPickle

from wonderbuild import UserReadableException
from task import Task
from script import ScriptLoaderTask
from filesystem import FileSystem
from logger import is_debug, debug

if __debug__ and is_debug: import time

class Project(Task):
	known_options = set(['src-dir', 'bld-dir', 'aliases', 'list-aliases'])

	@staticmethod
	def generate_option_help(help):
		help['src-dir'] = ('<dir>', 'use <dir> as the source dir', 'current working dir: ' + os.getcwd())
		help['bld-dir'] = ('<dir>', 'use <dir> as the build dir', '<src-dir>' + os.sep + '++wonderbuild')
		help['aliases'] = ('<name,...>', 'build tasks with aliases <name,...>, comma-separated list', 'default')
		help['list-aliases'] = (None, 'list the available task aliases')

	def __init__(self, options, option_collector):
		Task.__init__(self)
		self.options = options
		self.option_collector = option_collector
		option_collector.option_decls.add(self.__class__)
	
		src_path = options.get('src-dir', None)
		if src_path is None: src_path = os.getcwd()
		
		bld_path = options.get('bld-dir', None)
		if bld_path is None: bld_path = os.path.join(src_path, '++wonderbuild')
		
		if src_path == bld_path: raise Exception, 'build dir and source dir are the same'

		self.list_aliases = 'list-aliases' in options

		self.task_aliases = {} # {name: [tasks]}
		aliases = options.get('aliases', None)
		if aliases is not None:
			if len(aliases): self.requested_task_aliases = aliases.split(',')
			else: self.requested_task_aliases = (None,)
		else: self.requested_task_aliases = None

		self.processsing = False

		gc_enabled = gc.isenabled()
		if gc_enabled: gc.disable()
		try:
			try:
				try: f = open(os.path.join(bld_path, 'persistent.pickle'), 'rb')
				except IOError: raise
				else:
					try:
						try:
							if __debug__ and is_debug: t0 = time.time()
							self.persistent = cPickle.load(f)
							if __debug__ and is_debug: debug('project: pickle: load time: ' + str(time.time() - t0) + ' s')
						except Exception, e:
							print >> sys.stderr, 'could not load pickle:', e
							raise
					finally: f.close()
			except: self.persistent = {}
		finally:
			if gc_enabled: gc.enable()

		self.fs = FileSystem(self.persistent)
		self.top_src_dir = self.fs.cur / src_path
		self.bld_dir = self.fs.cur / bld_path

	def __call__(self, sched_ctx):
		self.sched_ctx = sched_ctx
		raise StopIteration

	def add_task_aliases(self, task, *aliases):
		if self.processsing: return # no need to add aliases during processing
		if len(aliases) == 0:
			if __debug__ and is_debug: aliases = (None,)
			else: return
		if __debug__ and is_debug: debug('project: aliases: ' + str(aliases) + ' ' + str(task.__class__))
		for a in aliases:
			try: self.task_aliases[a].append(task)
			except KeyError: self.task_aliases[a] = [task]

	def tasks_with_aliases(self, task_aliases = None):
		tasks = set()
		if task_aliases is None: task_aliases = (None,)
		try:
			for a in task_aliases: tasks |= set(self.task_aliases[a])
		except KeyError: raise UserReadableException, 'no such task alias: ' + a
		return tasks
		
	def process_tasks_by_aliases(self):
		try:
			if self.list_aliases:
				if 'help' in self.options: return
				keys = self.task_aliases.keys()
				keys.sort()
				for k in keys:
					if not (__debug__ and is_debug) and k is None: continue
					v = [str(v) for v in self.task_aliases[k]]
					v.sort()
					print (k and str(k) or '<none>') + '\n\t' + '\n\t'.join(v)
			else:
				if self.requested_task_aliases is not None: tasks = self.tasks_with_aliases(self.requested_task_aliases)
				else: tasks = self.task_aliases.get('default', ())
				self.processsing = True
				for x in self.sched_ctx.parallel_wait(*tasks): yield x
				self.processsing = False
		finally:
			if 'help' in self.options: return
			#self.bld_dir.forget()
			if False and __debug__ and is_debug: print self.persistent
			gc_enabled = gc.isenabled()
			if gc_enabled: gc.disable()
			try:
				path = os.path.join(self.bld_dir.path, 'persistent.pickle')
				if __debug__ and is_debug: t0 = time.time()
				try: f = open(path, 'wb')
				except IOError:
					self.bld_dir.make_dir()
					f = open(path, 'wb')
				try: cPickle.dump(self.persistent, f, cPickle.HIGHEST_PROTOCOL)
				finally: f.close()
				if __debug__ and is_debug:
					debug('project: pickle: dump time: ' + str(time.time() - t0) + ' s')
					debug('project: pickle: file size: ' + str(int(os.path.getsize(path) * 1000. / (1 << 20)) * .001) + ' MiB')
			finally:
				if gc_enabled: gc.enable()
