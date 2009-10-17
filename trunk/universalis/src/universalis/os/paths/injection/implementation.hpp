// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2008 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\implementation universalis::os::paths

namespace {
	boost::filesystem::path const & process_executable_file_path() {
		struct once {
			boost::filesystem::path const static path() throw(std::exception) {
				#if defined DIVERSALIS__OS__MICROSOFT
					char module_file_name[UNIVERSALIS__OS__MICROSOFT__MAX_PATH];
					if(!::GetModuleFileNameA(0, module_file_name, sizeof module_file_name))
						throw
							///\todo the following is making link error on mingw
							//universalis::os::exceptions::runtime_error(universalis::os::exceptions::code_description(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
							std::runtime_error("could not get filename of program module");
					return boost::filesystem::path(module_file_name, boost::filesystem::no_check); // boost::filesystem::native yells when there are spaces
				#else
					///\todo use binreloc instead
					return boost::filesystem::path(".") /
						#if defined UNIVERSALIS__META__MODULE__NAME
							UNIVERSALIS__META__MODULE__NAME;
						#else
							"unknown-module";
						#endif
				#endif
			}
		};
		boost::filesystem::path const static once(once::path());
		return once;
	}
}
			
boost::filesystem::path const & bin() {
	boost::filesystem::path const static once(process_executable_file_path().branch_path());
	return once;
}

boost::filesystem::path const & lib() {
	boost::filesystem::path const static once(
		bin()
		#if defined UNIVERSALIS__OS__PATH__BIN_TO_LIB
			/ UNIVERSALIS__OS__PATH__BIN_TO_LIB
		#endif
	);
	return once;
}

boost::filesystem::path const & share() {
	boost::filesystem::path const static once(
		bin() /
		#if defined UNIVERSALIS__OS__PATH__BIN_TO_SHARE
			UNIVERSALIS__OS__PATH__BIN_TO_SHARE
		#else
			"../share"
		#endif
	);
	return once;
}

boost::filesystem::path const & var() {
	boost::filesystem::path const static once(
		bin() /
		#if defined UNIVERSALIS__OS__PATH__BIN_TO_VAR
			UNIVERSALIS__OS__PATH__BIN_TO_VAR
		#else
			"../var"
		#endif
	);
	return once;
}

boost::filesystem::path const & etc() {
	boost::filesystem::path const static once(
		bin() /
		#if defined UNIVERSALIS__OS__PATH__BIN_TO_ETC
			UNIVERSALIS__OS__PATH__BIN_TO_ETC
		#else
			"../etc"
		#endif
	);
	return once;
}

boost::filesystem::path const & home() {
	struct once {
		boost::filesystem::path const static path() {
			char const env_var [] = {
				#if defined DIVERSALIS__OS__MICROSOFT
					"USERPROFILE"
				#else
					"HOME"
				#endif
			};
			char const * const path(std::getenv(env_var));
			if(!path) {
				std::ostringstream s; s << "The user has no defined home directory: the environment variable " << env_var << " is not set.";
						throw
							///\todo the following is making link error on mingw
							//universalis::os::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
							std::runtime_error(s.str());
			}
			return boost::filesystem::path(path, boost::filesystem::no_check); // boost::filesystem::native yells when there are spaces
		}
	};
	boost::filesystem::path const static once(once::path());
	return once;
}

namespace package {
	std::string const & name() {
		std::string const static once(
			#if defined UNIVERSALIS__META__PACKAGE__NAME
				UNIVERSALIS__META__PACKAGE__NAME
			#else
				"unknown-package"
			#endif
		);
		return once;
	}
	
	namespace version {
		std::string const & string() {
			std::string const static once(
				#if defined UNIVERSALIS__META__PACKAGE__VERSION
					UNIVERSALIS__COMPILER__STRINGIZED(UNIVERSALIS__META__PACKAGE__VERSION)
				#else
					"unknown-version"
				#endif
				" "
				#if defined DIVERSALIS__COMPILER__NAME
					DIVERSALIS__COMPILER__NAME
					#if defined DIVERSALIS__COMPILER__VERSION__STRING
						" " DIVERSALIS__COMPILER__VERSION__STRING
					#elif defined DIVERSALIS__COMPILER__VERSION
						" " UNIVERSALIS__COMPILER__STRINGIZED(DIVERSALIS__COMPILER__VERSION)
					#endif
				#else
					"unknown-compiler"
				#endif
				#if !defined NDEBUG
					" debug"
				#endif
			);
			return once;
		}

		unsigned int major_number() throw() {
			#if defined UNIVERSALIS__META__PACKAGE__VERSION__MAJOR
				return UNIVERSALIS__META__PACKAGE__VERSION__MAJOR;
			#else
				return 0;
			#endif
		}

		unsigned int minor_number() throw() {
			#if defined UNIVERSALIS__META__PACKAGE__VERSION__MINOR
				return UNIVERSALIS__META__PACKAGE__VERSION__MINOR;
			#else
				return 0;
			#endif
		}

		unsigned int patch_number() throw() {
			#if defined UNIVERSALIS__META__PACKAGE__VERSION__PATCH
				return UNIVERSALIS__META__PACKAGE__VERSION__PATCH;
			#else
				return 0;
			#endif
		}
	}
	
	boost::filesystem::path const & lib() {
		boost::filesystem::path const static once(
			#if defined DIVERSALIS__OS__MICROSOFT
				lib()
			#else
				paths::lib() / name()
			#endif
		);
		return once;
	}
	
	boost::filesystem::path const & share() {
		boost::filesystem::path const static once(paths::share() / name());
		return once;
	}
	
	boost::filesystem::path const & pixmaps() {
		boost::filesystem::path const static once(paths::share() / "pixmaps" / name());
		return once;
	}

	boost::filesystem::path const & doc() {
		boost::filesystem::path const static once(paths::share() / "doc" / name());
		return once;
	}

	boost::filesystem::path const & var() {
		boost::filesystem::path const static once(paths::var() / name());
		return once;
	}
	
	boost::filesystem::path const & log() {
		boost::filesystem::path const static once(paths::var() / "log" / name());
		return once;
	}
	
	boost::filesystem::path const & etc() {
		boost::filesystem::path const static once(paths::etc() / name());
		return once;
	}

	boost::filesystem::path const & home() {
		boost::filesystem::path const static once(paths::home() / boost::filesystem::path("." + name(), boost::filesystem::portable_posix_name));
		return once;
	}
}
