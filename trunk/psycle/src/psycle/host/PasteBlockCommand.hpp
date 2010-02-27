#pragma once
#include "PatHelperCommand.hpp"

namespace psycle { namespace host {

class PasteBlockCommand : public PatHelperCommand {
	public:
		PasteBlockCommand(class PatternView* pat_view, int tx, int lx, bool mix);

		virtual void Execute();

	private:
		int tx_, lx_;
		bool mix_;
};

}}
