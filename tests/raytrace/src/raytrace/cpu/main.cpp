// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#include "main.hpp"
#include "render.hpp"
#include "window.hpp"
#include "lock.hpp"
#include "matrix4.hpp"
#include <gtkmm/main.h>
#include <limits>

namespace raytrace {

class object {
	public:
		bool virtual hit(vertex3 const & from, vertex3 const & to, vertex3 & pos) = 0;
		vertex3 virtual normal(vertex3 const & pos) = 0;
};

class quadric : public /*symmetric_*/matrix4, public object {
	public:
		bool hit(vertex3 const & from, vertex3 const & to, vertex3 & pos) /*override*/ {
			vertex4 const to_matrix(to * *this);
			real const a(to_matrix * to);
			real const b(2 * (to_matrix * from));
			real const c(from * *this * from);
			real const discriminant(b * b - 4 * a * c);
			if(discriminant < 0) return false;
			real const discriminant_sqrt(std::sqrt(discriminant));
			real const inversed_denominator(1 / (2 * a));
			real const opposed_b(-b);
			real const root1((opposed_b + discriminant_sqrt) * inversed_denominator);
			real const root2((opposed_b - discriminant_sqrt) * inversed_denominator);
			real root;
			if(root1 < 0) {
				if(root2 < 0) return false;
				root = root2;
			} else if(root2 < 0) root = root1;
			else root = std::min(root1, root2);
			pos = from + root * to;
			return true;
		}

		vertex3 normal(vertex3 const & pos) /*override*/ {
			vertex3 result;
			vertex4 const n(*this * pos);
			result.x = n.x;
			result.y = n.y;
			result.z = n.z;
			return result;
		}
};

class scene0 : public scene {
	public:
		quadric q;

		scene0() {
			q.identity();
			q(3, 3) = -1;
		}
		
		color trace(vertex3 const & from, vertex3 const & to) /*override*/ {
			color c;
			vertex3 pos;
			if(!q.hit(from, to, pos)) c(0, 0, 0);
			else {
				//c(0.5, 1, 1);
				real const mag(pos.mag());
				c(mag, mag, 1);
				/*
				vertex3 normal(q.normal(pos));
				normal.normalize();
				c(normal.x, normal.y, normal.z);
				*/
			}
			return c;
		}
};

int main(int /*const*/ argument_count, char /*const*/ * /*const*/ arguments[]) {
	Gtk::Main main(argument_count, arguments);
	scene0 scene;
	view view; view.from(0, 0, 0);
	pixels pixels(800, 800);
	pixels.fill(color(1, 1, 1));
	render render(scene, view, pixels);
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
