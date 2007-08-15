// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\implementation psycle::engine::graph
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "graph.hpp"
#include "node.hpp"
#include "ports/output.hpp"
#include "ports/input.hpp"
#include <universalis/compiler/typenameof.hpp>
#include <map>

namespace psycle
{
	namespace engine
	{
		graph::graph(graph::name_type const & name)
		:
			named(name)
		{
			if(loggers::information()())
			{
				std::ostringstream s;
				s << "new graph: " << qualified_name();
				loggers::information()(s.str());
			}
			events_per_buffer_ = 8192;
		}
		
		graph::name_type graph::qualified_name() const
		{
			return name();
		}
		
		graph::~graph()
		{
			if(loggers::information()())
			{
				std::ostringstream s;
				s << "delete graph: " << qualified_name();
				loggers::information()(s.str());
			}
			while(!empty())
			{
				node & node(**begin());
				erase(*begin()); ///< work around to bypass msvc7.1's "assume no aliasing (accross functions)" optimization
		
				//while(!empty())
				//{
				//	node & node(*begin());
			
				//while(!empty())
				//{
				//	node & node(*--end());
			
				//for(reverse_iterator i(rbegin()) ; i != rend() ; ++i) /// maybe without ++i, or using a if(i == end()) break; else ++i;
				//{
			
				//for(iterator i(begin()) ; i != end() ; ++i) /// maybe without ++i, or using a if(i == end()) break; else ++i;
				//{
				//	node & node(*i);
				//	erase(i);
		
				if(loggers::information()())
				{
					std::ostringstream s;
					s << node.qualified_name() << ": deleting node instance of " << universalis::compiler::typenameof(node) << " from loaded library " << node.plugin_library_reference().name();
					loggers::information()(s.str());
				}
				reference_counter & reference_counter(node.plugin_library_reference());
				node.destroy();
				--reference_counter;
			}
			assert(empty());
		}
			
		void graph::dump(std::ostream & out, int const & tabulations) const
		{
			for(int t = 0 ; t < tabulations ; ++t) out << '\t';
			out << name() << std::endl;
			for(const_iterator i = begin() ; i != end() ; ++i) (**i).dump(out, tabulations + 1);
		}
		
		std::ostream & operator<<(std::ostream & out, graph const & graph)
		{
			graph.dump(out);
			return out;
		}
	}
}
