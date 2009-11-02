// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\interface psycle::stream::format
#pragma once
#include <psycle/detail/project.hpp>
#include <universalis/compiler/numeric.hpp>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__STREAM__FORMAT
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace stream {
	class UNIVERSALIS__COMPILER__DYNAMIC_LINK format {
		public:
			typedef universalis::compiler::numeric<>::floating_point real;

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
			void                                     virtual sample_endianness(universalis::processor::endianness::type)       = 0;
			universalis::processor::endianness::type virtual sample_endianness(                                        ) const = 0;
		#endif
	};
}}
#include <universalis/compiler/dynamic_link/end.hpp>

