#! /usr/bin/env python
# This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
# copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

from cxx_chain import BuildCheckTask
from signature import Sig
from logger import silent, is_debug, debug

class StdMathCheckTask(BuildCheckTask):
	def __init__(self, base_cfg): BuildCheckTask.__init__(self, 'c++-std-math', base_cfg)

	def apply_to(self, cfg):
		self.result
		if self.m: cfg.libs.append('m')

	class SubCheckTask(BuildCheckTask):
		def __init__(self, name, base_cfg, m):
			BuildCheckTask.__init__(self, name + '-with' + (not m and 'out' or '') + '-libm', base_cfg)
			self.m = m

		def apply_to(self, cfg):
			if self.m: cfg.libs.append('m')

		@property
		def source_text(self): return '#include <cmath>\nfloat math() { float const f(std::sin(1.f)); return f; }'
		
	def _make_t0(self): return StdMathCheckTask.SubCheckTask(self.name, self.base_cfg, False)
	def _make_t1(self): return StdMathCheckTask.SubCheckTask(self.name, self.base_cfg, True)

	def __call__(self, sched_ctx):
		changed = False
		try: old_sig, self._result, self.m = self.project.persistent[self.uid]
		except KeyError: changed = True
		else:
			if old_sig != self.sig: changed = True
		if not changed:
			if __debug__ and is_debug: debug('task: skip: no change: ' + self.name)
		else:
			if not silent:
				desc = 'checking for ' + self.name
				self.print_check(desc)
			self._t0 = self._make_t0()
			self._t0(sched_ctx)
			if self._t0.result: self._t1 = self._t0
			else:
				self._t1 = self._make_t1()
				self._t1(sched_ctx)
			if not silent:
				if self.result: self.print_check_result(desc, 'yes with' + (not self.m and 'out' or '') + ' libm', '32')
				else: self.print_check_result(desc, 'no', '31')
			self.project.persistent[self.uid] = self.sig, self.result, self.m
		
	@property
	def result(self):
		try: return self._result
		except AttributeError:
			self._result = self._t0.result or self._t1.result
			if self._t0.result: self.m = False
			elif self._t1.result: self.m = True
			else: self.m = None
			return self._result

	@property
	def sig(self):
		try: return self._sig
		except AttributeError:
			sig = Sig()
			sig.update(self.base_cfg.cxx_sig)
			sig.update(self.base_cfg.ld_sig)
			sig = self._sig = sig.digest()
			return sig

class ThreadSupportCheckTask(BuildCheckTask):
	def __init__(self, base_cfg): BuildCheckTask.__init__(self, 'thread-support', base_cfg)

	def apply_to(self, cfg):
		pass # TODO

	def __call__(self, sched_ctx):
		pass # TODO

class DlfcnCheckTask(BuildCheckTask):
	def __init__(self, base_cfg): BuildCheckTask.__init__(self, 'dlfcn', base_cfg)

	def apply_to(self, cfg):
		pass # TODO

	def __call__(self, sched_ctx):
		pass # TODO

class BoostCheckTask(BuildCheckTask):
	def __init__(self, version_wanted_raw, libraries, base_cfg):
	 	BuildCheckTask.__init__(self, 'boost' + ' '.join(libraries), base_cfg)
		self._version_wanted_raw = version_wanted_raw
		self._version_wanted_major = str(version_wanted_raw / 100000)
		self._version_wanted_minor = str(version_wanted_raw / 100 % 1000)
		self._version_wanted_patch = str(version_wanted_raw % 100)
		self._version_wanted = self._version_wanted_major + '.' + self._version_wanted_minor + '.' + self._version_wanted_patch
		self._libraries = libraries

	def apply_to(self, cfg):
		cfg.includes.append(self.include_path)
		cfg.lib_path.append(self.lib_path)
		cfg.libs.append(self.libs)

	def __call__(self, sched_ctx):
		source_texts = {}
		source_texts['version'] = \
			"""
				#include <boost/version.hpp>
				#if BOOST_VERSION < %i
					#error
				#endif
			""" % self._version_wanted_raw
		def auto_link(name, dynamic = True):
			result = \
				"""
					#define BOOST_LIB_NAME boost_%s
				""" % name
			if dynamic: result += \
				"""
					#if defined _DLL || defined _RTLDLL // as tested in <boost/config/auto_link.hpp>
						#define BOOST_DYN_LINK
					#endif
				"""
			result += \
				"""
					#define BOOST_LIB_DIAGNOSTIC
					#include <boost/config/auto_link.hpp>
				"""
			return result
		source_texts['signals'] = \
			"""
				#include <boost/signals/slot.hpp>
				void signals() { /* todo do something with it for a complete check */ }
			""" + auto_link('signals')
		source_texts['thread'] = \
			"""
				#include <boost/thread/thread.hpp>
				void thread() { boost::thread thread; }
			""" + auto_link('thread')
		source_texts['filesystem'] = \
			"""
				#include <boost/filesystem/path.hpp>
				#include <boost/filesystem/operations.hpp>
				void filesystem() { boost::filesystem::path path(boost::filesystem::current_path()); }
			""" + auto_link('filesystem')
		source_texts['serialization'] = \
			"""
				#include <boost/serialization/serialization.hpp>
				#include <boost/archive/basic_archive.hpp>
				void serialization() { /* todo do something with it for a complete check */ }
			""" + auto_link('serialization')
		source_texts['wserialization'] = \
			"""
				#include <boost/serialization/serialization.hpp>
				#include <boost/archive/basic_archive.hpp>
				void wserialization() { /* todo do something with it for a complete check */ }
			""" + auto_link('wserialization')
		source_texts['iostream'] = \
			"""
				//#include <boost/...>
				void iostream() { /* todo do something with it for a complete check */ }
			""" + auto_link('iostream')
		source_texts['regex'] = \
			"""
				#include <boost/regex.hpp>
				void regex() { /* todo do something with it for a complete check */ }
			""" + auto_link('regex')
		source_texts['program_options'] = \
			"""
				#include <boost/program_options.hpp>
				void program_options() { /* todo do something with it for a complete check */ }
			""" + auto_link('program_options')
		source_texts['python'] = \
			"""
				#include <boost/python.hpp>
				void python() { /* todo do something with it for a complete check */ }
			""" + auto_link('python')
		source_texts['date_time'] = \
			"""
				#include <boost/date_time/period.hpp>
				void date_time() { /* todo do something with it for a complete check */ }
			""" + auto_link('date_time')
		source_texts['unit_test_framework'] = \
			"""
				#include <boost/test/framework.hpp>
				void unit_test_framework() { /* todo do something with it for a complete check */ }
			""" + auto_link('unit_test_framework', dynamic = False)
		source_texts['prg_exec_monitor'] = \
			"""
				#include <boost/test/execution_monitor.hpp>
				void prg_exec_monitor() { /* todo do something with it for a complete check */ }
			""" + auto_link('prg_exec_monitor', dynamic = False)
		source_texts['test_exec_monitor'] = \
			"""
				#include <boost/test/execution_monitor.hpp>
				void test_exec_monitor() { /* todo do something with it for a complete check */ }
			""" + auto_link('test_exec_monitor', dynamic = False)
		source_texts['wave'] = \
			"""
				#include <boost/wave/wave_version.hpp>
				void wave() { /* todo do something with it for a complete check */ }
			""" + auto_link('wave', dynamic = False)

		cxx_compiler_paths = []
		libraries = self._libraries[:]

		if self.project.platform == 'posix':
			for library in self._libraries:
				library_select = library + '-mt'
				source_texts[library_select] = source_texts[library]
				libraries.remove(library)
				libraries.append(library_select)
		elif self.project.platform == 'cygwin': # todo and no -mno-cygwin passed to the compiler
			# damn cygwin installs boost headers in e.g. /usr/include/boost-1_33_1/ and doesn't give symlinks for library files
			import os
			dir = self.project.fs.root / 'usr' / 'include'
			if dir.is_dir:
				boost_dir = dir / 'boost'
				if not boost_dir.is_dir:
					for entry in dir.actual_children:
						if entry.startswith('boost-'):
							path = dir / entry
							if __debug__ and is_debug: debug('cfg: found boost headers in ' + str(path))
							# TODO better version comparion
							if entry >= 'boost-' + self._version_wanted_major + '_' + self._version_wanted_minor + '_' + self._version_wanted_patch:
								if __debug__ and is_debug: debug('cfg: selecting boost headers in ' + str(path))
								include_path = path
								break
			dir = self.project.fs.root / 'usr' / 'lib'
			if dir.is_dir:
				children = dir.actual_children
				libraries = self._libraries[:]
				for library in self._libraries:
					library_select = library + '-gcc-mt-s' # in e.g. cygwin, always static :-(. todo the version could even differ from the headers we selected above
					library_search = 'libboost_' + library_select + '.a' # in e.g. cygwin, always static :-( but if there's also a .dll.a, the linker will still pick it :)
					if library_search in children:
						if __debug__ and is_debug: debug('cfg: selecting boost library ' + library_select + ' as name ' + library + ' (found ' + os.path.join(dir, library_search) + ')')
						source_texts[library_select] = source_texts[library]
						libraries.remove(library)
						libraries.append(library_select)

		link_libraries = libraries
		
	 	class AutoLinkSupportCheckTask(BuildCheckTask):
	 		def __init__(self): BuildCheckTask.__init__(self, 'auto-link', base_cfg)
	 		
	 		@property
	 		def source_text(self):
	 			try: return self._source_text
	 			except AttributeError:
	 				self._source_text = \
						"""
							// text below copied from <boost/config/auto_link.hpp>
							#include <boost/config.hpp>
							#if \\
								!( \\
									defined(BOOST_MSVC) || \\
									defined(__BORLANDC__) || \\
									(defined(__MWERKS__) && defined(_WIN32) && (__MWERKS__ >= 0x3000)) || \\
									(defined(__ICL) && defined(_MSC_EXTENSIONS) && (_MSC_VER >= 1200)) \\
								)
								#error no auto link
							#endif
						"""
						
		auto_link_support_check_taks = AutoLinkSupportCheckTask()
		sched_ctx.parallel_wait(auto_link_support_check_taks)
		if auto_link_support_check_taks.result: link_libraries = []

		cfg_link_libraries = ['boost_' + library for library in link_libraries]

		class AllInOneCheckTask(BuildCheckTask):
			def __init__(self): BuildCheckTask.__init__(self, 'boost ' + ' '.join(link_libraries) + ' >= ' + self._version_wanted, base_cfg)
			
	 		@property
	 		def source_text(self):
	 			try: return self._source_text
	 			except AttributeError:
	 				self._source_text = source_texts['version'] + '\n' + '\n'.join([source_texts[library] for library in link_libraries])
	 		
	 		def __call__(self, sched_ctx):
	 			cfg.include_path.append(include_path)
	 			cfg.libs += cfg_link_libraries
	 			BuildCheckTask.__call__(self, sched_ctx)			

		all_in_one = AllInOneCheckTask()
		sched_ctx.parallel_wait(all_in_one)
		if all_in_one.result:
			self.result = True
			self.include_path = include_path
			self.libs = cfg_link_libraries
		elif self.project.platform == 'posix':
			link_libraries = self._libraries
			cfg_link_libraries = ['boost_' + library for library in link_libraries]
			all_in_one = AllInOneCheckTask()
			sched_ctx.parallel_wait(all_in_one)
			if all_in_one.result:
				self.result = True
				self.include_path = include_path
				self.libs = cfg_link_libraries
			else: self.result = False
		else: self.result = False
