#! /usr/bin/env python
# This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
# copyright 2009-2009 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

from logger import silent, is_debug, debug
from task import ProjectTask
from option_cfg import OptionCfg
from fhs import FHS
from signature import Sig

import os, errno, shutil
if os.name == 'posix':
	def install(src, dst):
		try: os.link(src, dst) # try to do a hard link
		except OSError, e:
			if e.errno != errno.EXDEV: raise 
			# error: cross-device link: dst is on another filesystem than src
			shutil.copy2(src, dst) # fallback to copying
else: # hard links unavailable, do a copy instead
	install = shutil.copy2

class InstallTask(ProjectTask, OptionCfg):
	known_options = set(['check-missing'])

	@staticmethod
	def generate_option_help(help): help['check-missing'] = ('<yes|no>', 'check for missing built files (rebuilds files you manually deleted in the build dir)', 'no')
	
	def __init__(self, project):
		ProjectTask.__init__(self, project)
		OptionCfg.__init__(self, project)
		self.fhs = FHS(project)

		try: old_sig, self.check_missing = self.project.persistent[str(self.__class__)]
		except KeyError: old_sig = None
		if old_sig != self.options_sig:
			if __debug__ and is_debug: debug('cfg: install: user: parsing options')
			o = self.options

			if 'check-missing' in o: self.check_missing = o['check-missing'] == 'yes'
			else: self.check_missing = False

			self.project.persistent[str(self.__class__)] = self.options_sig, self.check_missing

	def __str__(self): return \
		'installing from ' + str(self.trim_prefix) + \
		' to ' + str(self.dest_dir) + \
		': ' + ' '.join([s.rel_path(self.trim_prefix) for s in self.sources])

	@property
	def uid(self):
		try: return self._uid
		except AttributeError:
			self._uid = str(self.__class__) + '#' + str(self.sources[0])
			return self._uid
	
	@property
	def trim_prefix(self): raise Exception, str(self.__class__) + ' did not redefine the property.'
	
	@property
	def sources(self): raise Exception, str(self.__class__) + ' did not redefine the property.'
	
	@property
	def dest_dir(self): raise Exception, str(self.__class__) + ' did not redefine the property.'
	
	def __call__(self, sched_context):
		try: old_sig = self.project.persistent[self.uid]
		except KeyError: old_sig = None
		sigs = [s.sig for s in self.sources]
		sigs.sort()
		sig = Sig(''.join(sigs))
		sig.update(self.dest_dir.abs_path)
		sig = sig.digest()
		if old_sig != sig or self.check_missing:
			if False: # TODO This is needed on mswindows were timestamps cannot reliably be copied
				changed_sources = []
				for s in self.sources:
					try: old_source_sig = old_source_sigs[s]
					except KeyError:
						# This is a new source.
						changed_sources.append(s)
						continue
					if old_source_sig != s.sig:
						# The source changed.
						changed_sources.append(s)
			sched_context.lock.release()
			try:
				self.dest_dir.lock.acquire()
				try:
					install_tuples = []
					for s in self.sources:
						rel_path = s.rel_path(self.trim_prefix)
						dest = self.dest_dir / rel_path
						if not dest.exists or dest.time != s.time: install_tuples.append((s, dest, rel_path))
					if len(install_tuples) != 0:
						if not silent: self.print_desc(
							'installing from ' + str(self.trim_prefix) +
							' to ' + str(self.dest_dir) +
							':\n\t' + '\n\t'.join((t[2] for t in install_tuples)),
							'47;34'
						)
						for t in install_tuples:
							s, dest = t[0], t[1]
							if dest.exists: os.remove(dest.path)
							else: dest.parent.make_dir()
							install(s.path, dest.path)
					self.project.persistent[self.uid] = sig
				finally: self.dest_dir.lock.release()
			finally: sched_context.lock.acquire()
