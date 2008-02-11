/* -*- mode:c++, indent-tabs-mode:t -*- */
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\implementation universalis::operating_system::dynamic_link::resolver
#define UNIVERSALIS__QUAQUAVERSALIS // \todo [bohan] I'm setting this for now to prevent glibmm from being used because it seems it's not honouring $ORIGIN
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <universalis/detail/project.private.hpp>
#include "resolver.hpp"
#include <universalis/exception.hpp>
#include <universalis/operating_system/operating_system.hpp>
#include <universalis/operating_system/loggers.hpp>
#include <sstream>
#include <cassert>
#include <iostream>
#if defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__POSIX
	#include <dlfcn.h> // dlopen, dlsym, dlclose, dlerror
	#include <csignal>
#elif defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#include <windows.h>
	#include <universalis/operating_system/exceptions/code_description.hpp>
#else
	#include <glibmm/module.h>
#endif
namespace universalis
{
	namespace operating_system
	{
		namespace dynamic_link
		{
			void resolver::open_error(boost::filesystem::path const & path, std::string const & message)
			{
				throw exceptions::runtime_error("could not open library " + path.string() + ": " + message, UNIVERSALIS__COMPILER__LOCATION__NO_CLASS /* __NO_CLASS because it is a static member function */);
			}

			void resolver::close_error(std::string const & message) const
			{
				throw exceptions::runtime_error("could not close library "+ path().string() + ": " + message, UNIVERSALIS__COMPILER__LOCATION);
			}

			void resolver::resolve_symbol_error(std::string const & name, std::string const & message) const
			{
				throw exceptions::runtime_error("could not resolve symbol " + name + " (" + decorated_symbol(name) + ") in library " + path().string() + ": " + message, UNIVERSALIS__COMPILER__LOCATION);
			}

			boost::filesystem::path resolver::decorated_filename(boost::filesystem::path const & path, unsigned int const & version) throw()
			{
				std::ostringstream version_string; version_string << version;
				return
					#if 1 || \
					( \
						defined UNIVERSALIS__QUAQUAVERSALIS && \
						( \
							defined DIVERSALIS__OPERATING_SYSTEM__LINUX || \
							defined DIVERSALIS__OPERATING_SYSTEM__APPLE || \
							defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT || \
							defined DIVERSALIS__OPERATING_SYSTEM__CYGWIN \
						) \
					)
						//"lib" +
						path.branch_path() /
						(
							path.leaf() +
							#if defined DIVERSALIS__OPERATING_SYSTEM__LINUX
								".so" // "." + version_string.str() // lib-foo.so.0
							#elif defined DIVERSALIS__OPERATING_SYSTEM__APPLE
								// to create a bundle with libtool, we pass the -module option.
								//".bundle" // lib-foo.bundle
								// actually, libtool names bundles with the usual .so extension.
								".so" // lib-foo.so
							#elif defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT || defined DIVERSALIS__OPERATING_SYSTEM__CYGWIN
								".dll" // "-" + version_string.str() + ".dll" // lib-foo-0.dll
								// [bohan] it is only necessary to append the .dll suffix when the given name itself contains a dot,
								// [bohan] otherwise, we do not need to explicitly tell the suffix.
							#else
								#error bogus preprocessor conditions
							#endif
						);
					#else
						boost::filesystem::path
						(
							Glib::Module::build_path
							(
								path.branch_path().native_directory_string(),
								path.leaf()
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
										//+ "-" + version_string.str()
									#endif
							),
							boost::filesystem::no_check // boost::filesystem::native yells when there are spaces
						);
					#endif
			}

			resolver::resolver(boost::filesystem::path const & path, unsigned int const & version)
			:
				#if defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__POSIX
					path_(decorated_filename(path, version)),
				#endif
				underlying_(0)
			{
				#if defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__POSIX
					underlying_ = ::dlopen(path_.native_file_string().c_str(), RTLD_LAZY /*RTLD_NOW*/);
					if(!opened()) open_error(path_, std::string(::dlerror()));
				#elif defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
					// we use \ here instead of / because ::LoadLibraryEx will not use the LOAD_WITH_ALTERED_SEARCH_PATH option if it does not see a \ character in the file path:
					boost::filesystem::path const final_path(decorated_filename(path, version));
					underlying_ = ::LoadLibraryEx(final_path.native_file_string().c_str(), 0, LOAD_WITH_ALTERED_SEARCH_PATH);
					if(!opened()) open_error(final_path, exceptions::code_description());
				#else
					assert(Glib::Module::get_supported());
					boost::filesystem::path const final_path(decorated_filename(path, version));
					underlying_ = new Glib::Module(final_path.native_file_string(), Glib::MODULE_BIND_LAZY);
					if(!*underlying_)
					{
						delete underlying_;
						underlying_ = 0;
						open_error(final_path, Glib::Module::get_last_error());
					}
				#endif
			}

			bool resolver::opened() const throw()
			{
				return
					#if defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__POSIX
						//
					#elif defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
						//
					#else
						*
					#endif
					underlying_;
			}

			void resolver::close()
			{
				assert(opened());
				#if defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__POSIX
					if(::dlclose(underlying_)) close_error(std::string(::dlerror()));
				#elif defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
					if(!::FreeLibrary(underlying_)) close_error(exceptions::code_description());
				#else
					delete underlying_;
				#endif
				underlying_ = 0;
			}

			resolver::~resolver() throw()
			{
				if(opened())
				try
				{
					close();
				}
				catch(std::exception const & e)
				{
					if(loggers::exception())
					{
						std::ostringstream s;
						s << (e.what() ? e.what() : "no message");
						loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
				}
			}

			resolver::function_pointer resolver::resolve_symbol_untyped(std::string const & name) const
			{
				assert(opened());
				union result_union
				{
					function_pointer typed;
					#if defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
						::PROC
					#else
						void *
					#endif
						untyped;
				} result;
				#if defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__POSIX
					result.untyped = ::dlsym(underlying_, decorated_symbol(name).c_str());
					if(!result.untyped) resolve_symbol_error(name, std::string(::dlerror()));
				#elif defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
					result.untyped = ::GetProcAddress(underlying_, decorated_symbol(name).c_str());
					if(!result.untyped) resolve_symbol_error(name, exceptions::code_description());
				#else
					if(!underlying_->get_symbol(decorated_symbol(name), result.untyped)) resolve_symbol_error(name, Glib::Module::get_last_error());
				#endif
				return result.typed;
			}

			boost::filesystem::path resolver::path() const throw()
			{
				assert(opened());
				#if defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__POSIX
					return path_;
				#elif defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
					char module_file_name[UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__MAX_PATH];
					::GetModuleFileName(underlying_, module_file_name, sizeof module_file_name);
					return boost::filesystem::path(module_file_name, boost::filesystem::no_check); // boost::filesystem::native yells when there are spaces
				#else
					return boost::filesystem::path(underlying_->get_name(), boost::filesystem::no_check); // boost::filesystem::native yells when there are spaces
				#endif
			}

			std::string resolver::decorated_symbol(const std::string & name) throw()
			{
				// calling convention | modifier keyword | parameters stack push            | parameters stack pop | extern "C" symbol name mangling                   | extern "C++" symbol name mangling
				// register           | fastcall         | 3 registers then pushed on stack | callee               | '@' and arguments' byte size in decimal prepended | no standard
				// pascal             | pascal           | left to right                    | callee               | uppercase                                         | no standard
				// standard call      | stdcall          | right to left                    | callee               | preserved                                         | no standard
				// c declaration      | cdecl            | right to left, variable count    | caller               | '_' prepended                                     | no standard
				
				// note: the register convention is different from one compiler to another (for example left to right for borland, right to left for microsoft).
				// for borland's compatibility with microsoft's register calling convention: #define fastcall __msfastcall.
				// note: on the gnu compiler, one can use the #define __USER_LABEL_PREFIX__ to know what character is prepended to extern "C" symbols.
				// note: on microsoft's compiler, with cdecl, there's is no special decoration for extern "C" declarations, i.e., no '_' prepended.

				#if defined DIVERSALIS__COMPILER__FEATURE__NOT_CONCRETE
					return name;
				#elif defined DIVERSALIS__COMPILER__GNU
					return /*UNIVERSALIS__COMPILER__STRINGIZED(__USER_LABEL_PREFIX__) +*/ name;
				#elif defined DIVERSALIS__COMPILER__BORLAND
					return name;
				#elif defined DIVERSALIS__COMPILER__MICROSOFT
					return name;
				#else
					return name;
					//#error "Unsupported compiler."
				#endif
			}
		}
	}
}
