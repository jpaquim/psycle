// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface psycle::host::graph
#pragma once
#include "forward_declarations.hpp"
#include <psycle/engine.hpp>
#include "host.hpp"
#include <universalis/compiler/numeric.hpp>
#include <boost/signal.hpp>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK PSYCLE__HOST
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace host {

class UNIVERSALIS__COMPILER__DYNAMIC_LINK graph : public bases::graph {
	protected: friend class virtual_factory_access;
		graph(underlying_type &);
};

class coordinates {
	public:
		typedef universalis::compiler::numeric<32>::floating_point real;

		coordinates(real const & x, real const & y) : x_(x), y_(y) { /* signal_move()(*this); */ }
		
		void operator()(real const & x, real const & y) { this->x_ = x; this->y_ = y; signal_move()(*this); }
		
		void         x(real const & x) { this->x_ = x; signal_move()(*this); }
		real const & x() const throw() { return x_; }
		
		void         y(real const & y) { this->y_ = y; signal_move()(*this); }
		real const & y() const throw() { return y_; }

	private:
		real x_, y_;

		/*
		friend class boost::serialization::access;
		template<typename Archive> void serialize(Archive & a, unsigned int const & version) { a & x_; a & y_; }
		*/
		
	public:
		boost::signal<void (coordinates &)> & signal_move() throw() { return signal_move_; }
	private:
		boost::signal<void (coordinates &)>   signal_move_;
};

class UNIVERSALIS__COMPILER__DYNAMIC_LINK port : public bases::port, public coordinates {
	protected: friend class virtual_factory_access;
		port(class node &, underlying_type &, real /*const*/ x = 0, real /*const*/ y = 0);
};

namespace ports {
	class UNIVERSALIS__COMPILER__DYNAMIC_LINK output : public bases::ports::output {
		protected: friend class virtual_factory_access;
			output(class node &, underlying_type &, real /*const*/ x = 0, real /*const*/ y = 0);
	};

	class UNIVERSALIS__COMPILER__DYNAMIC_LINK input : public bases::ports::input {
		protected: friend class virtual_factory_access;
			input(class node &, underlying_type &, real /*const*/ x = 0, real /*const*/ y = 0);
	};
	
	namespace inputs {
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK single : public bases::ports::inputs::single {
			protected: friend class virtual_factory_access;
				single(class node &, underlying_type &, real /*const*/ x = 0, real /*const*/ y = 0);
		};

		class UNIVERSALIS__COMPILER__DYNAMIC_LINK multiple : public bases::ports::inputs::multiple {
			protected: friend class virtual_factory_access;
				multiple(class node &, underlying_type &, real const /*const*/ x = 0, real /*const*/ y = 0);
		};
	}
}

class UNIVERSALIS__COMPILER__DYNAMIC_LINK node : public bases::node, public coordinates {
	protected: friend class virtual_factory_access;
		node(class graph &, underlying_type &, real /*const*/ x = 0, real /*const*/ y = 0);
		void after_construction() /*override*/;
		
	/*
	private:
		friend class boost::serialization::access;
		template<typename Archive> void serialize(Archive & a, unsigned int const & version)
		{
			a & boost::serialization::base_object<underlying_type>(*this);
			a & boost::serialization::base_object<coordinates>(*this);
		}
	*/
};

}}
#include <universalis/compiler/dynamic_link/end.hpp>
