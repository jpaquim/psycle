#! /usr/bin/env python
# This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
# copyright 2009-2015 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>


##############################################################################
#
# This script simply builds all of the trunk
# (except qpsycle and psycle-mfc which are a bit special to build).
#
##############################################################################


if __name__ == '__main__':
	import sys, os
	dir = os.path.dirname(__file__)
	sys.argv.append('--src-dir=' + dir)
	try: from wonderbuild.main import main
	except ImportError:
		dir = os.path.abspath(os.path.join(dir, os.pardir, os.pardir, 'external-packages', 'wonderbuild'))
		if dir not in sys.path: sys.path.append(dir)
		try: from wonderbuild.main import main
		except ImportError:
			print >> sys.stderr, 'could not import wonderbuild module with path', sys.path
			sys.exit(1)
		else: main()
	else: main()

else:
	from wonderbuild.script import ScriptTask, ScriptLoaderTask

	class Wonderbuild(ScriptTask):
		def __call__(self, sched_ctx):
			tasks = [
				ScriptLoaderTask.shared(self.project, self.src_dir.parent.parent / dir) \
				for dir in ( # Note that there's no need to list dependencies (e.g. psycle-core, psycle-helpers etc) since those are pulled automatically
					'psycle-player',
					'psycle-plugins'
				)
			]
			for x in sched_ctx.parallel_wait(*tasks): yield x
			for t in tasks: self.default_tasks += t.script_task.default_tasks
