///\interface psycle::helpers
#pragma once
#include <cassert>

namespace psycle { namespace helpers {

namespace value_mapper {
/// maps a byte (0 to 128) to a float (0 to 1).
float inline map_128_1(int byte) {
		return byte * 0.0078125f;
}

/// maps a byte (0 to 256) to a float (0 to 1).
float inline map_256_1(int byte) {
		return byte * 0.00390625f;
}
/// maps a byte (0 to 256) to a float (0 to 100).
float inline map_256_100(int byte) {
		return byte * 0.390625f;
}
/// maps a byte (0 to 32768) to a float (0 to 1).
float inline map_32768_1(int byte) {
		return byte * 0.000030517578125f;
}
/// maps a byte (0 to 65536) to a float (0 to 1).
float inline map_65536_1(int byte) {
		return byte * 0.0000152587890625f;
}


}}}
