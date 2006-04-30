
// copyright 2006 johan boule

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "pluginout.h"
#include <dlfcn.h>

PlugingOut::do_initialize()
{
	void * library(::dlopen(library_file_name_.c_str()));
	if(!library) throw; ///\todo better

	new_ = ::dlsym(library, interface::new_symbol);
	if(!new_) throw; ///\todo better

	delete_ = ::dlsym(library, interface::new_symbol);
	if(!delete_) throw; ///\todo better

	interface_ = &new_(callback);
	if(!interface_) throw; ///\todo better
}

PlugingOut::~PlugingOut()
{
	if(interface_) delete_(*interface_);
}
