// -*- mode:c++; indent-tabs-mode:t -*-
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
		class MachineFactory;

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

					Converter();
					virtual ~Converter() throw();

					Machine & redirect(MachineFactory & factory, const int & index, const int & type, RiffFile & riff);

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
			};
		}
	}
}
