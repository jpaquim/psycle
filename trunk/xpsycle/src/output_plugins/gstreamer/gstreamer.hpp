
// copyright 2006 johan boule

/***************************************************************************
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

#ifndef GSTOUT_H
#define GSTOUT_H

#include <xpsycle/audiodriver.h>
#include <psycle/plugins/output/gstreamer.hpp> // we're wrapping freepsycle's gstreamer output

namespace psycle
{
	namespace plugin
	{
		namespace output
		{
			class gstreamer_wrapper : public AudioDriver
			{
				public:
					GstOutOut();
					~GstOutOut();
					void Initialize(AUDIODRIVERWORKFN, void *);
					bool Initialized();
					void configure();
					bool Enable(bool);
				private:
					/// dummy graph
					engine::graph graph_;
					/// the freepsycle's gstreamer output we're wrapping
					gstreamer gstreamer_;
			};
		}
	}
}

#endif
