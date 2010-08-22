#pragma once

namespace universalis {
	namespace stdlib {}
	namespace os { namespace loggers {} }
}

namespace psycle {
	namespace helpers {
		namespace math {}
	}

	namespace host {
		namespace loggers = universalis::os::loggers;
		using namespace helpers;
		using namespace helpers::math;
		using namespace universalis::stdlib;
	}
}
