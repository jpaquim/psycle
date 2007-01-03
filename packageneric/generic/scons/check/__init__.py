# This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
# copyright 2006-2007 johan boule <bohan@jabber.org>
# copyright 2006-2007 psycledelics http://psycle.pastnotecut.org

from packageneric.generic.scons.node import node
from packageneric.generic.scons.named import named

class check(node, named):
	def __init__(self, project, name, dependencies = None, auto_add = True, *environment_args, **environment_kw):
		node.__init__(self, dependencies)
		named.__init__(self, name)
		self._project = project
		self._auto_add = auto_add
		self._environment_args = environment_args
		self._environment_kw = environment_kw

	def project(self): return self._project
	
	def auto_add(self): return self._auto_add
	
	def input_env(self):
		try: return self._input_env
		except AttributeError:
			self._input_env = self.project().env_class()(self.project(), *self._environment_args, **self._environment_kw)
			del self._environment_args
			del self._environment_kw
			for dependency in self.dependencies(): self._input_env.attach(dependency.output_env())
			return self._input_env
	
	def execute_env(self):
		try: return self._execute_env
		except AttributeError:
			self._execute_env = self.project().contexes().check_and_build().attached()
			self._execute_env.attach(self.input_env())
			return self._execute_env

	def output_env(self):
		try: return self._output_env
		except AttributeError:
			self._output_env = self.input_env().__class__(self.project())
			if self.auto_add(): self._output_env.attach(self.input_env())
			return self._output_env
	
	def add_dependency(self, check_):
		assert isinstance(check_, check)
		node.add_dependency(self, check_)

	def result(self):
		try: results = self.__class__._results
		except AttributeError:
			self.__class__._results = {}
			results = self.__class__._results
		try: return results[self.name()]
		except KeyError:
			result = node.result(self)
			results[self.name()] = result
			return result

	def targets(self): # used by the module class
		try: return self._targets
		except AttributeError:
			self._targets = []
			return self._targets
