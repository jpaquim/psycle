#pragma once
#include <cstdlib>
#include <sigslot/sigslot.h>

namespace psycle {
	namespace host {

		class Song;
		class RiffFile;

		class Psy3Filter
		{
		public:
			Psy3Filter();
			virtual ~Psy3Filter();

			static bool Test(std::string fourcc);
			bool Load(RiffFile* file,Song& song);
			bool Save(RiffFile* file,const Song& song);

			sigslot::signal2<const std::string &, const std::string &> report;
			sigslot::signal3<const std::uint32_t& , const std::uint32_t& , const std::string& > progress;
		protected:
			int LoadSONGv0(RiffFile* file,Song& song);
			bool LoadINFOv0(RiffFile* file,Song& song,int minorversion);
			bool LoadSNGIv0(RiffFile* file,Song& song,int minorversion);
			bool LoadSEQDv0(RiffFile* file,Song& song,int minorversion);
			bool LoadPATDv0(RiffFile* file,Song& song,int minorversion);
			bool LoadMACDv0(RiffFile* file,Song& song,int minorversion);
			bool LoadINSDv0(RiffFile* file,Song& song,int minorversion);
			bool LoadWAVEv0(RiffFile* file,Song& song,int minorversion);

			bool SaveSONGv0(RiffFile* file,const Song& song);
			bool SaveINFOv0(RiffFile* file,const Song& song);
			bool SaveSNGIv0(RiffFile* file,const Song& song);
			bool SaveSEQDv0(RiffFile* file,const Song& song,int index);
			bool SavePATDv0(RiffFile* file,const Song& song,int index);
			bool SaveMACDv0(RiffFile* file,const Song& song,int index);
			bool SaveINSDv0(RiffFile* file,const Song& song,int index);
			bool SaveWAVEv0(RiffFile* file,const Song& song,int index);

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