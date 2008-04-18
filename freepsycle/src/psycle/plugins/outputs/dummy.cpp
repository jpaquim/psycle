// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2008 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\implementation psycle::plugins::outputs::dummy
#include <psycle/detail/project.private.hpp>
#include "dummy.hpp"
#include <diversalis/processor.hpp>
#include <universalis/processor/exception.hpp>
#include <universalis/operating_system/exceptions/code_description.hpp>
namespace psycle { namespace plugins { namespace outputs {

	using engine::exceptions::runtime_error;

	PSYCLE__PLUGINS__NODE_INSTANTIATOR(dummy)

	dummy::dummy(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, const std::string & name) throw(engine::exception)
	:
		resource(plugin_library_reference, graph, name),
		free_wheeling_(),
		opened_(),
		thread_()
	{
		engine::ports::inputs::single::create_on_heap(*this, "in");
		engine::ports::inputs::single::create_on_heap(*this, "amplification", boost::cref(1));
	}

	void dummy::do_open() throw(engine::exception) {
		resource::do_open();
		loggers::warning()("This is the dummy output plugin. You will hear no sound.", UNIVERSALIS__COMPILER__LOCATION);
		opened_ = true;
	}

	bool dummy::opened() const {
		return opened_;
	}

	void dummy::do_start() throw(engine::exception) {
		resource::do_start();
		sleep_ = static_cast<std::nanoseconds::tick_type>(1e9 * parent().events_per_buffer() / in_port().events_per_second());
		io_ready(false);
		stop_requested_ = false;
		// start the thread
		thread_ = new std::thread(boost::bind(&dummy::thread_function, this));
	}

	bool dummy::started() const {
		if(!opened()) return false;
		return thread_;
	}
	
	void dummy::thread_function() {
		if(loggers::information()()) loggers::information()("thread started", UNIVERSALIS__COMPILER__LOCATION);

		{ // set thread name and install cpu/os exception handler/translator
			std::string thread_name(universalis::compiler::typenameof(*this) + "#" + qualified_name());
			universalis::processor::exception::install_handler_in_thread(thread_name);
		}
		
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
				s << "exception: " << universalis::compiler::exceptions::ellipsis();
				loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			throw;
		}
		loggers::information()("thread " + qualified_name() + " terminated", UNIVERSALIS__COMPILER__LOCATION);
	}
	
	void dummy::thread_loop() throw(engine::exception) {
		while(true) {
			{ scoped_lock lock(mutex_);
				if(stop_requested_) return;
			}
			if(!free_wheeling_) std::this_thread::sleep(sleep_);
			if(loggers::trace()()) loggers::trace()("io ready: true", UNIVERSALIS__COMPILER__LOCATION);
			io_ready(true);
			condition_.notify_one();
		}
	}

	void dummy::do_process() throw(engine::exception) {
		if(!in_port()) return;
		{ scoped_lock lock(mutex_);
			if(false && loggers::warning()() && !io_ready()) loggers::warning()("blocking", UNIVERSALIS__COMPILER__LOCATION);
			while(!io_ready()) condition_.wait(lock);
		}
		if(loggers::trace()()) loggers::trace()("io ready: false", UNIVERSALIS__COMPILER__LOCATION);
		io_ready(false);
	}

	void dummy::do_stop() throw(engine::exception) {
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

	void dummy::do_close() throw(engine::exception) {
		opened_ = false;
		resource::do_close();
	}
	
	dummy::~dummy() throw() {
		close();
	}
}}}

