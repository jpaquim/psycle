#! /usr/bin/env python
# This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
# copyright 2006-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

import os

from signature import Sig, raw_to_hexstring
from cmd import exec_subprocess
from logger import is_debug, debug

class Task(object):	
	def __init__(self, project, aliases = None):
		self.in_tasks = []
		self.out_tasks = []
		self.processed = False
		self.executed = False
		self.project = project
		project.add_aliases(self, aliases)
		project.tasks.append(self)

	def dyn_in_tasks(self): return None
		
	def process(self): pass
	
	def uid(self): return None
	
	def old_sig(self):
		try: return self.project.task_sigs[self.uid()]
		except KeyError: return None
	
	def actual_sig(self): return None
	
	def update_sig(self): self.project.task_sigs[self.uid()] = self.actual_sig()
	
class Obj(Task):
	def uid(self):
		try: return self._uid
		except AttributeError:
			sig = self._uid = Sig(self.target.rel_path).digest()
			return sig

	def actual_sig(self):
		try: return self._actual_sig
		except AttributeError:
			sig = self._actual_sig = self.source.sig_to_string()
			return sig
		
	def process(self):
		if not os.path.exists(self.target.parent.rel_path):
			if __debug__ and is_debug: debug('mkdir: ' + self.target.parent.rel_path)
			os.makedirs(self.target.parent.rel_path)
		i = []
		for p in self.include_paths: i += ['-I', p.rel_path]
		return exec_subprocess(['c++', '-fPIC', '-o', self.target.rel_path, '-c', self.source.rel_path] + i)
		
class Lib(Task):
	def uid(self):
		try: return self._uid
		except AttributeError:
			sig = self._uid = Sig(self.target.rel_path).digest()
			return sig

	def actual_sig(self):
		try: return self._actual_sig
		except AttributeError:
			sig = Sig()
			for s in self.sources: sig.update(s.sig_to_string())
			sig = self._actual_sig = sig.digest()
			return sig

	def process(self):
		return exec_subprocess(['c++', '-shared', '-o', self.target.rel_path] + [s.rel_path for s in self.sources])

