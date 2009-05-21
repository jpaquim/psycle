#! /usr/bin/env python
# This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
# copyright 2006-2009 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

import sys

from wonderbuild.cxx_tool_chain import MultiBuildCheckTask, BuildCheckTask
from wonderbuild.signature import Sig
from wonderbuild.logger import silent, is_debug, debug

from wonderbuild.std_checks import AutoLinkSupportCheckTask

class BoostCheckTask(MultiBuildCheckTask):
	def __init__(self, min_version_wanted_raw, libraries, base_cfg):
		MultiBuildCheckTask.__init__(self, 'boost' + ' ' + ' '.join(libraries), base_cfg)
		self._min_version_wanted_raw = min_version_wanted_raw
		self._min_version_wanted_major = str(min_version_wanted_raw // 100000)
		self._min_version_wanted_minor = str(min_version_wanted_raw // 100 % 1000)
		self._min_version_wanted_patch = str(min_version_wanted_raw % 100)
		self._min_version_wanted = self._min_version_wanted_major + '.' + self._min_version_wanted_minor + '.' + self._min_version_wanted_patch
		self._libraries = libraries

	@property
	def result(self): return self.results[0]

	@property
	def include_path(self): return self.results[1]

	@property
	def lib_path(self): return self.results[2]
	
	@property
	def libs(self): return self.results[3]

	def apply_to(self, cfg):
		if self.include_path is not None: cfg.include_paths.append(self.include_path)
		if self.lib_path is not None: cfg.lib_path.append(self.lib_path)
		cfg.libs.extend(self.libs)

	@property
	def source_text(self): return '' # TODO see property in AllInOneCheckTask class

	class ReadVersion(BuildCheckTask):
			def __init__(self, outer):
				BuildCheckTask.__init__(self, 'boost-version-' + str(outer._min_version_wanted_raw), outer.base_cfg, pipe_preproc=True)
				self._outer = outer
			
			@property
			def source_text(self):
				try: return self._source_text
				except AttributeError:
					self._source_text = \
						'#include <boost/version.hpp>\n' \
						'BOOST_VERSION\n' \
						'BOOST_LIB_VERSION\n'
					return self._source_text
			
			def do_check_and_set_result(self, sched_ctx):
				BuildCheckTask.do_check_and_set_result(self, sched_ctx)
				r, out = self.results
				if not r: self.results = False, None, None
				else:
					out = out.split()[-2:]
					int_version = int(out[0])
					ok = int_version >= self._outer._min_version_wanted_raw
					self.results = ok, int_version, out[1].strip('"')
			
			@property
			def result(self): return self.results[0]
			
			@property
			def int_version(self): return self.results[1]
			
			@property
			def lib_version(self): return self.results[2]
			
			@property
			def result_display(self):
				if self.result: return 'yes (version ' + str(self.int_version) + ' ' + self.lib_version + ')', '32'
				else: return 'no', '31'

	def do_check_and_set_result(self, sched_ctx):
		read_version = BoostCheckTask.ReadVersion(self)
		sched_ctx.parallel_wait(read_version)
		
		source_texts = {}
		source_texts['version'] = \
			"""
				#include <boost/version.hpp>
				#if BOOST_VERSION < %i
					#error
				#endif
			""" % self._min_version_wanted_raw
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

		include_path = None
		libraries = self._libraries[:]

		if sys.platform == 'cygwin':
			# damn cygwin installs boost headers in e.g. /usr/include/boost-1_33_1/ and doesn't give symlinks for library files
			dir = self.project.fs.root / 'usr' / 'include'
			if dir.exists and dir.is_dir:
				boost_dir = dir / 'boost'
				if not boost_dir.exists or not boost_dir.is_dir:
					for entry in dir.actual_children.itervalues():
						if entry.name.startswith('boost-'):
							if __debug__ and is_debug: debug('cfg: found boost headers in ' + str(entry))
							# TODO better version comparion
							if entry.name >= 'boost-' + self._min_version_wanted_major + '_' + self._min_version_wanted_minor + '_' + self._min_version_wanted_patch:
								if __debug__ and is_debug: debug('cfg: selecting boost headers in ' + str(entry))
								include_path = entry
								break
			dir = self.project.fs.root / 'usr' / 'lib'
			if dir.exists and dir.is_dir:
				children = dir.actual_children
				libraries = self._libraries[:]
				for library in self._libraries:
					library_select = library + '-gcc-mt' # TODO the version could even differ from the headers we selected above
					library_search = 'libboost_' + library_select + '.a'
					if library_search in children:
						if __debug__ and is_debug: debug('cfg: selecting boost library ' + library_select + ' as name ' + library + ' (found ' + str(library_search) + ')')
						source_texts[library_select] = source_texts[library]
						libraries.remove(library)
						libraries.append(library_select)
		else:
			for library in self._libraries:
				library_select = library + '-mt'
				source_texts[library_select] = source_texts[library]
				libraries.remove(library)
				libraries.append(library_select)
		
		outer = self

		auto_link_support_check_task = AutoLinkSupportCheckTask(outer.base_cfg)
		sched_ctx.parallel_wait(auto_link_support_check_task)
		if auto_link_support_check_task.result: link_libraries = []
		else: link_libraries = libraries

		lib_path = None

		cfg_link_libraries = ['boost_' + library for library in link_libraries]

		class AllInOneCheckTask(BuildCheckTask):
			def __init__(self): BuildCheckTask.__init__(self, 'boost,' + ','.join(link_libraries) + ',' + outer._min_version_wanted, outer.base_cfg)
			
			@property
			def source_text(self):
				try: return self._source_text
				except AttributeError:
					self._source_text = source_texts['version'] + '\n' + '\n'.join([source_texts[library] for library in link_libraries])
					return self._source_text
			
			def do_check_and_set_result(self, sched_ctx):
				if include_path is not None: self.cfg.include_paths.append(include_path)
				self.cfg.libs += cfg_link_libraries
				BuildCheckTask.do_check_and_set_result(self, sched_ctx)

		all_in_one = AllInOneCheckTask()
		sched_ctx.parallel_wait(all_in_one)
		if all_in_one.result:
			self.results = True, include_path, lib_path, cfg_link_libraries
		elif True: # posix?
			link_libraries = self._libraries
			cfg_link_libraries = ['boost_' + library for library in link_libraries]
			all_in_one = AllInOneCheckTask()
			sched_ctx.parallel_wait(all_in_one)
			if all_in_one.result:
				self.results = True, include_path, lib_path, cfg_link_libraries
			else: self.results = False, None, None, None
		else: self.results = False, None, None, None
