// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2006 johan boule <bohan@jabber.org>
// copyright 2006 psycledelics development team <http://psycle.sourceforge.net>

#include <stdexcept>
#include <cassert>
//#include <boost/utility/enable_if.hpp>
namespace ngrs
{
	/// converts a color from client to server
	template
	<
		unsigned char Client_Bits,
		typename Value_Type,
		Value_Type const Opaque_Value,
		unsigned char Channel_Count
	>
	class basic_color_converter
	{
		public:
			/// number of channels a color comprises
			unsigned char const static channel_count = Channel_Count;
			
			/// type used for client-side pixel value and masks
			typedef Value_Type value_type;
			
			///\name client-side channel configuration, same for all channels
			///\{
				/// number of bits per channel used by clients
				unsigned char const static client_bits = Client_Bits;
				/// max value corresponding to client_bits
				value_type const static client_max_value = (1 << client_bits) - 1;
			///\}

		protected:
			class channel
			{
				///\name server-side channel configuration
				///\{
					private:
						value_type mask;
						unsigned char decrease_bits, shift;
					public:
						value_type const static opaque_value = Opaque_Value;
				///\}

				public:
					/// convert a client value to a server value.
					///\return the server value
					value_type inline operator()(value_type client_value) const throw()
					{
						assert(client_value <= client_max_value);
						return client_value >> decrease_bits << shift;
					}
					
				public:
					channel(value_type mask) : mask(mask)
					{
						// would make an endless loop is no bit is on
						if(!mask)
						{
							decrease_bits = client_bits;
							shift = 0;
							return;
						}

						// we know the mask has all its bits contiguous,
						// i.e., we cannot have something like this in one mask: 0011001100.
						// skip the bits that are off.
						for(shift = 0; !(mask & 1); mask >>= 1) ++shift;

						// count the bits that are on.
						unsigned char bits(0); for(; mask & 1; mask >>= 1) ++bits;
						
						// difference between client and server bits
						assert(client_bits >= bits); decrease_bits = client_bits - bits;

						// assert that we computed it right
						assert((*this)(client_max_value) == this->mask);
					}

				private: friend class basic_color_converter;
					/// uninitialized storage used by the enclosing class only
					inline channel() {} 
				};

			channel channels[Channel_Count];

			/// uninitialized storage used by derived classes only
			inline basic_color_converter() {}
			
		public:
			
			basic_color_converter(value_type const masks[Channel_Count])
			{
				for(unsigned int i(0); i < Channel_Count; ++i) this->channels[i] = channel(masks[i]);
			}
			
			/// converts a client color to a server one.
			/// integral numeric values in the range [0, client_max_value].
			///\return the server value
			value_type inline operator()(value_type const values[Channel_Count]) const throw()
			{
				value_type result(0);
				for(unsigned int i(0); i < Channel_Count; ++i) result |= channels[i](values[i]);
				return result;
			}
			
			/// converts a client color to a server one.
			/// floating point numeric values in the range [0, 1].
			template<typename Floating_Point_Numeric>
			value_type inline real(Floating_Point_Numeric const values[Channel_Count]) const throw()
			{
				value_type result(0);
				for(unsigned int i(0); i < Channel_Count; ++i) result |= channels[i](values[i] * client_max_value);
				return result;
			}
	};

	template
	<
		unsigned char Client_Bits = 8,
		typename Value_Type = unsigned int,
		Value_Type const Opaque_Value = 0,
		unsigned char Channel_Count = 4
	>
	class color_converter : public  basic_color_converter<Client_Bits, Value_Type, Opaque_Value, Channel_Count>
	{
		public:
			color_converter(typename color_converter::value_type const masks[Channel_Count]) : basic_color_converter<Client_Bits, Value_Type, Opaque_Value, Channel_Count>(masks) {}
	};
	
	/// template specialization for 4 channels
	template
	<
		unsigned char Client_Bits,
		typename Value_Type,
		Value_Type const Opaque_Value
	>
	class color_converter<Client_Bits, Value_Type, Opaque_Value> : public  basic_color_converter<Client_Bits, Value_Type, Opaque_Value, 4>
	{
		protected:
			typedef typename color_converter::channel channel;
		public:
			typedef typename color_converter::value_type value_type;
			
			/// default initialization
			color_converter()
			{
				for(unsigned int i(0); i < this->channel_count; ++i) this->channels[i] = channel(0);
			}
			
			/// 4 channels, or 3 channels with default value for alpha
			color_converter
			(
				value_type const red,
				value_type const green,
				value_type const blue,
				value_type const alpha = 0
			)
			{
				this->channels[0] = channel(red  );
				this->channels[1] = channel(green);
				this->channels[2] = channel(blue );
				this->channels[3] = channel(alpha);
			}
			
			/// converts a client color to a server one.
			/// integral numeric values in the range [0, client_max_value].
			/// 4 channels, or 3 channels with default value for alpha
			value_type inline operator()
			(
				value_type const red,
				value_type const green,
				value_type const blue,
				value_type const alpha = Opaque_Value
			) const throw()
			{
				return
					this->channels[0](red  ) |
					this->channels[1](green) |
					this->channels[2](blue ) |
					this->channels[3](alpha);
			}
			
			/// converts a client color to a server one.
			/// floating point numeric values in the range [0, 1].
			/// 4 channels, or 3 channels with default value for alpha
			template<typename Floating_Point_Numeric>
			value_type inline real
			(
				Floating_Point_Numeric const red,
				Floating_Point_Numeric const green,
				Floating_Point_Numeric const blue,
				Floating_Point_Numeric const alpha = Opaque_Value
			) const throw()
			{
				return
					(*this)
					(
						red   * this->client_max_value,
						green * this->client_max_value,
						blue  * this->client_max_value,
						alpha * this->client_max_value
					);
			}
	};
}
