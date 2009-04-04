#pragma once
#include "Global.hpp"
#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/commands.h>
#include <psycle/core/patternevent.h>
namespace psy {
	namespace core {
		class Song;
		class XMSampler;
	}
}
using namespace psy::core;
#else
#include "SongStructs.hpp"
#endif
#include "FileIO.hpp"
#include "XMFile.hpp"
#include <cstdint>
#include <map>

namespace psycle { namespace host {
	#if !PSYCLE__CONFIGURATION__USE_PSYCORE
		class Song;
	#endif

	class XMSongExport : public OldPsyFile
	{
	public:
		XMSongExport(void);
		virtual ~XMSongExport(void);
		/// RIFF 
		virtual void exportsong(Song& song);
	private:
		void writeSongHeader(Song &song);
		void SavePatterns(Song & song);
		void SaveSinglePattern(Song & song, const int patIdx);
		
		void SaveInstruments(Song & song);
		void SaveEmptyInstrument(std::string name);
		void SaveInstrument(Song& song, int instIdx);
		void SaveSampleHeader(Song & song, const int instrIdx);
		void SaveSampleData(Song & song, const int instrIdx);
		void SetEnvelopes(Song & song, int instIdx, XMSAMPLEHEADER & sampleHeader);		
		
		XMFILEHEADER m_Header;
		int lastMachine;
		std::map<int,int> isSampler;
	};
}}
