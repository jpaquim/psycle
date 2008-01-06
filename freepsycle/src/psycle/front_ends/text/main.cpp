// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\implementation psycle::front_ends::text::main
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "main.hpp"
#include <psycle/paths.hpp>
#include <psycle/engine/engine.hpp>
#include <psycle/plugins/pulse.hpp>
#include <psycle/host/host.hpp>
#include <universalis/processor/exception.hpp>
#include <universalis/operating_system/loggers.hpp>
#include <universalis/compiler/typenameof.hpp>
#include <universalis/compiler/exceptions/ellipsis.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <thread>
namespace psycle { namespace front_ends { namespace text {

void paths() {
	#if defined $
		#error "macro clash"
	#endif
	#define $($) loggers::trace()(paths::$()./*native_directory_*/string() + " == " UNIVERSALIS__COMPILER__STRINGIZED($), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS)
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

void stuff() {
	try {
		loggers::information()("########################################### instanciations ##################################################");
		using engine::graph;
		using engine::node;
		//#define PSYCLE__FRONT_ENDS__TEXT__MANUAL_CLEANING
		#if !defined PSYCLE__FRONT_ENDS__TEXT__MANUAL_CLEANING
			// on stack
			host::plugin_resolver resolver;
			graph::create_on_stack stack_graph("graph"); graph & graph(stack_graph);
		#else
			// on heap
			host::plugin_resolver & resolver(*new host::plugin_resolver);
			graph & graph(engine::graph::create_on_heap("graph"));
		#endif

		node & out(resolver("output", graph, "out"));
		node & additioner(resolver("additioner", graph, "+"));
		//node & multiplier(resolver("multiplier", graph, "*"));

		node & sine1(resolver("sine", graph, "sine1"));
		node & sine2(resolver("sine", graph, "sine2"));
		node & sine3(resolver("sine", graph, "sine3"));

		plugins::pulse & pulse1(static_cast<plugins::pulse&>(resolver("pulse", graph, "pulse1").node()));
		plugins::pulse & pulse2(static_cast<plugins::pulse&>(resolver("pulse", graph, "pulse2").node()));
		plugins::pulse & pulse3(static_cast<plugins::pulse&>(resolver("pulse", graph, "pulse3").node()));

		float freq(200);
		float freq2(400);
		float freq3(800);

		loggers::information()("############################################### settings ####################################################");
		{
			engine::real const events_per_second(44100), beats_per_second(1);
			out.input_port("in")->events_per_second(events_per_second);
			pulse1.beats_per_second(beats_per_second);
			pulse2.beats_per_second(beats_per_second);
			pulse3.beats_per_second(beats_per_second);
		}
		if(loggers::information()()) {
			std::ostringstream s;
			s
				<< "################################################ dump #######################################################"
				<< std::endl
				<< graph;
			loggers::information()(s.str());
		}
		loggers::information()("############################################## connections ##################################################");
		{
			/*
			|
			| (out)---(+)-------\
			|   |               |
			|   \------(*)----(sine1)---(pulse1)
			|          | \
			|          |  \---(sine2)---(pulse2)
			|          |
			|          \------(sine3)---(pulse3)
			|
			| pulse1 > sine1 > + > out
			| pulse1 > sine1 > * > out
			| pulse2 > sine2 > * > out
			| pulse2 > sine2 > * > out
			|
			*/
			
			additioner.output_port("out")->connect(*out.input_port("in"));
			//multiplier.output_port("out")->connect(*additioner.input_port("in"));

			sine1.output_port("out")->connect(*additioner.input_port("in"));
			sine2.output_port("out")->connect(*additioner.input_port("in"));
			sine3.output_port("out")->connect(*additioner.input_port("in"));
			//sine1.output_port("out")->connect(*additioner.input_port("in"));
			//sine1.output_port("out")->connect(*multiplier.input_port("in"));
			//sine2.output_port("out")->connect(*multiplier.input_port("in"));
			//sine3.output_port("out")->connect(*multiplier.input_port("in"));

			sine1.input_port("frequency")->connect(*pulse1.output_port("out"));
			sine2.input_port("frequency")->connect(*pulse2.output_port("out"));
			sine3.input_port("frequency")->connect(*pulse3.output_port("out"));
		}
		if(loggers::information()()) {
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
			std::seconds const seconds(60);
			{
				unsigned int const notes(4000);
				engine::real const duration(engine::real(seconds.get_count()) / notes);
				engine::real beat(0);
				float ratio(1.1);
				for(unsigned int note(0); note < notes; ++note) {
					//std::clog << beat << ' ' << freq << ' ' << freq2 << ' ' << freq3 << '\n';
					pulse1.add_event(beat, freq);
					pulse2.add_event(beat, freq2 * 1.1);
					pulse3.add_event(beat, freq3 * 1.17);
					freq *= ratio;
					if(freq > 5000) { freq /= 15; ratio *= 1.05; }
					freq2 *= ratio * ratio;
					if(freq2 > 5000) freq2 /= 15;
					freq3 *= ratio * ratio * ratio;
					if(freq3 > 5000) freq3 /= 15;
					if(ratio > 1.5) ratio -= 0.5;
					if(ratio < 1.01) ratio += 0.01;
					beat += duration;
				}
			}
			if(loggers::information()()) {
				std::ostringstream s;
				s << "will end scheduler thread in " << seconds.get_count() << " seconds ...";
				loggers::information()(s.str());
			}
			scheduler.start();
			std::this_thread::sleep(seconds);
			scheduler.stop();
		}
		loggers::information()("############################################# clean up ######################################################");
		#if defined PSYCLE__FRONT_ENDS__TEXT__MANUAL_CLEANING
			loggers::information()("############################################# manual clean up ######################################################");
			graph.free_heap();
			delete &resolver;
			loggers::information()("############################################# manual clean up done ######################################################");
		#endif
	} catch(...) {
		loggers::exception()("############################################# exception #####################################################", UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
		throw;
	}
}

int main(int /*const*/ argument_count, char /*const*/ * /*const*/ arguments[]) {
	try {
		try {
			universalis::operating_system::loggers::multiplex_logger::singleton().add(universalis::operating_system::loggers::stream_logger::default_logger());
			universalis::processor::exception::install_handler_in_thread("psycle.text");
			{
				if(universalis::operating_system::loggers::information()()) {
					std::ostringstream s;
					s << paths::package::name() << " " << paths::package::version::string();
					universalis::operating_system::loggers::information()(s.str());
				}
			}
			paths();
			#if 1
				stuff();
			#else // some weird multithreaded test
				std::thread t1(stuff);
				std::thread t2(stuff);
				t1.join();
				t2.join();
			#endif
		} catch(std::exception const & e) {
			if(loggers::exception()()) {
				std::ostringstream s;
				s << "exception: " << universalis::compiler::typenameof(e) << ": " << e.what();
				loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
			}
			throw;
		} catch(...) {
			if(loggers::exception()()) {
				std::ostringstream s;
				s << "exception: " << universalis::compiler::exceptions::ellipsis();
				loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
			}
			throw;
		}
	} catch(...) {
		std::string s; std::getline(std::cin, s);
		throw;
	}
	return 0;
}

}}}

int main(int /*const*/ argument_count, char /*const*/ * /*const*/ arguments[]) {
	psycle::front_ends::text::main(argument_count, arguments);
}

