#include "install_paths.hpp"
#if defined XPSYCLE__CONFIGURATION
	#include <xpsycle/install_paths.defines.hpp>
#endif

namespace xpsycle
{
	namespace paths
	{
		std::string const & doc()
		{
			std::string const static result
			(
				#if defined XPSYCLE__INSTALL_PATHS__DOC
					XPSYCLE__INSTALL_PATHS__DOC
				#else
					"doc"
				#endif
			);
			return result;
		}
		
		std::string const & pixmaps()
		{
			std::string const static result
			(
				#if defined XPSYCLE__INSTALL_PATHS__PIXMAPS
					XPSYCLE__INSTALL_PATHS__PIXMAPS
				#else
					"pixmaps"
				#endif
			);
			return result;
		}	
		
		std::string const & presets()
		{
			std::string const static result
			(
				#if defined XPSYCLE__INSTALL_PATHS__PRESETS
					XPSYCLE__INSTALL_PATHS__PRESETS
				#else
					"presets"
				#endif
			);
			return result;
		}
	}
}
