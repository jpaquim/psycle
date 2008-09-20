// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2008 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\implementation universalis::operating_system::paths

namespace {
	boost::filesystem::path const & process_executable_file_path() {
		class once {
			public:
				boost::filesystem::path const static path() throw(std::exception) {
					#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
						char module_file_name[UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__MAX_PATH];
						if(!::GetModuleFileName(0, module_file_name, sizeof module_file_name)) throw universalis::operating_system::exceptions::runtime_error(universalis::operating_system::exceptions::code_description(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
						return boost::filesystem::path(module_file_name, boost::filesystem::no_check); // boost::filesystem::native yells when there are spaces
					#else
						///\todo use main's argument #0 instead <- not! ... use binreloc instead
						///\todo and distinguish symlinks
						return boost::filesystem::path(".") / PACKAGENERIC__MODULE__NAME;
					#endif
				}
		};
		boost::filesystem::path const static once(once::path());
		return once;
	}
}
			
boost::filesystem::path const & bin() {
	boost::filesystem::path const static once(
		#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
			process_executable_file_path().branch_path()
		#else
			///\todo and distinguish symlinks
			process_executable_file_path().branch_path()
		#endif
	);
	return once;
}

#if defined PACKAGENERIC__CONFIGURATION__STAGE_PATH__BUILD_TO_SOURCE
	namespace {
		bool const stage() throw(std::exception) {
			char const stage [] = {
				#if defined PACKAGENERIC__CONFIGURATION__STAGE_PATH__BUILD
					PACKAGENERIC__CONFIGURATION__STAGE_PATH__BUILD
				#else
					".libs"
				#endif
			};
			return bin().leaf() == stage;
		}
	}
#endif

boost::filesystem::path const & lib() {
	class once {
		public:
			boost::filesystem::path const static path() throw(std::exception) {
				#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
					return bin();
				#else
					#if defined PACKAGENERIC__CONFIGURATION__STAGE_PATH__BUILD_TO_SOURCE
						if(stage()) return bin();
						else
					#endif
					return bin() / PACKAGENERIC__CONFIGURATION__INSTALL_PATH__BIN_TO_LIB;
				#endif
			}
	};
	boost::filesystem::path const static once(once::path());
	return once;
}

boost::filesystem::path const & share() {
	class once {
		public:
			boost::filesystem::path const static path() throw(std::exception) {
				#if defined PACKAGENERIC__CONFIGURATION__STAGE_PATH__BUILD_TO_SOURCE
					if(stage()) return bin() / boost::filesystem::path(PACKAGENERIC__CONFIGURATION__STAGE_PATH__BUILD_TO_SOURCE, boost::filesystem::no_check); // boost::filesystem::native yells when there are spaces
					else
				#endif
					return bin() / PACKAGENERIC__CONFIGURATION__INSTALL_PATH__BIN_TO_SHARE;
			}
	};
	boost::filesystem::path const static once(once::path());
	return once;
}

boost::filesystem::path const & var() {
	boost::filesystem::path const static once(bin() / PACKAGENERIC__CONFIGURATION__INSTALL_PATH__BIN_TO_VAR);
	return once;
}

boost::filesystem::path const & etc() {
	boost::filesystem::path const static once(bin() / PACKAGENERIC__CONFIGURATION__INSTALL_PATH__BIN_TO_ETC);
	return once;
}

boost::filesystem::path const & home() {
	class once {
		public:
			boost::filesystem::path const static path() throw(std::exception) {
				char const env_var [] = {
					#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
						"USERPROFILE"
					#else
						"HOME"
					#endif
				};
				char const * const path(std::getenv(env_var));
				if(!path) {
					std::ostringstream s; s << "The user has no defined home directory: the environment variable " << env_var << " is not set.";
					throw universalis::operating_system::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
				}
				return boost::filesystem::path(path, boost::filesystem::no_check); // boost::filesystem::native yells when there are spaces
			}
	};
	boost::filesystem::path const static once(once::path());
	return once;
}

namespace package {
	const std::string & name() {
		static std::string once(PACKAGENERIC__PACKAGE__NAME);
		return once;
	}
	
	namespace version {
		std::string const & string() {
			static std::string once(
				PACKAGENERIC__PACKAGE__VERSION " " PACKAGENERIC__CONFIGURATION__COMPILER__HOST
				#if !defined NDEBUG
					" debug"
				#endif
			);
			return once;
		}

		unsigned int major_number() throw() {
			return PACKAGENERIC__PACKAGE__VERSION__MAJOR;
		}

		unsigned int minor_number() throw() {
			return PACKAGENERIC__PACKAGE__VERSION__MINOR;
		}

		unsigned int patch_number() throw() {
			return PACKAGENERIC__PACKAGE__VERSION__PATCH;
		}
	}
	
	boost::filesystem::path const & lib() {
		class once {
			public:
				boost::filesystem::path const static path() throw(std::exception) {
					#if !defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
						#if defined PACKAGENERIC__CONFIGURATION__STAGE_PATH__BUILD_TO_SOURCE
							if(stage()) return paths::lib();
							else
						#endif
						return paths::lib() / name();
					#else
						return bin();
					#endif
				}
		};
		boost::filesystem::path const static once(once::path());
		return once;
	}
	
	boost::filesystem::path const & share() {
		#if defined DIVERSALIS__COMPILER__MICROSOFT && DIVERSALIS__COMPILER__VERSION <= 1300
			using namespace paths; // work around msvc7 name lookup bug
		#endif
		class once {
			public:
				boost::filesystem::path const static path() throw(std::exception) {
					#if defined PACKAGENERIC__CONFIGURATION__STAGE_PATH__BUILD_TO_SOURCE
						if(stage())
							return
								#if !defined DIVERSALIS__COMPILER__MICROSOFT || DIVERSALIS__COMPILER__VERSION > 1300 // work around msvc7 name lookup bug
									paths::
								#endif
								share();
						else
					#endif
					return
						#if !defined DIVERSALIS__COMPILER__MICROSOFT || DIVERSALIS__COMPILER__VERSION > 1300 // work around msvc7.1 name lookup bug
							paths::
						#endif
						share() / name();
				}
		};
		boost::filesystem::path const static once(once::path());
		return once;
	}
	
	boost::filesystem::path const & pixmaps() {
		#if defined DIVERSALIS__COMPILER__MICROSOFT && DIVERSALIS__COMPILER__VERSION <= 1300
			using namespace paths; // work around msvc7 name lookup bug
		#endif
		class once {
			public:
				boost::filesystem::path const static path() throw(std::exception) {
					char const pixmaps [] = {"pixmaps"};
					#if defined PACKAGENERIC__CONFIGURATION__STAGE_PATH__BUILD_TO_SOURCE
						if(stage())
							return
								#if !defined DIVERSALIS__COMPILER__MICROSOFT || DIVERSALIS__COMPILER__VERSION > 1300 // work around msvc7 name lookup bug
									paths::
								#endif
								share() / pixmaps;
						else
					#endif
					return
						#if !defined DIVERSALIS__COMPILER__MICROSOFT || DIVERSALIS__COMPILER__VERSION > 1300 // work around msvc7 name lookup bug
							paths::
						#endif
						share() / pixmaps / name();
				}
		};
		boost::filesystem::path const static once(once::path());
		return once;
	}

	boost::filesystem::path const & doc() {
		#if defined DIVERSALIS__COMPILER__MICROSOFT && DIVERSALIS__COMPILER__VERSION <= 1300
			using namespace paths; // work around msvc7 name lookup bug
		#endif
		class once {
			public:
				boost::filesystem::path const static path() throw(std::exception) {
					char const doc [] = {"doc"};
					#if defined PACKAGENERIC__CONFIGURATION__STAGE_PATH__BUILD_TO_SOURCE
						if(stage())
							return
								#if !defined DIVERSALIS__COMPILER__MICROSOFT || DIVERSALIS__COMPILER__VERSION > 1300 // work around msvc7 name lookup bug
									paths::
								#endif
								share() / doc;
						else
					#endif
					return
						#if !defined DIVERSALIS__COMPILER__MICROSOFT || DIVERSALIS__COMPILER__VERSION > 1300 // work around msvc7 name lookup bug
							paths::
						#endif
						share() / doc / name(); ///\todo name().string() + "-doc"
				}
		};
		boost::filesystem::path const static once(once::path());
		return once;
	}

	boost::filesystem::path const & var() {
		boost::filesystem::path const static once(bin() / PACKAGENERIC__CONFIGURATION__INSTALL_PATH__BIN_TO_VAR / name());
		return once;
	}
	
	boost::filesystem::path const & log() {
		boost::filesystem::path const static once(paths::var() / "log" / name());
		return once;
	}
	
	boost::filesystem::path const & etc() {
		boost::filesystem::path const static once(bin() / PACKAGENERIC__CONFIGURATION__INSTALL_PATH__BIN_TO_ETC / name());
		return once;
	}

	boost::filesystem::path const & home() {
		boost::filesystem::path const static once(paths::home() / boost::filesystem::path("." + name(), boost::filesystem::portable_posix_name));
		return once;
	}
}
