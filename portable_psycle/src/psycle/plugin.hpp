/// \file
/// \brief yet another psycle plugin interface api by bohan
/// This one is more object-oriented than the original plugin_interface.hpp one.
#pragma once
#include <string>
#include <sstream>
#include <iomanip>
#include <exception>
#include "scale.hpp"
namespace psycle
{
	namespace plugin
	{
		/// the plugin class used by the host hence to which all plugins must conform.
		class Host_Plugin
		{
		public:
			class Information
			{
			public:
				int const interface_version;
				enum Type { effect = 0, generator = 3 };
				int /*Type*/ const type;
				int const parameter_count;
				class Parameter;
			private:
				const Parameter * * const parameters;
			public:
				Parameter const inline & parameter(int const & parameter) const throw() { return *parameters[parameter]; }
				char const * const description;
				char const * const name;
				char const * const author;
				char const * const help_caption;
				int const columns;
				Information
				(
					Type const & type,
					char const description[],
					char const name[],
					char const author[],
					int const & columns,
					Parameter const parameters[],
					int const & parameter_count
				)
				:
					interface_version(11),
					type(type),
					parameter_count(parameter_count),
					parameters(new const Parameter * [parameter_count]),
					description(description),
					name(name),
					author(author),
					help_caption("Help"),
					columns(columns)
				{
					for(int i(0) ; i < parameter_count ; ++i) this->parameters[i] = &parameters[i];
				}
			public:
				class Parameter
				{
				public:
					char const * const name;
					char const * const unused_name;
					int const minimum_value;
					int const maximum_value;
					enum Type { null = 0, state = 2 };
					int /*Type*/ const type;
					int const default_value;
					Scale const & scale;
				public:
					Parameter(const char name[] = "")
					:
						type(Type::null),
						name(name),
						unused_name(name),
						scale(* new scale::Discrete(0)),
						minimum_value(0),
						default_value(0),
						maximum_value(0)
					{}
				public:
				#if defined COMPILER__MICROSOFT && COMPILER__VERSION__MAJOR < 7
					enum { inmput_maximum_value = 0xffff };
				#else
					static const int input_maximum_value = 0xffff;
				#endif
				private:
					Parameter(const char name[], const Scale & scale, const Real & default_value, const int & input_maximum_value = Parameter::input_maximum_value)
					:
						type(Type::state),
						name(name),
						unused_name(name),
						scale(scale),
						minimum_value(0),
						default_value(static_cast<int>(scale.apply_inverse(default_value))),
						maximum_value(input_maximum_value)
					{}
				public:
					static const Parameter & discrete(const char name[], const int & default_value, const int & maximum_value)
					{
						return * new Parameter(name, * new scale::Discrete(static_cast<Real>(maximum_value)), static_cast<Real>(default_value), maximum_value);
					}
					static const Parameter & linear(const char name[], const double & minimum_value, const double & default_value, const double & maximum_value)
					{
						return * new Parameter(name, * new scale::Linear(static_cast<Real>(input_maximum_value), static_cast<Real>(minimum_value), static_cast<Real>(maximum_value)), static_cast<Real>(default_value));
					}
					static const Parameter & exponential(const char name[], const double & minimum_value, const double & default_value, const double & maximum_value)
					{
						return * new Parameter(name, * new scale::Exponential(static_cast<Real>(input_maximum_value), static_cast<Real>(minimum_value), static_cast<Real>(maximum_value)), static_cast<Real>(default_value));
					}
					static const Parameter & logarithmic(const char name[], const double & minimum_value, const double & default_value, const double & maximum_value)
					{
						return * new Parameter(name, * new scale::Logarithmic(static_cast<Real>(input_maximum_value), static_cast<Real>(minimum_value), static_cast<Real>(maximum_value)), static_cast<Real>(default_value));
					}
				};
			};
		public:
			virtual ~Host_Plugin() throw() {}
		protected:
			virtual void host() {}
		public:
			virtual void sequencer_tick_event() {}
			virtual void parameter(const int parameter, const int value) {}
			virtual void process(Sample stereo_left_channel_samples[], Sample stereo_right_channel_samples[], int samples, int tracks) {}
			virtual void stop() {}
			virtual void write_data(unsigned char data[]) const {}
			virtual void read_data(const unsigned char data[]) {}
			virtual const int data_size() const { return 0; }
			virtual void help() const throw() {}
		private:
			virtual void unused__mute(const int track) {}
			virtual bool unused__muted(const int track) const { return false; }
			virtual void unused__midi_note(const int channel, const int note, const int velocity) {}
			virtual void unused__event(const unsigned long int) {}
		protected:
			virtual bool describe(char out[], const int parameter, const int value) const { return false; }
		private:
			virtual bool unused__play_wave(const int, const int, const Sample) { return false; }
		public:
			virtual void sequencer_note_event(const int channel, const int note, const int instrument, const int command, const int value) {}
		private:
			virtual void unused__stop_wave() {}
		protected:
			int * parameters_;
		protected:
			class Host
			{
			public:
				virtual void message(const char message[], const char caption[], unsigned int type = 0) const {}
				virtual const int unused__raw(const int, const int, const int, const int) const { return 0; }
				virtual const float * const unused__wave_data_left(int, int) const { return 0; }
				virtual const float * const unused__wave_data_right(int, int) const { return 0; }
				virtual const int samples_per_tick() const { return 0; }
				virtual const int samples_per_second() const { return 0; }
				virtual const int sequencer_beats_per_minute() const { return 0; }
				virtual const int sequencer_ticks_per_sequencer_beat() const { return 0; }
			} const * host_;
		};

		class Plugin : protected Host_Plugin
		{
		protected:
			Plugin(const Information & information)
			:
				information(information),
				initialized_(false),
				samples_per_second_(0),
				sequencer_ticks_per_sequencer_beat_(0),
				sequencer_beats_per_minute_(0)
			{
				parameters_ = new int[information.parameter_count];
				scaled_parameters_ = new Real[information.parameter_count];
			}
			virtual ~Plugin() throw() { delete [] parameters_; delete [] scaled_parameters_; }
			inline const int & operator[](const int & parameter) const throw() { return parameters_[parameter]; }
			inline const Real & operator()(const int & parameter) const throw() { return scaled_parameters_[parameter]; }
		protected:
			const Information & information;
			Real * scaled_parameters_;
		private:
			virtual void parameter(const int parameter, const int value)
			{
				this->parameters_[parameter] = value;
				this->scaled_parameters_[parameter] = information.parameter(parameter).scale.apply(static_cast<Real>(value));
				this->parameter(parameter);
			}
		protected:
			virtual void parameter(const int &) {}
		protected:
			virtual void describe(std::ostream & out, const int & parameter) const
			{
				out << (*this)(parameter);
			}
		private:
			virtual bool describe(char out[], const int parameter, const int) const
			{
				std::stringstream s;
				describe(s, parameter);
				strcpy(out, s.str().c_str());
				return out[0];
			}
		protected:
			virtual void host()
			{
				Plugin::sequencer_tick_event();
				init();
				initialized_ = true;
			}
			virtual void init() {}
		private:
			Real
				samples_per_second_, seconds_per_sample_,
				sequencer_ticks_per_seconds_;
			int sequencer_ticks_per_sequencer_beat_, sequencer_beats_per_minute_;
			bool initialized_;
		protected:
			virtual void sequencer_tick_event()
			{
				if(samples_per_second_ != static_cast<Real>(host_->samples_per_second()))
				{
					samples_per_second_ = static_cast<Real>(host_->samples_per_second());
					seconds_per_sample_ = 1 / samples_per_second_;
					if(initialized_) samples_per_second_changed();
				}
				if
				(
					sequencer_ticks_per_sequencer_beat_ != host_->sequencer_ticks_per_sequencer_beat() ||
					sequencer_beats_per_minute_ != host_->sequencer_beats_per_minute()
				)
				{
					sequencer_ticks_per_sequencer_beat_ = host_->sequencer_ticks_per_sequencer_beat();
					sequencer_beats_per_minute_ = host_->sequencer_beats_per_minute();
					sequencer_ticks_per_seconds_ = Real(sequencer_ticks_per_sequencer_beat_ * sequencer_beats_per_minute_) / 60;
					if(initialized_) sequencer_ticks_per_second_changed();
				}
			}
			virtual void samples_per_second_changed() {}
			virtual void sequencer_ticks_per_second_changed() {}
			inline const Real samples_per_second() const { return samples_per_second_; }
			inline const Real seconds_per_sample() const { return 1 / samples_per_second(); }
			inline const Real sequencer_ticks_per_seconds() const { return sequencer_ticks_per_seconds_; }
			inline const Real samples_per_sequencer_tick() const { return samples_per_second() / sequencer_ticks_per_seconds(); }
		protected:
			inline void message(const std::string & message) const throw() { this->message("message", message); }
		private:
			inline void message(const std::string & caption, const std::string & message) const
			{
				std::stringstream s;
				s << information.name << " - " << caption;
				host_->message(message.c_str(), s.str().c_str());
			}
		protected:
			virtual void help(std::ostream & out) const throw() { out << "no help available"; }
		private:
			virtual void help() const throw()
			{
				std::ostringstream message;
				/*
				message <<
					"(¯`'·.¸(¯`'·.¸¸.·'`¯)¸.·'`¯)\n"
					" ¯`'·.¸              ¸.·'`¯ \n"
					"       )PSYCLEDELICS(       \n"
					" ¸.·'`¯              ¯`'·.¸ \n"
					"(¸.·'`¯(¸.·'`¯¯`'·.¸)¯`'·.¸)" << std::endl;
				*/
				help(message);
				this->message(information.help_caption, message.str());
			}
		protected:
			class Exception : public std::runtime_error
			{
			public:
				Exception(const std::string & what) : std::runtime_error(what.c_str()) {}
			};
		};

		#define PSYCLE__PLUGIN__INSTANCIATOR(typename) \
			extern "C" \
			{ \
				__declspec(dllexport) const Host_Plugin::Information & cdecl GetInfo() { return typename::information(); } \
				__declspec(dllexport) Plugin & cdecl CreateMachine() { return * new typename; } \
				__declspec(dllexport) void cdecl DeleteMachine(Plugin & plugin) { delete &plugin; } \
			} \
	}
}
