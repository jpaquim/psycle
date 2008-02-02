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
#include <psycle/plugins/sequence.hpp>
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

		node & sine1(resolver("sine", graph, "sine1"));
		node & sine2(resolver("sine", graph, "sine2"));
		node & sine3(resolver("sine", graph, "sine3"));

		//plugins::sequence & freq1(node::virtual_factory_access::create_on_heap<plugins::sequence>(graph, "freq1"));
		plugins::sequence & freq1(static_cast<plugins::sequence&>(resolver("sequence", graph, "freq1").node()));
		plugins::sequence & freq2(static_cast<plugins::sequence&>(resolver("sequence", graph, "freq2").node()));
		plugins::sequence & freq3(static_cast<plugins::sequence&>(resolver("sequence", graph, "freq3").node()));

		node & decay1(resolver("decay", graph, "decay1"));
		node & decay2(resolver("decay", graph, "decay2"));
		node & decay3(resolver("decay", graph, "decay3"));

		plugins::sequence & sequence1(static_cast<plugins::sequence&>(resolver("sequence", graph, "sequence1").node()));
		plugins::sequence & sequence2(static_cast<plugins::sequence&>(resolver("sequence", graph, "sequence2").node()));
		plugins::sequence & sequence3(static_cast<plugins::sequence&>(resolver("sequence", graph, "sequence3").node()));

		plugins::sequence & decay_sequence1(static_cast<plugins::sequence&>(resolver("sequence", graph, "decay_sequence1").node()));
		plugins::sequence & decay_sequence2(static_cast<plugins::sequence&>(resolver("sequence", graph, "decay_sequence2").node()));
		plugins::sequence & decay_sequence3(static_cast<plugins::sequence&>(resolver("sequence", graph, "decay_sequence3").node()));

		engine::real const events_per_second(44100), beats_per_second(1);

		loggers::information()("############################################### settings ####################################################");
		{
			out.input_port("in")->events_per_second(events_per_second);
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
			additioner.output_port("out")->connect(*out.input_port("in"));

			sine1.output_port("out")->connect(*additioner.input_port("in"));
			sine2.output_port("out")->connect(*additioner.input_port("in"));
			sine3.output_port("out")->connect(*additioner.input_port("in"));

			sine1.input_port("frequency")->connect(*freq1.output_port("out"));
			sine2.input_port("frequency")->connect(*freq2.output_port("out"));
			sine3.input_port("frequency")->connect(*freq3.output_port("out"));

			sine1.input_port("amplitude")->connect(*decay1.output_port("out"));
			sine2.input_port("amplitude")->connect(*decay2.output_port("out"));
			sine3.input_port("amplitude")->connect(*decay3.output_port("out"));

			decay1.input_port("pulse")->connect(*sequence1.output_port("out"));
			decay2.input_port("pulse")->connect(*sequence2.output_port("out"));
			decay3.input_port("pulse")->connect(*sequence3.output_port("out"));

			decay1.input_port("decay")->connect(*decay_sequence1.output_port("out"));
			decay2.input_port("decay")->connect(*decay_sequence2.output_port("out"));
			decay3.input_port("decay")->connect(*decay_sequence3.output_port("out"));
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
				unsigned int const notes(10000);
				engine::real beat(0);
				engine::real duration(0.1 / beats_per_second);
				float slowdown(0.01);
				float f1(200), f2(400), f3(800);
				float ratio(1.1);
				for(unsigned int note(0); note < notes; ++note) {
					//std::clog << beat << ' ' << f1 << ' ' << f2 << ' ' << f3 << '\n';
					
					engine::real const b1(beat), b2(beat * 1.1), b3(beat * 1.2);
					
					freq1.insert_event(b1, f1);
					freq2.insert_event(b2 * 1.1, f2 * 1.1);
					freq3.insert_event(b3 * 1.2, f3 * 1.17);
					
					sequence1.insert_event(b1, 0.3);
					sequence2.insert_event(b2, 0.3);
					sequence3.insert_event(b3, 0.3);

					decay_sequence1.insert_event(b1, 0.0001);
					decay_sequence2.insert_event(b2, 0.0001);
					decay_sequence3.insert_event(b3, 0.0001);

					f1 *= ratio;
					if(f1 > 5000) { f1 /= 15; ratio *= 1.05; }
					f2 *= ratio * ratio;
					if(f2 > 5000) f2 /= 15;
					f3 *= ratio * ratio * ratio;
					if(f3 > 5000) f3 /= 15;
					if(ratio > 1.5) ratio -= 0.5;
					if(ratio < 1.01) ratio += 0.01;
					beat += duration;
					duration += duration * slowdown;
					if(
						duration > 0.25  / beats_per_second ||
						duration < 0.001 / beats_per_second
					) slowdown = -slowdown;
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

