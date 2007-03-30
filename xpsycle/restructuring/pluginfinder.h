/***************************************************************************
	*   Copyright (C) 2006 by  Stefan Nattkemper   *
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
#ifndef PLUGINFINDER_H
#define PLUGINFINDER_H

/**
@author  Stefan Nattkemper
*/

#include <string>
#include <vector>

namespace psycle
{
	namespace host
	{

			class AudioOutDriverInfo {
			public:

				AudioOutDriverInfo( const std::string& path );
				~AudioOutDriverInfo();

				// path and name of teh audiodriver library ( e.g adriv.so )
				// unique key
				std::string path() const;

			private:

				std::string path_;

			};

		class PluginFinder  {
		public:
			PluginFinder();

			~PluginFinder();

			void scanAll();

		private:

					std::vector<AudioOutDriverInfo> audioOutDriverList;

	      void scanLadspa();
		  void scanNatives();
					void scanAudioOutDriver();

		};
	}
}

#endif

