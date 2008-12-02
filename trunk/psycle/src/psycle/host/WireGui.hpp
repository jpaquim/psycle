#pragma once

#include "canvas.hpp"

namespace psycle {
	namespace host {

		class WireGui : public TestCanvas::Line {
		public:
			WireGui();
			~WireGui();

			void UpdatePosition();

			void setStart(class MachineGui* start);
			void setGuiConnectors(MachineGui* from, MachineGui* to,
								  MachineGui* start);

			MachineGui* fromGUI() const;
			MachineGui* toGUI() const;
			MachineGui* start() const;

		private:

			MachineGui* fromGUI_;
			MachineGui* toGUI_;
			MachineGui* start_;
		};

	}
}