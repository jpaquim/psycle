// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2006 Johan Boule <bohan@jabber.org>
// copyright 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\file
///\interface psycle::generic
#pragma once
#include "typenames.hpp"
#include <psycle/engine/exception.hpp>
#include <universalis/compiler/cast.hpp>
#include <universalis/operating_system/loggers.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_and_derived.hpp>
#include <boost/call_traits.hpp>
#include <boost/signal.hpp>
#include <boost/bind.hpp>
#include <set>
#include <vector>
#include <algorithm>
namespace psycle
{
	namespace generic
	{
		#include <boost/preprocessor/repetition/repeat.hpp>
		#include <boost/preprocessor/control/expr_if.hpp>
		#include <boost/preprocessor/repetition/enum_params.hpp>
		#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
		#include <boost/preprocessor/repetition/enum_binary_params.hpp>
		#include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>
		#define PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT__MINIMUM UNIVERSALIS__COMPILER__CAST__UNDERLYING_WRAPPER__TEMPLATE_CONSTRUCTORS_LIMIT__MINIMUM
		#if PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT < PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT__MINIMUM
			#undef  PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT
			#define PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT__MINIMUM
		#endif

		using engine::exception;
		namespace loggers = universalis::operating_system::loggers;
		
		template<typename Typenames>
		class graph
		:
			public universalis::compiler::cast::derived<typename Typenames::graph>,
			public std::set<typename Typenames::node*>
		{
			public:
				class generic_access
				{
					friend class graph;
					private:
						#define constructor(_, count, __) \
							BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
							typename Typenames::graph static & create(BOOST_PP_ENUM_BINARY_PARAMS(count, Xtra, & xtra)) \
							{ \
								typename Typenames::graph & instance(*new typename Typenames::graph(BOOST_PP_ENUM_PARAMS(count, xtra))); \
								instance.init(); \
								return instance; \
							}
							BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT, constructor, ~)
						#undef constructor
				};

				#define constructor(_, count, __) \
					BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
					typename Typenames::graph static & create(BOOST_PP_ENUM_BINARY_PARAMS(count, Xtra, & xtra)) \
					{ \
						return generic_access::create(BOOST_PP_ENUM_PARAMS(count, xtra)); \
					}
					BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT, constructor, ~)
				#undef constructor

			protected:
				graph()
				{
					new_node_signal()   .connect(boost::bind(&graph::on_new_node   , this, _1));
			//		delete_node_signal().connect(boost::bind(&graph::on_delete_node, this, _1));
				}

				void virtual init() {}

			public:
				virtual ~graph() throw() {}
			
			///\name signals
			///\{
				public:  boost::signal<void (typename Typenames::node &)> inline & new_node_signal()    throw() { return new_node_signal_; }
				private: boost::signal<void (typename Typenames::node &)>          new_node_signal_;
					
			//	public:  boost::signal<void (typename Typenames::node &)> inline & delete_node_signal() throw() { return delete_node_signal_; }
			//	private: boost::signal<void (typename Typenames::node &)>          delete_node_signal_;

				public:  boost::signal<void (typename Typenames::ports::input &, typename Typenames::ports::output &)> inline &    new_connection_signal() throw() { return new_connection_signal_; }
				private: boost::signal<void (typename Typenames::ports::input &, typename Typenames::ports::output &)>             new_connection_signal_;

				public:  boost::signal<void (typename Typenames::ports::input &, typename Typenames::ports::output &)> inline & delete_connection_signal() throw() { return delete_connection_signal_; }
				private: boost::signal<void (typename Typenames::ports::input &, typename Typenames::ports::output &)>          delete_connection_signal_;
			///\}
			
			///\name signal slots
			///\{
				private:
					void on_new_node   (typename Typenames::node & node) { insert(&node); node.delete_signal().connect(boost::bind(&graph::on_delete_node, this, _1)); }
					void on_delete_node(typename Typenames::node & node) { erase (&node); }
			///\}
		};
		
		template<typename Parent>
		class child_of
		{
			protected:
				child_of(Parent & parent) : parent_(parent) {}
			public:
				typedef Parent parent_type;
				Parent inline & parent() const throw() { return parent_; }
			private:
				Parent        & parent_;
		};

		template<typename Typenames>
		class node
		:
			public universalis::compiler::cast::derived<typename Typenames::node>,
			public child_of<typename Typenames::graph>
		{
			private:
			#if 0
				BOOST_STATIC_ASSERT((boost::is_base_and_derived<child_of<typename Typenames::graph>, typename Typenames::node                   >::value));
				BOOST_STATIC_ASSERT((boost::is_base_and_derived<child_of<typename Typenames::node >, typename Typenames::port                   >::value));
			#endif
				BOOST_STATIC_ASSERT((boost::is_base_and_derived<typename Typenames::port           , typename Typenames::ports::output          >::value));
				BOOST_STATIC_ASSERT((boost::is_base_and_derived<typename Typenames::port           , typename Typenames::ports::input           >::value));
				BOOST_STATIC_ASSERT((boost::is_base_and_derived<typename Typenames::ports::input   , typename Typenames::ports::inputs::single  >::value));
				BOOST_STATIC_ASSERT((boost::is_base_and_derived<typename Typenames::ports::input   , typename Typenames::ports::inputs::multiple>::value));

			public:
				class generic_access
				{
					friend class node;
					private:
						#define constructor(_, count, __) \
							BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
							typename Typenames::node static & create(typename Typenames::graph & graph BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(count, Xtra, & xtra)) \
							{ \
								typename Typenames::node & instance(*new typename Typenames::node(graph BOOST_PP_ENUM_TRAILING_PARAMS(count, xtra))); \
								instance.init(); \
								return instance; \
							}
							BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT, constructor, ~)
						#undef constructor

						#define constructor(_, count, __) \
							template<typename Type BOOST_PP_ENUM_TRAILING_PARAMS(count, typename Xtra)> \
							Type static & create(typename Typenames::graph & graph BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(count, Xtra, & xtra)) \
							{ \
								Type & instance(*new Type(graph BOOST_PP_ENUM_TRAILING_PARAMS(count, xtra))); \
								instance.init(); \
								return instance; \
							}
							BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT, constructor, ~)
						#undef constructor
				};

				#define constructor(_, count, __) \
					BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
					typename Typenames::node static & create(typename Typenames::graph & graph BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(count, Xtra, & xtra)) \
					{ \
						return generic_access::create(graph BOOST_PP_ENUM_TRAILING_PARAMS(count, xtra)); \
					}
					BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT, constructor, ~)
				#undef constructor

				#define constructor(_, count, __) \
					template<typename Type BOOST_PP_ENUM_TRAILING_PARAMS(count, typename Xtra)> \
					Type static & create(typename Typenames::graph & graph BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(count, Xtra, & xtra)) \
					{ \
						return generic_access::template create<Type BOOST_PP_ENUM_TRAILING_PARAMS(count, Xtra)>(graph BOOST_PP_ENUM_TRAILING_PARAMS(count, xtra)); \
					}
					BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT, constructor, ~)
				#undef constructor
				
			protected:
				node(typename Typenames::graph & graph)
				:
					child_of<typename Typenames::graph>(graph),
					multiple_input_port_()
				{
					if(loggers::trace()())
					{
						loggers::trace()("new generic node", UNIVERSALIS__COMPILER__LOCATION);
					}
					new_output_port_signal()        .connect(boost::bind(&node::        on_new_output_port, this, _1));
					new_single_input_port_signal()  .connect(boost::bind(&node::  on_new_single_input_port, this, _1));
					new_multiple_input_port_signal().connect(boost::bind(&node::on_new_multiple_input_port, this, _1));
				}

				void virtual init()
				{
					if(loggers::trace()())
					{
						loggers::trace()("generic node init", UNIVERSALIS__COMPILER__LOCATION);
					}
					this->parent().new_node_signal()(*this);
				}
				
			///\name destruction
			///\{
				virtual ~node() throw()
				{
					//this->parent().delete_node_signal()(*this);
					delete_signal()(*this);
				}

				public:  boost::signal<void (node &)> inline & delete_signal() throw() { return delete_signal_; }
				private: boost::signal<void (node &)>          delete_signal_;
			///\}

			///\name ports: outputs
			///\{
				public:
					typedef std::vector<typename Typenames::ports::output*> output_ports_type;
					/// the output ports owned by this node
					output_ports_type inline const &  output_ports() const throw() { return output_ports_; }
				private:
					output_ports_type                 output_ports_;
			///\}

			///\name ports: outputs: signals
			///\{
				public:  boost::signal<void (typename Typenames::ports::output &)> inline & new_output_port_signal() throw() { return new_output_port_signal_; }
				private: boost::signal<void (typename Typenames::ports::output &)>          new_output_port_signal_;
			///\}
			
			///\name ports: outputs: signals: slots
			///\{
				private:
					void on_new_output_port(typename Typenames::ports::output & port)
					{
						output_ports_.push_back(&port);
					}
			///\}
			
			///\name ports: inputs
			///\{
				public:
					typedef std::vector<typename Typenames::ports::inputs::single*> single_input_ports_type;
					/// the input ports owned by this node
					single_input_ports_type inline const & single_input_ports() const throw() { return single_input_ports_; }
				private:
					single_input_ports_type                single_input_ports_;
			///\}
			
			///\name ports: inputs: single: signals
			///\{
				public:  boost::signal<void (typename Typenames::ports::inputs::single &)> inline & new_single_input_port_signal() throw() { return new_single_input_port_signal_; }
				private: boost::signal<void (typename Typenames::ports::inputs::single &)>          new_single_input_port_signal_;
			///\}
			
			///\name ports: inputs: single: signals: slots
			///\{
				private:
					void on_new_single_input_port(typename Typenames::ports::inputs::single & port)
					{
						single_input_ports_.push_back(&port); 
					}
			///\}

			///\name ports: inputs: multiple
			///\{
				public:
					/// the multiple input port owned by this node, if any, or else 0
					typename Typenames::ports::inputs::multiple inline * const multiple_input_port() const throw() { return multiple_input_port_; }
				private:
					typename Typenames::ports::inputs::multiple        *       multiple_input_port_;
				public: // node_friends protected:
					/// gives a multiple input port for this node
					///\pre the node doesn't already have a multiple input port
					void                                        inline         multiple_input_port(typename Typenames::ports::inputs::multiple & multiple_input_port) throw() { assert(!this->multiple_input_port()); this->multiple_input_port_ = &multiple_input_port; }
			///\}
			
			///\name ports: inputs: multiple: signals
			///\{
				public:  boost::signal<void (typename Typenames::ports::inputs::multiple &)> inline & new_multiple_input_port_signal() throw() { return new_multiple_input_port_signal_; }
				private: boost::signal<void (typename Typenames::ports::inputs::multiple &)>          new_multiple_input_port_signal_;
			///\}
			
			///\name ports: inputs: multiple: signals: slots
			///\{
				private:
					void on_new_multiple_input_port(typename Typenames::ports::inputs::multiple & port)
					{
						multiple_input_port(port); 
					}
			///\}
		};

		template<typename Typenames>
		class port
		:
			public universalis::compiler::cast::derived<typename Typenames::port>,
			public child_of<typename Typenames::node>
		{
			public:
				class generic_access
				{
					friend class port;
					private:
						#define constructor(_, count, __) \
							template<typename Type BOOST_PP_ENUM_TRAILING_PARAMS(count, typename Xtra)> \
							Type static & create(typename Typenames::node & node BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(count, Xtra, & xtra)) \
							{ \
								Type & instance(*new Type(node BOOST_PP_ENUM_TRAILING_PARAMS(count, xtra))); \
								instance.init(); \
								return instance; \
							}
							BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT, constructor, ~)
						#undef constructor
				};

				#define constructor(_, count, __) \
					template<typename Type BOOST_PP_ENUM_TRAILING_PARAMS(count, typename Xtra)> \
					Type static & create(typename Typenames::node & node BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(count, Xtra, & xtra)) \
					{ \
						return generic_access::template create<Type>(node BOOST_PP_ENUM_TRAILING_PARAMS(count, xtra)); \
					}
					BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT, constructor, ~)
				#undef constructor

			protected:
				port(typename Typenames::node & node) : child_of<typename Typenames::node>(node) {}
				void virtual init() = 0;
			public:
				virtual ~port() throw() {}
		};
		
		namespace ports
		{
			template<typename Typenames>
			class output
			:
				public universalis::compiler::cast::derived<typename Typenames::ports::output>,
				public Typenames::port
			{
				public:
					#define constructor(_, count, __) \
						BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
						typename Typenames::ports::output static & create(typename Typenames::node & node BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(count, Xtra, & xtra)) \
						{ \
							return Typenames::port::template create<typename Typenames::ports::output>(node BOOST_PP_ENUM_TRAILING_PARAMS(count, xtra)); \
						}
						BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT, constructor, ~)
					#undef constructor
				protected:
					#define constructor(_, count, __) \
						BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
						output(typename output::parent_type & parent BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(count, Xtra, & xtra)) \
						: Typenames::port(parent BOOST_PP_ENUM_TRAILING_PARAMS(count, xtra)) {}
						BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT, constructor, ~)
					#undef constructor

					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES init()
					{
						this->parent().new_output_port_signal()(*this);
					}

				public:
					virtual ~output() throw()
					{
						disconnect_all();
					}
					
				///\name connected input ports
				///\{
					public:
						typedef std::vector<typename Typenames::ports::input*> input_ports_type;
					public:
						input_ports_type inline const & input_ports() const throw() { return input_ports_; }
				//	protected:
				//		input_ports_type inline       & input_ports()       throw() { return input_ports_; }
					private:
						input_ports_type                input_ports_;
				///\}
				
	
				///\name (dis)connection functions
				///\{
					public:
						void connect(typename Typenames::ports::input & input_port) throw(exception)
						{
							input_port.connect(*this);
							this->parent().parent().new_connection_signal()(input_port, *this);
						}
					public: // private:
						void connect_internal_side(typename Typenames::ports::input & input_port)
						{
							typename input_ports_type::iterator i(std::find(input_ports_.begin(), input_ports_.end(), &input_port));
							if(i != input_ports_.end())
							{
								if(loggers::warning()())
								{
									std::ostringstream s;
									s << "already connected";
									loggers::warning()(s.str());
								}
								return;
							}
							input_ports_.push_back(&input_port);
						}
					public:
						void disconnect_all()
						{
							while(!input_ports_.empty()) disconnect(*input_ports_.back());
						}
						void disconnect(typename Typenames::ports::input & input_port)
						{
							input_port.disconnect(*this);
							this->parent().parent().delete_connection_signal()(input_port, *this);
						}
					public: // private:
						void disconnect_internal_side(typename Typenames::ports::input & input_port)
						{
							typename input_ports_type::iterator i(std::find(input_ports_.begin(), input_ports_.end(), &input_port));
							if(i == input_ports_.end())
							{
								if(loggers::warning()())
								{
									std::ostringstream s;
									s << "was not connected";
									loggers::warning()(s.str());
								}
								return;
							}
							input_ports_.erase(i);
						}
				///\}
			};

			template<typename Typenames>
			class input
			:
				public universalis::compiler::cast::derived<typename Typenames::ports::input>,
				public Typenames::port
			{
				protected:
					#define constructor(_, count, __) \
						BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
						input(typename input::parent_type & parent BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(count, Xtra, & xtra)) \
						: Typenames::port(parent BOOST_PP_ENUM_TRAILING_PARAMS(count, xtra)) {}
						BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT, constructor, ~)
					#undef constructor

				public:
					virtual ~input() throw() {}
					
					///\name (dis)connection functions
					///\{
						public:
							void connect(typename Typenames::ports::output & output_port) throw(exception)
							{
								assert("ports must belong to different nodes:" && &output_port.parent() != &this->parent());
								assert("nodes of both ports must belong to the same graph:" && &output_port.parent().parent() == &this->parent().parent());
								output_port.connect_internal_side(*this);
								Typenames::port::connect(output_port);
								this->connect_internal_side(output_port);
								this->parent().parent().new_connection_signal()(*this, output_port);
							}
						protected:
							void virtual connect_internal_side(typename Typenames::ports::output &) = 0;
			
						public:
							void virtual disconnect_all() = 0;
							void disconnect(typename Typenames::ports::output & output_port)
							{
								this->disconnect_internal_side(output_port);
								output_port.disconnect_internal_side(*this);
								this->parent().parent().delete_connection_signal()(*this, output_port);
							}
						protected:
							void virtual disconnect_internal_side(typename Typenames::ports::output &) = 0;
					///\}
			};
			
			namespace inputs
			{
				template<typename Typenames>
				class single
				:
					public universalis::compiler::cast::derived<typename Typenames::ports::inputs::single>,
					public Typenames::ports::input
				{
					public:
						#define constructor(_, count, __) \
							BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
							typename Typenames::ports::inputs::single static & create(typename Typenames::node & node BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(count, Xtra, & xtra)) \
							{ \
								return Typenames::ports::input::template create<typename Typenames::ports::inputs::single>(node BOOST_PP_ENUM_TRAILING_PARAMS(count, xtra)); \
							}
							BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT, constructor, ~)
						#undef constructor
					protected:
						#define constructor(_, count, __) \
							BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
							single(typename single::parent_type & parent BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(count, Xtra, & xtra)) \
							: \
								Typenames::ports::input(parent BOOST_PP_ENUM_TRAILING_PARAMS(count, xtra)), \
								output_port_() \
							{}
							BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT, constructor, ~)
						#undef constructor

						void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES init()
						{
							this->parent().new_single_input_port_signal()(*this);
						}

					public:
						virtual ~single() throw()
						{
							disconnect_all();
						}
						
					///\name connected output port
					///\{
						public:
							typename Typenames::ports::output inline * const & output_port() const throw() { return output_port_; }
						private:
							typename Typenames::ports::output        *         output_port_;
					///\}
					
					///\name (dis)connection functions
					///\{
						public:
							void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES disconnect_all()
							{
								if(output_port_) disconnect(*output_port_);
							}
						
						protected:
							void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES connect_internal_side(typename Typenames::ports::output & output_port)
							{
								if(&output_port == this->output_port_)
								{
									if(loggers::warning()())
									{
										std::ostringstream s;
										s << "already connected";
										loggers::warning()(s.str());
									}
									return;
								}
								this->output_port_ = &output_port;
							}
							void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES disconnect_internal_side(typename Typenames::ports::output & output_port)
							{
								if(&output_port != this->output_port_)
								{
									if(loggers::warning()())
									{
										std::ostringstream s;
										s << "was not connected";
										loggers::warning()(s.str());
									}
									return;
								}
								this->output_port_ = 0;
							}
					///\}
				};

				template<typename Typenames>
				class multiple
				:
					public universalis::compiler::cast::derived<typename Typenames::ports::inputs::multiple>,
					public Typenames::ports::input
				{
					public:
						#define constructor(_, count, __) \
							BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
							typename Typenames::ports::inputs::multiple static & create(typename Typenames::node & node BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(count, Xtra, & xtra)) \
							{ \
								return Typenames::ports::input::template create<typename Typenames::ports::inputs::multiple>(node BOOST_PP_ENUM_TRAILING_PARAMS(count, xtra)); \
							}
							BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT, constructor, ~)
						#undef constructor
					protected:
						#define constructor(_, count, __) \
							BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
							multiple(typename multiple::parent_type & parent BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(count, Xtra, & xtra)) \
							: Typenames::ports::input(parent BOOST_PP_ENUM_TRAILING_PARAMS(count, xtra)) {}
							BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT, constructor, ~)
						#undef constructor

						void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES init()
						{
							this->parent().new_multiple_input_port_signal()(*this);
						}

					public:
						virtual ~multiple() throw()
						{
							disconnect_all();
						}
						
					///\name connected output ports
					///\{
						public:
							typedef std::vector<typename Typenames::ports::output*> output_ports_type;
						public:
							output_ports_type inline const & output_ports() const throw() { return output_ports_; }
					//	protected:
					//		output_ports_type inline       & output_ports()       throw() { return output_ports_; }
						private:
							output_ports_type                output_ports_;
					///\}

					///\name (dis)connection functions
					///\{					
						public:	
							void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES disconnect_all()
							{
								while(!output_ports_.empty()) disconnect(*output_ports_.back());
							}
						protected:
							void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES connect_internal_side(typename Typenames::ports::output & output_port)
							{
								typename output_ports_type::iterator i(std::find(output_ports_.begin(), output_ports_.end(), &output_port));
								if(i != output_ports_.end())
								{
									if(loggers::warning()())
									{
										std::ostringstream s;
										s << "already connected";
										loggers::warning()(s.str());
									}
									return;
								}
								output_ports_.push_back(&output_port);
							}
							void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES disconnect_internal_side(typename Typenames::ports::output & output_port)
							{
								typename output_ports_type::iterator i(std::find(output_ports_.begin(), output_ports_.end(), &output_port));
								if(i == output_ports_.end())
								{
									if(loggers::warning()())
									{
										std::ostringstream s;
										s << "was not connected";
										loggers::warning()(s.str());
									}
									return;
								}
								output_ports_.erase(i);
							}
					///\}
				};
			}
		}
	}
}
