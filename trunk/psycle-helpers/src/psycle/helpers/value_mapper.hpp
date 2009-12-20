///\interface psycle::helpers
#pragma once

namespace psycle { namespace helpers {

/// linearly maps a byte (0 to 255) to a float (0 to 1).
///\todo needs some explanation about why the tables have a length of 257.
namespace value_mapper {

namespace detail {
	float static map_table_255_1[257];
	float static map_table_255_100[257];
}

/// maps a byte (0 to 255) to a float (0 to 1).
float inline map_255_1(int byte) {
	///\todo the conditional branches probably make it slower than direct calculation
	if(0 <= byte && byte <= 256)
		return detail::map_table_255_1[byte];
	else
		return byte * 0.00390625f;
}


/// maps a byte (0 to 255) to a float (0 to 100).
float inline map_255_100(int byte) {
	///\todo the conditional branches probably make it slower than direct calculation
	if(0 <= byte && byte <= 256)
		return detail::map_table_255_100[byte];
	else
		return byte * 0.390625f;
}

}}}
