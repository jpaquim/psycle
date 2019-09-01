// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\implementation psycle::stream::format::riff_wave
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "format.hpp"
#include <universalis/operating_system/exceptions/code_description.hpp>
#include <cstring>
#include <sstream>
#include <climits>
#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	//#include <msacm.h>
#endif
namespace psycle { namespace stream { namespace formats { namespace riff_wave {

	format::format(unsigned char channels, real samples_per_second, unsigned char significant_bits_per_channel_sample, unsigned char bits_per_channel_sample)
	{
		allocate_chunk(0);
		chunk().tag(tags::pcm);
		chunk().set(channels, static_cast<std::uint32_t>(samples_per_second), bits_per_channel_sample);
	};

	format::format(format const & format)
	{
		allocate_chunk(format.chunk_->extra_information_size());
		std::memcpy(&this->chunk(), &format.chunk(), chunk().size());
	};

	format::~format() throw()
	{
		std::free(&chunk());
	}

	std::string format::description() const
	{
		#if 0 && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
			if(bits_per_channel_sample() <= 16) // acm does not support more
			{
				::ACMFORMATDETAILS details;
				std::memset(&details, 0, sizeof details);
				details.cbStruct = sizeof details;
				details.dwFormatTag = tag();
				details.pwfx = &const_cast<format&>(*this).wave_format_ex();
				details.cbwfx = chunk().extra_information_size();
				if(!::acmFormatDetails(0, &details, ACM_FORMATDETAILSF_FORMAT))
					throw exception("acm format details: " + universalis::operating_system::exceptions::code_description(), UNIVERSALIS__COMPILER__LOCATION);
				return std::string(details.szFormat);
			}
			else
		#endif
		{
			std::ostringstream s; s
				<< tag_description() << ", "
				<< static_cast<unsigned int>(channels()) << " channels of "
				<< static_cast<unsigned int>(bits_per_channel_sample()) << "-bit samples at "
				<< samples_per_second()  << " hertz";
			return s.str();
		}
	}

	std::string format::tag_description() const
	{
		#if 0 && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
			::ACMFORMATTAGDETAILS tag_details;
			std::memset(&tag_details, 0, sizeof tag_details);
			tag_details.cbStruct = sizeof tag_details;
			tag_details.dwFormatTag = tag();
			if(!::acmFormatTagDetails(0, &tag_details, ACM_FORMATTAGDETAILSF_FORMATTAG))
				throw exception("acm format tag details: " + universalis::operating_system::exceptions::code_description(), UNIVERSALIS__COMPILER__LOCATION);
			return std::string(tag_details.szFormatTag);
		#else
			switch(tag())
			{
				case tags::pcm: return "pcm";
				default: return "non pcm";
			}
		#endif
	}

	void format::allocate_chunk(std::size_t extra_information_size) throw(std::bad_alloc)
	{
		if(!(chunk_ = static_cast<chunk_type*>(std::malloc(sizeof(chunk_type) + extra_information_size)))) throw std::bad_alloc();
		chunk().extra_information_size_ = static_cast<std::uint16_t>(extra_information_size);
	}

	namespace
	{
		///\todo this is general purpose, should be moved to universalis
		unsigned int inline bits_to_bytes(unsigned int bits)
		{
			#if !defined CHAR_BIT
				#error missing #include <climits>
			#elif CHAR_BIT == 8
				unsigned int const bytes(bits >> 3);
				return bits & 7 ? bytes + 1 : bytes;
			#else
				return (bits + CHAR_BITS - 1) / CHAR_BIT;
			#endif
		}
	}

	void format::chunk_type::recompute_if_pcm(bool full)
	{
		if(tag() == tags::pcm)
		{
			if(full) bytes_per_sample_ = static_cast<std::uint16_t>(channels() * bits_to_bytes(bits_per_channel_sample()));
			average_bytes_per_second_ = samples_per_second() * bytes_per_sample_;
		}
	}

	void format::chunk_type::set(std::uint16_t channels, std::uint32_t samples_per_second, std::uint16_t bits_per_channel_sample)
	{
		this->channels_                = static_cast<std::uint16_t>(channels);
		this->samples_per_second_      = static_cast<std::uint32_t>(samples_per_second);
		this->bits_per_channel_sample_ = static_cast<std::uint16_t>(bits_per_channel_sample);
		recompute_if_pcm();
	};

	void format::chunk_type::channels(std::uint16_t channels)
	{
		this->channels_ = static_cast<std::uint16_t>(channels);
		recompute_if_pcm();
	}

	void format::chunk_type::samples_per_second(std::uint32_t samples_per_second)
	{
		this->samples_per_second_ = static_cast<std::uint32_t>(samples_per_second);
		recompute_if_pcm(false);
	}

	void format::chunk_type::bits_per_channel_sample(std::uint16_t bits_per_channel_sample)
	{
		this->bits_per_channel_sample_ = static_cast<std::uint16_t>(bits_per_channel_sample);
		recompute_if_pcm();
	}

	format::real format::chunk_type::bytes_per_sample() const
	{
		if(tag() == tags::pcm) return bytes_per_sample_;
		else return average_bytes_per_second_ / static_cast<real>(samples_per_second());
	}

	unsigned int format::chunk_type::bytes_to_samples(unsigned int bytes) const
	{
		if(tag() == tags::pcm) return bytes / bytes_per_sample_;
		else return static_cast<unsigned int>(bytes / bytes_per_sample());
	}

	void format::chunk_type::tag(std::uint16_t tag)
	{
		this->tag_ = tag;
	}

	#if 0
		format::format(riff & riff) throw(exception)
		{
			std::uint16_t size;
			allocate_chunk(extra_information_size_which_fits_all_tags());
			if(!riff.read_chunk('fmt ', chunk(), sizeof chunk() - 2) throw exception("bad fmt chunk", UNIVERSALIS__COMPILER__LOCATION);
			if(riff.current_chunk_data_size() < sizeof chunk() - 2) throw exception("file " + file_name() + " corrupted", UNIVERSALIS__COMPILER__LOCATION);
			if(tag() != pcm)
			{
				std::uint16_t extra_information_size;
				riff >> extra_information_size;
				if(riff.current_chunk_data_size() < sizeof chunk() + extra_information_size) throw exception("file: '" + riff.file_name() + "' corrupted: fmt chunk too small to contain the declared extra data", UNIVERSALIS__COMPILER__LOCATION);
				riff(-2);
				riff.read(&chunk().extra_information_size_, 2 + extra_information_size_);
			}
		}

		format::chunk::write(riff & riff)
		{
			if(extra_information_size()) riff.update_or_create_chunk("fmt ", *this, riff.size());
			else riff.update_or_create_chunk("fmt ", *this, riff.size() - sizeof extra_information_size());
		}
	
		format::chunk::write(riff & riff, std::size_t bytes);
		{
			write(riff);
			if(tag() != pcm)
			{
				std::uint32_t samples = bytes_to_samples(data_size);
				riff.update_or_create_chunk('fact', samples, sizeof samples);
			}
		}
	#endif

	#if 0 && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
		namespace
		{
			std::uint16_t extra_information_size_which_fits_all_tags() throw(exception)
			{
				std::uint16_t static extra_information_size_which_fits_all_tags(0);
				if(!extra_information_size_which_fits_all_tags)
				{
					if(!::acmMetrics(0, ACM_METRIC_MAX_SIZE_FORMAT, &extra_information_size_which_fits_all_tags)) throw exception("acm metrics: " + universalis::operating_system::exceptions::code_description(), UNIVERSALIS__COMPILER__LOCATION);
					chunk_extra_information_size_which_fits_all_tags -= sizeof(chunk_type);
				}
				return chunk_extra_information_size_which_fits_all_tags;
			}
		}
	
		bool format::user_choose_dialog(const HWnd & window_handle, const format * const source_format, const format * const proposed_format, const string & title)
		{
			::ACMFORMATCHOOSE choose;
			std::memset(&choose, 0, sizeof choose);
			choose.cbStruct = sizeof choose;
			choose.hwndOwner = window_handle;
			choose.fdwStyle := ACMFORMATCHOOSE_STYLEF_INITTOWFXSTRUCT;
			if(proposed_format && (proposed_format.tag() != unknown)
			{
				choose.pwfx := proposed_format.chunk();
				choose.cbwfx := proposed_format.chunk().extra_information_size();
			}
			else
			{
				if(tag() != unknown) Format();
				choose.pwfx := this->chunk();
				choose.cbwfx := this->chunk().extra_information_size();
			}
			if(source_format)
			{
				if(!title.empty()) choose.pszTitle = title.c_str();
				else choose.pszTitle = "ACM Chooser: supported destination formats";
				choose.fdwEnum := ACM_FORMATENUMF_CONVERT;
				choose.pwfxEnum := source_format.chunk();
			}
			else if(!title.empty()) choose.pszTitle = title.c_str();
			else choose.pszTitle = "ACM Choose: all formats";
			MMResult result = ::acmFilterChoose(choose);
			switch(result)
			{
				case: MMSysErr_NoError: return true;
				case: ACMErr_Canceled: return false;
				default: throw exception("acm format choose: " + universalis::operating_system::exceptions::code_description(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}
	#endif
}}}}
