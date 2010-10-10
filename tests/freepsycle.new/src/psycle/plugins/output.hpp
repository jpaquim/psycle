// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface psycle::plugins::output
#ifndef PSYCLE__PLUGINS__OUTPUT__INCLUDED
#define PSYCLE__PLUGINS__OUTPUT__INCLUDED
#pragma once
#include <psycle/detail/project.hpp>

#if defined PSYCLE__PLUGINS__OUTPUT__DEFAULT__GSTREAMER
	#include "outputs/gstreamer.hpp"
#elif defined PSYCLE__PLUGINS__OUTPUT__DEFAULT__JACK
	#include "outputs/jack.hpp"
#elif defined PSYCLE__PLUGINS__OUTPUT__DEFAULT__ALSA
	#include "outputs/alsa.hpp"
#elif defined PSYCLE__PLUGINS__OUTPUT__DEFAULT__DIRECT_SOUND
	#include "outputs/direct_sound.hpp"
#elif defined PSYCLE__PLUGINS__OUTPUT__DEFAULT__DUMMY
	#include "outputs/dummy.hpp"
#else
	#error no default output
#endif

#define PSYCLE__DECL  PSYCLE__PLUGINS__OUTPUT
#include <psycle/detail/decl.hpp>
namespace psycle { namespace plugins {

///\internal
/// plugins driving an underlying output device.
namespace outputs {

	class settings_capabilities {
		std::list<std::string> device_names;
		bool buffer_frames;
		bool period_frames;
		bool channels;
		bool bit_per_channel_sample;
		bool samples_per_seconds;
	};
	
	class settings {
		unsigned int buffer_frames;
		unsigned int period_frames;

		unsigned int channels;
		unsigned int bits_per_channel_sample;
		real samples_per_second;
	};

	class requested_settings : public settings {
		std::string device_name;
	};

	class actual_settings : public settings {
		unsigned int significant_bits_per_channel_sample;
	};
}

typedef outputs::
	#if defined PSYCLE__PLUGINS__OUTPUT__DEFAULT__GSTREAMER
		gstreamer
	#elif defined PSYCLE__PLUGINS__OUTPUT__DEFAULT__JACK
		jack
	#elif defined PSYCLE__PLUGINS__OUTPUT__DEFAULT__ALSA
		alsa
	#elif defined PSYCLE__PLUGINS__OUTPUT__DEFAULT__DIRECT_SOUND
		direct_sound
	#elif defined PSYCLE__PLUGINS__OUTPUT__DEFAULT__DUMMY
		dummy
	#else
		#error no default output
	#endif
	output_base;
	
/// default output.
class PSYCLE__DECL output : public output_base {
	public:
		typedef output_base base;
		output(class plugin_library_reference &, name_type const &);
	public:
		//ports::inputs::single & in_port() { return *single_input_ports()[0]; }
};

}}
#include <psycle/detail/decl.hpp>
#endif
