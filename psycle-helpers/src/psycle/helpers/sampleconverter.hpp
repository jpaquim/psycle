#pragma once
#include <universalis.hpp>

namespace psycle
{
	namespace helpers
	{
		using namespace universalis::stdlib;

		inline uint8_t int16touint8(int16_t);
		inline uint8_t int24touint8(int32_t);
		inline uint8_t int32touint8(int32_t);
		inline uint8_t floattouint8(float val, double multi=127.f); // 7bits
		inline int16_t uint8toint16(uint8_t);
		inline int16_t int24toint16(int32_t);
		inline int16_t int32toint16(int32_t);
		inline int16_t floattoint16(float val, double multi=32767.f); // 15bits
		inline int32_t uint8toint24(uint8_t);
		inline int32_t int16toint24(int16_t);
		inline int32_t int32toint24(int32_t);
		inline int32_t floattoint24(float val, double multi=8388607.0);//23bits
		inline int32_t uint8toint32(uint8_t);
		inline int32_t int16toint32(int16_t);
		inline int32_t int24toint32(int32_t);
		inline int32_t floattoint32(float val, double multi=2147483647.0);//31bits
		inline float uint8tofloat(uint8_t val, double multi=0.0078125f); // 1/128
		inline float int16tofloat(int16_t val, double multi=0.000030517578125f); // 1/32768
		inline float int24tofloat(int32_t val, double multi=0.00000011920928955078125); // 1/8388608
		inline float int32tofloat(int32_t val, double multi=0.0000000004656612873077392578125); // 1/2147483648

		//unpack 4 int24 values (in 12bytes, out 16bytes).
		inline void unpackint24(int32_t *in, int32_t *out);
		//pack 4 int24 values (in 16bytes, out 12bytes).
		inline void packint24(int32_t *in, int32_t *out);



		/// usage: integerconverter<uint8_t,int8_t,assignconverter<uint8_t, uint8_t>(bla,bla2,10);
		template<typename in_type, typename out_type>
		inline out_type assignconverter(in_type a) { return a; }

		/// usage: integerconverter<uint8_t,int16_t,uint8toint16>(bla,bla2,10);
		template<typename in_type, typename out_type, out_type (*converter_func)(in_type)>
		void integerconverter(in_type* in, out_type* out, std::size_t numsamples) {
			for(std::size_t io = 0 ; io < numsamples ; ++io){
				*out=converter_func(*in);
				out++;
				in++;
			}
		}

		/// usage: floatconverter<float,int16_t,floattoint16>(bla,bla2,10, 32767.f);
		template<typename in_type, typename out_type, out_type (*converter_func)(in_type, double)>
		void floatconverter(in_type* in, out_type* out, std::size_t numsamples, double multi) {
			for(std::size_t io = 0 ; io < numsamples ; ++io){
				*out=converter_func(*in, multi);
				out++;
				in++;
			}
		}
	}
}

