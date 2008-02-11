// -*- mode:c++; indent-tabs-mode:t -*-
/***************************************************************************
*   Copyright (C) 2007 Psycledelics     *
*   psycle.sf.net   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#ifndef PSYCLE__CORE__PLUGIN_FINDER
#define PSYCLE__CORE__PLUGIN_FINDER

#include "machine.h"
#include "pluginFinderKey.hpp"

namespace psy
{
	namespace core
	{
		/**
		@author  Psycledelics  
		*/
		class PluginInfo
		{
			public:
				PluginInfo();

				~PluginInfo();

				void setType( Machine::type_type type );
				Machine::type_type type() const;

				void setMode( Machine::mode_type mode );
				Machine::mode_type mode() const;

				void setName( const std::string & name );
				const std::string & name() const;

				void setAuthor( const std::string & name );
				const std::string & author() const;

				void setDesc( const std::string & desc );
				const std::string & desc() const;

				void setVersion( const std::string & version );
				const std::string & version() const;

				void setLibName( const std::string & libName );
				const std::string & libName() const;

				void setFileTime( time_t fileTime );
				time_t fileTime() const;

				void setError( const std::string & error );
				const std::string error() const;

				void setAllow( bool allow );
				bool allow() const;

				void setCategory( const std::string & category );
				const std::string & category() const;

			private:
				Machine::type_type subclass_;
				Machine::mode_type mode_;
				std::string name_;
				std::string author_;
				std::string desc_;
				std::string version_;
				std::string libName_;
				time_t fileTime_;
				std::string error_;
				bool allow_;
				std::string category_;
		};

		class PluginFinder
		{
			public:
				PluginFinder(std::string const & psycle_path, std::string const & ladspa_path);
				~PluginFinder();

				void scanAll();

				PluginInfo info( const PluginFinderKey & key ) const;
			
				std::map< PluginFinderKey, PluginInfo >::const_iterator begin() const;
				std::map< PluginFinderKey, PluginInfo >::const_iterator end() const;
				
			public:
				std::string const & psycle_path() const { return psycle_path_; }
			private:
				std::string const psycle_path_;
				
			public:
				std::string const & ladspa_path() const { return ladspa_path_; }
			private:
				std::string const ladspa_path_;

			private:
				std::map< PluginFinderKey, PluginInfo > map_;

				void scanInternal();
				void scanLadspa();
				void scanNatives();
		};
	}
}
#endif
