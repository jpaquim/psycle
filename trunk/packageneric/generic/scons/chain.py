# This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
# copyright 2006 johan boule <bohan@jabber.org>
# copyright 2006 psycledelics http://psycle.pastnotecut.org

_template = {}

def template(base):
	try: return _template[base]
	except KeyError:

		class result(base):
			class target_types:
				loadable = 0
				shared = 1
				static = 2
				program = 3
				
			def __init__(self, project,
				target_type = target_types.loadable,
				archiver = None,
				archiver_flags = None,
				archive_indexer = None,
				archive_indexer_flags = None,
				linker = None,
				library_paths = None,
				libraries = None,
				linker_flags = None,
				*args, **kw
			):
				base.__init__(*(self, project) + args, **kw)
				self._target_type = target_type
				if archiver is not None: self.archiver().command().set(archiver)
				if archiver_flags is not None: self.archiver().flags().add(archiver_flags)
				if archive_indexer is not None: self.archiver().indexer().command().set(archive_indexer)
				if archive_indexer_flags is not None: self.archiver().indexer().flags().add(archive_indexer_flags)
				if linker is not None: self.linker().command().set(linker)
				if library_paths is not None: self.linker().paths().add(library_paths)
				if libraries is not None: self.linker().libraries().add(libraries)
				if linker_flags is not None: self.linker().flags().add(linker_flags)

			def target_type(self): return self._target_type
			
			def compilers(self):
				try: return self._compilers
				except AttributeError:
					from compilers import compilers
					self._compilers = compilers()
					return self._compilers

			def archiver(self):
				try: return self._archiver
				except AttributeError:
					#assert self.target_type() == target_types.static
					import archiver
					self._archiver = archiver.template(base)(self.project())
					return self._archiver
					
			def linker(self):
				try: return self._linker
				except AttributeError:
					#assert self.target_type() != target_types.static
					import linker
					self._linker = linker.template(base)(self.project())
					return self._linker

			def attach(self, source):
				base.attach(self, source)
				if isinstance(source, result):
					for self_compiler in self.compilers():
						for source_compiler in source.compilers(): self_compiler.attach(source_compiler)
					self.archiver().attach(source.archiver())
					self.linker().attach(source.linker())
					
			def _scons(self, scons):
				base._scons(self, scons)
				for compiler in self.compilers(): compiler._scons(scons)
				self.archiver()._scons(scons)
				self.linker()._scons(scons)

			def parse(self, command):
				self._scons_environment().ParseConfig(command)
				return True # scons doesn't return anything (fixed in a recent version)
				
		_template[base] = result
		return result
