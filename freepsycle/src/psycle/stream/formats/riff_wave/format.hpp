// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2011 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

#pragma once
#include <psycle/detail/project.hpp>
#include "../../format.hpp"
#include <string>
#include <exception>
#include <cstddef>
#if defined DIVERSALIS__OS__MICROSOFT
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
#define PSYCLE__DECL  PSYCLE__STREAM
#include <psycle/detail/decl.hpp>
namespace psycle { namespace stream { namespace formats { namespace riff_wave {

class PSYCLE__DECL format : public stream::format {
	///\name riff wave fmt chunk
	///\{
		public:

			#if 0
				format(riff &);
				/// fmt chunk only
				void write(riff &);
				/// fmt & fact chunks
				void write(riff &, std::size_t);
			#endif

			#if 0 && defined DIVERSALIS__OS__MICROSOFT
				bool user_choose_dialog(HWnd const window_handle, format const * const source_format = 0, format const * const proposed_format = 0; std::string const & caption = "");
			#endif

		private:
			class UNIVERSALIS__COMPILER__ALIGN(1) chunk_type {
				friend class format;

				public:
					#if defined DIVERSALIS__OS__MICROSOFT
						operator ::WAVEFORMATEX & () { return reinterpret_cast< ::WAVEFORMATEX& >(*this); }
					#endif

				public:
					void     tag(uint16_t);
					uint16_t tag() const { return tag_; }
				private:
					uint16_t tag_; // tags::type enumerated type, but forced to be 16-bit long

				public:
					void     channels(uint16_t);
					uint16_t channels() const { return channels_; }
				private:
					uint16_t channels_;

				public:
					void     samples_per_second(uint32_t);
					uint32_t samples_per_second() const { return samples_per_second_; }
				private:
					uint32_t samples_per_second_;

				private:
					uint32_t average_bytes_per_second_;

				public:
					real     bytes_per_channel_sample() const { return bytes_per_sample() / channels(); }
					real     bytes_per_sample        () const;
				private:
					uint16_t bytes_per_sample_;
					unsigned int bytes_to_samples(unsigned int) const;

				public:
					void     bits_per_channel_sample(uint16_t);
					uint16_t bits_per_channel_sample() const { return bits_per_channel_sample_; }
				private:
					uint16_t bits_per_channel_sample_;

				public:
					void set(uint16_t channels = 2, uint32_t samples_per_second = 44100, uint16_t bits_per_channel_sample = 16);

				void recompute_if_pcm(bool full = true);

				public:
					std::size_t size() const { return sizeof *this + extra_information_size(); }
				private:
					/// the count in bytes of the size of extra information, after the size itself
					uint16_t extra_information_size() const { return extra_information_size_; }
					uint16_t extra_information_size_;
					// variable size ... \todo make a custom allocator by overloading operator new(const size_t &)
					//int8_t extra[];
			};

			chunk_type * chunk_;
			chunk_type & chunk() const { return *chunk_; }

			void allocate_chunk(std::size_t extra_information_size);

		public:
			struct tags { enum type { // not meant to be exhaustive
				unknown          = 0x0000,
				pcm              = 0x0001,
				ieee_float       = 0x0003,
				iso_gsm_610      = 0x0031,
				iso_mpeg_layer_3 = 0x0055,
				vorbis           = unknown // i do not know what would be the identifier ... actually, i only saw vorbis in a ogg container, never in a riff one
			}; };

			void       tag(tags::type tag)       { chunk().tag(tag); }
			tags::type tag(              ) const { return static_cast<tags::type>(chunk().tag()); }

		private:
			std::string tag_description() const;
	///\}

	public:
		format(unsigned char channels = 2, real samples_per_second = 44100, unsigned char bits_per_channel_sample = 16, unsigned char significant_bits_per_channel_sample = 16);
		format(format const &);
		virtual ~format() throw();

		virtual inline void          samples_per_second(unsigned int value)       {        chunk().samples_per_second(value); }
		virtual inline unsigned int  samples_per_second(                  ) const { return chunk().samples_per_second(     ); }

		virtual inline void          channels(unsigned char value)       {        chunk().channels(value); }
		virtual inline unsigned char channels(                   ) const { return chunk().channels(     ); }

		virtual inline void          significant_bits_per_channel_sample(unsigned char value)       {        bits_per_channel_sample(value); }
		virtual inline unsigned char significant_bits_per_channel_sample(                   ) const { return bits_per_channel_sample(     ); }

		virtual inline void          bits_per_channel_sample(unsigned char value)                         {        chunk().bits_per_channel_sample(value); }
		virtual inline unsigned char bits_per_channel_sample(                                     ) const { return chunk().bits_per_channel_sample(     ); }

		virtual inline real          bytes_per_channel_sample() const { return chunk().bytes_per_sample(); }
		virtual inline real          bytes_per_sample        () const { return chunk().bytes_per_sample(); }
		
		///\todo
		virtual inline void sample_signed(bool)       {              }
		///\todo
		virtual inline bool sample_signed(    ) const { return true; }

		#if 0 // it's handled by riff's root chunk header id: either "RIFF" or "RIFX"
			void virtual sample_endianness(universalis::cpu::endianness::type) {}
			universalis::cpu::endianness::type virtual sample_endianness() const { return universalis::cpu::endianness::big; }
		#endif

		std::string description() const;

		#if defined DIVERSALIS__OS__MICROSOFT
			operator ::WAVEFORMATEX & () { return chunk(); }
			::WAVEFORMATEX & wave_format_ex() { return *this; }
		#endif
};

}}}}
#include <psycle/detail/decl.hpp>
