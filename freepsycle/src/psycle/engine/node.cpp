// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\implementation psycle::engine::node
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
#include <psycle/detail/project.private.hpp>
#include "node.hpp"
#include <universalis/compiler/typenameof.hpp>

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
		node::node(engine::plugin_library_reference & plugin_library_reference, node::parent_type & parent, node::name_type const & name)
		:
			node_base(parent),
			named(name),
			plugin_library_reference_(plugin_library_reference)
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << "new node: " << qualified_name();
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}
		
		node::~node() throw()
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << "delete node: " << qualified_name();
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			if(multiple_input_port()) delete multiple_input_port();
			for(single_input_ports_type::const_iterator i(single_input_ports().begin()) ; i != single_input_ports().end() ; ++i)
			{
				//(**i).disconnect_all();
				delete *i;
			}
			for(output_ports_type::const_iterator i(output_ports().begin()) ; i != output_ports().end() ; ++i)
			{
				//(**i).disconnect_all();
				delete *i;
			}
		}
	
		node::name_type node::qualified_name() const
		{
			return parent().qualified_name() + '.' + name();
		}
	
		ports::input * const node::input_port(name_type const & name) const
		{
			if(multiple_input_port() && multiple_input_port()->name() == name) return multiple_input_port();
			for(single_input_ports_type::const_iterator i(single_input_ports().begin()) ; i != single_input_ports().end() ; ++i) if((**i).name() == name) return *i;
			if(loggers::warning()())
			{
				std::ostringstream s;
				s << qualified_name() << ": input port not found: " << name;
				loggers::warning()(s.str());
			}
			return 0;
		}
	
		ports::output * const node::output_port(name_type const & name) const
		{
			for(output_ports_type::const_iterator i(output_ports().begin()) ; i != output_ports().end() ; ++i) if((**i).name() == name) return *i;
			if(loggers::warning()())
			{
				std::ostringstream s;
				s << qualified_name() << ": output port not found: " << name;
				loggers::warning()(s.str());
			}
			return 0;
		}
	
		void node::do_open() throw(std::exception)
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << qualified_name() << ": opening";
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}
	
		bool node::opened() const
		{
			return false;
		}
	
		void node::do_start() throw(std::exception)
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << qualified_name() << ": starting";
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}
	
		bool node::started() const
		{
			return false;
		}
	
		void node::do_stop() throw(std::exception)
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << qualified_name() << ": stopping";
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}
	
		void node::do_close() throw(std::exception)
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << qualified_name() << ": closing";
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}
	
		void node::dump(std::ostream & out, const int & tabulations) const
		{
			for(int t(0) ; t < tabulations ; ++t) out << '\t';
			out
				<< name()
				<< " (loaded library "
				<< plugin_library_reference_instance().name()
				<< ", " << universalis::compiler::typenameof(*this)
				<< ')'
				<< std::endl;
			if(multiple_input_port()) multiple_input_port()->dump(out, tabulations + 1);
			for(single_input_ports_type::const_iterator i(single_input_ports().begin()) ; i != single_input_ports().end() ; ++i) (**i).dump(out, tabulations + 1);
			for(output_ports_type::const_iterator i(output_ports().begin()) ; i != output_ports().end() ; ++i) (**i).dump(out, tabulations + 1);;
		}
	
		std::ostream & operator<<(std::ostream & out, const node & node)
		{
			node.dump(out);
			return out;
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
		node::node(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, node::name_type const & name)
		:
			named(name),
			plugin_library_reference_(plugin_library_reference),
			graph_(graph),
			multiple_input_port_(0)
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << qualified_name() << " new node";
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			graph.insert(this);
		}
		
		node::~node() throw()
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << qualified_name() << " delete node";
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			graph_instance().erase(this);
			if(multiple_input_port()) delete multiple_input_port();
			for(single_input_ports_type::const_iterator i(single_input_ports().begin()) ; i != single_input_ports().end() ; ++i)
			{
				//(**i).disconnect_all();
				delete *i;
			}
			for(output_ports_type::const_iterator i(output_ports().begin()) ; i != output_ports().end() ; ++i)
			{
				//(**i).disconnect_all();
				delete *i;
			}
		}
	
		node::name_type node::qualified_name() const
		{
			return graph_instance().qualified_name() + '.' + name();
		}
	
		ports::input * const node::input_port(const std::string & name) const
		{
			if(multiple_input_port() && multiple_input_port()->name() == name) return multiple_input_port();
			for(single_input_ports_type::const_iterator i(single_input_ports().begin()) ; i != single_input_ports().end() ; ++i) if((**i).name() == name) return *i;
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << qualified_name() << " input port not found: " << name;
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			return 0;
		}
	
		ports::output * const node::output_port(const std::string & name) const
		{
			for(output_ports_type::const_iterator i(output_ports().begin()) ; i != output_ports().end() ; ++i) if((**i).name() == name) return *i;
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << qualified_name() << " output port not found: " << name;
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			return 0;
		}
	
		void node::do_open() throw(std::exception)
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << qualified_name() << " opening";
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}
	
		bool node::opened() const
		{
			return false;
		}
	
		void node::do_start() throw(std::exception)
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << qualified_name() << " starting";
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}
	
		bool node::started() const
		{
			return false;
		}
	
		void node::do_stop() throw(std::exception)
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << qualified_name() << " stopping";
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}
	
		void node::do_close() throw(std::exception)
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << qualified_name() << " closing";
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}
	
		void node::dump(std::ostream & out, const int & tabulations) const
		{
			for(int t(0) ; t < tabulations ; ++t) out << '\t';
			out
				<< name()
				<< " (loaded library "
				<< plugin_library_reference_instance().name()
				<< ", " << universalis::compiler::typenameof(*this)
				<< ')'
				<< std::endl;
			if(multiple_input_port()) multiple_input_port()->dump(out, tabulations + 1);
			for(single_input_ports_type::const_iterator i(single_input_ports().begin()) ; i != single_input_ports().end() ; ++i) (**i).dump(out, tabulations + 1);
			for(output_ports_type::const_iterator i(output_ports().begin()) ; i != output_ports().end() ; ++i) (**i).dump(out, tabulations + 1);;
		}
	
		std::ostream & operator<<(std::ostream & out, const node & node)
		{
			node.dump(out);
			return out;
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
