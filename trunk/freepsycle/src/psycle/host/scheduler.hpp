// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::host::scheduler
#pragma once
#include "forward_declarations.hpp"
#include <psycle/engine.hpp>
#define PSYCLE__DECL PSYCLE__HOST
#include <psycle/detail/decl.hpp>
namespace psycle { namespace host {

namespace underlying = engine;
typedef underlying::exception exception;

/// simply, a "player".
template<typename Graph>
class PSYCLE__DECL scheduler {
	protected:
		scheduler(typename Graph::underlying_type & graph) throw(std::exception) : graph_(Graph::create_on_heap(graph)) {}
	public:
		virtual ~scheduler() throw() {}
		bool virtual started() = 0;
		void         started(bool value) throw(exception) { if(value) start(); else stop(); }
		void virtual start() throw(exception) = 0;
		void virtual stop() = 0;
	protected:
		typedef Graph graph_type;
		Graph const & graph() const throw() { return graph_; }
		Graph       & graph()       throw() { return graph_; }
	private:
		Graph & graph_; ///\todo remove reference with Graph::create_on_stack graph_;
};

}}
#include <psycle/detail/decl.hpp>
