/* -*- mode:c++, indent-tabs-mode:t -*- */
#pragma once

#include <string>
#include <map>

namespace psy
{
	namespace core
	{

		class Machine;
		class RiffFile;
		class CoreSong;

		namespace convert_internal_machines
		{
			class Converter
			{
				public:
					enum Type
					{
						master,
						ring_modulator, distortion, sampler, delay, filter_2_poles, gainer, flanger,
						plugin,
						vsti, vstfx,
						scope,
						dummy = 255
					};

					Converter(std::string const & plugin_path);
					virtual ~Converter() throw();

					Machine & redirect(const int & index, const int & type, RiffFile & riff,CoreSong &song);

					void retweak(CoreSong & song) const;

				private:
					class Plugin_Names : private std::map<const int, const std::string *>
					{
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
				
					std::string plugin_path_;
			};
		}
	}
}
