#pragma once
namespace psycle
{
	namespace common
	{
		namespace math
		{
			/// Cure for malicious samples
			/// Type : Filters Denormals, NaNs, Infinities
			/// References : Posted by urs[AT]u-he[DOT]com
			void inline erase_all_nans_infinities_and_denormals(Sample samples [], unsigned int const sample_count)
			{
				std::uint32_t* inArrayOfFloats(reinterpret_cast<std::uint32_t*>(samples));
				std::uint32_t sample;
				std::uint32_t exponent;
				for(unsigned int i(0); i < sample_count; ++i)
				{
					sample = *inArrayOfFloats;
					exponent = sample & 0x7F800000;

					// exponent < 0x7F800000 is 0 if NaN or Infinity, otherwise 1
					// exponent > 0 is 0 if denormalized, otherwise 1

					*inArrayOfFloats++ = sample * ((exponent < 0x7F800000) & (exponent > 0));
				}
			}
		}
	}
}
