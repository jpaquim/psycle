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

#include "machinekey.hpp"

namespace psy
{
	namespace core
	{
		namespace MachineRole
		{
			typedef enum
			{
				GENERATOR = 0,
				EFFECT,
				MASTER,
				CONTROLLER
			} type;
		}

		/**
		@author  Psycledelics  
		*/
		class PluginInfo
		{
			public:
				///FIXME: Should MachineKey be part of the PluginInfo? It is the key to the map.
				PluginInfo(MachineKey, MachineRole::type, std::string, std::string, std::string, std::string, std::string, std::string)

				virtual ~PluginInfo();

				void setKey( MachineKey );
				MachineKey key() cons;

				void setRole( MachineRole::type role );
				MachineRole::type role() const;

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
				MachineKey key_;
				MachineRole::type role_;
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
	}
}

#endif
