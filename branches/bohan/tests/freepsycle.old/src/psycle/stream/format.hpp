// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::stream::format
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

		void         virtual samples_per_second(unsigned int)       = 0;
		unsigned int virtual samples_per_second(            ) const = 0;

		void          virtual channels(unsigned char)       = 0;
		unsigned char virtual channels(             ) const = 0;

		void          virtual significant_bits_per_channel_sample(unsigned char)       = 0;
		unsigned char virtual significant_bits_per_channel_sample(             ) const = 0;

		void          virtual bits_per_channel_sample(unsigned char)       = 0;
		unsigned char virtual bits_per_channel_sample(             ) const = 0;

		real virtual bytes_per_channel_sample() const = 0;
		real virtual bytes_per_sample        () const = 0;

		void virtual sample_signed(bool)       = 0;
		bool virtual sample_signed(    ) const = 0;

	#if 0 // should go in a subclass
		void virtual sample_endianness(universalis::cpu::endianness::type) = 0;
		universalis::cpu::endianness::type virtual sample_endianness() const = 0;
	#endif
};

}}
#include <psycle/detail/decl.hpp>
