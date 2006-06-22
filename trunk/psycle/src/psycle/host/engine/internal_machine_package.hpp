#pragma once
#include <cstdlib>
#include <psycle/host/engine/machine.hpp>

namespace psycle{
	namespace host{

		class InternalMachinePackage
		{
		public:
			InternalMachinePackage();
			virtual ~InternalMachinePackage();

			const InternalMachineInfo* getInfo(Machine::type_type type) const ;
			const InternalMachineInfo* getFirst();
			const InternalMachineInfo* getNext();
			const bool end() const ;
			const std::uint32_t size() const ;

		protected:
			std::map<Machine::type_type,InternalMachineInfo> infomap;
			std::map<Machine::type_type,InternalMachineInfo>::const_iterator pos;
		};
	}
}