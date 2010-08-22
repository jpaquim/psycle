// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2003-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

/// \file
/// \brief yet another psycle plugin interface api by bohan
/// This one is more object-oriented than the original plugin_interface.hpp one.
/// The benefits of it are:
/// - it can apply a scale to parameter values, transparently, so you only deal with the meaningful real scaled value.
/// - it provides a bugfree initialization of the plugin.
/// - it provides the events samples_per_second_changed and sequencer_ticks_per_second_changed, which were not part of the original interface (i don't know if it's been fixed in the host nowadays).
#pragma once
#include <psycle/helpers/scale.hpp>
#include <universalis/compiler.hpp>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cstring> // strcpy
namespace psycle { namespace plugin {

/// the plugin class used by the host hence to which all plugins must conform.
/// You don't normally derive your plugin class directly from this class, but rather from the Plugin class below
///\see Plugin
///\todo nested definition is hard to read ; make it flat.
class Host_Plugin {
	public:
		typedef double Real;
		typedef float Sample;

		/// information describing a plugin
		class Information {
			public:
				/// version of the binary interface of the plugin.
				int const interface_version;
				/// type of the plugin, either generator or effect.
				struct Types { enum Type { effect = 0, generator = 3 }; };
				/// type of the plugin, either generator or effect.
				/// declared as int rather than enum for binary compatibility.
				int /*Types::Type*/ const type;
				/// number of parameters the plugin has.
				int const parameter_count;
				class Parameter; // forward declaration
			private:
				///\internal
				Parameter const * * const parameters;
			public:
				///\return the Parameter at the index parameter.
				Parameter const inline & parameter(int const & parameter) const throw() { return *parameters[parameter]; }
				/// description of the plugin.
				char const *
					#if defined NDEBUG
						const
					#endif
					description;
				/// name of the plugin.
				char const * const name;
				/// author of the plugin.
				char const * const author;
				/// caption to be displated in the help message.
				char const * const help_caption;
				/// number of columns that the parameter grid should be displayed with.
				int const columns;
				/// creates the information object.
				Information(
					Types::Type const & type,
					char const description[],
					char const name[],
					char const author[],
					int const & columns,
					Parameter const parameters[],
					int const & parameter_count
				) :
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
					#if !defined NDEBUG
					{
						std::string & s(*new std::string(this->description));
						s += " (debug build)";
						this->description = s.c_str();
					}
					#endif
				}
			public:
				/// information describing a parameter
				class Parameter {
					public:
						/// name of the parameter.
						char const * const name;
						/// useless field, set to the same value as name.
						char const * const unused_name;
						/// the lower bound the parameter's range.
						/// If using a non-discrete scale (i.e. real), you can't chose this, it's always the minimum value possible.
						int const minimum_value;
						/// the upper bound of the parameter's range.
						/// If using a non-discrete scale (i.e. real), you can't chose this, it's always the maximum value possible.
						int const maximum_value;
						/// type of parameter, either null (label), or state.
						struct Types { enum Type { null = 0, state = 2 }; };
						/// type of parameter, either null (label), or state.
						/// declared as int rather than enum for binary compatibility.
						int /*Types::Type*/ const type;
						int const default_value;
						helpers::Scale const & scale;
					public:
						/// creates a separator with an optional label.
						Parameter(char const name[] = "")
						:
							name(name),
							unused_name(name),
							minimum_value(0),
							maximum_value(0),
							type(Types::null),
							default_value(0),
							scale(* new helpers::scale::Discrete(0))
						{}
					public:
						/// minimum lower bound authorized by the binary interface. (unsigned 16-bit integer)
						int const static input_minimum_value = 0;
						/// maximum upper bound authorized by the binary interface. (unsigned 16-bit integer)
						int const static input_maximum_value = 0xffff;
					private:
						/// creates a scaled parameter ; you don't use this directly, but rather the public static creation functions.
						Parameter(char const name[], helpers::Scale const & scale, Real const & default_value, int const & input_maximum_value = Parameter::input_maximum_value)
						:
							name(name),
							unused_name(name),
							minimum_value(input_minimum_value),
							maximum_value(input_maximum_value),
							type(Types::state),
							default_value(
								std::min(
									std::max(
										input_minimum_value,
										static_cast<int>(scale.apply_inverse(default_value))
									),
									input_maximum_value
								)
							),
							scale(scale)
						{}
					public:
						/// creates a discrete scale, i.e. integers.
						static const Parameter & discrete(char const name[], int const & default_value, int const & maximum_value) {
							return * new Parameter(name, * new helpers::scale::Discrete(static_cast<Real>(maximum_value)), static_cast<Real>(default_value), maximum_value);
						}
						/// creates a linear real scale.
						static const Parameter & linear(char const name[], double const & minimum_value, double const & default_value, double const & maximum_value) {
							return * new Parameter(name, * new helpers::scale::Linear(static_cast<Real>(input_maximum_value), static_cast<Real>(minimum_value), static_cast<Real>(maximum_value)), static_cast<Real>(default_value));
						}
						/// creates an exponential scale.
						static const Parameter & exponential(char const name[], double const & minimum_value, double const & default_value, double const & maximum_value) {
							return * new Parameter(name, * new helpers::scale::Exponential(static_cast<Real>(input_maximum_value), static_cast<Real>(minimum_value), static_cast<Real>(maximum_value)), static_cast<Real>(default_value));
						}
						/// creates a logarithmic scale.
						static const Parameter & logarithmic(char const name[], double const & minimum_value, double const & default_value, double const & maximum_value) {
							return * new Parameter(name, * new helpers::scale::Logarithmic(static_cast<Real>(input_maximum_value), static_cast<Real>(minimum_value), static_cast<Real>(maximum_value)), static_cast<Real>(default_value));
						}
				}; // class Parameter
		}; // class Information
	public:
		virtual ~Host_Plugin() throw() {}
	protected:
		/// event called by the host to ask the plugin to initialize.
		virtual void host() {}
	public:
		/// event called by the host every tick.
		virtual void sequencer_tick_event() {}
		/// event called by the host when it wants a parameter value to be changed.
		virtual void parameter(int const parameter, int const value) {}
		/// event called by the host when it wants the plugin to process an audio buffer.
		virtual void process(Sample stereo_left_channel_samples[], Sample stereo_right_channel_samples[], int samples, int tracks) {}
		/// event called by the host when the user presses the stop button. Generally used to note-off everything.
		virtual void stop() {}
		/// event called by the host when it wants to serialize the plugin's state.
		virtual void write_data(unsigned char data[]) const {}
		/// event called by the host when it wants to deserialize the plugin's state.
		virtual void read_data(unsigned char const data[]) {}
		/// event called by the host in relation with read_data and write_data.
		/// must be overridden in your plugin class to return the size of the data that read_data and write_data should use.
		virtual int const data_size() const { return 0; }
		/// event called by the host when the user queries information about your plugin.
		virtual void help() const throw() {}
	private:
		/// unused virtual table slot kept for binary compatibility.
		virtual void unused__mute(int const track) {}
		/// unused virtual table slot kept for binary compatibility.
		virtual bool unused__muted(int const track) const { return false; }
		/// unused virtual table slot kept for binary compatibility.
		virtual void unused__midi_note(int const channel, int const note, int const velocity) {}
		/// unused virtual table slot kept for binary compatibility.
		virtual void unused__event(unsigned long int const) {}
	protected:
		/// event called by the host went it wants a string description of a parameter's current value.
		virtual bool describe(char out[], int const parameter, int const value) const { return false; }
	private:
		/// unused virtual table slot kept for binary compatibility.
		virtual bool unused__play_wave(int const, int const, Sample const) { return false; }
	public:
		/// event called by the host
		virtual void sequencer_note_event(int const channel, int const note, int const instrument, int const command, int const value) {}
	private:
		/// unused virtual table slot kept for binary compatibility.
		virtual void unused__stop_wave() {}
	protected:
		/// You don't deal directly with this array. It's part of the binary interface.
		int * parameters_;
	protected:
		/// Interface that the host provides to plugins, i.e. what plugins can query the host for.
		class Host {
			public:
				/// you can ask the host to display a message.
				virtual void message(char const message[], char const caption[], unsigned int type = 0) const {}
			private:
				/// unused virtual table slot kept for binary compatibility.
				virtual int const unused__raw(int const, int const, int const, int const) const { return 0; }
				/// unused virtual table slot kept for binary compatibility.
				virtual float const * const unused__wave_data_left(int, int) const { return 0; }
				/// unused virtual table slot kept for binary compatibility.
				virtual float const * const unused__wave_data_right(int, int) const { return 0; }
			public:
				/// asks the host to return this value.
				virtual int const samples_per_tick() const { return 0; }
				/// asks the host to return this value.
				virtual int const samples_per_second() const { return 0; }
				/// asks the host to return this value.
				virtual int const sequencer_beats_per_minute() const { return 0; }
				/// asks the host to return this value.
				virtual int const sequencer_ticks_per_sequencer_beat() const { return 0; }
				virtual inline ~Host() throw() {}
		} const * host_;
}; // class Host_Plugin

/// This class is the one your plugin class should derive from.
class Plugin : protected Host_Plugin {
	protected:
		Information const & information;
		Real * scaled_parameters_;
	public:
		virtual ~Plugin() throw() { delete [] parameters_; delete [] scaled_parameters_; }
	protected:
		/// creates an instance of your plugin.
		/// Note: functions that queries the host should only be called after the init() event.
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
		///\returns the integral internal value (i.e. non-scaled) of a parameter ; use this for discrete scales.
		inline int const & operator[](int const & parameter) const throw() { return parameters_[parameter]; }
		///\returns the real scaled value of a parameter.
		inline Real const & operator()(int const & parameter) const throw() { return scaled_parameters_[parameter]; }
	private:
		///\internal
		void parameter_internal(int const parameter, int const value) {
			this->parameters_[parameter] = value;
			this->scaled_parameters_[parameter] = information.parameter(parameter).scale.apply(static_cast<Real>(value));
		}
		///\internal
		void parameter(int const parameter, int const value) {
			parameter_internal(parameter, value);
			this->parameter(parameter);
		}
	protected:
		/// event called by the host when it wants a parameter value to be changed.
		/// override this virtual function in your plugin to take any action in response to this event.
		/// You don't need to update the parameter value itself when this event occurs,
		/// but you might want to update other data dependent on the parameter's value.
		virtual void parameter(int const &) {}
	protected:
		/// event called by the host went it wants a string description of a parameter's current value.
		/// override this virtual function in your plugin to return a specialized text instead of just the value of the number,
		/// i.e. you can append the unit (Hz, seconds, etc), or show the value in deciBell, etc.
		virtual void describe(std::ostream & out, int const & parameter) const {
			out << (*this)(parameter);
		}
	private:
		///\internal
		bool describe(char out[], int const parameter, int const) const {
			std::stringstream s;
			describe(s, parameter);
			strcpy(out, s.str().c_str());
			return out[0];
		}
	protected:
		/// event called by the host to ask the plugin to initialize.
		/// You should rather override the virtual init() function.
		///\see init()
		virtual void host() {
			Plugin::sequencer_tick_event();
			init();
			initialized_ = true;
		}
		/// event called by the host to ask the plugin to initialize.
		/// override this virtual function in your plugin to take any action in response to this event.
		/// Note: functions that queries the host should only be called after this event.
		virtual void init() {}
	private:
		///\internal
		bool initialized_;
		///\internal
		Real samples_per_second_, seconds_per_sample_, sequencer_ticks_per_seconds_;
		///\internal
		int sequencer_ticks_per_sequencer_beat_, sequencer_beats_per_minute_;
	protected:
		/// event called by the host every tick.
		/// override this virtual function in your plugin to take any action in response to this event.
		virtual void sequencer_tick_event() {
			if(samples_per_second_ != static_cast<Real>(host_->samples_per_second())) {
				samples_per_second_ = static_cast<Real>(host_->samples_per_second());
				seconds_per_sample_ = 1 / samples_per_second_;
				if(initialized_) samples_per_second_changed();
			}
			if(
				sequencer_ticks_per_sequencer_beat_ != host_->sequencer_ticks_per_sequencer_beat() ||
				sequencer_beats_per_minute_ != host_->sequencer_beats_per_minute()
			) {
				sequencer_ticks_per_sequencer_beat_ = host_->sequencer_ticks_per_sequencer_beat();
				sequencer_beats_per_minute_ = host_->sequencer_beats_per_minute();
				sequencer_ticks_per_seconds_ = Real(sequencer_ticks_per_sequencer_beat_ * sequencer_beats_per_minute_) / 60;
				if(initialized_) sequencer_ticks_per_second_changed();
			}
		}
		/// event called when the sample rate changed.
		/// override this virtual function in your plugin to take any action in response to this event.
		virtual void samples_per_second_changed() {}
		/// event called when the duration of a tick changed.
		/// override this virtual function in your plugin to take any action in response to this event.
		virtual void sequencer_ticks_per_second_changed() {}
		/// asks the host to return this value.
		inline Real const samples_per_second() const { return samples_per_second_; }
		/// asks the host to return this value.
		inline Real const seconds_per_sample() const { return 1 / samples_per_second(); }
		/// asks the host to return this value.
		inline Real const sequencer_ticks_per_seconds() const { return sequencer_ticks_per_seconds_; }
		/// asks the host to return this value.
		inline Real const samples_per_sequencer_tick() const { return samples_per_second() / sequencer_ticks_per_seconds(); }
	protected:
		/// you can ask the host to display a message.
		inline void message(std::string const & message) const throw() { this->message("message", message); }
	private:
		///\internal
		inline void message(std::string const & caption, std::string const & message) const {
			std::stringstream s;
			s << information.name << " - " << caption;
			host_->message(message.c_str(), s.str().c_str());
		}
	protected:
		/// event called by the host when the user queries information about your plugin.
		/// override this virtual function in your plugin to output a help text describing what your plugin does, what tracker commands are available etc.
		virtual void help(std::ostream & out) const throw() { out << "no help available"; }
	private:
		///\internal
		void help() const throw() {
			std::ostringstream message;
			help(message);
			this->message(information.help_caption, message.str());
		}
	protected:
		class Exception : public std::runtime_error {
			public:
				Exception(const std::string & what) : std::runtime_error(what.c_str()) {}
		}; // class Exception
}; // class Plugin

/// spelling INSTANCIATOR -> INSTANTIATOR
#define PSYCLE__PLUGIN__INSTANCIATOR(typename) PSYCLE__PLUGIN__INSTANTIATOR(typename)

/// call this from your plugin's source file to export the necessary function from the dynamically linked library.
#define PSYCLE__PLUGIN__INSTANTIATOR(typename) \
	extern "C" { \
		UNIVERSALIS__COMPILER__DYNAMIC_LINK__EXPORT Host_Plugin::Information const & UNIVERSALIS__COMPILER__CALLING_CONVENTION__C GetInfo() { return typename::information(); } \
		UNIVERSALIS__COMPILER__DYNAMIC_LINK__EXPORT psycle::plugin::Plugin &         UNIVERSALIS__COMPILER__CALLING_CONVENTION__C CreateMachine() { return * new typename; } \
		UNIVERSALIS__COMPILER__DYNAMIC_LINK__EXPORT void                             UNIVERSALIS__COMPILER__CALLING_CONVENTION__C DeleteMachine(psycle::plugin::Plugin & plugin) { delete &plugin; } \
	}
	
int const Host_Plugin::Information::Parameter::input_minimum_value;
int const Host_Plugin::Information::Parameter::input_maximum_value;

}}
