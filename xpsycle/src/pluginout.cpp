
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

PluginOut::do_initialize()
{
	void * library(::dlopen(library_file_name_.c_str()));
	if(!library) throw; ///\todo better

	#define stringized(tokens) stringized__no_expansion(tokens)
	#define stringized__no_expansion(tokens) #tokens

	new_interface_ = ::dlsym(library, stringized(PSYCLE__OUTPUT_PLUGIN__INSTANCIATOR__SYMBOL(new)));
	if(!new_) throw; ///\todo better

	delete_interface_ = ::dlsym(library, stringized(PSYCLE__OUTPUT_PLUGIN__INSTANCIATOR__SYMBOL(delete)));
	if(!delete_) throw; ///\todo better

	assert(callback_);
	interface_ = &new_interface_(*callback_);
	if(!interface_) throw; ///\todo better
}

PluginOut::~PluginOut()
{
	if(delete_interface_ && interface_) delete_(*interface_);
}
