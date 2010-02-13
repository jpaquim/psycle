// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\implementation psycle::front_ends::text::main
#include <psycle/detail/project.private.hpp>
#include "main.hpp"
namespace psycle { namespace tests { namespace classic_player {

void play() {
}

int main(int /*const*/ argument_count, char /*const*/ * /*const*/ arguments[]) {
	play();
	return 0;
}

}}}

int main(int /*const*/ argument_count, char /*const*/ * /*const*/ arguments[]) {
	psycle::tests::classic_player::main(argument_count, arguments);
}
