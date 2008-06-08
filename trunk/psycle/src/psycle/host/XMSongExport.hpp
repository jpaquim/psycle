// -*- mode:c++; indent-tabs-mode:t -*-
#pragma once

#include "SongStructs.hpp"
#include "FileIO.hpp"
#include "XMFile.hpp"
#include <cstdint>
#include <map>

namespace psycle { namespace host {
	class Song;

	class XMSongExport : public OldPsyFile
	{
	public:
		XMSongExport(void);
		virtual ~XMSongExport(void);
		/// RIFF 
		virtual void exportsong(Song& song,const bool fullopen = true);
	private:
		void writeSongHeader(Song &song);
		void SavePatterns(Song & song);
		void SaveSinglePattern(Song & song, const int patIdx);
		/*
		const bool SaveInstruments(XMSampler & sampler, LONG iInstrStart);
		const LONG SaveInstrument(XMSampler & sampler, LONG iStart, const int idx,int& curSample);
		const LONG SaveSampleHeader(XMSampler & sampler, LONG iStart, const int InstrIdx, const int SampleIdx);
		const LONG SaveSampleData(XMSampler & sampler, LONG iStart, const int InstrIdx, const int SampleIdx);
		void SetEnvelopes(XMInstrument & inst,const XMSAMPLEHEADER & sampleHeader);		
		*/
		XMFILEHEADER m_Header;
		int lastMachine;
		std::map<int,int> isSampler;
	};
}}
