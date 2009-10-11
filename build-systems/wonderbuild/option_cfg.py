#! /usr/bin/env python
# This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
# copyright 2008-2009 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

from signature import Sig

from options import OptionDecl

class OptionCfg(OptionDecl):
	signed_options = set()

	def __init__(self, shared_task_holder):
		self.shared_task_holder = shared_task_holder
		shared_task_holder.option_collector.option_decls.add(self.__class__)

	@property
	def options(self): return self.shared_task_holder.options
	
	@property
	def options_sig(self):
		try: return self._options_sig # TODO this could actually be stored in the shared_task_holder since it's per (shared_task_holder, class)
		except AttributeError:
			sig = Sig()
			options = self.options
			for name in self.__class__.signed_options:
				value = options.get(name, None)
				if value is not None:
					if len(value) != 0: sig.update(value)
					else: sig.update('\0')
			sig = self._options_sig = sig.digest()
			return sig
