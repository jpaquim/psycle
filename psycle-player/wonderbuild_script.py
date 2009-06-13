#! /usr/bin/env python
# This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
# copyright 2009-2009 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

if __name__ == '__main__':
	try: from wonderbuild.main import main
	except ImportError:
		import sys, os
		dir = os.path.abspath(os.path.join(os.path.dirname(__file__), os.pardir, 'build-systems'))
		if dir not in sys.path: sys.path.append(dir)
		try: from wonderbuild.main import main
		except ImportError:
			print >> sys.stderr, 'could not find wonderbuild'
			sys.exit(1)
		else: main()
	else: main()

from wonderbuild.script import ScriptTask, ScriptLoaderTask

class Wonderbuild(ScriptTask):

	def __call__(self, sched_ctx):
		project = self.project
		top_src_dir = self.src_dir.parent
		src_dir = self.src_dir / 'src'

		core = ScriptLoaderTask.shared(project, src_dir.parent.parent / 'psycle-core')
		pch = ScriptLoaderTask.shared(project, src_dir.parent.parent / 'universalis')
		for x in sched_ctx.parallel_wait(core, pch): yield x
		core = core.script_task.mod_dep_phases
		pch = pch.script_task.pch

		from wonderbuild.cxx_tool_chain import UserBuildCfg, PkgConfigCheckTask, ModTask
		from wonderbuild.install import InstallTask

		xml = PkgConfigCheckTask.shared(project, ['libxml++-2.6'])

		cfg = UserBuildCfg.new_or_clone(project)
		for x in sched_ctx.parallel_wait(cfg): yield x
		
		if cfg.kind == 'msvc': # XXX flags are a mess with msvc
			#cfg.defines['WINVER'] = '0x501' # select win xp explicitly because msvc 2008 defaults to vista
			cfg.defines['BOOST_ALL_DYN_LINK'] = None # choose to link against boost dlls
			cfg.cxx_flags += ['-EHa', '-MD'] # basic compilation flags required

		class PlayerMod(ModTask):
			def __init__(self): ModTask.__init__(self, 'psycle-player', ModTask.Kinds.PROG, cfg, 'psycle-player', 'default')

			def __call__(self, sched_ctx):
				self.private_deps = [pch.prog_task]
				self.public_deps = [core]
				req = self.all_deps
				opt = [xml]
				for x in sched_ctx.parallel_wait(*(req + opt)): yield x				
				self.result = min(bool(r) for r in req)
				self.public_deps += [o for o in opt if o]
				for x in ModTask.__call__(self, sched_ctx): yield x

			def apply_cxx_to(self, cfg):
				if xml: cfg.defines['PSYCLE__LIBXMLPP_AVAILABLE'] = None
				ModTask.apply_cxx_to(self, cfg)
			
			def do_mod_phase(self):
				self.cfg.include_paths.appendleft(src_dir)
				for s in (src_dir / 'psycle' / 'player').find_iter(
					in_pats = ('*.cpp',), prune_pats = ('todo',)
				): self.sources.append(s)

		player = PlayerMod()
