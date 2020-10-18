// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_XMEXPORT_H
#define psy_audio_XMEXPORT_H

#include "xmdefs.h"
#include "constants.h"
#include "songio.h"
#include "song.h"

	// Song;
/*
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
		void GetCommand(const Song& song, int i, const PatternEntry *entry, unsigned char &vol, unsigned char &type, unsigned char &param);
		
		void SaveInstruments(const Song & song);
		void SaveEmptyInstrument(const std::string& name);
		void SaveSamplerInstrument(const Song& song, int instIdx);
		void SaveSampulseInstrument(const Song& song, int instIdx);
		void SaveSampleHeader(const Song & song, const int instrIdx);
		void SaveSampleData(const Song & song, const int instrIdx);
		void SetSampulseEnvelopes(const Song & song, int instIdx, XMSAMPLEHEADER & sampleHeader);
		void SetSamplerEnvelopes(const Song & song, int instIdx, XMSAMPLEHEADER & sampleHeader);
		
		XMFILEHEADER m_Header;
		int macInstruments;
		int xmInstruments;
		int correctionIndex;
		bool isSampler[MAX_BUSES];
		bool isSampulse[MAX_BUSES];
		bool isBlitzorVst[256];
		int lastInstr[32];
		const PatternEntry* extraEntry[32];
		int addTicks;
	};
}} */


#ifdef __cplusplus
extern "C" {
#endif

typedef struct XMSongExport {
	struct XMFILEHEADER m_Header;
	int macInstruments;
	int xmInstruments;
	int correctionIndex;
	bool isSampler[MAX_BUSES];
	bool isSampulse[MAX_BUSES];
	bool isBlitzorVst[256];
	int lastInstr[32];
	// const PatternEntry* extraEntry[32];
	int addTicks;
} XMSongExport;

void xmsongexport_init(XMSongExport*);
void xmsongexport_dispose(XMSongExport*);
void xmsongexport_exportsong(XMSongExport*, psy_audio_SongFile*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_XMEXPORT_H */
