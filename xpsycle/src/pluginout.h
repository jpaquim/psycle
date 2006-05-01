
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

#ifndef PLUGINOUT_H
#define PLUGINOUT_H

#include "output_plugins/interface.hpp"
#include <string>

class PluginOut : public AudioDriver
{
	/// [bohan] sorry for the crap here
	/// [bohan] it comes from the original AudioDriver class of psyclemfc

	///\name fukcin multistep initialization
	///\{
		public:
			/// step 1, no that's not enough.
			/// this ctor does NOTHING, don't forget to call the Initialize function
			PluginOut(std::string const & library_file_name)
			:
				initialized_(false),
				configured_(false),
				library_file_name_(library_file_name)
			{}

		public:
			/// step 2, don't forget to call this function
			void Initialize(AUDIODRIVERWORKFN callback_function, void * callback_function_argument)
			{
				callback_ = new callback(callback_function, callback_function_argument);
				do_initialize();
				initialized_ = true;
			}
		public:
			/// do-we-really-have-an-instance?-style runtime check
			bool Initialized() { return initialized_; }
		private:
			/// no comment
			bool initialized_;
		private:
			/// the real ctor is here.
			/// finally we can start creating the object!
			void do_initialize(AUDIODRIVERWORKFN, void *);
	///\}

	///\name configuration -- oh fgod and you thought it was initialized?
	///\{
		public:
			/// oh, yeah, let's read some config somewhere from the outer space
			void Configure() { configured_ = true; }
			/// are we still not configured yet?!??
			bool Configured() { return configured_; }
		private:
			/// no comment
			bool configured_;
	///\}

	///\name unkown stuff
	///\{
		public:
			// no idea what it's for, is it documented?
			bool Enable(bool enable) { return enable; }

			// no idea what it's for, is it documented?
			void Reset() {}
	///\}

	public:
		~PlugingOut();

	private:
		std::string library_file_name_;
		void * library_;
		output_plugins::interface::   new_function_type    new_interface_;
		output_plugins::interface::delete_function_type delete_interface_;
		output_plugins::interface *                            interface_;
		class callback : public output_plugins::interface::callback_type
		{
			public:
				callback(AUDIODRIVERWORKFN callback_function, void * callback_function_argument)
				:
					callback_function_(callback_function),
					callback_function_argument_(callback_function_argument)
				{}
			private:
				AUDIODRIVERWORKFN callback_function_
				void * callback_function_argument_;

			protected:
				float const * process(unsigned int samples) { return callback_function_(callback_function_argument_, samples); }

				///\todo hardcoded
				unsigned int channels() { return 2; }

				///\todo hardcoded
				float samples_per_seconds { return 44100; }
		} * callback_;
};

#endif
