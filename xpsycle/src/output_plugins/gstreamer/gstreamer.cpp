
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

#include "gstreamer.hpp"
#include <boost/thread/thread.hpp>

namespace psycle
{
	namespace output_plugins
	{
		extern "C"
		{
			UNIVERSALIS__COMPILER__DYNAMIC_LINK__EXPORT AudioDriver & UNIVERSALIS__COMPILER__CALLING_CONVENTION_C new_()
			{
				return new gstreamer_wrapper;
			}

			UNIVERSALIS__COMPILER__DYNAMIC_LINK__EXPORT void          UNIVERSALIS__COMPILER__CALLING_CONVENTION_C delete_(AudioDriver & instance)
			{
				delete &instance;
			}
		}

		namespace
		{
			/// source node from where the audio data from psycle come from
			class source : public engine::node
			{
				protected: friend class generic_access;
					source(parent_type &, std::string const & name)
					{
						engine::ports::output::create(*this, "out");
					}

				public:
					~source() throw() {}

				public:
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_process() throw(engine::exception)
					{
						engine::buffer & out(output_ports()[0]);
						float const * in(callback_(callback_argument_, out.size());
						unsigned int const samples(out.samples_per_channel_buffer());
						for(unsigned int event(0); event < samples; ++event)
							for(unsigned int channel(0); channel < out.channels(); ++channel)
							{
								out[channel][event].index(event);
								out[channel][event].sample(*in++ / 32767);
							}
					}

				private:
					typedef float const * (*callback_type)(void*);
					callback_type callback_;
					void * callback_argument_;
			};
		}

		freepsycle_sink::freepsycle_sink()
		:
			graph_(engine::graph::create("psycle -> gstreamer")),
			source_(engine::node::create<source>(graph_, "psycle")
			sink_(engine::node::create<plugins::output::grstreamer>(graph_, "gstreamer")),
			scheduler_(host::schedulers::single_threaded::create(graph_)
		{
		}

		void freepsycle_sink::opened(bool value)
		{
		}

		void freepsycle_sink::started(bool value)
		{
			scheduler_.started(value);
		}
	}
}
