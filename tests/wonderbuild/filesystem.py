#! /usr/bin/env python
# This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
# copyright 2008-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

import sys, os, os.path, stat, gc, cPickle

from logger import do_debug, debug

class FileSystem(object):
	def __init__(self, project):
		self.project = project
		self.cache_path = os.path.join(project.cache_dir, 'filesystem')
		self.load()

	def load(self):
		gc.disable()
		try:
			try:
				f = file(self.cache_path, 'rb')
				try: self.nodes = cPickle.load(f)
				finally: f.close()
			except Exception, e:
				print >> sys.stderr, 'could not load pickle:', e
				self.nodes = {}
		finally: gc.enable()

	def dump(self):
		gc.disable()
		try:
			f = file(self.cache_path, 'wb')
			try: cPickle.dump(self.nodes, f, cPickle.HIGHEST_PROTOCOL)
			finally: f.close()
		finally: gc.enable()
		
	def src_node(self, name):
		name = os.path.normpath(name)
		try: return self.nodes[name]
		except KeyError:
			n = Node(None, name)
			self.nodes[name] = n
			return n
			
	def built_node(self, name):
		name = os.path.join(self.project.build_dir, os.path.normpath(name))
		try: return self.nodes[name]
		except KeyError:
			n = Node(None, name)
			self.nodes[name] = n
			return n
	
	def display(self):
		print 'fs:'
		for n in self.nodes.itervalues(): n.display()

DIR = 1
FILE = 2

class Node(object):
	__slots__ = ('parent', 'name', '_kind', '_old_children', '_actual_children', 'old_time', '_actual_time', '_sig', '_abs_path')

	def __getstate__(self):
		return self.parent, self.name, self._kind, self._old_children, self.old_time, self._sig, self._abs_path

	def __setstate__(self, data):
		self.parent, self.name, self._kind, self._old_children, self.old_time, self._sig, self._abs_path = data
		self._actual_children = None
		self._actual_time = None

	def __init__(self, parent, name):
		self.parent = parent
		self.name = name
		self._kind = None
		self._old_children = None
		self._actual_children = None
		self.old_time = None
		self._actual_time = None
		self._sig = None
		self._abs_path = None
	
	def do_stat(self):
		global do_debug
		if do_debug: debug('os.stat   : ' + self.abs_path)
		
		# try-except is a tiny bit faster
		#st = os.lstat(self.abs_path)
		#if stat.S_ISLNK(st.st_mode): os.stat(self.abs_path)

		try: st = os.stat(self.abs_path)
		except OSError:
			# may be a broken symlink
			st = os.lstat(self.abs_path)

		if stat.S_ISDIR(st.st_mode): self._kind = DIR
		else: self._kind = FILE
		
		self._actual_time = st.st_mtime

	def kind(self):
		if self._kind is None: self.do_stat()
		return self._kind
	kind = property(kind)

	def is_dir(self): return self.kind == DIR
	is_dir = property(is_dir)

	def actual_time(self):
		if self._actual_time is None: self.do_stat()
		return self._actual_time
	actual_time = property(actual_time)

	def has_changed(self):
		if self.actual_time != self.old_time: return True
		if self.is_dir:
			for n in self.old_children:
				if n.has_changed(): return True
		return False
		
	def mark_read(self):
		self.old_time = self.actual_time
		if self.is_dir: self._old_children = self.actual_children

	def actual_children(self):
		if self._actual_children is None:
			if self.actual_time == self.old_time: self._actual_children = self.old_children
			else:
				global do_debug
				if do_debug: debug('os.listdir: ' + self.abs_path)
				children = {}
				for name in os.listdir(self.abs_path):
					if name in self.old_children: children[name] = self.old_children[name]
					else: children[name] = Node(self, name)
				self._actual_children = children
		return self._actual_children
	actual_children = property(actual_children)
		
	def old_children(self):	
		if self._old_children is None:
			global do_debug
			if do_debug: debug('os.listdir: ' + self.abs_path)
			children = {}
			for name in os.listdir(self.abs_path): children[name] = Node(self, name)
			self._old_children = children
		return self._old_children
	old_children = property(old_children)

	def sig(self):
		if self._sig is None:
			time = self.actual_time
			assert time is not None
			if self._kind == DIR:
				for n in self.actual_children.itervalues():
					sub_time = n.sig
					if sub_time > time: time = sub_time
			self._sig = time
			self.mark_read()
		return self._sig
	sig = property(sig)

	def sig_to_string(self): return str(self.sig)
	
	def find(self, path): return self._find(path)
	def _find(self, path, start = 0):
		sep = path.find(os.sep, start)
		if sep > start:
			name = path[start:sep]
			if name == os.pardir: return self.parent or self
			if name == os.curdir: return self
			if self.kind != DIR: return None
			child = self.actual_children.get(name, None)
			if child is None: return None
			if sep == len(path) - 1: return child
			if child.kind != DIR: return None
			return child._find(path, sep + 1)
		elif sep < 0:
			name = path[start:]
			if name == os.pardir: return self.parent or self
			if name == os.curdir: return self
			if self.kind != DIR: return None
			return self.actual_children.get(name, None)
		else: # sep == start, absolute path
			top = self
			while top.parent: top = top.parent
			if top.name != os.sep:
				print >> sys.stderr, 'creating root node due to:', path
				root = Node(None, os.sep)
				root._kind = DIR
				top.parent = root.find(os.path.dirname(os.getcwd()))
			else: root = top
			return root._find(path, 1)

	def abs_path(self):
		if self._abs_path is None:
			if not self.parent: abs_path = self.name
			else: abs_path = os.path.join(self.parent.abs_path, self.name)
			self._abs_path = abs_path
		return self._abs_path
	abs_path = property(abs_path)

	def rel_path(self): return self.abs_path
	rel_path = property(rel_path)

	def display(self, tabs = 0):
		if False: path = '  |' * tabs + '- ' + (self.parent and self.name  or self.abs_path)
		else: path = self.abs_path

		print \
			self.sig_to_string().rjust(12), \
			(self.old_time is None and '?' or str(self.old_time)).rjust(12), \
			(self._kind is None and '?' or {DIR: 'dir', FILE: 'file'}[self._kind]).rjust(4) + \
			' ' + path

		if self._actual_children is not None:
			tabs += 1
			for n in self._actual_children.itervalues(): n.display(tabs)
