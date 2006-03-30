// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\interface psycle::engine::ports::output
#pragma once
#include "../port.hpp"
#include <boost/signal.hpp>
#include <vector>
#if defined PSYCLE__EXPERIMENTAL
	#include <psycle/generic/generic.hpp>
#endif
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__ENGINE__PORTS__OUTPUT
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
		namespace ports
		{
			typedef generic::ports::output<typenames::typenames> output_base;
			/// handles an output stream of signal parting from a node
			class UNIVERSALIS__COMPILER__DYNAMIC_LINK output : public output_base
			{
				friend class graph;
				friend class node;
				friend class input;
				friend class inputs::single;
				friend class inputs::multiple;
					
				protected:
					output(parent_type &, name_type const &, int const & channels = 0); friend class generic_access;
				public:
					virtual ~output() throw();
	
				protected:
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_buffer(engine::buffer * const);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_propagate_channels() throw(exception);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_propagate_seconds_per_event();
	
				public:
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES dump(std::ostream &, int const & tabulations = 0) const;
			};
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
		namespace ports
		{
			/// handles an output stream of signal parting from a node
			class UNIVERSALIS__COMPILER__DYNAMIC_LINK output : public port
			{
				friend class graph;
				friend class node;
				friend class input;
				friend class inputs::single;
				friend class inputs::multiple;
					
				public:
					output(engine::node &, name_type const &, int const & channels = 0);
					virtual ~output() throw();
	
				public:
					void connect(input & input_port) throw(exception);
				private:
					void connect_internal_side(input &);
	
				public:
					void disconnect_all();
					void disconnect(input & input_port);
				private:
					void disconnect_internal_side(input &);
	
				protected:
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_buffer(engine::buffer * const);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_propagate_channels() throw(exception);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_propagate_seconds_per_event();
	
				public:
					typedef std::vector<ports::input*> input_ports_type;
					input_ports_type inline const & input_ports() const throw() { return input_ports_; }
				private:
					input_ports_type                input_ports_;
	
				public:
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES dump(std::ostream &, int const & tabulations = 0) const;
	
				public:
					/// Exra information on ports of the graph set and used by psycle::host::scheduler.
					/// This class is empty: the effective class is a derived class chosen by the psycle::host::scheduler.
					class schedule {};
					/// Exra information on nodes of the graph set and used by psycle::host::scheduler.
					schedule inline *& schedule_instance() throw() { return schedule_; }
				private:
					schedule        *  schedule_;

				///\name signals
				///\{
					public:
						boost::signal<void (ports::input &, ports::output &)> inline & signal_connect() { return signal_connect_; }
					private:
						boost::signal<void (ports::input &, ports::output &)> signal_connect_;
				///\}
			};
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
