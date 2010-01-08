#include "Psy3Saver.hpp"
#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include "FileIO.hpp"
#include "ProgressDialog.hpp"
#include "Zap.hpp"

#include <psycle/core/machine.h>
#include <psycle/core/song.h>
#include <psycle/core/xmsampler.h>
#include <psycle/core/fileio.h>
#include <psycle/core/patternevent.h>
#include <psycle/helpers/datacompression.hpp>


namespace psycle  {
	namespace host {

		Psy3Saver::Psy3Saver(psycle::core::Song& song)
			: song_(&song)
		{
			InitTranslationList();
		}

		Psy3Saver::~Psy3Saver()
		{
		}


		std::string Psy3Saver::ModifyDllNameWithIndex(const std::string& name, int index)
		{
			std::string new_name = name;
			if (index != 0)
			{
				std::ostringstream buffer;
				buffer.setf(std::ios::uppercase);
				buffer.str("");
				buffer << std::setfill('0') << std::hex << std::setw(4);
				buffer << static_cast<int>(index);
				new_name += buffer.str();
			}
			return new_name;
		}


		void Psy3Saver::InitTranslationList()
		{
			underscore_plugins_.push_back("dw-eq.dll");
			underscore_plugins_.push_back("dw-granulizer.dll");
			underscore_plugins_.push_back("dw-iopan.dll");
			underscore_plugins_.push_back("dw-tremolo.dll");
			underscore_plugins_.push_back("filter-2_poles.dll");
			underscore_plugins_.push_back("fs-sf2-player.dll");
			underscore_plugins_.push_back("legasynth-303.dll");
			underscore_plugins_.push_back("ring-modulator.dll");
		}

		// maybe we have some method like this already ?
		std::string Psy3Saver::replaceString(const std::string& text,
			const std::string& old_substr,
			const std::string& new_substr) const {
				std::string replaced = text;
				std::string::size_type search_pos = 0;
				while ( (search_pos = replaced.find(old_substr, search_pos)) != std::string::npos )
					replaced.replace(search_pos++, 1, new_substr);
				return replaced;
		}

		std::string Psy3Saver::ConvertName(const std::string& name) const
		{
			std::string new_name;
			std::vector<std::string>::const_iterator it;
			it = std::find(underscore_plugins_.begin(), underscore_plugins_.end(), name);
			if (it != underscore_plugins_.end()) {
				// translate to underscore
				new_name = replaceString(name,"-","_");
			} else {
				// translate to whitespace
				new_name = replaceString(name,"-"," ");
			}
			return new_name;
		}

		int Psy3Saver::ConvertType(const psycle::core::MachineKey& key) const
		{
			int old_type = OldMachineType::MACH_DUMMY; // default
			if (key == MachineKey::dummy()) {
				old_type = OldMachineType::MACH_DUMMY;
			} else
				if (key == MachineKey::master() ) {
					old_type = OldMachineType::MACH_MASTER;
				} else
					if (key == MachineKey::duplicator() ) {
						old_type = OldMachineType::MACH_DUPLICATOR;
					} else
						if (key == MachineKey::sampler()) {
							old_type = OldMachineType::MACH_SAMPLER;
						} else
							if (key.host() == Hosts::NATIVE) {
								old_type = OldMachineType::MACH_PLUGIN;
							}


							return old_type;
		}


		bool Psy3Saver::Save(psycle::core::RiffFile* pFile,bool autosave)
		{
			// NEW FILE FORMAT!!!
			// this is much more flexible, making maintenance a breeze compared to that old hell.
			// now you can just update one module without breaking the whole thing.

			// header, this has to be at the top of the file

			CProgressDialog Progress;
			if ( !autosave ) 
			{
				Progress.Create();
				Progress.SetWindowText("Saving...");
				Progress.ShowWindow(SW_SHOW);
			}


			int chunkcount = 3; // 3 chunks plus:	

			chunkcount += song_->patternSequence().numpatterns(); // limit to MAXPATTERNS ..

			for (int i = 0; i < MAX_MACHINES; i++)
			{
				// check every pattern for validity
				if (song_->machine(i))
				{
					chunkcount++;
				}
			}

			for (int i = 0; i < MAX_INSTRUMENTS; i++)
			{
				if (!song_->_pInstrument[i]->Empty())
				{
					chunkcount++;
				}
			}

			// Instrument Data Save
			int numInstruments = 0;	
			for(int i = 0;i < psycle::core::XMSampler::MAX_INSTRUMENT;i++){
				if(song_->rInstrument(i).IsEnabled()){
					numInstruments++;
				}
			}
			if (numInstruments >0)
			{
				chunkcount++;
			}

			if ( !autosave ) 
			{
				Progress.m_Progress.SetRange(0,chunkcount);
				Progress.m_Progress.SetStep(1);
			}

			//
			// ===================
			// FILE HEADER
			// ===================
			// id = "PSY3SONG"; // PSY2 was 1.66
			//

			pFile->WriteArray("PSY3SONG", 8);

			UINT version = CURRENT_FILE_VERSION;
			UINT size = sizeof(chunkcount);
			UINT index = 0;
			int temp;

			pFile->Write(version);
			pFile->Write(size);
			pFile->Write(chunkcount);

			if ( !autosave ) 
			{
				Progress.m_Progress.StepIt();
				::Sleep(1);
			}

			// the rest of the modules can be arranged in any order

			//
			// ===================
			// SONG INFO TEXT
			// ===================
			// id = "INFO"; 
			//

			pFile->WriteArray("INFO",4);
			version = CURRENT_FILE_VERSION_INFO;
			size = song_->name().length() + song_->author().length() + song_->comment().length() + 3; // +3 for \0

			pFile->Write(version);
			pFile->Write(size);

			pFile->WriteArray(song_->name().c_str(), song_->name().length()+1);
			pFile->WriteArray(song_->author().c_str(), song_->author().length()+1);
			pFile->WriteArray(song_->comment().c_str(), song_->comment().length()+1);

			if ( !autosave ) 
			{
				Progress.m_Progress.StepIt();
				::Sleep(1);
			}

			//
			// ===================
			// SONG INFO
			// ===================
			// id = "SNGI"; 
			//

			pFile->WriteArray("SNGI",4);
			version = CURRENT_FILE_VERSION_SNGI;
			size = (11*sizeof(temp))+(song_->tracks()*(sizeof(song_->_trackMuted[0])+sizeof(song_->_trackArmed[0])));
			pFile->Write(version);
			pFile->Write(size);

			temp = song_->tracks();
			pFile->Write(temp);
			temp = song_->bpm();
			pFile->Write(temp);
			int lines_per_beat = ComputeLinesPerBeat();
			temp = lines_per_beat;
			pFile->Write(temp);
			temp = song_->currentOctave;
			pFile->Write(temp);
			temp = song_->machineSoloed;
			pFile->Write(temp);
			temp = song_->_trackSoloed;
			pFile->Write(temp);

			temp = song_->seqBus;
			pFile->Write(temp);

			temp = song_->midiSelected;
			pFile->Write(temp);
			temp = song_->auxcolSelected;
			pFile->Write(temp);
			temp = song_->instSelected();
			pFile->Write(temp);

			temp = 1; // sequence width
			pFile->Write(temp);

			for (int i = 0; i < song_->tracks(); i++)
			{
				pFile->Write(song_->_trackMuted[i]);
				pFile->Write(song_->_trackArmed[i]); // remember to count them
			}

			if ( !autosave ) 
			{
				Progress.m_Progress.StepIt();
				::Sleep(1);
			}

			//
			// ===================
			// SEQUENCE DATA
			// ===================
			// id = "SEQD"; 
			//

			// we use only the first track in mfc
			psycle::core::SequenceLine* line = *song_->patternSequence().begin();
			int playLength = line->size();
			index = 0; // index
			for (index=0;index<MAX_SEQUENCES;index++)
			{
				char* pSequenceName = "seq0\0"; // This needs to be replaced when converting to Multisequence.

				pFile->WriteArray("SEQD",4);
				version = CURRENT_FILE_VERSION_SEQD;
				size = ((playLength+2)*sizeof(temp))+strlen(pSequenceName)+1;
				pFile->Write(version);
				pFile->Write(size);

				pFile->Write(index); // Sequence Track number
				temp = playLength;
				pFile->Write(temp); // Sequence length

				pFile->WriteArray(pSequenceName,strlen(pSequenceName)+1); // Sequence Name

				psycle::core::SequenceLine::iterator sit = line->begin();
				for ( ; sit != line->end(); ++sit) {
					temp = sit->second->pattern()->id();
					pFile->Write(temp);	// Sequence data.
				}
			}
			if ( !autosave ) 
			{
				Progress.m_Progress.StepIt();
				::Sleep(1);
			}

			//
			// ===================
			// PATTERN DATA
			// ===================
			// id = "PATD"; 
			//

			// a first test version

			for(int i(0) ; i < MAX_PATTERNS; ++i) ppPatternData[i] = NULL;
			psycle::core::Sequence::patterniterator  it = song_->patternSequence().patternbegin();
			for ( ; it != song_->patternSequence().patternend(); ++it ) {
				psycle::core::Pattern* pattern = *it;
				unsigned char* data = CreateNewPattern(pattern->id());
				psycle::core::Pattern::iterator ev_it = pattern->begin();
				int num_lines = pattern->beats() * lines_per_beat;
				for ( ; ev_it != pattern->end(); ++ev_it ) {
					psycle::core::PatternEvent& ev = ev_it->second;
					double pos = ev_it->first;
					int line = pos * lines_per_beat;
					unsigned char* data_ptr = data + line * EVENT_SIZE * song_->tracks();
					ConvertEvent(ev, data_ptr);
				}		
				// ok save it
				byte* pSource=new byte[song_->tracks()*num_lines*EVENT_SIZE];
				byte* pCopy = pSource;

				for (int y = 0; y < num_lines; y++)
				{
					unsigned char* pData = ppPatternData[pattern->id()]+(y*MULTIPLY);
					memcpy(pCopy,pData,EVENT_SIZE*song_->tracks());
					pCopy+=EVENT_SIZE*song_->tracks();
				}

				int sizez77 = psycle::helpers::DataCompression::BEERZ77Comp2(pSource, &pCopy, song_->tracks()*num_lines*EVENT_SIZE);
				zapArray(pSource);

				pFile->WriteArray("PATD",4);
				version = CURRENT_FILE_VERSION_PATD;

				pFile->Write(version);
				size = sizez77+(4*sizeof(temp))+strlen(pattern->name().c_str())+1;
				pFile->Write(size);

				index = pattern->id(); // index
				pFile->Write(index);
				temp = num_lines;
				pFile->Write(temp);
				temp = song_->tracks(); // eventually this may be variable per pattern
				pFile->Write(temp);

				pFile->WriteArray((pattern->name().c_str()),strlen(pattern->name().c_str())+1);

				pFile->Write(sizez77);
				pFile->WriteArray(pCopy,sizez77);
				zapArray(pCopy);

				if ( !autosave ) 
				{
					Progress.m_Progress.StepIt();
					::Sleep(1);
				}
			}

			//
			// ===================
			// MACHINE DATA
			// ===================
			// id = "MACD"; 
			//
			// machine and instruments handle their save and load in their respective classes

			for (int i = 0; i < MAX_MACHINES; i++)
			{
				if (song_->machine(i))
				{
					pFile->WriteArray("MACD",4);
					version = 0; // CURRENT_FILE_VERSION_MACD; Why is it 256 ??
					pFile->Write(version);
					long pos = pFile->GetPos();
					size = 0;
					pFile->Write(size);

					index = i; // index					
					// chunk data

					MachineKey key = song_->machine(i)->getMachineKey();
					pFile->Write(std::uint32_t(index));
					pFile->Write(std::uint32_t(ConvertType(key)));
					std::string dllName = ModifyDllNameWithIndex(ConvertName(key.dllName())+".dll", key.index());
					pFile->WriteArray(dllName.c_str(),dllName.length()+1);
					// pFile->Write(std::uint32_t(key.index())); // is saved in the dllName
					song_->machine(i)->SaveFileChunk(pFile);

					long pos2 = pFile->GetPos(); 
					size = pos2-pos-sizeof(size);
					pFile->Seek(pos);
					pFile->Write(size);
					pFile->Seek(pos2);

					if ( !autosave ) 
					{
						Progress.m_Progress.StepIt();
						::Sleep(1);
					}
				}
			}
			//
			// ===================
			// Instrument DATA
			// ===================
			// id = "INSD"; 
			//
			for (int i = 0; i < MAX_INSTRUMENTS; i++)
			{
				if (!song_->_pInstrument[i]->Empty())
				{
					pFile->WriteArray("INSD",4);
					version = CURRENT_FILE_VERSION_INSD;
					pFile->Write(version);
					long pos = pFile->GetPos();
					size = 0;
					pFile->Write(size);

					index = i; // index
					pFile->Write(index);

					song_->_pInstrument[i]->SaveFileChunk(pFile);

					long pos2 = pFile->GetPos(); 
					size = pos2-pos-sizeof(size);
					pFile->Seek(pos);
					pFile->Write(size);
					pFile->Seek(pos2);

					if ( !autosave ) 
					{
						Progress.m_Progress.StepIt();
						::Sleep(1);
					}
				}
			}

			//
			// ===================================
			// Extended Instrument DATA (Sampulse)
			// ===================================
			// id = "EINS"; 
			//

			// Instrument Data Save
			if (numInstruments >0)
			{
				pFile->WriteArray("EINS",4);
				version = CURRENT_FILE_VERSION_XMSAMPLER;
				pFile->Write(version);
				long pos = pFile->GetPos();
				size = 0;
				pFile->Write(size);

				pFile->Write(numInstruments);

				for(int i = 0;i < XMSampler::MAX_INSTRUMENT;i++){
					if(song_->rInstrument(i).IsEnabled()){
						pFile->Write(i);
						song_->rInstrument(i).Save(*pFile);
					}
				}

				// Sample Data Save
				int numSamples = 0;	
				for(int i = 0;i < XMSampler::MAX_INSTRUMENT;i++){
					if(song_->SampleData(i).WaveLength() != 0){
						numSamples++;
					}
				}

				pFile->Write(numSamples);

				for(int i = 0;i < XMSampler::MAX_INSTRUMENT;i++){
					if(song_->SampleData(i).WaveLength() != 0){
						pFile->Write(i);
						song_->SampleData(i).Save(*pFile);
					}
				}
				long pos2 = pFile->GetPos(); 
				size = pos2-pos-sizeof(size);
				pFile->Seek(pos);
				pFile->Write(size);
				pFile->Seek(pos2);
			}


			if ( !autosave ) 
			{
				Progress.m_Progress.SetPos(chunkcount);
				::Sleep(1);

				Progress.OnCancel();
			}

			if (!pFile->Close())
			{
				std::ostringstream s;
				s << "Error writing to file '" << pFile->file_name() << "'" << std::endl;
				MessageBox(NULL,s.str().c_str(),"File Error!!!",0);
			}

			return true;
		}

		unsigned char* Psy3Saver::CreateNewPattern(int ps)
		{			
			ppPatternData[ps] = new unsigned char[MULTIPLY2];
			PatternEvent blank;
			unsigned char * pData = ppPatternData[ps];
			for(int i = 0; i < MULTIPLY2; i+= EVENT_SIZE)
			{
				memcpy(pData,&blank,EVENT_SIZE);
				pData+= EVENT_SIZE;
			}
			return ppPatternData[ps];
		}

		void Psy3Saver::ConvertEvent(const psycle::core::PatternEvent& ev, unsigned char* data) const {
			*data = ev.note();  ++data;
			*data = ev.instrument(); ++data;
			*data = ev.machine(); ++data;
			*data = ev.command(); ++data;
			*data = ev.parameter(); 
		}

		int Psy3Saver::ComputeLinesPerBeat() {
			psycle::core::Sequence& seq = song_->patternSequence();
			psycle::core::Sequence::patterniterator it = seq.patternbegin();
			double min = 1.0;
			for ( ; it != seq.patternend(); ++it) {
				psycle::core::Pattern* pattern = *it;
				psycle::core::Pattern::iterator pat_it = pattern->begin();
				double old_pos = 0;
				for ( ; pat_it != pattern->end(); ++pat_it ) {
					double pos = pat_it->first;
					double delta = pos - old_pos;
					if ( delta != 0 && delta < min)
						min = delta;
				}
			}
			return static_cast<int>(1 / min);
		}


	}	// namespace host
}  // // namespace psycle

#endif
