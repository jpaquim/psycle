#include "PatHelperCommand.hpp"
#include "configuration_options.hpp"

namespace psycle {
	namespace host {

		class ClearCurrCommand : public PatHelperCommand {
		public:
			ClearCurrCommand(class PatternView* pat_view);
			~ClearCurrCommand() {}

			virtual void Execute();

		};

	}	// namespace host
}	// namespace psycle
