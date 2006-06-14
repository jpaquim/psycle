#pragma once
#include <cstdlib>
#include <sigslot/sigslot.h>

namespace psycle {
	namespace host {

		class Song;
		class RiffFile;

		class Psy2Loader
		{
		public:
			Psy2Loader();
			virtual ~Psy2Loader();

			bool Test(std::string fourcc);
			Song* Load(RiffFile* file);
			bool Save(RiffFile* file,Song& song);

			sigslot::signal2<const std::string &, const std::string &> report;
			sigslot::signal3<const std::uint32_t& , const std::uint32_t& , const std::string& > progress;
		protected:
			/// PSY2-fileformat Constants
			static int const OLD_MAX_TRACKS;
			static int const OLD_MAX_WAVES;
			static int const OLD_MAX_INSTRUMENTS;
			static int const OLD_MAX_PLUGINS;
		};
	}
}