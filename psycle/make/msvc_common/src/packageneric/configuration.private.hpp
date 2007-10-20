///\file
///\brief packageneric build configuration.
/// This public configuration was not generated by an autoconfiguration script;
/// it was handwritten hardcoded for the following canonical host triplet:
/// - compiler: microsoft msvc 7.1
/// - operating system: microsoft (it is unclear what other operating systems are supported by microsoft's compiler)
/// - architecture: it is unclear what architectures are supported by microsoft's compiler and operating system
#pragma once
#include <packageneric/package.private.hpp>



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///\name host environment: platform for which the source package is built, and compiler/linker options used.
///\{
	/// host environment: platform for which the source package is built by the tool chain.
	#define PACKAGENERIC__CONFIGURATION__COMPILER__HOST "unknown-microsoft-unknown"
	/// compiler options used to build the source package.
	#define PACKAGENERIC__CONFIGURATION__COMPILER__CXXFLAGS "microsoft proprietary corpobscurantism unknown compiler"
	/// linker options used to build the source package.
	#define PACKAGENERIC__CONFIGURATION__COMPILER__LDFLAGS  "microsoft proprietary corpobscurantism unknown linker"
///\}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///\name installation paths
///\{
	/// relative path from the bin dir to the lib dir
	#define PACKAGENERIC__CONFIGURATION__INSTALL_PATH__BIN_TO_LIB     "."
	/// relative path from the bin dir to the libexec dir
 	#define PACKAGENERIC__CONFIGURATION__INSTALL_PATH__BIN_TO_LIBEXEC "../libexec"
	/// relative path from the bin dir to the share dir
	#define PACKAGENERIC__CONFIGURATION__INSTALL_PATH__BIN_TO_SHARE   "../share"
	/// relative path from the bin dir to the var dir
	#define PACKAGENERIC__CONFIGURATION__INSTALL_PATH__BIN_TO_VAR     "../var"
	/// relative path from the bin dir to the etc dir
	#define PACKAGENERIC__CONFIGURATION__INSTALL_PATH__BIN_TO_ETC     "../etc"
///\}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///\name stage paths (to be able to execute from the build dir without installing)
///\{
	/// relative path from the build dir to the source dir
	#define PACKAGENERIC__CONFIGURATION__STAGE_PATH__BUILD_TO_SOURCE "../../../../.."
///\}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///\name build environment from which the source package was built (optional information not really needed).
///\{
	/// tool chain used to build the source package.
	#define PACKAGENERIC__CONFIGURATION__COMPILER__BUILD "unknown-microsoft-msvc-unknown"
	/// name of the operating system from which the source package was built $(uname --all)
	#define PACKAGENERIC__OPERATING_SYSTEM__UNAME "microsoft proprietary corpobscurantism unknown operating system" " version " PACKAGENERIC__STRINGIZED(PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__VERSION__MAJOR) "." PACKAGENERIC__STRINGIZED(PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__VERSION__MINOR) "." PACKAGENERIC__STRINGIZED(PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__VERSION__PATCH)
	/// who built the source package $(id --user --name)@$(hostname --long)
	#define PACKAGENERIC__OPERATING_SYSTEM__ID "evangelist@microsoft.com"
///\}



#define BOOST_THREAD_USE_DLL
