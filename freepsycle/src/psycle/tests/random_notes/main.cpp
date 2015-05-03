// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2011 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#include <psycle/detail/project.private.hpp>
#include "main.hpp"
#include "score2.hpp"
#include <psycle/engine.hpp>
#include <psycle/host/plugin_resolver.hpp>
#include <psycle/host/scheduler.hpp>
#include <universalis/os/thread_name.hpp>
#include <universalis/stdlib/thread.hpp>
#include <universalis/stdlib/chrono.hpp>
#include <string>
#include <sstream>
#include <iostream>
namespace psycle { namespace tests { namespace random_notes {

typedef score2 score_type;
using namespace universalis::stdlib;

void play() {
	try {
		loggers::information()("########################################### instanciations ##################################################");
		using engine::graph;
		using engine::node;
		host::plugin_resolver resolver;
		class graph graph("graph");

		std::string output_plugin_name("output");
		{ // output env var
			char const * const env = std::getenv("PSYCLE_OUTPUT");
			if(env) {
				std::stringstream s;
				s << env;
				s >> output_plugin_name;
			}
		}
		node & out = resolver(output_plugin_name, "out");
		out.graph(graph);
		score_type score(resolver, graph);

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
			score.connect(out);
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
			host::scheduler scheduler(graph);
			std::size_t threads(thread::hardware_concurrency());
			{ // thread count env var
				char const * const env(std::getenv("PSYCLE_THREADS"));
				if(env) {
					std::stringstream s;
					s << env;
					s >> threads;
				}
			}
			scheduler.threads(threads);
				
			if(loggers::information()()) loggers::information()("generating input ...");
			chrono::seconds const seconds(60);
			score.generate();
			if(loggers::information()()) loggers::information()("generating input ... done");
			if(loggers::information()()) {
				std::ostringstream s;
				s << "will end scheduler thread in " << seconds.count() << " seconds ...";
				loggers::information()(s.str());
			}
			scheduler.start();
			this_thread::sleep_for(seconds);
			scheduler.stop();
		}
		loggers::information()("############################################# clean up ######################################################");
		{
			delete &out;
		}
	} catch(...) {
		loggers::exception()("############################################# exception #####################################################", UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
		throw;
	}
}

int main(int /*const*/ argument_count, char /*const*/ * /*const*/ arguments[]) {
	try {
		try {
			universalis::os::loggers::multiplex_logger::singleton().add(universalis::os::loggers::stream_logger::default_logger());
			universalis::os::thread_name thread_name("main");
			universalis::cpu::exceptions::install_handler_in_thread();
			#if 1
				play();
			#else // some weird multithreaded test
				std::thread t1(play);
				std::thread t2(play);
				std::thread t3(play);
				std::thread t4(play);
				std::thread t5(play);
				std::thread t6(play);
				std::thread t7(play);
				std::thread t8(play);
				std::thread t9(play);
				std::thread t10(play);
				t1.join();
				t2.join();
				t3.join();
				t4.join();
				t5.join();
				t6.join();
				t7.join();
				t8.join();
				t9.join();
				t10.join();
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
				s << "exception: " << universalis::compiler::exceptions::ellipsis_desc();
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
	psycle::tests::random_notes::main(argument_count, arguments);
}
