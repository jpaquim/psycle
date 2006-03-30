// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\implementation psycle::stream::format::riff_wave
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
#include <psycle/detail/project.private.hpp>
#include <cstring>
#include <sstream>
#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	//#include <msacm.h>
#endif
#include "format.hpp"
namespace psycle
{
	namespace stream
	{
		namespace formats
		{
			namespace riff_wave
			{
				format::format(const int & channels, const int & samples_per_second, const int & bits_per_channel_sample) throw(std::bad_alloc)
				{
					allocate_chunk(0);
					chunk().tag(pcm);
					chunk().set(channels, samples_per_second, bits_per_channel_sample);
				};

				format::format(const format & format) throw(std::bad_alloc)
				{
					allocate_chunk(format.chunk_->extra_information_size());
					std::memcpy(&this->chunk(), &format.chunk(), chunk().size());
				};
			
				format::~format() throw()
				{
					std::free(&chunk());
				}
			
				void format::allocate_chunk(const int & extra_information_size) throw(std::bad_alloc)
				{
					if(!(chunk_ = static_cast<Chunk*>(std::malloc(sizeof(Chunk) + extra_information_size)))) throw std::bad_alloc();
					chunk().extra_information_size_ = static_cast<universalis::compiler::numeric<16>::unsigned_int>(extra_information_size);
				}
			
				/*
				inline int extra_information_size_which_fits_all_tags() throw(exception)
				{
					static int extra_information_size_which_fits_all_tags = 0;
					if(!extra_information_size_which_fits_all_tags)
					{
						if(!::acmMetrics(0, ACM_METRIC_MAX_SIZE_FORMAT, &extra_information_size_which_fits_all_tags)) throw exception("acm metrics", UNIVERSALIS__COMPILER__LOCATION);
						chunk_extra_information_size_which_fits_all_tags -= sizeof(chunk);
					}
					return chunk_extra_information_size_which_fits_all_tags;
				}
				*/
			
				namespace
				{
					inline int bits_to_bytes(const int & bits)
					{
						const int bytes(bits >> 3);
						return bits & 7 ? bytes + 1 : bytes;
					}
				}
			
				void format::chunk::recompute_if_pcm(const bool & full)
				{
					if(tag() == pcm)
					{
						if(full) bytes_per_sample_ = static_cast<universalis::compiler::numeric<16>::unsigned_int>(channels() * bits_to_bytes(bits_per_channel_sample()));
						average_bytes_per_second_ = samples_per_second() * bytes_per_sample_;
					}
				}
			
				void format::chunk::set(const int & channels, const int & samples_per_second, const int & bits_per_channel_sample)
				{
					this->channels_ = static_cast<universalis::compiler::numeric<16>::unsigned_int>(channels);
					this->samples_per_second_ = static_cast<universalis::compiler::numeric<32>::unsigned_int>(samples_per_second);
					this->bits_per_channel_sample_ = static_cast<universalis::compiler::numeric<16>::unsigned_int>(bits_per_channel_sample);
					recompute_if_pcm();
				};
			
				void format::chunk::channels(const int & channels)
				{
					this->channels_ = static_cast<universalis::compiler::numeric<16>::unsigned_int>(channels);
					recompute_if_pcm();
				}
			
				void format::chunk::samples_per_second(const int & samples_per_second)
				{
					this->samples_per_second_ = static_cast<universalis::compiler::numeric<32>::unsigned_int>(samples_per_second);
					recompute_if_pcm(false);
				}
			
				void format::chunk::bits_per_channel_sample(const int & bits_per_channel_sample)
				{
					this->bits_per_channel_sample_ = static_cast<universalis::compiler::numeric<16>::unsigned_int>(bits_per_channel_sample);
					recompute_if_pcm();
				}
			
				format::real format::chunk::bytes_per_sample() const
				{
					if(tag() == pcm) return bytes_per_sample_;
					else return average_bytes_per_second_ / static_cast<real>(samples_per_second());
				}
			
				int format::chunk::bytes_to_samples(const int & bytes) const
				{
					if(tag() == pcm) return bytes / bytes_per_sample_;
					else return static_cast<int>(bytes / bytes_per_sample());
				}
			
				void format::chunk::tag(const Tag & tag)
				{
					this->tag_ = static_cast<universalis::compiler::numeric<16>::unsigned_int>(tag);
				}
			
				std::string format::tag_description() const
				{
					/*
					::ACMFORMATTAGDETAILS tag_details;
					::memset(&tag_details, 0, sizeof tag_details);
					tag_details.cbStruct = sizeof tag_details;
					tag_details.dwFormatTag = tag();
					if(!::acmFormatTagDetails(0, &tag_details, ACM_FORMATTAGDETAILSF_FORMATTAG))
						throw exception("acm format details", UNIVERSALIS__COMPILER__LOCATION);
					return std::string(tag_details.szFormatTag);
					*/
					return "pcm";
				}
			
				std::string format::description() const
				{
					/*
					if(bits_per_channel_sample() <= 16) // acm does not support more
					{
						::ACMFORMATDETAILS details;
						memset(&details, 0, sizeof details);
						details.cbStruct = sizeof details;
						details.dwFormatTag = tag();
						details.pwfx = &const_cast<Format&>(*this).wave_format_ex();
						details.cbwfx = chunk().extra_information_size();
						if(!::acmFormatDetails(0, &details, ACM_FORMATDETAILSF_FORMAT))
							throw exception("acm format details", UNIVERSALIS__COMPILER__LOCATION);
						return std::string(details.szFormat);
					}
					else
					*/
					{
						std::stringstream s; s
							<< tag_description() << ", "
							<< channels() << " channels of "
							<< bits_per_channel_sample() << "-bit samples at "
							<< samples_per_second()  << " hertz";
						return s.str();
					}
				}
			
				/*
				format::chunk::write(riff & riff)
				{
					if(extra_information_size()) riff.update_or_create_chunk("fmt ", this(), size());
					else riff.update_or_create_chunk("fmt ", this, size() - sizeof extra_information_size_);
				}
			
				format::chunk::write(riff & riff, const int & bytes);
				{
					write(riff);
					if(tag() != pcm)
					{
						universalis::compiler::numeric<32>::unsigned_int samples = bytes_to_samples(data_size);
						riff.update_or_create_chunk('fact', samples, sizeof samples);
					}
				}
			
				format::format(riff & riff) throw(exception)
				{
					var ExSize: UInt16;
					allocate_chunk(extra_information_size_which_fits_all_tags());
					if(!riff.read_chunk('fmt ', chunk(), sizeof(Chunk) - 2) throw Exception("bad fmt chunk");
					if(riff.current_chunk_data_size() < sizeof(Chunkt) - 2) throw Exception("file " + file_name() + " corrupted");
					if(tag() != pcm)
					{
						universalis::compiler::numeric<16>::unsigned_int extra_information_size;
						riff >> extra_information_size;
						if(riff.current_chunk_data_size() < sizeof(Chunk) + extra_information_size) throw Exception("file " + file_name() + " corrupted: fmt chunk too small to contain the declared extra data");
						riff(-2);
						riff.read(&chunk().extra_information_size_, 2 + extra);
					}
				}
				*/
			
				/*
				bool format::user_choose_dialog(const HWnd & window_handle, const format * const source_format, const format * const proposed_format, const string & title)
				{
					::ACMFORMATCHOOSE choose;
					::memset(&choose, 0, sizeof choose);
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
					default: throw exception("acm format choose", UNIVERSALIS__COMPILER__LOCATION);
					}
				}
				*/
			}
		}
	}
}
