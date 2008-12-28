#pragma once

#include "canvas.hpp"

namespace psycle {
	namespace host {

		class CWireDlg;

		class WireGui : public TestCanvas::Line {
		public:
			WireGui(class MachineView* view);
			~WireGui();

			void UpdatePosition();

			void SetStart(class MachineGui* start) {
				start_ = start;
			}

			void SetGuiConnectors(MachineGui* from, MachineGui* to,
								  MachineGui* start) {
				fromGUI_ = from;
				toGUI_ = to;
				start_ = start;
			}

			void SetWirePoints(int source, int dest) {
				wire_source_point_ = source;
				wire_dest_point_ = source;
			}

			int wire_source() const { return wire_source_point_; }
			int wire_dest() const { return wire_dest_point_; }

			MachineGui* fromGUI() const { return fromGUI_; }
			MachineGui* toGUI() const { return toGUI_; }
			MachineGui* start() const { return start_; }

			void dragging_start(int pickpoint);
			void dragging(double x, double y);
			void dragging_stop();
			bool dragging() const { return dragging_; }

			virtual bool OnEvent(TestCanvas::Event* ev);

			void BeforeWireDlgDeletion();
			void RemoveWire();

			virtual void Draw(CDC* cr,
							  const CRgn& repaint_region,
							  TestCanvas::Canvas* widget);

			virtual const CRgn& region() const;

		private:

			void AmosDraw(CDC *devc, int oX, int oY,int dX, int dY);

			class MachineView* view_;
			MachineGui* fromGUI_;
			MachineGui* toGUI_;
			MachineGui* start_;
			// this could be a mapping in a future Wire class
			int wire_source_point_;
			int wire_dest_point_;

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

	}
}