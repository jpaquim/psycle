// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2006 johan boule <bohan@jabber.org>
// copyright 2004-2006 psycledelics http://psycle.pastnotecut.org

///\file
///\implementation psycle::front_ends::text::main
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
#include <psycle/detail/project.private.hpp>
#include "main.hpp"
#include <psycle/paths.hpp>
#include <psycle/engine/engine.hpp>
#include <psycle/host/host.hpp>
#include <universalis/processor/exception.hpp>
#include <universalis/operating_system/loggers.hpp>
#include <universalis/operating_system/threads/sleep.hpp>
#include <universalis/compiler/typenameof.hpp>
#include <universalis/compiler/exceptions/ellipsis.hpp>
#include <string>
#include <sstream>
#include <iostream>

#if defined PSYCLE__EXPERIMENTAL
	#include <psycle/generic/generic.hpp>
#endif

namespace psycle
{
	namespace front_ends
	{
		namespace text
		{
			void paths()
			{
				#if defined $
					#error "macro clash"
				#endif
				#define $($) loggers::trace()(paths::$().string() + " == " UNIVERSALIS__COMPILER__STRINGIZED($), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS)
				{
					$(bin);
					$(package::lib);
					$(package::share);
					$(package::pixmaps);
					$(package::var);
					$(package::log);
					$(package::etc);
					$(package::home);
				}
				#undef $
			}

			void stuff()
			{
				try
				{
					loggers::information()("########################################### instanciations ##################################################");
					using engine::graph;
					using engine::node;
					#define PSYCLE__FRONT_ENDS__TEXT__MANUAL_CLEANING
					#if !defined PSYCLE__FRONT_ENDS__TEXT__MANUAL_CLEANING
						host::plugin_resolver resolver;
						//graph graph("graph");
						graph & graph(engine::graph::create("graph"));
					#else
						host::plugin_resolver & resolver(*new host::plugin_resolver);
						graph & graph(engine::graph::create("graph"));
					#endif
					node & sine1(resolver("sine", graph, "sine1"));
					node & sine2(resolver("sine", graph, "sine2"));
					node & sine3(resolver("sine", graph, "sine3"));
					node & multiplier(resolver("multiplier", graph, "*"));
					node & additioner(resolver("additioner", graph, "+"));
					node & out(resolver("output.default", graph, "out"));
					loggers::information()("############################################### settings ####################################################");
					{
						out.input_port("in")->events_per_second(44100);
					}
					if(loggers::information()())
					{
						std::ostringstream s;
						s
							<< "################################################ dump #######################################################"
							<< std::endl
							<< graph;
						loggers::information()(s.str());
					}
					loggers::information()("############################################## connections ##################################################");
					{
						additioner.output_port("out")->connect(*out.input_port("in"));
						multiplier.output_port("out")->connect(*additioner.input_port("in"));
						sine2.output_port("out")->connect(*multiplier.input_port("in"));
						sine3.output_port("out")->connect(*multiplier.input_port("in"));
						sine1.output_port("out")->connect(*multiplier.input_port("in"));
						sine1.output_port("out")->connect(*additioner.input_port("in"));
					}
					if(loggers::information()())
					{
						std::ostringstream s;
						s
							<< "################################################ dump #######################################################"
							<< std::endl
							<< graph;
						loggers::information()(s.str());
					}
					loggers::information()("############################################## schedule ########################################################");
					{
						host::schedulers::single_threaded::scheduler scheduler(graph);
						universalis::compiler::numeric<64>::floating_point const seconds(15);
						if(loggers::information()())
						{
							std::ostringstream s;
							s << "will end thread in " << seconds << " seconds ...";
							loggers::information()(s.str());
						}
						scheduler.start();
						universalis::operating_system::threads::sleep(seconds);
						scheduler.stop();
					}
					loggers::information()("############################################# clean up ######################################################");
					#if defined PSYCLE__FRONT_ENDS__TEXT__MANUAL_CLEANING
						loggers::information()("############################################# manual clean up ######################################################");
						delete &graph;
						delete &resolver;
						loggers::information()("############################################# manual clean up done ######################################################");
					#endif
				}
				catch(...)
				{
					loggers::exception()("############################################# exception #####################################################", UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
					throw;
				}
			}

			int main(int /*const*/ argument_count, char /*const*/ * /*const*/ arguments[])
			{
				try
				{
					try
					{
						universalis::operating_system::loggers::multiplex_logger::singleton().add(universalis::operating_system::loggers::stream_logger::default_logger());
						universalis::processor::exception::new_thread("psycle.text");
						{
							if(universalis::operating_system::loggers::information()())
							{
								std::ostringstream s;
								s << paths::package::name() << " " << paths::package::version::string();
								universalis::operating_system::loggers::information()(s.str());
							}
						}
						paths();
						{
							psycle::engine::hello hello;
							loggers::information()(hello.say_hello());
						}
						stuff();
					}
					catch(std::exception const & e)
					{
						if(loggers::exception()())
						{
							std::ostringstream s;
							s << "exception: " << universalis::compiler::typenameof(e) << ": " << e.what();
							loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
						}
						throw;
					}
					catch(...)
					{
						if(loggers::exception()())
						{
							std::ostringstream s;
							s << "exception: " << universalis::compiler::exceptions::ellipsis();
							loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
						}
						throw;
					}
				}
				catch(...)
				{
					std::string s; std::getline(std::cin, s);
					throw;
				}
				return 0;
			}
		}
	}
}
