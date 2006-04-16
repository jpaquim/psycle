// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

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
#include <vector>
#if defined PSYCLE__EXPERIMENTAL
	#include <psycle/generic/generic.hpp>
#endif
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__ENGINE__NODE
#include <universalis/compiler/dynamic_link/begin.hpp>

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
#if defined PSYCLE__EXPERIMENTAL
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

namespace psycle
{
	namespace engine
	{
		typedef generic::node<typenames::typenames> node_base;
		/// node of a graph, placeholder for a dsp, aka "plugin machine".
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK node : public node_base, public named
		{
			friend class graph;
			friend class port;
			friend class ports::output;
			friend class ports::inputs::single;
			friend class ports::inputs::multiple;
			
			protected:
				node(plugin_library_reference &, parent_type &, name_type const &); friend class generic_access;
			public:
				virtual ~node() throw();

			public:
				//typenames::plugin_library_reference inline & plugin_library_reference() const throw() { return plugin_library_reference_; }
				plugin_library_reference inline & plugin_library_reference_instance() const throw() { return plugin_library_reference_; }
			private:
				plugin_library_reference        & plugin_library_reference_;
	
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
					void inline     open() throw(std::exception) { if(!opened()) do_open(); }
					bool virtual    opened() const;
				protected:
					void virtual do_open() throw(std::exception);
			///\}
	
			///\name start
			///\{
				public:
					/// called by schedulers
					void virtual    start() throw(std::exception) { open(); if(!started()) do_start(); }
					bool virtual    started() const;
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
					/// called by schedulers, reset the state of this node so that it prepares for the next call to visit_and_process()
					void inline     reset();
				protected:
					void virtual do_reset() {}
			///\}

			///\name stop
			///\{
				public:
					/// called by schedulers
					void inline     stop() throw(std::exception) { if(started()) do_stop(); }
				protected:
					void virtual do_stop() throw(std::exception);
			///\}
			
			///\name close
			///\{
				public:
					/// called by schedulers
					void inline     close() throw(std::exception) { stop(); if(opened()) do_close(); }
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
	}
}

//////////////////////////
// implementation details

namespace psycle
{
	namespace engine
	{
		void node::process_first() throw(std::exception)
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << qualified_name() << " processing node first input";
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			do_process_first();
		}

		void node::process() throw(std::exception)
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << qualified_name() << " processing node";
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			do_process();
		}

		void node::reset()
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << qualified_name() << " resetting node";
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			do_reset();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
#else // !defined PSYCLE__EXPERIMENTAL
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

namespace psycle
{
	namespace engine
	{
		/// Node of a Graph, placeholder for a dsp, often called "plugin machine" in other softwares.
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK node : public named
		{
			public:
				node(plugin_library_reference &, graph &, name_type const &);
				virtual ~node() throw();
	
			public:
				plugin_library_reference inline & plugin_library_reference_instance() const throw() { return plugin_library_reference_; }
			private:
				plugin_library_reference        & plugin_library_reference_;
	
			public:
				/// a reference to the graph that owns the node
				graph inline & graph_instance() const { return graph_; }
			private:
				graph        & graph_;
				friend class   graph;
			
			friend class port;
	
			///\name output ports
			///\{
				public:
					/// finds an output port by its name
					ports::output * const               output_port(std::string const & name) const;
					typedef std::vector<ports::output*> output_ports_type;
					/// the output ports owned by this node
					output_ports_type inline const &    output_ports() const throw() { return output_ports_; }
				private:
					output_ports_type                   output_ports_; friend class ports::output;
			///\}

			///\name input ports	
			///\{
				friend class ports::input;
		
				public:
					/// finds an input port by its name
					ports::input * const                               input_port(std::string const & name) const;
					
				public:
					typedef std::vector<ports::inputs::single*> single_input_ports_type;
					/// the input ports owned by this node
					single_input_ports_type inline const &      single_input_ports() const throw() { return single_input_ports_; }
				private:
					single_input_ports_type                     single_input_ports_; friend class ports::inputs::single;
					
				public:
		             /// the multiple input port owned by this node, if any, or else 0
		             ports::inputs::multiple inline * const multiple_input_port() const throw() { return multiple_input_port_; }
				protected:
					/// gives a multiple input port for this node
					///\pre the node doesn't already have a multiple input port
					void                     inline         multiple_input_port(ports::inputs::multiple & multiple_input_port) throw() { assert(!this->multiple_input_port()); this->multiple_input_port_ = &multiple_input_port; }
				private:
					ports::inputs::multiple        *        multiple_input_port_; friend class ports::inputs::multiple;
			///\}

			///\name open
			///\{
				public:
					/// called by schedulers
					void inline     open() throw(std::exception) { if(!opened()) do_open(); }
					bool virtual    opened() const;
				protected:
					void virtual do_open() throw(std::exception);
			///\}
	
			///\name start
			///\{
				public:
					/// called by schedulers
					void virtual    start() throw(std::exception) { open(); if(!started()) do_start(); }
					bool virtual    started() const;
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
					/// called by schedulers, reset the state of this node so that it prepares for the next call to visit_and_process()
					void inline    reset();
				private:
					void inline do_reset(); // [bohan] note: msvc6 crashes when this function is inlined.
			///\}

			///\name stop
			///\{
				public:
					/// called by schedulers
					void inline     stop() throw(std::exception) { if(started()) do_stop(); }
				protected:
					void virtual do_stop() throw(std::exception);
			///\}
			
			///\name close
			///\{
				public:
					/// called by schedulers
					void inline close() throw(std::exception) { stop(); if(opened()) do_close(); }
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

			///\name scheduler
			///\{				
				public:
					/// Exra information on nodes of the graph set and used by psycle::host::scheduler.
					/// This class is empty: the effective class is a derived class chosen by the psycle::host::scheduler.
					class schedule {};
					/// Exra information on nodes of the graph set and used by psycle::host::scheduler.
					schedule inline *& schedule_instance() throw() { return schedule_; }
				private:
					schedule * schedule_;
			///\}
		};
		/// outputs a textual representation of a node.
		///\relates node
		///\see node::dump
		UNIVERSALIS__COMPILER__DYNAMIC_LINK std::ostream & operator<<(std::ostream &, node const &);
	}
}

//////////////////////////
// implementation details

namespace psycle
{
	namespace engine
	{
		void node::process_first() throw(std::exception)
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << qualified_name() << " processing node first input";
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			do_process_first();
		}

		void node::process() throw(std::exception)
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << qualified_name() << " processing node";
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			do_process();
		}

		void node::reset()
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << qualified_name() << " resetting node";
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			do_reset();
		}

		void node::do_reset()
		{
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
#endif // !defined PSYCLE__EXPERIMENTAL
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

#include <universalis/compiler/dynamic_link/end.hpp>
