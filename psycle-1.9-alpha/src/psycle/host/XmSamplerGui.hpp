// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "GeneratorGui.hpp"

namespace psycle { namespace host {

class XMSamplerUI;

class XmSamplerGui : public GeneratorGui {
	public:
		XmSamplerGui(MachineView* view, psycle::core::Machine* mac);
		~XmSamplerGui();

		virtual void BeforeDeleteDlg();
		virtual void UpdateVU(CDC* devc);

	protected:
		virtual void ShowDialog(double x, double y);

	private:
		class XMSamplerUI* dialog_;
};

}}
