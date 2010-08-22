// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__PLAYER__CONFIGURATION__INCLUDED
#define PSYCLE__PLAYER__CONFIGURATION__INCLUDED
#pragma once

#include <string>
#include <map>

namespace psycle { namespace audiodrivers {
	class AudioDriver;
}}

namespace psycle { namespace player {

using audiodrivers::AudioDriver;

/// configuration for the user interface
class Configuration {
	public:
		Configuration();
		~Configuration();
	
	///\name configuration file
	///\{
		public:
			void loadConfig();
			void loadConfig( const std::string & path );
		private:
			/// creates an initial default xml config file
			//void setXmlDefaults();
	///\}
	
	///\name paths
	///\{
		public:
			std::string const & pluginPath() const { return pluginPath_; } // could be in core?
			std::string const & ladspaPath() const { return ladspaPath_; } // could be in core?
		private:
			std::string pluginPath_;
			std::string ladspaPath_;
	///\}

	///\name audio drivers
	///\{
		public:
			void add_driver(AudioDriver &);
			std::map<std::string, AudioDriver*> const & driver_map() const throw() { return driver_map_; }
		private:
			std::map<std::string, AudioDriver*> driver_map_;


		public:
			void set_driver_by_name(std::string const & driver_name);


		public:
			bool enable_sound() const throw() { return enable_sound_; }
		private:
			bool enable_sound_;
			bool do_enable_sound_;


		public:
			AudioDriver & output_driver() throw() { return *output_driver_; }
		private:
			AudioDriver * output_driver_, * dummy_driver_;
	///\}
};

}}
#endif
