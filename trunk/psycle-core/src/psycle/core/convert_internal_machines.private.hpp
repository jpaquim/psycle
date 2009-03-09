// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2004-2009 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#ifndef PSYCLE__CORE__CONVERT_INTERNAL_MACHINES__INCLUDED
#define PSYCLE__CORE__CONVERT_INTERNAL_MACHINES__INCLUDED
#pragma once

#include <psycle/core/config.hpp>

#include <string>
#include <map>

namespace psy { namespace core {

class Machine;
class RiffFile;
class CoreSong;
class MachineFactory;

namespace convert_internal_machines {
	class PSYCLE__CORE__DECL Converter {
		public:
			enum Type {
				master,
				ring_modulator, distortion, sampler, delay, filter_2_poles, gainer, flanger,
				plugin,
				vsti, vstfx,
				scope,
				abass,
				asynth1,
				asynth2,
				asynth21,
				dummy = 255
			};

			Converter();
			virtual ~Converter() throw();

			Machine & redirect(MachineFactory & factory, const int & index, const int & type, RiffFile & riff);

			void retweak(CoreSong & song) const;

		private:
			class Plugin_Names : private std::map<const int, const std::string *> {
				public:
					Plugin_Names();
					~Plugin_Names();
					const bool exists(const int & type) const throw();
					const std::string & operator()(const int & type) const;
			};

		public:
			static const Plugin_Names & plugin_names();

		private:
			std::map<Machine * const, const int *> machine_converted_from;

			template<typename Parameter> void retweak(Machine & machine, const int & type, Parameter parameters [], const int & parameter_count, const int & parameter_offset = 1);
			void retweak(const int & type, int & parameter, int & integral_value) const;
	};
}

}}
#endif
