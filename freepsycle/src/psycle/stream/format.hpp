// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\interface psycle::stream::format
#pragma once
#include <psycle/detail/project.hpp>
#include <universalis/compiler/numeric.hpp>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__STREAM__FORMAT
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace stream
	{
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK format
		{
			public:
				typedef universalis::compiler::numeric<>::floating_point real;
				virtual ~format() throw() {}
			protected:
				int  virtual channels(           ) const = 0;
				void virtual channels(int const &)       = 0;

				int  virtual samples_per_second(           ) const = 0;
				void virtual samples_per_second(int const &) = 0;

				int  virtual bits_per_channel_sample(           ) const = 0;
				void virtual bits_per_channel_sample(int const &)       = 0;

				real virtual bytes_per_channel_sample() const = 0;

				real virtual bytes_per_sample() const = 0;
		};
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
