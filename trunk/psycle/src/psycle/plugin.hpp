/// \file
/// \brief yet another psycle plugin interface api by bohan
/// This one is more object-oriented than the original plugin_interface.hpp one.
#pragma once
#include "scale.hpp"
#include <universalis/compiler.hpp>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>
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
						struct Types { enum Type { effect = 0, generator = 3 }; };
						int /*Types::Type*/ const type;
						int const parameter_count;
						class Parameter;
					private:
						Parameter const * * const parameters;
					public:
						Parameter const inline & parameter(int const & parameter) const throw() { return *parameters[parameter]; }
						char const * const description;
						char const * const name;
						char const * const author;
						char const * const help_caption;
						int const columns;
						Information
						(
							Types::Type const & type,
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
							parameters(new Parameter const * [parameter_count]),
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
								struct Types { enum Type { null = 0, state = 2 }; };
								int /*Types::Type*/ const type;
								int const minimum_value;
								int const maximum_value;
								int const default_value;
								Scale const & scale;
							public:
								Parameter(char const name[] = "")
								:
									name(name),
									unused_name(name),
									type(Types::null),
									minimum_value(0),
									maximum_value(0),
									default_value(0),
									scale(* new scale::Discrete(0))
								{}
							public:
							int const static input_minimum_value = 0;
							int const static input_maximum_value = 0xffff;
							private:
								Parameter(char const name[], Scale const & scale, Real const & default_value, int const & input_maximum_value = Parameter::input_maximum_value)
								:
									name(name),
									unused_name(name),
									type(Types::state),
									minimum_value(input_minimum_value),
									maximum_value(input_maximum_value),
									default_value
									(
										std::min
										(
											std::max
											(
												input_minimum_value,
												static_cast<int>(scale.apply_inverse(default_value))
											),
											input_maximum_value
										)
									),
									scale(scale)
								{}
							public:
								static const Parameter & discrete(char const name[], int const & default_value, int const & maximum_value)
								{
									return * new Parameter(name, * new scale::Discrete(static_cast<Real>(maximum_value)), static_cast<Real>(default_value), maximum_value);
								}
								static const Parameter & linear(char const name[], double const & minimum_value, double const & default_value, double const & maximum_value)
								{
									return * new Parameter(name, * new scale::Linear(static_cast<Real>(input_maximum_value), static_cast<Real>(minimum_value), static_cast<Real>(maximum_value)), static_cast<Real>(default_value));
								}
								static const Parameter & exponential(char const name[], double const & minimum_value, double const & default_value, double const & maximum_value)
								{
									return * new Parameter(name, * new scale::Exponential(static_cast<Real>(input_maximum_value), static_cast<Real>(minimum_value), static_cast<Real>(maximum_value)), static_cast<Real>(default_value));
								}
								static const Parameter & logarithmic(char const name[], double const & minimum_value, double const & default_value, double const & maximum_value)
								{
									return * new Parameter(name, * new scale::Logarithmic(static_cast<Real>(input_maximum_value), static_cast<Real>(minimum_value), static_cast<Real>(maximum_value)), static_cast<Real>(default_value));
								}
						}; // class Parameter
				}; // class Information
			public:
				virtual ~Host_Plugin() throw() {}
			protected:
				virtual void host() {}
			public:
				virtual void sequencer_tick_event() {}
				virtual void parameter(int const parameter, int const value) {}
				virtual void process(Sample stereo_left_channel_samples[], Sample stereo_right_channel_samples[], int samples, int tracks) {}
				virtual void stop() {}
				virtual void write_data(unsigned char data[]) const {}
				virtual void read_data(unsigned char const data[]) {}
				virtual int const data_size() const { return 0; }
				virtual void help() const throw() {}
			private:
				virtual void unused__mute(int const track) {}
				virtual bool unused__muted(int const track) const { return false; }
				virtual void unused__midi_note(int const channel, int const note, int const velocity) {}
				virtual void unused__event(unsigned long int const) {}
			protected:
				virtual bool describe(char out[], int const parameter, int const value) const { return false; }
			private:
				virtual bool unused__play_wave(int const, int const, Sample const) { return false; }
			public:
				virtual void sequencer_note_event(int const channel, int const note, int const instrument, int const command, int const value) {}
			private:
				virtual void unused__stop_wave() {}
			protected:
				int * parameters_;
			protected:
				class Host
				{
					public:
						virtual inline ~Host() throw() {}
						virtual void message(char const message[], char const caption[], unsigned int type = 0) const {}
						virtual int const unused__raw(int const, int const, int const, int const) const { return 0; }
						virtual float const * const unused__wave_data_left(int, int) const { return 0; }
						virtual float const * const unused__wave_data_right(int, int) const { return 0; }
						virtual int const samples_per_tick() const { return 0; }
						virtual int const samples_per_second() const { return 0; }
						virtual int const sequencer_beats_per_minute() const { return 0; }
						virtual int const sequencer_ticks_per_sequencer_beat() const { return 0; }
				} const * host_;
		}; // class Host_Plugin

		class Plugin : protected Host_Plugin
		{
			protected:
				Information const & information;
				Real * scaled_parameters_;
			public:
				virtual ~Plugin() throw() { delete [] parameters_; delete [] scaled_parameters_; }
			protected:
				Plugin(Information const & information)
				:
					information(information),
					initialized_(false),
					samples_per_second_(0),
					sequencer_ticks_per_sequencer_beat_(0),
					sequencer_beats_per_minute_(0)
				{
					parameters_ = new int[information.parameter_count];
					scaled_parameters_ = new Real[information.parameter_count];
					for(int parameter(0) ; parameter < information.parameter_count ; ++parameter)
						parameter_internal(parameter, information.parameter(parameter).default_value);
				}
				inline int const & operator[](int const & parameter) const throw() { return parameters_[parameter]; }
				inline Real const & operator()(int const & parameter) const throw() { return scaled_parameters_[parameter]; }
			private:
				void parameter_internal(int const parameter, int const value)
				{
					this->parameters_[parameter] = value;
					this->scaled_parameters_[parameter] = information.parameter(parameter).scale.apply(static_cast<Real>(value));
				}
				void parameter(int const parameter, int const value)
				{
					parameter_internal(parameter, value);
					this->parameter(parameter);
				}
			protected:
				virtual void parameter(int const &) {}
			protected:
				virtual void describe(std::ostream & out, int const & parameter) const
				{
					out << (*this)(parameter);
				}
			private:
				bool describe(char out[], int const parameter, int const) const
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
				bool initialized_;
				Real samples_per_second_, seconds_per_sample_, sequencer_ticks_per_seconds_;
				int sequencer_ticks_per_sequencer_beat_, sequencer_beats_per_minute_;
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
				inline Real const samples_per_second() const { return samples_per_second_; }
				inline Real const seconds_per_sample() const { return 1 / samples_per_second(); }
				inline Real const sequencer_ticks_per_seconds() const { return sequencer_ticks_per_seconds_; }
				inline Real const samples_per_sequencer_tick() const { return samples_per_second() / sequencer_ticks_per_seconds(); }
			protected:
				inline void message(std::string const & message) const throw() { this->message("message", message); }
			private:
				inline void message(std::string const & caption, std::string const & message) const
				{
					std::stringstream s;
					s << information.name << " - " << caption;
					host_->message(message.c_str(), s.str().c_str());
				}
			protected:
				virtual void help(std::ostream & out) const throw() { out << "no help available"; }
			private:
				void help() const throw()
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
				}; // class Exception
		}; // class Plugin

		#define PSYCLE__PLUGIN__INSTANCIATOR(typename) \
			extern "C" \
			{ \
				UNIVERSALIS__COMPILER__DYNAMIC_LINK__EXPORT Host_Plugin::Information const & UNIVERSALIS__COMPILER__CALLING_CONVENTION__C GetInfo() { return typename::information(); } \
				UNIVERSALIS__COMPILER__DYNAMIC_LINK__EXPORT psycle::plugin::Plugin &         UNIVERSALIS__COMPILER__CALLING_CONVENTION__C CreateMachine() { return * new typename; } \
				UNIVERSALIS__COMPILER__DYNAMIC_LINK__EXPORT void                             UNIVERSALIS__COMPILER__CALLING_CONVENTION__C DeleteMachine(psycle::plugin::Plugin & plugin) { delete &plugin; } \
			}
			
		int const Host_Plugin::Information::Parameter::input_minimum_value;
		int const Host_Plugin::Information::Parameter::input_maximum_value;
	} // namespace plugin
} // namespace psycle
