#pragma once
#include "PatHelperCommand.hpp"
#include "configuration_options.hpp"


namespace psycle {
	namespace host {

		class PatPasteCommand : public PatHelperCommand {
		public:
			PatPasteCommand(class PatternView* pat_view, bool mix = false);
			~PatPasteCommand() {}

			virtual void Execute();

		private:
			bool mix_;
		};

	}	// namespace host
}	// namespace psycle
