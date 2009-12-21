#pragma once
#include "PatHelperCommand.hpp"
#include "configuration_options.hpp"

namespace psycle {
	namespace host {

		class PasteBlockCommand : public PatHelperCommand {
		public:
			PasteBlockCommand(class PatternView* pat_view, int tx, int lx, bool mix);
			~PasteBlockCommand() {}

			virtual void Execute();

		private:
			int tx_, lx_;
			bool mix_;
		};

	}	// namespace host
}	// namespace psycle
