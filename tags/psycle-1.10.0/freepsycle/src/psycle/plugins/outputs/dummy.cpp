// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2011 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

#include <psycle/detail/project.private.hpp>
#include "dummy.hpp"
#include <universalis/os/thread_name.hpp>
namespace psycle { namespace plugins { namespace outputs {

using engine::exceptions::runtime_error;

PSYCLE__PLUGINS__NODE_INSTANTIATOR(dummy)

dummy::dummy(class plugin_library_reference & plugin_library_reference, name_type const & name)
:
	resource(plugin_library_reference, name),
	free_wheeling_(),
	opened_(),
	thread_(),
	in_(*this, "in"),
	amp_(*this, "amplification", 1)
{}

void dummy::do_open() {
	resource::do_open();
	loggers::warning()("This is the dummy output plugin. You will hear no sound.", UNIVERSALIS__COMPILER__LOCATION);
	opened_ = true;
}

bool dummy::opened() const {
	return opened_;
}

void dummy::do_start() {
	resource::do_start();
	sleep_ = static_cast<chrono::nanoseconds::rep>(1e9 * graph().events_per_buffer() / in_port().events_per_second());
	io_ready(false);
	stop_requested_ = false;
	// start the thread
	thread_ = new thread(boost::bind(&dummy::thread_function, this));
}

bool dummy::started() const {
	if(!opened()) return false;
	return thread_;
}

void dummy::thread_function() {
	if(loggers::information()()) loggers::information()("thread started", UNIVERSALIS__COMPILER__LOCATION);

	// set thread name
	universalis::os::thread_name thread_name(universalis::compiler::typenameof(*this) + "#" + qualified_name());

	// install cpu/os exception handler/translator
	universalis::cpu::exceptions::install_handler_in_thread();
	
	try {
		try {
			thread_loop();
		} catch(...) {
			loggers::exception()("caught exception in poller thread", UNIVERSALIS__COMPILER__LOCATION);
			throw;
		}
	} catch(std::exception const & e) {
		if(loggers::exception()()) {
			std::ostringstream s;
			s << "exception: " << universalis::compiler::typenameof(e) << ": " << e.what();
			loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		throw;
	} catch(...) {
		if(loggers::exception()()) {
			std::ostringstream s;
			s << "exception: " << universalis::compiler::exceptions::ellipsis_desc();
			loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		throw;
	}
	loggers::information()("thread " + qualified_name() + " terminated", UNIVERSALIS__COMPILER__LOCATION);
}

void dummy::thread_loop() {
	while(true) {
		{ scoped_lock lock(mutex_);
			if(stop_requested_) return;
		}
		if(!free_wheeling_) this_thread::sleep_for(sleep_);
		if(loggers::trace()()) loggers::trace()("io ready: true", UNIVERSALIS__COMPILER__LOCATION);
		io_ready(true);
		condition_.notify_one();
	}
}

void dummy::do_process() {
	if(!in_port()) return;
	{ scoped_lock lock(mutex_);
		if(false && loggers::warning()() && !io_ready()) loggers::warning()("blocking", UNIVERSALIS__COMPILER__LOCATION);
		while(!io_ready()) condition_.wait(lock);
	}
	if(loggers::trace()()) loggers::trace()("io ready: false", UNIVERSALIS__COMPILER__LOCATION);
	io_ready(false);
}

void dummy::do_stop() {
	if(loggers::information()()) loggers::information()("terminating and joining thread ...", UNIVERSALIS__COMPILER__LOCATION);
	if(!thread_) {
		if(loggers::information()()) loggers::information()("thread was not running", UNIVERSALIS__COMPILER__LOCATION);
		return;
	}
	{ scoped_lock lock(mutex_);
		stop_requested_ = true;
	}
	condition_.notify_one();
	thread_->join();
	if(loggers::information()()) loggers::information()("thread joined", UNIVERSALIS__COMPILER__LOCATION);
	delete thread_; thread_ = 0;
	resource::do_stop();
}

void dummy::do_close() {
	opened_ = false;
	resource::do_close();
}

dummy::~dummy() throw() {
	close();
}

}}}
