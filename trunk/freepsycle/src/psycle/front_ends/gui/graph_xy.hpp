// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2011 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#pragma once
#include "forward_declarations.hpp"
#include <psycle/engine.hpp>
#define PSYCLE__DECL PSYCLE__HOST
#include <psycle/detail/decl.hpp>
namespace psycle { namespace host {

typedef engine::graph graph;

class coordinates {
	public:
		typedef float real;

		coordinates(real const & x, real const & y) : x_(x), y_(y) { /* signal_move()(*this); */ }
		
		void operator()(real const & x, real const & y) { this->x_ = x; this->y_ = y; signal_move()(*this); }
		
		void         x(real const & x) { this->x_ = x; signal_move()(*this); }
		real const & x() const { return x_; }
		
		void         y(real const & y) { this->y_ = y; signal_move()(*this); }
		real const & y() const { return y_; }

	private:
		real x_, y_;

		/*
		friend class boost::serialization::access;
		template<typename Archive> void serialize(Archive & a, unsigned int const & version) { a & x_; a & y_; }
		*/
		
	public:
		boost::signal<void (coordinates &)> & signal_move() { return signal_move_; }
	private:
		boost::signal<void (coordinates &)>   signal_move_;
};

class PSYCLE__DECL port : public coordinates {
	protected:
		port(class node &, engine::port &, real /*const*/ x = 0, real /*const*/ y = 0);

	public:
		class node & node() const { return node_; }
		class node & node() { return node_; }
	private:
		class node & node_;

	public:
		engine::port & engine() const { return engine_; }
		engine::port & engine() { return engine_; }
	protected:
		engine::port & engine_;
};

namespace ports {
	class PSYCLE__DECL output : public port {
		public:
			output(class node &, engine::ports::output &, real /*const*/ x = 0, real /*const*/ y = 0);
		public:
			engine::ports::output & engine() const { return static_cast<engine::ports::output&>(engine_); }
			engine::ports::output & engine() { return static_cast<engine::ports::output&>(engine_); }
	};

	class PSYCLE__DECL input : public port {
		protected:
			input(class node &, engine::ports::input &, real /*const*/ x = 0, real /*const*/ y = 0);
		public:
			engine::ports::input & engine() const { return static_cast<engine::ports::input&>(engine_); }
			engine::ports::input & engine() { return static_cast<engine::ports::input&>(engine_); }
	};
	
	namespace inputs {
		class PSYCLE__DECL single : public input {
			public:
				single(class node &, engine::ports::inputs::single &, real /*const*/ x = 0, real /*const*/ y = 0);
				engine::ports::inputs::single & engine() const { return static_cast<engine::ports::inputs::single&>(engine_); }
				engine::ports::inputs::single & engine() { return static_cast<engine::ports::inputs::single&>(engine_); }
		};

		class PSYCLE__DECL multiple : public input {
			public:
				multiple(class node &, engine::ports::inputs::multiple &, real const /*const*/ x = 0, real /*const*/ y = 0);
				engine::ports::inputs::multiple & engine() const { return static_cast<engine::ports::inputs::multiple&>(engine_); }
				engine::ports::inputs::multiple & engine() { return static_cast<engine::ports::inputs::multiple&>(engine_); }
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
