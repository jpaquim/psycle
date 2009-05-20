#! /usr/bin/env python
# This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
# copyright 2008-2009 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

import sys, os, re

from wonderbuild.logger import is_debug, debug

_line_continuations = re.compile(r'\\\r*\n', re.MULTILINE)
_cpp = re.compile(r'''(/\*[^*]*\*+([^/*][^*]*\*+)*/)|//[^\n]*|("(\\.|[^"\\])*"|'(\\.|[^'\\])*'|.[^/"'\\]*)''', re.MULTILINE)
_include = re.compile(r'^[ \t]*#[ \t]*include[ \t]*(["<])([^">]*)[">].*$', re.MULTILINE)

need_sep_fix = os.sep != '/'

class IncludeScanner(object):
	'C/C++ dependency scanner. #include statements, and nothing else, no #if, no #define (dumb)'
	
	def __init__(self, persistent):
		self.contents = {} # {node: (rel_includes, abs_includes)}
		self.not_found = set() # of nodes collected from #include "" but not from #include <>
		if False and __debug__ and is_debug: self.display()
	
	def scan_deps(self, source, paths):
		not_found = set() # contains either nodes (for #include "") or paths (for #include <>)
		seen = self._scan_deps(source, paths, not_found)
		return seen, not_found
	def _scan_deps(self, source, paths, not_found, seen = None):
		if __debug__ and is_debug: debug('cpp: dep       : ' + source.path)

		if seen is None: seen = set()
		else:
			if source in seen: return seen
			seen.add(source)

		try: rel_includes, abs_includes = self.contents[source]
		except KeyError:
			if __debug__ and is_debug: debug('cpp: parsing   : ' + str(source))
			try: f = open(source.path, 'rb')
			except IOError:
				if __debug__ and is_debug: debug('cpp: not found : ' + str(source))
				self.not_found.add(source)
				return seen
			try: s = f.read()
			finally: f.close()
			rel_includes, abs_includes = self.parse_string(s)
			self.contents[source] = rel_includes, abs_includes

		if len(rel_includes) != 0:
			dir = source.parent
			for include in rel_includes:
				sub_source = self.search_rel(dir, include, paths, not_found)
				if sub_source is not None: self._scan_deps(sub_source, paths, not_found, seen)
		if len(abs_includes) != 0:
			for include in abs_includes:
				sub_source = self.search_abs(include, paths, not_found)
				if sub_source is not None: self._scan_deps(sub_source, paths, not_found, seen)
		return seen
	
	def search_rel(self, dir, include, paths, not_found):
		if need_sep_fix: include = include.replace('/', os.sep)
		n = dir / include
		if n in self.not_found:
			not_found.add(n)
			return None
		if n.parent.exists:
			n.parent.actual_children # not needed, just an optimisation
			if n.exists: return n
		if os.path.isabs(include):
			if __debug__ and is_debug: debug('cpp: not found : #include "' + include + '"')
		else:
			if not include.startswith(os.pardir + os.sep) and not include.startswith(os.curdir + os.sep):
				abs = self.search_abs(include, paths, not_found)
				if abs is not None: return abs
			if __debug__ and is_debug: debug('cpp: not found : #include "' + include + '"')
		self.not_found.add(n)
		not_found.add(n)
		return None
		
	def search_abs(self, include, paths, not_found):
		if need_sep_fix: include = include.replace('/', os.sep)
		if include in not_found: return None
		for dir in paths:
			n = dir / include
			if n.parent.exists:
				n.parent.actual_children # not needed, just an optimisation
				if n.exists: return n
		if __debug__ and is_debug: debug('cpp: not found : #include <' + include + '>')
		not_found.add(include)
		return None

	def parse_string(self, s): return self._parse_string_fast(s)

	def _parse_string_fast(self, s):
		s = _line_continuations.sub('', s)

		def repl(m):
			s = m.group(1)
			if s is not None: return ' '
			s = m.group(3)
			if s is None: return ''
			return s
		s = _cpp.sub(repl, s)

		rel_includes = set()
		abs_includes = set()
		for m in _include.finditer(s):
			kind = m.group(1)
			if   kind == '"': rel_includes.add(m.group(2))
			elif kind == '<': abs_includes.add(m.group(2))
		return rel_includes, abs_includes
	
	def _parse_string_slow(self, s):
		normal = 0
		single_line_comment = 1
		multi_line_comment = 2
		single_quoted_string = 3
		double_quoted_string = 4
		token = 5
		token_quote = 6
		other = 7
	
		prev_state = state = normal; prev = '\0'
	
		rel_includes = set()
		abs_includes = set()
		for c in s:
			if c == '\r': continue
			if prev == '\\' and c == '\n': continue
			if c == '\t': c = ' '

			new_state = state

			if   state == single_line_comment:
				if c == '\n': new_state = normal
			elif state == multi_line_comment:
				if prev == '*' and c == '/': new_state = prev_state
			elif state == single_quoted_string:
				if prev != '\\' and c == "'": new_state = prev_state
				elif c == '\n': new_state = normal
			elif state == double_quoted_string:
				if prev != '\\' and c == '"': new_state = prev_state
				elif c == '\n': new_state = normal
			elif state == normal or state == other:
				if   prev == '/' and c == '/': new_state = single_line_comment
				elif prev == '/' and c == '*': new_state =  multi_line_comment
				elif c == "'": new_state = single_quoted_string
				elif c == '"': new_state = double_quoted_string
				elif state == normal:
					if c == '#' and prev != '/':
						new_state = token
						token_string = c
						token_quote = False
					elif c != ' ' and c != '\n' and c != '/': new_state = other
				elif state == other:
					if c == '\n': new_state = normal
			elif state == token:
				token_end = False
				if c == '\\':
					if token_quote: token_string += c
				elif prev == '/' and c == '/':
					new_state = single_line_comment
					token_end = True
				elif prev == '/' and c == '*': new_state = multi_line_comment
				elif c == '\n': token_end = True
				elif c == '"':
					token_quote = not token_quote
					token_string += c
				elif c == '<':
					token_quote = True
					token_string += c
				elif c == '>':
					token_quote = False
					token_string += c
				elif c == '/':
					if token_quote or prev_state == multi_line_comment: token_string += '/'
				elif c != ' ' or (token_string[-1] != ' ' and token_string[-1] != '#'): token_string += c

				if token_end:
					search = '#include '
					if token_string.startswith(search):
						l = len(search)
						kind = token_string[l]
						if   kind == '"': rel_includes.add(token_string[l + 1 : token_string.rfind('"')])
						elif kind == '<': abs_includes.add(token_string[l + 1 : token_string.rfind('>')])
					new_state = normal

			if new_state != state:
				prev_state = state
				state = new_state
			prev = c;
		return rel_includes, abs_includes
		
	def display(self):
		print 'cpp:'
		for source, includes in self.contents.iteritems(): print str(source), includes
		print 'not found:', self.not_found
