#pragma once

#include "Global.hpp"
#include "XMFile.hpp"

#include "FileIO.hpp"
#include <map>

namespace psycle { namespace host {
	class Song;

	class XMSongExport : public OldPsyFile
	{
	public:
		XMSongExport();
		virtual ~XMSongExport();
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
