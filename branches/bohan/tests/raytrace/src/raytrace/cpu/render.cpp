// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2010 psycledelics http://psycle.pastnotecut.org : johan boule

#include "render.hpp"
#include <universalis/stdlib/thread.hpp>
#include <boost/bind.hpp>
#include <cstdlib>

namespace raytrace {

render::render(class scene const & scene, class view const & view, class pixels & pixels)
:
	scene_(scene),
	view_(view),
	pixels_(pixels),
	process_requested_(),
	stop_requested_(),
	count_(),
	update_signal_count_() //pixels_.width() * pixels_.height() / 100)
{}

void render::compute_view() {
	unsigned int const width(pixels_.width()), height(pixels_.height());

	xx_ratio_ = 2 * std::sin(view_.x_fov / 2) / width;
	xy_ratio_ = 0;
	xz_ratio_ = 0;

	yx_ratio_ = 0;
	yy_ratio_ = -2 * std::sin(view_.x_fov / 2 * height / width) / height;
	yz_ratio_ = 0;
	
	x_offset_ = xx_ratio_ * width / -2;
	y_offset_ = -yy_ratio_ * height / 2;
	z_offset_ = -1;

	#if 0
		std::cout << x_offset_ << ' '<<  xx_ratio_ << ' '<<  yx_ratio_ << '\n';
		std::cout << y_offset_ << ' '<<  xy_ratio_ << ' '<<  yy_ratio_ << '\n';
		std::cout << z_offset_ << ' '<<  xz_ratio_ << ' '<<  yz_ratio_ << '\n';
	#endif
}

void inline render::trace(unsigned int x, unsigned int y) {
	vertex3 to;//(view_.from);
	to.x = x_offset_ + x * xx_ratio_ + y * yx_ratio_;
	to.y = y_offset_ + x * xy_ratio_ + y * yy_ratio_;
	to.z = z_offset_ + x * xz_ratio_ + y * yz_ratio_;

	//std::cout << to.x << ' ' << to.y << ' ' << to.z << '\n';
	color const c(scene_.trace(view_.from, view_.from + to));
	pixels_.put(x, y, c);
}

void render::start() {
	if(!threads_.empty()) return;
	process_requested_ = stop_requested_ = false;
	thread_count_ = universalis::stdlib::thread::hardware_concurrency();
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
			threads_.push_back(new std::thread(boost::bind(&render::process_loop, this, i, x, x + width, y + i, y + height, y_step)));
	} catch(...) {
		stop();
		throw;
	}
}
		
void render::stop() {
	if(threads_.empty()) return;
	{ scoped_lock lock(mutex_);
		stop_requested_ = true;
	}
	condition_.notify_all(); // TODO calling start() and stop() right after may not work
	for(threads_type::const_iterator i(threads_.begin()), e(threads_.end()); i != e; ++i) {
		(**i).join();
		delete *i;
	}
	threads_.clear();
}

void render::resize(unsigned int width, unsigned int height) {
	pixels_.resize(width, height);
}

void render::process() {
	{ scoped_lock lock(mutex_);
		compute_view();
		process_requested_ = true;
		count_ = thread_done_count_ = thread_done_count2_ = 0;
		done_ = false;
	}
	condition_.notify_all();
}

void render::wait() {
	{ scoped_lock lock(done_mutex_);
		while(!done_) condition_done_.wait(lock);
	}
}

void render::process_loop(std::size_t i, unsigned int min_x, unsigned int max_x, unsigned int min_y, unsigned int max_y, unsigned int y_step) {
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
				if(update_signal_count_) {
					count_ += inc;
					if(count_ > update_signal_count_) {
						count_ = 0;
						update_signal = true;
					}
				}
			}
			if(update_signal) {
				scoped_lock lock(update_signal_mutex_);
				update_signal_();
			}
		}
		bool notify(false), update_signal(false);
		{ scoped_lock lock(mutex_);
			if(++thread_done_count_ == thread_count_) condition_.notify_all();
			else while(thread_done_count_ != thread_count_ && !stop_requested_) condition_.wait(lock);
			if(stop_requested_) return;
			process_requested_ = false;
			if(++thread_done_count2_ == thread_count_) {
				notify = true;
				if(!update_signal_count_ || count_ != 0) update_signal = true;
			}
		}
		if(notify) {
			if(update_signal) {
				scoped_lock lock(update_signal_mutex_);
				update_signal_();
			}
			{ scoped_lock lock(done_mutex_);
				done_ = true;
			}
			condition_done_.notify_one();
		}
	}
}

}
