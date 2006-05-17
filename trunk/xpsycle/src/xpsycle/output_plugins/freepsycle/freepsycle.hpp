
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

#pragma once
#include "../interface.hpp"
#include <psycle/host/scheduler/single_threaded.hpp>

#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__OUTPUT_PLUGINS__FREEPSYCLE__FREEPSYCLE
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace output_plugins
	{
		/// connect psycle's master machine to an input port of a freepsycle node
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK freepsycle : public output_plugin
		{
			public:
				///\param sink_plugin the freepsycle node plugin to instanciate
				///\param sink_input_port the input port to connect to
				freepsycle(callback_type &, std::string const & sink_plugin = "output.default", std::string const & sink_input_port = "in");

			private:
				/// freepsycle graph connecting psycle with gstreamer
				engine::graph & graph_;

				/// freepsycle source node fed with the audio data from psycle
				engine::node & source_;

				///
				host::plugin_resolver resolver_;

				/// freepsycle sink node where the audio data from psycle's master machine is forwarded to
				engine::node & sink_;

				/// scheduler for processing the graph
				host::schedulers::single_threaded scheduler_;
		};
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
