// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\interface psycle::host::scheduler
#pragma once
#include "forward_declarations.hpp"
#include <psycle/engine.hpp>
#include <list>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK PACKAGENERIC__MODULE__SOURCE__PSYCLE__HOST__SCHEDULER
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace host
	{
		namespace underlying = engine;
		typedef underlying::exception exception;
		
		/// simply, a "player".
		template<typename Graph>
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK scheduler
		{
			protected:
				scheduler(underlying::graph & graph) throw(std::exception) : graph_(Graph::create_on_heap(graph)) {}
			public:
				virtual ~scheduler() throw() {}
				void inline started(bool const & started) { if(started) start(); else stop(); }
				void virtual start() throw(exception) = 0;
				void virtual stop() = 0;
			protected:
				typedef Graph graph_type;
				Graph const inline & graph() const throw() { return graph_; }
				Graph       inline & graph()       throw() { return graph_; }
			private:
				Graph & graph_; ///\todo remove reference
		};
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
