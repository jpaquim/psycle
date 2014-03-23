#pragma once
#include <psycle/host/detail/project.hpp>
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

		virtual void exportsong(const Song& song);
	private:
		void writeSongHeader(const Song &song);
		void SavePatterns(const Song & song);
		void SaveSinglePattern(const Song & song, const int patIdx);
		
		void SaveInstruments(const Song & song);
		void SaveEmptyInstrument(const std::string& name);
		void SaveInstrument(const Song& song, int instIdx);
		void SaveSampleHeader(const Song & song, const int instrIdx);
		void SaveSampleData(const Song & song, const int instrIdx);
		void SetEnvelopes(const Song & song, int instIdx, const XMSAMPLEHEADER & sampleHeader);
		
		XMFILEHEADER m_Header;
		int lastMachine;
		bool isSampler[MAX_BUSES];
		bool isSampulse[MAX_BUSES];
	};
}}
