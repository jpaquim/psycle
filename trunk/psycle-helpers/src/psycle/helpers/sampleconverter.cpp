#include "sampleconverter.hpp"
#include <psycle/helpers/math.hpp>
namespace psycle
{
	namespace helpers
	{
		uint8_t int16touint8(int16_t val) {
			return static_cast<uint8_t>((static_cast<uint16_t>(val) + 32768U) >> 8);
		}
		uint8_t int24touint8(int32_t val) {
			return static_cast<uint8_t>((val + 8388608) >> 16);
		}
		uint8_t int32touint8(int32_t val) {
			return static_cast<uint8_t>((static_cast<uint32_t>(val) + 2147483648U) >> 24);
		}
		uint8_t floattouint8(float val, double multi) {
			return math::lrint<uint8_t,float>(std::floor(val*multi));
		}
		int16_t uint8toint16(uint8_t val) {
			return static_cast<int16_t>((static_cast<int16_t>(val) << 8) - 32768);
		}
		int16_t int24toint16(int32_t val) {
			return static_cast<int16_t>(val >> 8);
		}
		int16_t int32toint16(int32_t val) {
			return static_cast<int16_t>(val >> 16);
		}
		int16_t floattoint16(float val, double multi) {
			return math::lrint<uint16_t,float>(std::floor(val*multi));
		}
		int32_t uint8toint24(uint8_t val) {
			return (static_cast<int32_t>(val) << 16) - 8388608;
		}
		int32_t int16toint24(int16_t val){
			return val << 8;
		}
		int32_t int32toint24(int32_t val){
			return val >> 8;
		}
		int32_t floattoint24(float val, double multi) {
			return math::lrint<uint32_t,float>(std::floor(val*multi));
		}
		int32_t uint8toint32(uint8_t val) {
			return (static_cast<int32_t>(val) << 24) - 2147483648;
		}
		int32_t int16toint32(int16_t val) {
			return val << 16;
		}
		int32_t int24toint32(int32_t val) {
			return val << 8;
		}
		int32_t floattoint32(float val, double multi) {
			return math::lrint<uint32_t,float>(std::floor(val*multi));
		}
		float uint8tofloat(uint8_t val, double multi) {
			return val*multi-1.f;
		}
		float int16tofloat(int16_t val, double multi) {
			return val*multi;
		}
		float int24tofloat(int32_t val, double multi) {
			return val*multi;
		}
		float int32tofloat(int32_t val, double multi) {
			return val*multi;
		}

		void unpackint24(int32_t *in, int32_t *out)
		{
			out[0]=                          in[0] >>  8;
			out[1]= ((in[0]&0xFF)  << 16) | (in[1] >> 16);
			out[2]= ((in[1]&0xFFFF)<<  8) | (in[2] >> 24);
			out[3]=   in[2]&0xFFFFFF;
		}
		void packint24(int32_t *in, int32_t *out)
		{
			out[0]=(in[0] <<  8) | ((in[1]&0xFFFFFF) >> 16);
			out[1]=(in[1] << 16) | ((in[2]&0xFFFFFF) >>  8);
			out[2]=(in[2] << 24) | ((in[3]&0xFFFFFF));
		}

	}
}

