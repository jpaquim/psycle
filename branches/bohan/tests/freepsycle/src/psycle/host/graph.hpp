// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface psycle::host::graph
#pragma once
#include "forward_declarations.hpp"
#include <psycle/engine.hpp>
#include "host.hpp"
#include <boost/signal.hpp>
#define PSYCLE__DECL PSYCLE__HOST
#include <psycle/detail/decl.hpp>
namespace psycle { namespace host {

class PSYCLE__DECL graph : public bases::graph {
	protected: friend class virtual_factory_access;
		graph(underlying_type &);
};

class coordinates {
	public:
		typedef float real;

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

class PSYCLE__DECL port : public bases::port, public coordinates {
	protected: friend class virtual_factory_access;
		port(class node &, underlying_type &, real /*const*/ x = 0, real /*const*/ y = 0);
};

namespace ports {
	class PSYCLE__DECL output : public bases::ports::output {
		protected: friend class virtual_factory_access;
			output(class node &, underlying_type &, real /*const*/ x = 0, real /*const*/ y = 0);
	};

	class PSYCLE__DECL input : public bases::ports::input {
		protected: friend class virtual_factory_access;
			input(class node &, underlying_type &, real /*const*/ x = 0, real /*const*/ y = 0);
	};
	
	namespace inputs {
		class PSYCLE__DECL single : public bases::ports::inputs::single {
			protected: friend class virtual_factory_access;
				single(class node &, underlying_type &, real /*const*/ x = 0, real /*const*/ y = 0);
		};

		class PSYCLE__DECL multiple : public bases::ports::inputs::multiple {
			protected: friend class virtual_factory_access;
				multiple(class node &, underlying_type &, real const /*const*/ x = 0, real /*const*/ y = 0);
		};
	}
}

class PSYCLE__DECL node : public bases::node, public coordinates {
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
#include <psycle/detail/decl.hpp>
