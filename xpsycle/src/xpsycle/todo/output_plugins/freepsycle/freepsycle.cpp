
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

#include "freepsycle.hpp"

namespace psycle
{
	namespace output_plugins
	{
		namespace
		{
			/// freepsycle source node fed with the audio data from psycle
			class source : public engine::node
			{
				protected: friend class generic_access;
					source(parent_type & parent, name_type const & name, output_plugin::callback_type & callback)
					:
						node_type(parent, name),
						callback_(callback)
					{
						engine::ports::output::create(*this, "out", 2);
					}

				public:
					~source() throw() {}

				public:
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_process() throw(engine::exception)
					{
						engine::buffer & out(output_ports()[0]);
						float const * in(callback().process(out.size()));
						unsigned int const samples(out.samples_per_channel_buffer());
						for(unsigned int event(0); event < samples; ++event)
							for(unsigned int channel(0); channel < out.channels(); ++channel)
							{
								out[channel][event].index(event);
								out[channel][event].sample(*in++ / 32767);
							}
					}

				private:
					interface::callback_type callback_;
			};
		}

		freepsycle::freepsycle(callback_type & callback, std::string const & sink_plugin, std::string const & sink_input_port)
		:
			output_plugin(callback),
			graph_(engine::graph::create("psycle -> freepsycle")),
			source_(engine::node::create<source>(graph_, "psycle", callback())
			sink_(resolver(sink_plugin, graph_, "freepsycle")),
			scheduler_(host::schedulers::single_threaded::create(graph_)
		{
			source_.output_port("out").connect(sink_.input_port(sink_input_port));
		}

		void freepsycle::opened(bool value)
		{
			scheduler_.opened(value);
		}

		void freepsycle::started(bool value)
		{
			scheduler_.started(value);
		}
	}
}
