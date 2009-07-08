// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#include "render.hpp"
#include <universalis/os/cpu_affinity.hpp>
#include <boost/bind.hpp>
#include <cstdlib>

namespace raytrace {

render::render(typenames::scene & scene, typenames::view & view, typenames::pixels & pixels)
:
	scene_(scene),
	view_(view),
	pixels_(pixels),
	process_requested_(),
	stop_requested_(),
	count_(),
	update_signal_count_(10000)
{}

void inline render::trace(unsigned int x, unsigned int y) {
	vertex3 const & from(view_.from);

	vertex3 to;
	to.x = x;
	to.y = y;
	to.z = 1;

	color const c(scene_.trace(from, to));
	pixels_.put(x, y, c);
}

void render::start() {
	process_requested_ = stop_requested_ = false;
	thread_count_ = universalis::os::cpu_affinity::cpu_count();
	{ // thread count env var
		char const * const env(std::getenv("RAYTRACE_THREADS"));
		if(env) {
			std::stringstream s;
			s << env;
			s >> thread_count_;
		}
	}
	unsigned int const width(pixels_.width()), height(pixels_.height());
	unsigned int x(0), y(0);
	try {
		unsigned int const y_step(thread_count_);
		for(std::size_t i(0); i < thread_count_; ++i)
			threads_.push_back(new std::thread(boost::bind(&render::process_loop, this, x, x + width, y + i, y + height, y_step)));
	} catch(...) {
		stop();
		throw;
	}
}
		
void render::stop() {
	{ scoped_lock lock(mutex_);
		stop_requested_ = true;
	}
	condition_.notify_all();
	for(threads_type::const_iterator i(threads_.begin()), e(threads_.end()); i != e; ++i) {
		(**i).join();
		delete *i;
	}
	threads_.clear();
}

void render::process() {
	{ scoped_lock lock(mutex_);
		process_requested_ = true;
		thread_done_count_ = 0;
	}
	condition_.notify_all();
}

void render::process_loop(unsigned int min_x, unsigned int max_x, unsigned int min_y, unsigned int max_y, unsigned int y_step) {
	//std::cout << "part: " << min_x << ' ' << max_x << ' ' << min_y << ' ' << max_y << ' ' << y_step << '\n';
	unsigned int const inc(max_x - min_x);
	while(true) {
		{ scoped_lock lock(mutex_);
			while(!process_requested_ && !stop_requested_) condition_.wait(lock);
			if(stop_requested_) return;
		}
		for(unsigned int y(min_y); y < max_y; y += y_step) {
			for(unsigned int x(min_x); x < max_x; ++x) {
				trace(x, y);
			}
			bool update_signal(false);
			{ scoped_lock lock(mutex_);
				if(stop_requested_) return;
				count_ += inc;
				if(count_ > update_signal_count_) {
					count_ = 0;
					update_signal = true;
				}
			}
			if(update_signal) {
				scoped_lock lock(update_signal_mutex_);
				update_signal_();
			}
		}
		bool update_signal(false);
		{ scoped_lock lock(mutex_);
			++thread_done_count_;
			while(thread_done_count_ != thread_count_ && !stop_requested_) condition_.wait(lock);
			if(stop_requested_) return;
			if(process_requested_) {
				process_requested_ = false;
				if(count_ != 0) update_signal = true;
			}
		}
		if(update_signal) {
			scoped_lock lock(update_signal_mutex_);
			update_signal_();
		}
	}
}

}
