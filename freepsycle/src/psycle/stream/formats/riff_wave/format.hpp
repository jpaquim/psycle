// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\interface psycle::stream::format::riff_wave
#pragma once
#include <psycle/detail/project.hpp>
#include "../../format.hpp"
#include <string>
#include <exception>
#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#include <windows.h>
	#if defined DIVERSALIS__COMPILER__MICROSOFT
		#pragma warning(push)
		#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
	#endif
	#include <mmsystem.h>
	#if defined DIVERSALIS__COMPILER__MICROSOFT
		#pragma warning(pop)
	#endif
	//#include <mmreg.h>
#endif
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__STREAM__FORMAT__RIFF_WAVE
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace stream
	{
		namespace formats
		{
			namespace riff_wave
			{
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK format : public stream::format
				{
				public:
					format(const int & channels = 2, const int & samples_per_second = 44100, const int & bits_per_channel_sample = 16) throw(std::bad_alloc);
					format(const format &) throw(std::bad_alloc);
					//bool user_choose_dialog(const HWnd window_handle, const format * const source_format = 0, const format * const proposed_format = 0; const std::string & title = "");
					//format(riff &) throw(exception)
					virtual ~format() throw();

					class tags
					{
						public:
							enum tag // from microsoft's <mmreg.h> ... not meant to be exhaustive
							{
								unknown         = 0x0000,
								pcm             = 0x0001,
								ieee_float      = 0x0003,
								gsm610          = 0x0031,
								iso_mpeg_layer3 = 0x0055,
								vorbis          = unknown // i do not know what would be the identifier ... actually, i only saw vorbis in a ogg container, never in a riff one
							};
					};
					using tags::tag;
					typedef tag Tag; // msvc (at least version 6) has problems with type name resolution
					
					//void write(riff &); // fmt chunk only
					//void write(riff &, const int & bytes); // fmt & fact chunks

					inline tag tag() const { return chunk().tag(); }
					inline void tag(const Tag & tag) { chunk().tag(tag); }
					std::string tag_description() const;
					std::string description() const;

					inline virtual int channels() const { return chunk().channels(); }
					inline virtual void channels(const int & channels) { chunk().channels(channels); }
					inline virtual int samples_per_second() const { return chunk().samples_per_second(); }
					inline virtual void samples_per_second(const int & samples_per_second) { chunk().samples_per_second(samples_per_second); }
					inline virtual int bits_per_channel_sample() const { return chunk().bits_per_channel_sample(); }
					inline virtual void bits_per_channel_sample(const int & bits_per_channel_sample) { chunk().bits_per_channel_sample(bits_per_channel_sample); }
					inline virtual real bytes_per_channel_sample() const { return chunk().bytes_per_sample(); }
					inline virtual real bytes_per_sample() const { return chunk().bytes_per_sample(); }
					
					#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
						inline ::WAVEFORMATEX & wave_format_ex() { return reinterpret_cast< ::WAVEFORMATEX& >(chunk()); }
					#endif

				private:
					class chunk;
					typedef chunk Chunk; // msvc has problems with type name resolution
					chunk * chunk_;
					inline chunk & chunk() const { return *chunk_; }
					void allocate_chunk(const int & extra_information_size) throw(std::bad_alloc);
					#if defined DIVERSALIS__COMPILER__MICROSOFT
						#pragma pack(push, 1)
					#endif
					class UNIVERSALIS__COMPILER__ATTRIBUTE(packed) chunk
					{
						friend class format;

						universalis::compiler::numeric<16>::unsigned_int tag_; // Tag enumerated type, but forced to be 16-bit long
						inline Tag tag() const { return static_cast<Tag>(tag_); }
						void tag(const Tag &);

						universalis::compiler::numeric<16>::unsigned_int channels_;
						inline int channels() const { return channels_; }
						void channels(const int &);

						universalis::compiler::numeric<32>::unsigned_int samples_per_second_;
						inline int samples_per_second() const { return samples_per_second_; }
						void samples_per_second(const int &);

						universalis::compiler::numeric<32>::unsigned_int average_bytes_per_second_;

						universalis::compiler::numeric<16>::unsigned_int bytes_per_sample_;
						inline double bytes_per_channel_sample() const { return bytes_per_sample() / channels(); }
						double bytes_per_sample() const;
						int bytes_to_samples(const int &) const;

						universalis::compiler::numeric<16>::unsigned_int bits_per_channel_sample_;
						inline int bits_per_channel_sample() const { return bits_per_channel_sample_; }
						void bits_per_channel_sample(const int &);

						void set(const int & channels = 2, const int & samples_per_second = 44100, const int & bits_per_channel_sample = 16);
						void recompute_if_pcm(const bool & full = true);

						universalis::compiler::numeric<16>::unsigned_int extra_information_size_; // the count in bytes of the size of extra information, after the size itself
						inline int size() const { return sizeof *this + extra_information_size(); }
						inline int extra_information_size() const { return extra_information_size_; }
						// variable size ... \todo make a custom allocator by overloading operator new(const size_t &)
					};
					#if defined DIVERSALIS__COMPILER__MICROSOFT
						#pragma pack(pop)
					#endif
				};
			}
		}
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
