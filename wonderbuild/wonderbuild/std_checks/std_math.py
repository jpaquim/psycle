#! /usr/bin/env python
# This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
# copyright 2006-2009 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

import sys

from wonderbuild.cxx_tool_chain import BuildCheckTask
from wonderbuild.signature import Sig
from wonderbuild.logger import silent, is_debug, debug

class StdMathCheckTask(BuildCheckTask):
	def __init__(self, base_cfg): BuildCheckTask.__init__(self, 'c++-std-math', base_cfg)

	def apply_to(self, cfg):
		self.result
		if self.m: cfg.libs.append('m')

	class SubCheckTask(BuildCheckTask):
		def __init__(self, name, base_cfg, m):
			BuildCheckTask.__init__(self, name + '-with' + (not m and 'out' or '') + '-libm', base_cfg)
			self.m = m

		def apply_to(self, cfg):
			if self.m: cfg.libs.append('m')

		@property
		def source_text(self): return '#include <cmath>\nfloat math() { float const f(std::sin(1.f)); return f; }'
		
	def _make_t0(self): return StdMathCheckTask.SubCheckTask(self.name, self.base_cfg, False)
	def _make_t1(self): return StdMathCheckTask.SubCheckTask(self.name, self.base_cfg, True)

	def __call__(self, sched_ctx):
		changed = False
		try: old_sig, self._result, self.m = self.project.persistent[self.uid]
		except KeyError: changed = True
		else:
			if old_sig != self.sig: changed = True
		if not changed:
			if __debug__ and is_debug: debug('task: skip: no change: ' + self.name)
		else:
			if not silent:
				desc = 'checking for ' + self.name
				self.print_check(desc)
			self._t0 = self._make_t0()
			self._t0(sched_ctx)
			if self._t0.result: self._t1 = self._t0
			else:
				self._t1 = self._make_t1()
				self._t1(sched_ctx)
			if not silent:
				if self.result: self.print_check_result(desc, 'yes with' + (not self.m and 'out' or '') + ' libm', '32')
				else: self.print_check_result(desc, 'no', '31')
			self.project.persistent[self.uid] = self.sig, self.result, self.m
		
	@property
	def result(self):
		try: return self._result
		except AttributeError:
			self._result = self._t0.result or self._t1.result
			if self._t0.result: self.m = False
			elif self._t1.result: self.m = True
			else: self.m = None
			return self._result

	@property
	def sig(self):
		try: return self._sig
		except AttributeError:
			sig = Sig(self.base_cfg.cxx_sig)
			sig.update(self.base_cfg.ld_sig)
			sig = self._sig = sig.digest()
			return sig
