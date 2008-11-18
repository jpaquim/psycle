#! /usr/bin/env python
# This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
# copyright 2008-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

import sys, os, os.path

import filesystem, cxx_preprocessor

if __name__ == '__main__':
	import gc, time
	
	t0 = time.time()
	fs = filesystem.FileSystem()
	print >> sys.stderr, 'load time:', time.time() - t0

	t0 = time.time()
	scanner = cxx_preprocessor.DepScanner(fs)
	print >> sys.stderr, 'load time:', time.time() - t0

	#top = fs.declare(os.path.join(os.path.dirname(__file__), 'laika'))
	top = fs.declare(os.path.join(os.path.dirname(__file__), 'waf-test'))

	src_dirs = set(['src'])

	# get the dirs generated by genbench
	for d in top.actual_children:
		if d.startswith('src-'): src_dirs.add(d)

	for src in src_dirs:
		src = top.find(src)
		scanner.paths.add(src.abs_path)
		#foo = src.find('foo/foo.cpp', monitor = True)
		foo = fs.declare(src.abs_path + '/foo/foo.cpp', monitor = True)
		main = fs.declare(src.abs_path + '/main/main.cpp', monitor = True)
		for s in foo, main:
			changed = s.changed()
			print 'translation unit:', s.abs_path, '(changed: ' + str(changed) + ')'
			if changed:
				for dep in scanner.scan_deps(s.abs_path):
					dep = fs.declare(dep, monitor = True)
					print '\tdep:', dep.abs_path


	if False:
		def find_sources_iter(dir):
			dir = fs.node(dir)
			sources = []
			yield _find_sources(d)
		def _find_sources_iter(dir):
			for n in dir.actual_children():
				if n.name.endswith('.cpp'): yield n
				elif n.is_dir:
					for sub_dir in n.children:
						yield _find_sources_iter(n)

		modules = []
		for m in modules:
			srcs = m.find_sources()
			for s in srcs:
				changed = s.changed()
				if not changed:
					deps = s.deps
					for d in deps:
						changed = d.changed()
						if changed: break
				if changed:
					s.deps = s.scanner.scan_deps(s)
					m.objs[s].compile()
			for o in m.objs:
				if o.changed:
					m.link()
					break;
	
	scanner.display()
	fs.display()

	t0 = time.time()
	scanner.dump()
	print >> sys.stderr, 'dump time:', time.time() - t0

	t0 = time.time()
	fs.dump()
	print >> sys.stderr, 'dump time:', time.time() - t0
