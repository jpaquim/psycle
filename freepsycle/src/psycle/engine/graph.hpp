// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycle development team http://psycle.sourceforge.net

///\interface psycle::engine::graph
#pragma once
#include "forward_declarations.hpp"
#include "named.hpp"
#include <psycle/generic/generic.hpp>
#include <set>
#include <boost/thread/mutex.hpp>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__ENGINE__GRAPH
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace engine
	{
		/// a set of nodes
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK graph : public typenames::typenames::bases::graph, public named
		{
			protected: friend class factory;
				graph(name_type const &);
				virtual ~graph();
	
			public:
				boost::mutex inline & mutex() const { return mutex_; }
			private:
				boost::mutex mutable mutex_;
	
			public:
				/// the length of each channel of the buffers
				int inline const & events_per_buffer() { return events_per_buffer_; }
			private:
				int events_per_buffer_;
	
			///\name name
			///\{
				public:
					/// the fully qualified name (a path)
					name_type qualified_name() const;
					void virtual dump(std::ostream &, int const & tabulations = 0) const;
			///\}
		};
		/// outputs a textual representation of a graph.
		///\relates graph
		///\see graph::dump
		UNIVERSALIS__COMPILER__DYNAMIC_LINK std::ostream & operator<<(std::ostream & out, graph const &);
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
