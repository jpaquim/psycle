#pragma once
#include <cstdlib>

namespace psycle{
	namespace host{

		class Machine;

		enum MachineType
		{
			MACH_UNDEFINED	= -1, //< :-(
			MACH_MASTER		= 0,
			MACH_SINE		= 1, //< for psycle old fileformat version 2
			MACH_DIST		= 2, //< for psycle old fileformat version 2
			MACH_SAMPLER	= 3,
			MACH_DELAY		= 4, //< for psycle old fileformat version 2
			MACH_2PFILTER	= 5, //< for psycle old fileformat version 2
			MACH_GAIN		= 6, //< for psycle old fileformat version 2
			MACH_FLANGER	= 7, //< for psycle old fileformat version 2
			MACH_PLUGIN		= 8,
			MACH_VST		= 9,
			MACH_VSTFX		= 10, //< Original host had a clear difference between VST gens and fx. Nowadays, they are almost the same.
			MACH_SCOPE		= 11, //< Deprecated machine (existed in some betas). It's a GUI element now. If encountered, load it as a Dummy.
			MACH_XMSAMPLER	= 12,
			MACH_DUPLICATOR	= 13,
			MACH_MIXER		= 14,
			MACH_LFO		= 15,
			MACH_AUTOMATOR	= 16,
			MACH_DUMMY		= 255
		};

		enum MachineMode
		{
			MACHMODE_UNDEFINED	= -1, //< :-(
			MACHMODE_GENERATOR	= 0,
			MACHMODE_FX			= 1,
			MACHMODE_MASTER		= 2,
		};

		// Helper class for Machine Creation.
		typedef Machine* (*CreatorFromType)(MachineType _id, std::string _dllname);

		class InternalMachineInfo
		{
		public:
			InternalMachineInfo(MachineType _type,MachineMode _mode,CreatorFromType _creator, bool _host,
				char const* _brandname,char const* _shortname,char const* _vendor,
				std::uint32_t _category, std::uint32_t _version, std::uint32_t _parameters)
				:type(_type),mode(_mode),CreateFromType(_creator), host(_host)
				,brandname(_brandname),shortname(_shortname),vendor(_vendor),category(_category)
				,version(_version),parameters(_parameters)
			{
			}
			bool operator<(const InternalMachineInfo & info) const { return type < info.type ; }
		public:
			///< Class of machine (master, sampler, dummy,...). See MachineType
			MachineType type;
			///< Mode of the plugin, ( generator, effect,...) See MachineMode
			MachineMode mode;
			///< Creator function. Needed for the loader.
			CreatorFromType CreateFromType;
			///< Indicates if the machine is unique or a host of machines (.dll's)
			bool host;
			///< Name of the machine
			char const *brandname;
			///< Default Display name.
			char const *shortname;
			///< Authority of the machine
			char const *vendor;
			///< Default category.
			//\todo: define categories.
			const std::uint32_t category;
			///< version numbering. Prefered form is " 1.0 -> 1000 "
			const std::uint32_t version;
			///< The Number of parameters that this machine exports.
			const std::uint32_t parameters;
			//\todo : description field?
		};


		class internal_machine_package
		{
		public:
			internal_machine_package();
			virtual ~internal_machine_package();

			InternalMachineInfo &getInfo(int type) const ;

		protected:
			std::map<int,const InternalMachineInfo*> infomap;
		};
	}
}