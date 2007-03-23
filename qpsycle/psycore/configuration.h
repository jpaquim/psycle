/***************************************************************************
*   Copyright (C) 2007 Psycledelics Community   *
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
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "audiodriver.h"
#include "inputhandler.h"

#include <vector>
#include <string>

namespace psy {
	namespace core {

		class Configuration {
		public:

			Configuration();

			~Configuration();

			void loadConfig();

			void setDriverByName( const std::string & driverName );        

			///\ todo private access

			AudioDriver* _pOutputDriver;
			AudioDriver* _pSilentDriver;
			///\ todo put this in player ..
			bool _RecordTweaks;
			bool _RecordUnarmed;
			///\end todo


			std::map<std::string, AudioDriver*> & driverMap() {
				return driverMap_;
			}

			// path 

			const std::string & iconPath() const;
			const std::string & pluginPath() const;
			const std::string & ladspaPath() const;
			const std::string & prsPath() const;
			const std::string & hlpPath() const;

			bool enableSound() const;

			InputHandler & inputHandler();

		private:

			InputHandler inputHandler_;

			// a map, that holds available drivers ..
			std::map<std::string, AudioDriver*> driverMap_;
			bool enableSound_;
			bool doEnableSound;

			std::string iconPath_;
			std::string pluginPath_;
			std::string prsPath_;
			std::string hlpPath_;
			std::string ladspaPath_;

			void setDefaults();
			void configureKeyBindings();
//			void setXmlDefaults();
//			void onConfigTagParse( const ngrs::XmlParser & parser, const std::string & tagName );
//			void loadConfig( const std::string & path );

		};

	}
}
#endif
