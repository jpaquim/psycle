// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net
#pragma once
#include <psycle/host/detail/project.hpp>
#include "EffectGui.hpp"
#include "Psycle.hpp"

namespace psycle { namespace host {
		
class CFrameMixerMachine;

class MixerGui : public EffectGui {
	public:
		MixerGui(class MachineView* view, psycle::core::Machine* mac);
		~MixerGui();

		virtual void BeforeDeleteDlg();
		void UpdateVU(CDC* devc);

	protected:
		virtual void ShowDialog(double x, double y);

	private:
		class CFrameMixerMachine* dialog_;
};

}}
