/***************************************************************************
*   Copyright (C) 2006 by Stefan   *
*   natti@linux   *
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
#include "defaultbitmaps.h"
#include <ngrs/ncolor.h>
#include <ngrs/nobject.h>
#include <ngrs/nxmlparser.h>
#include <vector>
#include <string>
#include <exception>

/**
@author Stefan
*/

namespace psycle {
	namespace host {

		class Configuration : public NObject {
		public:

			Configuration();

			~Configuration();

			void loadConfig();

			DefaultBitmaps & icons();

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
			const std::string & prsPath() const;
			const std::string & hlpPath() const;

			bool enableSound() const;

			InputHandler & inputHandler();

		private:

			InputHandler inputHandler_;
			DefaultBitmaps* bitmaps_;

			// a map, that holds available drivers ..
			std::map<std::string, AudioDriver*> driverMap_;
			bool enableSound_;
			bool doEnableSound;

			std::string iconPath_;
			std::string pluginPath_;
			std::string prsPath_;
			std::string hlpPath_;

			void setSkinDefaults();
			void setXmlDefaults();
			void onConfigTagParse( const NXmlParser & parser, const std::string & tagName );
			void loadConfig( const std::string & path ) throw(std::exception);

		};

	}
}
#endif
