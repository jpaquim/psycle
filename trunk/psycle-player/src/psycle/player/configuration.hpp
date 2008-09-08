// -*- mode:c++; indent-tabs-mode:t -*-

/**********************************************************************************************
	Copyright 2007-2008 members of the psycle project http://psycle.sourceforge.net

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
	You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**********************************************************************************************/

#pragma once
#include <string>
#include <map>

namespace psy { namespace core {
	class AudioDriver;
}}

using psy::core::AudioDriver;

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
			AudioDriver * output_driver_, * base_driver_, * dummy_driver_;
	///\}
};
