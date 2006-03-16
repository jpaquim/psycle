// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\file
///\interface psycle::generic::base
#pragma once
#include "generic.hpp"
namespace psycle
{
	namespace generic
	{
		using engine::exception;
		namespace loggers = universalis::operating_system::loggers;
		
		template<typename Typenames>
		class graph : public universalis::compiler::cast::derived<typename Typenames::graph>, public std::set<typename Typenames::node*>
		{
			public:
				graph()
				{
					new_node_signal()   .connect(boost::bind(&graph::on_new_node   , this, _1));
					delete_node_signal().connect(boost::bind(&graph::on_delete_node, this, _1));
				}
				virtual ~graph() throw() {}
			
			///\name signals
			///\{
				public:  boost::signal<void (typename Typenames::node &)> inline & new_node_signal()    throw() { return new_node_signal_; }
				private: boost::signal<void (typename Typenames::node &)>          new_node_signal_;
					
				public:  boost::signal<void (typename Typenames::node &)> inline & delete_node_signal() throw() { return delete_node_signal_; }
				private: boost::signal<void (typename Typenames::node &)>          delete_node_signal_;

				public:  boost::signal<void (typename Typenames::ports::input &, typename Typenames::ports::output &)> inline &    new_connection_signal() throw() { return new_connection_signal_; }
				private: boost::signal<void (typename Typenames::ports::input &, typename Typenames::ports::output &)>             new_connection_signal_;

				public:  boost::signal<void (typename Typenames::ports::input &, typename Typenames::ports::output &)> inline & delete_connection_signal() throw() { return delete_connection_signal_; }
				private: boost::signal<void (typename Typenames::ports::input &, typename Typenames::ports::output &)>          delete_connection_signal_;
			///\}
			
			///\name signal slots
			///\{
				private:
					void on_new_node   (typename Typenames::node & node) { insert(&node); }
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
		class node : public universalis::compiler::cast::derived<typename Typenames::node>, public child_of<typename Typenames::graph>
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
					this->parent().new_node_signal()(*this);
				}
				
				virtual ~node() throw()
				{
					this->parent().delete_node_signal()(*this);
				}

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
						if(loggers::trace()())
						{
							loggers::trace()("generic node on new output port", UNIVERSALIS__COMPILER__LOCATION);
						}
						output_ports_.push_back(&port);
					}
			///\}
			
			///\name ports: inputs
			///\{
				public:
					typedef std::vector<typename Typenames::ports::inputs::single*>        single_input_ports_type;
					/// the input ports owned by this node
					single_input_ports_type inline const &         single_input_ports() const throw() { return single_input_ports_; }
				private:
					single_input_ports_type                        single_input_ports_;
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
						if(loggers::trace()())
						{
							loggers::trace()("generic node on new single input port", UNIVERSALIS__COMPILER__LOCATION);
						}
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
						if(loggers::trace()())
						{
							loggers::trace()("generic node on new multiple input port", UNIVERSALIS__COMPILER__LOCATION);
						}
						multiple_input_port(port); 
					}
			///\}
		};

		template<typename Typenames>
		class port : public universalis::compiler::cast::derived<typename Typenames::port>, public child_of<typename Typenames::node>
		{
			protected:
				port(typename Typenames::node & node) : child_of<typename Typenames::node>(node) {}
			public:
				virtual ~port() throw() {}
		};
		
		namespace ports
		{
			#include <boost/preprocessor/iteration/local.hpp>
			#include <boost/preprocessor/repetition/enum_params.hpp>
			#include <boost/preprocessor/repetition/enum_binary_params.hpp>
			#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
			#include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>
			#include <boost/preprocessor/control/expr_if.hpp>
			#define $limits (1, 9)
			
			template<typename Typenames>
			class output : public universalis::compiler::cast::derived<typename Typenames::ports::output>, public Typenames::port
			{
				private:
					void init()
					{
						if(loggers::trace()())
						{
							loggers::trace()("new generic output port", UNIVERSALIS__COMPILER__LOCATION);
						}
						this->parent().new_output_port_signal()(*this);
					}
				public:
					output(typename output::parent_type & parent)
					:
						Typenames::port(parent)
					{
						init();
					}
					#define $(_, count, __) \
							/*BOOST_EXPR_IF(count, ...)*/ template<BOOST_PP_ENUM_PARAMS(count, typename Xtra)> \
							output(typename output::parent_type & parent BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(count, Xtra, & xtra)) \
							: \
								Typenames::port(parent BOOST_PP_ENUM_TRAILING_PARAMS(count, xtra)) \
							{ \
								init(); \
							}
						#define  BOOST_PP_LOCAL_MACRO(count) $(~, count, ~~)
						#define  BOOST_PP_LOCAL_LIMITS $limits
						#include BOOST_PP_LOCAL_ITERATE()
					#undef $
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
							if(loggers::trace()())
							{
								std::ostringstream s;
								s << "connecting output port internal side to input port";
								loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
							}
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
							if(loggers::trace()())
							{
								std::ostringstream s;
								s << "disconnecting output port internal side from input port";
								loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
							}
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
			class input : public universalis::compiler::cast::derived<typename Typenames::ports::input>, public Typenames::port
			{
				public:
					input(typename input::parent_type & parent) : Typenames::port(parent) {}
					#define $(_, count, __) \
							template<BOOST_PP_ENUM_PARAMS(count, typename Xtra)> \
							input(typename input::parent_type & parent, BOOST_PP_ENUM_BINARY_PARAMS(count, Xtra, & xtra)) \
							: \
								Typenames::port(parent, BOOST_PP_ENUM_PARAMS(count, xtra)) \
							{ \
							}
						#define  BOOST_PP_LOCAL_MACRO(count) $(~, count, ~~)
						#define  BOOST_PP_LOCAL_LIMITS $limits
						#include BOOST_PP_LOCAL_ITERATE()
					#undef $
					virtual ~input() throw() {}
					
					///\name (dis)connection functions
					///\{
						public:
							void connect(typename Typenames::ports::output & output_port) throw(exception)
							{
								assert("ports must belong to different nodes:" && &output_port.parent() != &this->parent());
								assert("nodes of both ports must belong to the same graph:" && &output_port.parent().parent() == &this->parent().parent());
								output_port.connect_internal_side(*this);
								this->connect_internal_side(output_port);
								parent().parent().new_connection_signal()(output_port);
							}
						protected:
							void virtual connect_internal_side(typename Typenames::ports::output &) = 0;
			
						public:
							void virtual disconnect_all() = 0;
							void disconnect(typename Typenames::ports::output & output_port)
							{
								this->disconnect_internal_side(output_port);
								output_port.disconnect_internal_side(*this);
								parent().parent().delete_connection_signal()(output_port);
							}
						protected:
							void virtual disconnect_internal_side(typename Typenames::ports::output &) = 0;
					///\}
			};
			
			namespace inputs
			{
				template<typename Typenames>
				class single : public universalis::compiler::cast::derived<typename Typenames::ports::inputs::single>, public Typenames::ports::input
				{
					private:
						void init()
						{
							if(loggers::trace()())
							{
								loggers::trace()("new single input port", UNIVERSALIS__COMPILER__LOCATION);
							}
							this->parent().new_single_input_port_signal()(*this);
						}
					public:
						single(typename single::parent_type & parent)
						:
							Typenames::ports::input(parent),
							output_port_()
						{
							init();
						}
						#define $(_, count, __) \
								template<BOOST_PP_ENUM_PARAMS(count, typename Xtra)> \
								single(typename single::parent_type & parent, BOOST_PP_ENUM_BINARY_PARAMS(count, Xtra, & xtra)) \
								: \
									Typenames::ports::input(parent, BOOST_PP_ENUM_PARAMS(count, xtra)), \
									output_port_() \
								{ \
									init(); \
								}
							#define  BOOST_PP_LOCAL_MACRO(count) $(~, count, ~~)
							#define  BOOST_PP_LOCAL_LIMITS $limits
							#include BOOST_PP_LOCAL_ITERATE()
						#undef $
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
								if(loggers::trace()())
								{
									std::ostringstream s;
									s << "connecting single input port internal side to output port";
									loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
								}
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
								if(loggers::trace()())
								{
									std::ostringstream s;
									s << "disconnecting single input port internal side from output port";
									loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
								}
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
				class multiple : public universalis::compiler::cast::derived<typename Typenames::ports::inputs::multiple>, public Typenames::ports::input
				{
					private:
						void init()
						{
							if(loggers::trace()())
							{
								loggers::trace()("new multiple input port", UNIVERSALIS__COMPILER__LOCATION);
							}
							this->parent().new_multiple_input_port_signal()(*this);
						}
					public:
						multiple(typename multiple::parent_type & parent)
						:
							Typenames::ports::input(parent)
						{
							init();
						}
						#define $(_, count, __) \
								template<BOOST_PP_ENUM_PARAMS(count, typename Xtra)> \
								multiple(typename multiple::parent_type & parent, BOOST_PP_ENUM_BINARY_PARAMS(count, Xtra, & xtra)) \
								: \
									Typenames::ports::input(parent, BOOST_PP_ENUM_PARAMS(count, xtra)) \
								{ \
									init(); \
								}
							#define  BOOST_PP_LOCAL_MACRO(count) $(~, count, ~~)
							#define  BOOST_PP_LOCAL_LIMITS $limits
							#include BOOST_PP_LOCAL_ITERATE()
						#undef $
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
								if(loggers::trace()())
								{
									std::ostringstream s;
									s << "connecting multiple input port internal side to output port";
									loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
								}
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
								if(loggers::trace()())
								{
									std::ostringstream s;
									s << "disconnecting multiple input port internal side from output port";
									loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
								}
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
			#undef $limits
		}
	}
}
