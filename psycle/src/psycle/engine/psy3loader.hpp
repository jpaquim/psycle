#pragma once
#include <cstdlib>
#include <sigslot/sigslot.h>

namespace psycle {
	namespace host {

		class Song;
		class RiffFile;

		class Psy3Loader
		{
		public:
			Psy3Loader();
			virtual ~Psy3Loader();

			bool Test(std::string fourcc);
			Song* Load(RiffFile* file);
			bool Save(RiffFile* file,const Song& song);

			sigslot::signal2<const std::string &, const std::string &> report;
			sigslot::signal3<const std::uint32_t& , const std::uint32_t& , const std::string& > progress;
		protected:
			bool LoadSONGv0(RiffFile* file,Song& song);
			bool LoadINFOv0(RiffFile* file,Song& song);
			bool LoadSNGIv0(RiffFile* file,Song& song);
			bool LoadSEQDv0(RiffFile* file,Song& song);
			bool LoadPATDv0(RiffFile* file,Song& song);
			bool LoadMACDv0(RiffFile* file,Song& song);
			bool LoadINSDv0(RiffFile* file,Song& song);
			bool LoadWAVEv0(RiffFile* file,Song& song);

			bool SaveSONGv0(RiffFile* file,const Song& song);
			bool SaveINFOv0(RiffFile* file,const Song& song);
			bool SaveSNGIv0(RiffFile* file,const Song& song);
			bool SaveSEQDv0(RiffFile* file,const Song& song);
			bool SavePATDv0(RiffFile* file,const Song& song);
			bool SaveMACDv0(RiffFile* file,const Song& song);
			bool SaveINSDv0(RiffFile* file,const Song& song);
			bool SaveWAVEv0(RiffFile* file,const Song& song);

		protected:
			static std::string const FILE_FOURCC;
			static std::uint32_t const VERSION_INFO;
			static std::uint32_t const VERSION_SNGI;
			static std::uint32_t const VERSION_SEQD;
			static std::uint32_t const VERSION_PATD;
			static std::uint32_t const VERSION_MACD;
			static std::uint32_t const VERSION_INSD;
			static std::uint32_t const VERSION_WAVE;

			static std::uint32_t const FILE_VERSION;
		};
	}
}