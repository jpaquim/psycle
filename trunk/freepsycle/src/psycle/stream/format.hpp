// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::stream::format
#ifndef PSYCLE__STREAM__FORMAT__INCLUDED
#define PSYCLE__STREAM__FORMAT__INCLUDED
#pragma once
#include <psycle/detail/project.hpp>
#define PSYCLE__DECL  PSYCLE__STREAM
#include <psycle/detail/decl.hpp>
namespace psycle { namespace stream {

class PSYCLE__DECL format {
	public:
		typedef double real;

	public:
		virtual ~format() throw() {}

	protected:

		virtual void         samples_per_second(unsigned int)       = 0;
		virtual unsigned int samples_per_second(            ) const = 0;

		virtual void          channels(unsigned char)       = 0;
		virtual unsigned char channels(             ) const = 0;

		virtual void          significant_bits_per_channel_sample(unsigned char)       = 0;
		virtual unsigned char significant_bits_per_channel_sample(             ) const = 0;

		virtual void          bits_per_channel_sample(unsigned char)       = 0;
		virtual unsigned char bits_per_channel_sample(             ) const = 0;

		virtual real bytes_per_channel_sample() const = 0;
		virtual real bytes_per_sample        () const = 0;

		virtual void sample_signed(bool)       = 0;
		virtual bool sample_signed(    ) const = 0;

	#if 0 // should go in a subclass
		virtual void sample_endianness(universalis::cpu::endianness::type) = 0;
		virtual universalis::cpu::endianness::type sample_endianness() const = 0;
	#endif
};

}}
#include <psycle/detail/decl.hpp>
#endif
