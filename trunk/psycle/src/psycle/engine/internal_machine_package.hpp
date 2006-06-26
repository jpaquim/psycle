#pragma once
#include <cstdlib>
#include <psycle/engine/machine.hpp>

namespace psycle{
	namespace host{

		class InternalMachinePackage
		{
		public:
			InternalMachinePackage();
			virtual ~InternalMachinePackage();

			const InternalMachineInfo* GetInfo(Machine::class_type subclass) const ;
			void MoveFirst();
			void MoveNext();
			const InternalMachineInfo* GetInfoAtPos();
			bool end() const ;
			std::uint32_t size() const ;

		protected:
			std::map<Machine::class_type,InternalMachineInfo> infomap;
			std::map<Machine::class_type,InternalMachineInfo>::const_iterator pos;
		};
	}
}