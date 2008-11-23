#! /usr/bin/env python
# This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
# copyright 2008-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

import sys, os, os.path

import filesystem, cxx_include_scanner

if __name__ == '__main__':
	import gc, time
	
	t0 = time.time()
	fs = filesystem.FileSystem()
	print >> sys.stderr, 'load time:', time.time() - t0

	t0 = time.time()
	scanner = cxx_include_scanner.IncludeScanner(fs)
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
		#foo = src.find('foo/foo.cpp')
		foo = fs.declare(src.abs_path + '/foo/foo.cpp')
		main = fs.declare(src.abs_path + '/main/main.cpp')
		for s in foo, main:
			changed = s.changed()
			print 'translation unit:', s.abs_path, '(changed: ' + str(changed) + ')'
			if changed:
				for dep in scanner.scan_deps(s.abs_path):
					dep = fs.declare(dep)
					print '\tdep:', dep.abs_path


	scanner.display()
	fs.display()

	t0 = time.time()
	scanner.dump()
	print >> sys.stderr, 'dump time:', time.time() - t0

	t0 = time.time()
	fs.dump()
	print >> sys.stderr, 'dump time:', time.time() - t0

if False:
	class Project(object):
		def __init__(self):
			self.aliases = {}
			self.tasks = {}
			self.task_sigs = {}
			self.fs = filesystem.FileSystem()
			
		def add_aliases(self, task, aliases)
			if aliases is not None:
				for a in aliases:
					try: self.aliases[a].append(task)
					except KeyError: self.aliases[a] = [task]

		def build(self, tasks):
			s = Scheduler()
			s.start()
			for t in tasks: s.add_task(t)
			s.join()
	
	class Task(object):	
		def __init__(self, project, aliases = None):
			self.in_tasks = []
			self.out_tasks = []
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
				sig = Sig(self.target.path)
				return self._uid = sig.digest()

		def actual_sig(self):
			try: return self._actual_sig
			except AttributeError:
				sig = Sig(self.source.actual_sig)
				return self._actual_sig = sig.digest()
			
		def process(self):
			if self.old_sig() != self.actual_sig()
				self.exec_subprocess(['c++', '-o', self.target.path, '-c', self.source.path])
				self.update_sig()
				return True
			else:
				self.out_tasks = []
				return False
			
	class Lib(Task):
		def uid(self):
			try: return self._uid
			except AttributeError:
				sig = Sig(self.target.path)
				return self._uid = sig.digest()

		def actual_sig(self):
			try: return self._actual_sig
			except AttributeError:
				sig = Sig()
				for s in self.sources: sig.update(s.actual_sig)
				return self._actual_sig = sig.digest()

		def process(self):
			if self.old_sig() != self.actual_sig()
				self.exec_subprocess(['c++', '-o', self.target.path] + [s.path for s in self.sources])
				self.update_sig()
				return True
			else:
				self.out_tasks = []
				return False

	class LibFoo(Lib):
		def __init__(self, project):
			Lib.__init__(self, project, aliases = ['foo'])
			
		def dyn_in_tasks(self):
			if len(self.in_tasks): return None
			src = self.project.fs.src_node('src')
			foo = src.built_node('foo/foo.o')
			self.sources = [foo]
			obj = Obj(project)
			obj.target = foo
			self.in_tasks = [obj]
			for t in self.in_tasks:
				t.source = t.target.src_node_ext('.cpp')
				t.out_tasks = [self]
				
		def process(self):
			self.target = self.project.fs.built_node('libfoo.so')
			Lib.process(self)
			
	lib_foo = LibFoo(project)
	
