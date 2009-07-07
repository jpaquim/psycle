// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#ifndef RAYTRACE__RENDER
#define RAYTRACE__RENDER
#pragma once

#include "scene.hpp"
#include "pixels.hpp"
#include "typenames.hpp"
#include <boost/signal.hpp>
#include <thread>
#include <mutex>
#include <condition>
#include <list>

namespace raytrace {

class render {
	public:
		render(typenames::scene & scene, typenames::pixels & pixels);
		void start();
		void stop();
		void process();

	public:
		typenames::scene & scene() { return scene_; }
	private:
		typenames::scene & scene_;
		
	public:
		typenames::pixels & pixels() { return pixels_; }
	private:
		typenames::pixels & pixels_;

	public:
		boost::signal<void ()> & update_signal() throw() { return update_signal_; }
	private:
		boost::signal<void ()> update_signal_;

	private:
		typedef std::scoped_lock<std::mutex> scoped_lock;
		std::mutex mutable mutex_;
		std::condition<scoped_lock> mutable condition_;
		bool process_requested_, stop_requested_;
		unsigned int count_, update_signal_count_;
		typedef std::list<std::thread *> threads_type;
		threads_type threads_;
		void process_loop(unsigned int min_x, unsigned int max_x, unsigned int min_y, unsigned int max_y, unsigned int y_step);
};

}

#endif
