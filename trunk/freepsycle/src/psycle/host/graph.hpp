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

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
#if defined PSYCLE__EXPERIMENTAL
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

namespace psycle
{
	namespace host
	{
		typedef generic::wrappers::graph<typenames::typenames> graph_base;
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK graph : public graph_base
		{
			protected:
				graph(underlying_type &); friend class generic_access;
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

		typedef generic::wrappers::port<typenames::typenames> port_base;
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK port : public port_base, public coordinates
		{
			protected:
				port(parent_type &, underlying_type &, real /*const*/ x = 0, real /*const*/ y = 0); friend class generic_access;
		};

		namespace ports
		{
			typedef generic::wrappers::ports::output<typenames::typenames> output_base;
			class UNIVERSALIS__COMPILER__DYNAMIC_LINK output : public output_base
			{
				protected:
					output(parent_type &, underlying_type &, real /*const*/ x = 0, real /*const*/ y = 0); friend class generic_access;
			};

			typedef generic::wrappers::ports::input<typenames::typenames> input_base;
			class UNIVERSALIS__COMPILER__DYNAMIC_LINK input : public input_base
			{
				protected:
					input(parent_type &, underlying_type &, real /*const*/ x = 0, real /*const*/ y = 0); friend class generic_access;
			};
			
			namespace inputs
			{
				typedef generic::wrappers::ports::inputs::single<typenames::typenames> single_base;
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK single : public single_base
				{
					protected:
						single(parent_type &, underlying_type &, real /*const*/ x = 0, real /*const*/ y = 0); friend class generic_access;
				};

				typedef generic::wrappers::ports::inputs::multiple<typenames::typenames> multiple_base;
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK multiple : public multiple_base
				{
					protected:
						multiple(parent_type &, underlying_type &, real const /*const*/ x = 0, real /*const*/ y = 0); friend class generic_access;
				};
			}
		}

		typedef generic::wrappers::node<typenames::typenames> node_base;
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK node : public node_base, public coordinates
		{
			protected:
				node(parent_type &, underlying_type &, real /*const*/ x = 0, real /*const*/ y = 0); friend class generic_access;
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

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
#elif !defined PSYCLE__EXPERIMENTAL
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

namespace psycle
{
	namespace host
	{
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK graph
		:
			public universalis::compiler::cast::underlying_wrapper<engine::graph>,
			public std::set<node*>
		{
			public:
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

		class UNIVERSALIS__COMPILER__DYNAMIC_LINK node
		:
			public universalis::compiler::cast::underlying_wrapper<engine::node>,
			public coordinates
		{
			public:
				node(graph &, underlying_type &, real const & x, real const & y);
				
			public:
				graph inline & graph_instance() throw() { return graph_; }
			private:
				graph        & graph_;
			
			friend class port;
	
			public:
				typedef std::vector<ports::output*> output_ports_type;
				/// the output ports owned by this node
				output_ports_type inline const & output_ports() const { return output_ports_; }
				/// finds an output port by its name
				ports::output * const output_port(std::string const & name) const;
			private:
				friend class ports::output;
				output_ports_type output_ports_;
	
			friend class ports::input;

			public:
				typedef std::vector<ports::inputs::single*> input_ports_type;
				/// the input ports owned by this node
				input_ports_type inline const & input_ports() const { return input_ports_; }
				/// finds an input port by its name
				ports::input * const input_port(std::string const & name) const;
			private:
				friend class ports::inputs::single;
				input_ports_type input_ports_;
				
			public:
				ports::inputs::multiple inline * const multiple_input_port() const { return multiple_input_port_; }
			protected:
				void inline multiple_input_port(ports::inputs::multiple & multiple_input_port) { assert(!this->multiple_input_port()); this->multiple_input_port_ = &multiple_input_port; }
			private:
				friend class ports::inputs::multiple;
				ports::inputs::multiple * multiple_input_port_;

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

		class UNIVERSALIS__COMPILER__DYNAMIC_LINK port
		:
			public universalis::compiler::cast::underlying_wrapper<engine::port>,
			public coordinates
		{
			protected:
				port(node &, underlying_type &, real const & x, real const & y);
			public:
				node inline & node_instance() { return node_; }
			private:
				node & node_;
		};

		namespace ports
		{
			namespace cast
			{
				template<typename Derived_Underlying, typename Derived, typename Base_Wrapper>
				class underlying_wrapper
				:
					public Base_Wrapper,
					public universalis::compiler::cast::underlying_wrapper<Derived_Underlying, Derived, Base_Wrapper>
				{
					public:
						typedef Derived_Underlying underlying_type;
						        // disambiguates with Base_Wrapper, must be defined before underlying_template_type is redefined
						        Derived_Underlying inline const &       underlying_instance() const throw() { return                          const_underlying_instance(); }
						        /// workaround for msvc7.1 bug
						        Derived_Underlying inline const & const_underlying_instance() const throw() { return underlying_wrapper_type::const_underlying_instance(); }
						        Derived_Underlying inline       &       underlying_instance()       throw() { return underlying_wrapper_type::      underlying_instance(); }
					protected:
						typedef underlying_wrapper underlying_wrapper_type;
						inline  underlying_wrapper(node & node, Derived_Underlying & underlying_instance, coordinates::real const & x, coordinates::real const & y) : Base_Wrapper(node, underlying_instance, x, y) {}
				};
			}

			class UNIVERSALIS__COMPILER__DYNAMIC_LINK output : public cast::underlying_wrapper<engine::ports::output, output, port>
			{
				public:
					output(node &, underlying_type &, real const & x, real const & y);
			};

			class UNIVERSALIS__COMPILER__DYNAMIC_LINK input : public cast::underlying_wrapper<engine::ports::input, input, port>
			{
				protected:
					input(node &, underlying_type &, real const & x, real const & y);
			};
			
			namespace inputs
			{
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK single : public cast::underlying_wrapper<engine::ports::inputs::single, single, input>
				{
					public:
						single(node &, underlying_type &, real const & x, real const & y);
				};

				class UNIVERSALIS__COMPILER__DYNAMIC_LINK multiple : public cast::underlying_wrapper<engine::ports::inputs::multiple, multiple, input>
				{
					public:
						multiple(node &, underlying_type &, real const & x, real const & y);
				};
			}
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
