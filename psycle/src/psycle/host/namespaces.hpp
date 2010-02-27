#pragma once

namespace universalis {
	namespace stdlib {}
	namespace os { namespace loggers {} }
}

namespace psycle {
	namespace core {
		namespace notetypes {}
		namespace commandtypes {}
	}

	namespace helpers {
		namespace math {}
	}

	namespace host {
		namespace notecommands = core::notetypes;
		namespace PatternCmd = core::commandtypes;
		namespace loggers = universalis::os::loggers;
		using namespace core;
		using namespace helpers;
		using namespace helpers::math;
		using namespace universalis::stdlib;
	}
}
