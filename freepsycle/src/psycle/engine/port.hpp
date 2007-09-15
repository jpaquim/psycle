// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2000-2007 psycledelics http://psycle.sourceforge.net

///\interface psycle::engine::port
#pragma once
#include "forward_declarations.hpp"
#include <psycle/generic/generic.hpp>
#include "named.hpp"
#include "exception.hpp"
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__ENGINE__PORT
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace engine
	{
		/// handles a stream of signal coming to or parting from a engine::node
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK port : public typenames::typenames::bases::port, public named
		{
			friend class ports::output;
			friend class ports::input;
			friend class ports::inputs::single;
			friend class ports::inputs::multiple;
			
			protected: friend class virtual_factory_access;
				port(parent_type &, name_type const &, int const & channels = 0);
				virtual ~port();
	
			protected:
				/// connects this port to another port.
				void connect(port &) throw(exception);

			///\name buffer
			///\{
				public:
					/// assigns a buffer to this port (or unassigns if 0) only if the given buffer is different.
					void buffer(typenames::buffer * const);
					/// the buffer to read or write data from or to (buffers are shared accross several ports).
					typenames::buffer inline & buffer() const throw() { return *buffer_; }
				protected:
					/// assigns a buffer to this port (or unassigns if 0) without checking if the given buffer is different.
					///\pre either the given buffer is 0, or else, this port has not yet been assigned a buffer.
					void virtual do_buffer(typenames::buffer * const);
				private:
					/// the buffer to read or write data from or to (buffers are shared accross several ports).
					typenames::buffer * buffer_;
			///\}
	
			///\name channels
			///\{
				private:
					int channels_;
					bool channels_immutable_;
					/// tries to set the channel count of this port and rolls back on failure.
					void channels_transaction(int const &) throw(exception);
					/// sets the channel count of this port and propagates it to its node.
					void propagate_channels_to_node(int const &) throw(exception);
				protected:
					/// sets the channel count of this port.
					void virtual do_channels(int const &) throw(exception);
					/// propagates the channel count to its connected ports.
					void virtual do_propagate_channels() throw(exception) {}
				public:
					/// the channel count of this port.
					int inline const & channels() const { return channels_; }
					/// wether or not the channel count is allowed to be changed.
					bool const & channels_immutable() { return channels_immutable_; }
					/// sets the channel count of this port and propagates it to both its node and its connected ports.
					///\pre the channel count is not immutable: !channels_immutable()
					void channels(int const &) throw(exception);
					/// sets the channel count of this port and propagates it to its connected ports
					void propagate_channels(int const &) throw(exception);
			///\}
	
			///\name event rate
			///\{
				private:
					real seconds_per_event_;
					/// sets the seconds per event of this port and propagates it to its node.
					void propagate_seconds_per_event_to_node(real const &);
					/// propagates the seconds per event to its connected ports.
					void virtual do_propagate_seconds_per_event() {}
				public:
					/// events per second.
					real inline events_per_second() const { return 1 / seconds_per_event(); }
					/// sets the events per seconds.
					void inline events_per_second(real const & events_per_second) { seconds_per_event(1 / events_per_second); }
					/// seconds per event.
					real inline const & seconds_per_event() const { return seconds_per_event_; }
					/// sets the seconds per event.
					void seconds_per_event(real const &);
					/// sets the seconds per event of this port and propagates it to its connected ports.
					void propagate_seconds_per_event(real const &);
			///\}
	
			///\name name
			///\{
				public:
					/// the full path of the node (within its node and graph).
					name_type      qualified_name() const;
					/// the      path of the node (within its node).
					name_type semi_qualified_name() const;
			///\}

			/// outputs a textual representation of this port.
			///\see operator<<()
			void virtual dump(std::ostream &, int const & tabulations = 0) const;
		};
		/// outputs a textual representation of a port.
		///\relates port
		///\see port::dump
		UNIVERSALIS__COMPILER__DYNAMIC_LINK std::ostream & operator<<(std::ostream &, port const &);
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
