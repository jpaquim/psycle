#! /usr/bin/env python
# This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
# copyright 2008-2009 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

import sys, os

if __name__ == '__main__':
	dir = os.path.abspath(os.path.dirname(os.path.dirname(sys.argv[0]))) # __file__
	if dir not in sys.path:
		sys.path.append(dir)
		from wonderbuild.main import main
	main()
else:
	from wonderbuild.options import options, validate_options, known_options, help

	def main():
		import gc
		gc_enabled = gc.isenabled()
		if gc_enabled: gc.disable()
		try:
			known_options.add('--profile')
			help['--profile'] = ('--profile', 'profile execution')
		
			if '--profile' in options:
				import cProfile, pstats
				cProfile.run(
					'''from wonderbuild.options import known_options;'''
					'''known_options.add('--profile');'''
					'''from wonderbuild.main import run;'''
					'''sys.exit(run())'''
					, '/tmp/profile'
				)
				p = pstats.Stats('/tmp/profile')
				#p.sort_stats('time').print_stats(45)
				p.sort_stats('cumulative').reverse_order().print_stats()
			else: sys.exit(run())
		finally:
			if gc_enabled: gc.enable()

	def run():
		from wonderbuild.project import Project
		project = Project()

		script = project.src_node.node_path('wonderbuild_script.py')
		if script.exists:
			d = {}
			execfile('wonderbuild_script.py', d)
			tasks = d['wonderbuild_script'](project)
			usage = False
		else:
			print >> sys.stderr, 'no ' + script.path + ' found'
			usage = True

		help['--version'] = ('--version', 'show the version of this tool and exit')

		def print_help(out):
			help['--help'] = ('--help', 'show this help and exit')

			project.help()
			keys = []
			just = 0
			for k, v in help.iteritems():
				if len(v[0]) > just: just = len(v[0])
				keys.append(k)
			keys.sort()
			just += 1
			for h in keys:
				h = help[h]
				print h[0].ljust(just), h[1]
				if len(h) >= 3: print >> out, ''.ljust(just), '(default: ' + h[2] + ')'

		if '--help' in options:
			print_help(sys.stdout)
			return 0

		if '--version' in options:
			print 'wonderbuild 0.1'
			return 0
	
		project.options()
		usage = not validate_options()

		if usage:
			print_help(sys.stderr)
			return 1

		project.configure()
		try: project.build(tasks)
		finally: project.dump()
		return 0
