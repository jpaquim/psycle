#pragma once
#include <psycle/helpers/eaiff.hpp>
namespace psycle
{
	namespace helpers
	{
		/*********  IFF file reader comforming to Apple Audio IFF pecifications ****/
		class AppleAIFF : EaIff {
			AppleAIFF();
			virtual ~AppleAIFF();
		};
	}
}

