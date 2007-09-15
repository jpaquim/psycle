// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\interface psycle::engine::node
#pragma once
#include "forward_declarations.hpp"
#include "graph.hpp"
#include "port.hpp"
#include "ports/input.hpp"
#include "ports/inputs/single.hpp"
#include "ports/inputs/multiple.hpp"
#include "ports/output.hpp"
#include "named.hpp"
#include "reference_counter.hpp"
#include "exception.hpp"
#include <psycle/generic/generic.hpp>
#include <vector>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__ENGINE__NODE
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace engine {
	/// node of a graph, placeholder for a dsp, aka "plugin machine".
	class UNIVERSALIS__COMPILER__DYNAMIC_LINK node : public typenames::bases::node, public named
	{
		friend class graph;
		friend class port;
		friend class ports::output;
		friend class ports::inputs::single;
		friend class ports::inputs::multiple;
		
		protected: friend class virtual_factory_access;
			node(typenames::plugin_library_reference &, parent_type &, name_type const &);

			void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES before_destruction() {
				typenames::typenames::bases::node::before_destruction();
				close();
			}

			virtual ~node();

		///\name reference to plugin library
		///\{
			public:
				typenames::plugin_library_reference & plugin_library_reference() const throw() { return plugin_library_reference_; }
			private:
				typenames::plugin_library_reference & plugin_library_reference_;
		///\}

		///\name output ports
		///\{
			public:
				/// finds an output port by its name
				ports::output * const output_port(name_type const &) const;
		///\}

		///\name input ports
		///\{
			friend class ports::input;
	
			public:
				/// finds an input port by its name
				ports::input * const input_port(name_type const &) const;
		///\}

		///\name open
		///\{
			public:
				/// called by schedulers
				void            open  (          ) throw(std::exception) { if(!opened()) try { do_open(); } catch(...) { do_close(); throw; } }
				void            opened(bool value) throw(std::exception) { if(value) open(); else close(); }
				bool virtual    opened(          ) const;
			protected:
				void virtual do_open() throw(std::exception);
		///\}

		///\name start
		///\{
			public:
				/// called by schedulers
				void            start  (          ) throw(std::exception) { open(); if(!started()) try { do_start(); } catch(...) { do_stop(); throw; } }
				void            started(bool value) throw(std::exception) { if(value) start(); else stop(); }
				bool virtual    started(          ) const;
			protected:
				void virtual do_start() throw(std::exception);
		///\}

		///\name process
		///\{
			public:
				/// called by schedulers
				void inline     process_first() throw(std::exception);
			protected:
				/// this function is the placeholder where to put the dsp algorithm.
				/// re-implement this function in a derived class and put your own code in it.
				void virtual do_process_first() throw(std::exception) {}
	
			public:
				/// called by schedulers
				void inline     process() throw(std::exception);
			protected:
				/// this function is the placeholder where to put the dsp algorithm.
				/// re-implement this function in a derived class and put your own code in it.
				void virtual do_process() throw(std::exception) = 0;
	
			public:
				/// called by schedulers, reset the state of this node so that it prepares for the next call to process()
				void inline     reset();
			protected:
				void virtual do_reset() {}
		///\}

		///\name stop
		///\{
			public:
				/// called by schedulers
				void            stop() throw(std::exception) { if(started()) do_stop(); }
			protected:
				void virtual do_stop() throw(std::exception);
		///\}
		
		///\name close
		///\{
			public:
				/// called by schedulers
				void            close() throw(std::exception) { stop(); if(opened()) do_close(); }
			protected:
				void virtual do_close() throw(std::exception);
		///\}

		protected:
			void virtual channel_change_notification_from_port(port const &) throw(std::exception) {}
			void virtual seconds_per_event_change_notification_from_port(port const &) {}

		///\name name
		///\{
			public:
				/// the full path of the node (within its graph)
				name_type qualified_name() const;
				void virtual dump(std::ostream &, int const & tabulations = 0) const;
		///\}
	};
	/// outputs a textual representation of a node.
	///\relates node
	///\see node::dump
	UNIVERSALIS__COMPILER__DYNAMIC_LINK std::ostream & operator<<(std::ostream &, node const &);
}}

//////////////////////////
// implementation details

namespace psycle { namespace engine {
	void node::process_first() throw(std::exception)
	{
		if(false && loggers::trace()()) {
			std::ostringstream s;
			s << qualified_name() << " processing node first input";
			loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		do_process_first();
	}

	void node::process() throw(std::exception)
	{
		if(false && loggers::trace()()) {
			std::ostringstream s;
			s << qualified_name() << " processing node";
			loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		do_process();
	}

	void node::reset()
	{
		if(false && loggers::trace()()) {
			std::ostringstream s;
			s << qualified_name() << " resetting node";
			loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		do_reset();
	}
}}
#include <universalis/compiler/dynamic_link/end.hpp>
