// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2006 Johan Boule <bohan@jabber.org>
// copyright 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\interface psycle::host::graph
#pragma once
#include "forward_declarations.hpp"
#include <psycle/engine.hpp>
#include "host.hpp"
#include <universalis/compiler/cast.hpp>
#include <universalis/compiler/numeric.hpp>
#include <boost/signal.hpp>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK PSYCLE__HOST__GRAPH
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace host
	{
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK graph : public typenames::typenames::bases::graph
		{
			protected: friend class factory;
				graph(underlying_type &);
		};

		class coordinates
		{
			public:
				typedef universalis::compiler::numeric<32>::floating_point real;
				           inline coordinates(real const & x, real const & y) : x_(x), y_(y) { /* signal_move()(*this); */ }
				void       inline  operator()(real const & x, real const & y) { this->x_ = x; this->y_ = y; signal_move()(*this); }
				void       inline   x(real const & x) { this->x_ = x; signal_move()(*this); }
				real const inline & x() const throw() { return x_; }
				void       inline   y(real const & y) { this->y_ = y; signal_move()(*this); }
				real const inline & y() const throw() { return y_; }
			private:
				real x_, y_;
				/*
				friend class boost::serialization::access;
				template<typename Archive> void serialize(Archive & a, unsigned int const & version) { a & x_; a & y_; }
				*/
				
			public:
				boost::signal<void (coordinates &)> inline & signal_move() throw() { return signal_move_; }
			private:
				boost::signal<void (coordinates &)>          signal_move_;
		};

		class UNIVERSALIS__COMPILER__DYNAMIC_LINK port : public typenames::typenames::bases::port, public coordinates
		{
			protected: friend class factory;
				port(parent_type &, underlying_type &, real /*const*/ x = 0, real /*const*/ y = 0);
		};

		namespace ports
		{
			class UNIVERSALIS__COMPILER__DYNAMIC_LINK output : public typenames::typenames::bases::ports::output
			{
				protected: friend class factory;
					output(parent_type &, underlying_type &, real /*const*/ x = 0, real /*const*/ y = 0);
			};

			class UNIVERSALIS__COMPILER__DYNAMIC_LINK input : public typenames::typenames::bases::ports::input
			{
				protected: friend class factory;
					input(parent_type &, underlying_type &, real /*const*/ x = 0, real /*const*/ y = 0);
			};
			
			namespace inputs
			{
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK single : public typenames::typenames::bases::ports::inputs::single
				{
					protected: friend class factory;
						single(parent_type &, underlying_type &, real /*const*/ x = 0, real /*const*/ y = 0);
				};

				class UNIVERSALIS__COMPILER__DYNAMIC_LINK multiple : public typenames::typenames::bases::ports::inputs::multiple
				{
					protected: friend class factory;
						multiple(parent_type &, underlying_type &, real const /*const*/ x = 0, real /*const*/ y = 0);
				};
			}
		}

		class UNIVERSALIS__COMPILER__DYNAMIC_LINK node : public typenames::typenames::bases::node, public coordinates
		{
			protected: friend class factory;
				node(parent_type &, underlying_type &, real /*const*/ x = 0, real /*const*/ y = 0);
				void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES after_construction();
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
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
