// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#include "main.hpp"
#include "render.hpp"
#include "window.hpp"
#include "lock.hpp"
#include <gtkmm/main.h>

namespace raytrace {

class scene0 : public scene {
	protected:
		color trace(unsigned int x, unsigned int y) /*override*/ {
			color c(x, y, x * y);
			return c;
		}
};

int main(int /*const*/ argument_count, char /*const*/ * /*const*/ arguments[]) {
	Gtk::Main main(argument_count, arguments);
	scene0 scene;
	pixels pixels(800, 800);
	pixels.fill(0xffffff00);
	render render(scene, pixels);
	window window(render);
	render.start();
	render.process();
	{ lock lock;
		main.run(window);
	}
	return 0;
}

}

int main(int /*const*/ argument_count, char /*const*/ * /*const*/ arguments[]) {
	return raytrace::main(argument_count, arguments);
}
