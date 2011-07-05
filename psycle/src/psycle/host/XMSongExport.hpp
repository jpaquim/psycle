#pragma once
#include <psycle/host/detail/project.hpp>
#include "Global.hpp"
#include "XMFile.hpp"

#include <psycle/core/commands.h>
#include <psycle/core/patternevent.h>
#include <psycle/core/fileio.h>

#include <map>

namespace psycle {
	namespace core {
		class Song;
		class XMSampler;
		class Pattern;
	}
	namespace host {

	class XMSongExport : public RiffFile
	{
	public:
		/// RIFF 
		virtual void exportsong(Song& song);
	private:
		void writeSongHeader(Song& song);
		void SavePatterns(Song& song);
		void SavePattern(Song& song, const int patIdx);
		void SavePattern(Song& song, psycle::core::Pattern* pattern);
		
		void SaveInstruments(Song & song);
		void SaveEmptyInstrument(std::string const & name);
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
