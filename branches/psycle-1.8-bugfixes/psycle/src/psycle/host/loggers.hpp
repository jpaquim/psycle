#pragma once
#include <string>
namespace psycle { namespace loggers {
	void inline trace(std::string const &) {}
	void inline info(std::string const &) {}
	void inline warning(std::string const &) {}
	void inline exception(std::string const &) {}
	void inline crash(std::string const &) {}
}}
