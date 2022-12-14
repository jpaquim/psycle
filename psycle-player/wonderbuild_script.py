#! /usr/bin/env python
# This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
# copyright 2009-2015 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

if __name__ == '__main__':
	import sys, os
	dir = os.path.dirname(__file__)
	sys.argv.append('--src-dir=' + dir)
	try: from wonderbuild.main import main
	except ImportError:
		dir = os.path.abspath(os.path.join(dir, os.pardir, 'external-packages', 'wonderbuild'))
		if dir not in sys.path: sys.path.append(dir)
		try: from wonderbuild.main import main
		except ImportError:
			print >> sys.stderr, 'could not import wonderbuild module with path', sys.path
			sys.exit(1)
		else: main()
	else: main()

from wonderbuild.script import ScriptTask, ScriptLoaderTask

class Wonderbuild(ScriptTask):
	@property
	def common(self): return self._common

	def __call__(self, sched_ctx):
		top_src_dir = self.src_dir.parent
		src_dir = self.src_dir / 'src'

		core = ScriptLoaderTask.shared(self.project, top_src_dir / 'psycle-core')
		for x in sched_ctx.parallel_wait(core): yield x
		core = core.script_task
		self._common = common = core.common
		core = core.mod_dep_phases
		pch = common.pch
		cfg = common.cfg.clone()

		from wonderbuild.cxx_tool_chain import PkgConfigCheckTask, ModTask
		from wonderbuild.install import InstallTask

		check_cfg = cfg.clone()
		xml = PkgConfigCheckTask.shared(check_cfg, ['libxml++-2.6'])

		class PlayerMod(ModTask):
			def __init__(self): ModTask.__init__(self, 'psycle-player', ModTask.Kinds.PROG, cfg)

			def do_set_deps(self, sched_ctx):
				self.private_deps = [pch.prog_task, core]
				req = self.all_deps
				opt = [xml]
				for x in sched_ctx.parallel_wait(*(req + opt)): yield x
				self.private_deps += [o for o in opt if o]

			def do_mod_phase(self):
				self.cfg.defines['UNIVERSALIS__META__MODULE__NAME'] = '"' + self.name +'"'
				self.cfg.defines['UNIVERSALIS__META__MODULE__VERSION'] = 0
				if xml: self.cfg.defines['PSYCLE__LIBXMLPP_AVAILABLE'] = None
				self.cfg.include_paths.appendleft(src_dir)
				for s in (src_dir / 'psycle' / 'player').find_iter(in_pats = ('*.cpp',), prune_pats = ('todo',)): self.sources.append(s)

		player = PlayerMod()
		self.default_tasks.append(player.mod_phase)
