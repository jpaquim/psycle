// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "canvas.hpp"

namespace psycle { namespace host {

class CWireDlg;
class MachineGui;

class WireGui : public canvas::Line {
	public:
		WireGui(class MachineView* view);
		~WireGui();

		void UpdatePosition();

		void SetStart(class MachineGui* start) { start_ = start; }

		void SetGuiConnectors(MachineGui* from, MachineGui* to, MachineGui* start) {
			fromGUI_ = from;
			toGUI_ = to;
			start_ = start;
		}

		void set_wires(int wiresrc, int wiredest) {
			wiresrc_ = wiresrc;
			wiredest_ = wiredest;
		}

		int wiresrc() const { return wiresrc_; }
		int wiredest() const { return wiredest_; }

		MachineGui* fromGUI() const { return fromGUI_; }
		MachineGui* toGUI() const { return toGUI_; }
		MachineGui* start() const { return start_; }

		void StartDragging(int pickpoint);
		void DoDragging(double x, double y);
		void StopDragging();			

		virtual bool OnEvent(canvas::Event* ev);
		//virtual TestCanvas::Item* intersect(double x, double y);

		void BeforeWireDlgDeletion();
		void RemoveWire();

		virtual void Draw(CDC* cr, const CRgn& repaint_region, canvas::Canvas* widget);

		virtual const CRgn& region() const;

	private:
		void AmosDraw(CDC *devc, int oX, int oY,int dX, int dY);
		void ShowDialog(double x, double y);

		class MachineView* view_;
		MachineGui* fromGUI_;
		MachineGui* toGUI_;
		MachineGui* start_;
		// this could be a mapping in a future Wire class
		int wiresrc_;
		int wiredest_;
		bool newcon_;

		int drag_picker_;
		bool dragging_;
		CWireDlg* wire_dlg_;
		int triangle_size_tall;
		int triangle_size_center;
		int triangle_size_wide;
		int triangle_size_indent;
		float deltaColR;
		float deltaColG;
		float deltaColB;
		CPen linepen1;
		CPen linepen2;
		CPen linepen3;
		CPen polyInnardsPen;
		mutable CRgn rgn_;
};

}}
