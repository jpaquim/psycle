#pragma once
#include "Global.hpp"
#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/commands.h>
#include <psycle/core/patternevent.h>
namespace psycle {
	namespace core {
		class Song;
		class XMSampler;
		class Pattern;
	}
}
using namespace psycle::core;
#else
#include "SongStructs.hpp"
#endif
#include <psycle/core/fileio.h>
#include "XMFile.hpp"
#include <universalis/stdlib/cstdint.hpp>
#include <map>

namespace psycle { namespace host {
	#if !PSYCLE__CONFIGURATION__USE_PSYCORE
		class Song;
	#endif

	class XMSongExport : public RiffFile
	{
	public:
		XMSongExport(void);
		virtual ~XMSongExport(void);
		/// RIFF 
		virtual void exportsong(Song& song);
	private:
		void writeSongHeader(Song& song);
		void SavePatterns(Song& song);
		void SavePattern(Song& song, const int patIdx);
		void SavePattern(Song& song, psycle::core::Pattern* pattern);
		
		void SaveInstruments(Song & song);
		void SaveEmptyInstrument(std::string name);
		void SaveInstrument(Song& song, int instIdx);
		void SaveSampleHeader(Song & song, const int instrIdx);
		void SaveSampleData(Song & song, const int instrIdx);
		void SetEnvelopes(Song & song, int instIdx, XMSAMPLEHEADER & sampleHeader);

		int ComputeLinesPerBeat(Song& song);
		
		bool WriteHeader(XMFILEHEADER header);
		bool WriteHeader(XMPATTERNHEADER header);
		bool WriteHeader(XMINSTRUMENTHEADER header);
		bool WriteHeader(XMSAMPLEHEADER header);
		bool WriteHeader(XMSAMPLESTRUCT header);


		XMFILEHEADER m_Header;
		int lastMachine;
		std::map<int,int> isSampler;
		int lines_per_beat_;
	};

}}
