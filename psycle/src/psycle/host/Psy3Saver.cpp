#include "Psy3Saver.hpp"
#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include "FileIO.hpp"
#include "ProgressDialog.hpp"
#include "Zap.hpp"

#include <psycle/core/fileio.h>
#include <psycle/core/machine.h>
#include <psycle/core/machinekey.hpp>
#include <psycle/core/song.h>
#include <psycle/core/patternevent.h>
#include <psycle/core/xmsampler.h>
#include <psycle/helpers/datacompression.hpp>

namespace psycle  {
	namespace host {

		Psy3Saver::Psy3Saver(psycle::core::Song& song)
			: song_(&song) {
			InitTranslationList();
		}
	
		void Psy3Saver::InitTranslationList() {
			underscore_plugins_.push_back("dw-eq.dll");
			underscore_plugins_.push_back("dw-granulizer.dll");
			underscore_plugins_.push_back("dw-iopan.dll");
			underscore_plugins_.push_back("dw-tremolo.dll");
			underscore_plugins_.push_back("filter-2_poles.dll");
			underscore_plugins_.push_back("fs-sf2-player.dll");
			underscore_plugins_.push_back("legasynth-303.dll");
			underscore_plugins_.push_back("ring-modulator.dll");
		}

		std::string Psy3Saver::ModifyDllNameWithIndex(const std::string& name, int index) {
			std::string new_name = name;
			if (index != 0) {
				std::ostringstream buffer;
				buffer.setf(std::ios::uppercase);
				buffer.str("");
				buffer << std::setfill('0') << std::hex << std::setw(4) << index;
				new_name += buffer.str();
			}
			return new_name;
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

		std::string Psy3Saver::ConvertName(const std::string& name) const {
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

		std::uint32_t Psy3Saver::ConvertType(const psycle::core::MachineKey& key,bool isGenerator) const {
			OldMachineType old_type = MACH_DUMMY; // default
			if (key.host() == Hosts::INTERNAL ) {
				if (key == MachineKey::master ) {
					old_type = MACH_MASTER;
				}
				else if (key == MachineKey::sampler) {
					old_type = MACH_SAMPLER;
				}
				else if (key == MachineKey::sampulse) {
					old_type = MACH_XMSAMPLER;
				}
				else if (key == MachineKey::duplicator ) {
					old_type = MACH_DUPLICATOR;
				}
				else if (key == MachineKey::mixer ) {
					old_type = MACH_MIXER;
				}
				else if (key == MachineKey::audioinput ) {
					old_type = MACH_RECORDER;
				}
			}
			else if (key.host() == Hosts::NATIVE ) {
				old_type = MACH_PLUGIN;
			}
			else if (key.host() == Hosts::VST ) {
				if (isGenerator)
					old_type = MACH_VST;
				else
					old_type = MACH_VSTFX;
			}
			return (std::uint32_t)old_type;
		}

		bool Psy3Saver::Save(psycle::core::RiffFile* pFile,bool autosave) {
			// NEW FILE FORMAT!!!
			// this is much more flexible, making maintenance a breeze compared to that old hell.
			// now you can just update one module without breaking the whole thing.

			// header, this has to be at the top of the file

			CProgressDialog Progress;
			if ( !autosave )
			{
				Progress.SetWindowText("Saving...");
				Progress.ShowWindow(SW_SHOW);
			}


			int chunkcount = 3; // 3 chunks plus:	

			chunkcount += song_->patternSequence().numpatterns()-1; // limit to MAXPATTERNS ..

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
			for (int i = 0; i < psycle::core::XMSampler::MAX_INSTRUMENT; i++) {
				if (song_->rInstrument(i).IsEnabled()) {
					numInstruments++;
				}
			}
			if (numInstruments >0)
			{
				chunkcount++;
			}

			if (!autosave)
			{
				Progress.SetRange(0,chunkcount);
				Progress.SetStep(1);
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

			if (!autosave)
			{
				Progress.StepIt();
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

			pFile->WriteString(song_->name());
			pFile->WriteString(song_->author());
			pFile->WriteString(song_->comment());

			if (!autosave) {
				Progress.StepIt();
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
				Progress.StepIt();
				::Sleep(1);
			}

			//
			// ===================
			// SEQUENCE DATA
			// ===================
			// id = "SEQD"; 
			//

			// we use only the first track in mfc
			psycle::core::SequenceLine* line = *(song_->patternSequence().begin()+1);
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
				Progress.StepIt();
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
				///\todo: need to do something with it?
				if (pattern == song_->patternSequence().master_pattern())
					continue;
				int num_lines = pattern->beats() * lines_per_beat;
				unsigned char* data = CreateNewPattern(pattern->id(), song_->tracks(), num_lines);
				psycle::core::Pattern::iterator ev_it = pattern->begin();
				for ( ; ev_it != pattern->end(); ++ev_it ) {
					psycle::core::PatternEvent& ev = ev_it->second;
					double pos = ev_it->first;
					int line = pos * lines_per_beat;
					unsigned char* data_ptr = data + (line * song_->tracks() + ev.track()) * EVENT_SIZE;
					ConvertEvent(ev, data_ptr);
				}		
				// ok save it
				index = pattern->id(); // index
				byte* pSource=new byte[song_->tracks()*num_lines*EVENT_SIZE];
				byte* pCopy = pSource;

				for (int y = 0; y < num_lines; y++)
				{
					unsigned char* pData = ppPatternData[index]+(y*song_->tracks()*EVENT_SIZE);
					memcpy(pCopy,pData,EVENT_SIZE*song_->tracks());
					pCopy+=EVENT_SIZE*song_->tracks();
				}

				int sizez77 = psycle::helpers::DataCompression::BEERZ77Comp2(pSource, &pCopy, song_->tracks()*num_lines*EVENT_SIZE);
				zapArray(pSource);

				pFile->WriteArray("PATD",4);
				version = CURRENT_FILE_VERSION_PATD;

				pFile->Write(version);
				size = sizez77+(4*sizeof(temp))+pattern->name().length()+1;
				pFile->Write(size);

				pFile->Write(index);
				temp = num_lines;
				pFile->Write(temp);
				temp = song_->tracks(); // eventually this may be variable per pattern
				pFile->Write(temp);

				pFile->WriteString(pattern->name());

				pFile->Write(sizez77);
				pFile->WriteArray(pCopy,sizez77);
				zapArray(pCopy);

				if ( !autosave ) 
				{
					Progress.StepIt();
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
					version = CURRENT_FILE_VERSION_MACD;
					pFile->Write(version);
					long pos = pFile->GetPos();
					size = 0;
					pFile->Write(size);
					// chunk data
					index = i; // index					
					pFile->Write(std::uint32_t(index));

					MachineKey key = song_->machine(i)->getMachineKey();
					pFile->Write(ConvertType(key,song_->machine(i)->IsGenerator()));
					#if (CURRENT_FILE_VERSION_MACD&0xFF00) == 0x0000
						if (key.host() != Hosts::INTERNAL) {
							std::string dllName = ModifyDllNameWithIndex(ConvertName(key.dllName())+".dll", key.index());
							pFile->WriteString(dllName);
						}
						else {
							pFile->WriteString("");
						}
					#elif (CURRENT_FILE_VERSION_MACD&0xFF00) == 0x0100
						pFile->WriteString(ConvertName(key.dllName())+".dll"));
						pFile->Write(std::uint32_t(key.index())); // is saved in the dllName
					#endif
					song_->machine(i)->SaveFileChunk(pFile);

					long pos2 = pFile->GetPos(); 
					size = pos2-pos-sizeof(size);
					pFile->Seek(pos);
					pFile->Write(size);
					pFile->Seek(pos2);

					if ( !autosave ) 
					{
						Progress.StepIt();
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
						Progress.StepIt();
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
				version = CURRENT_FILE_VERSION_EINS;
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
				Progress.SetPos(chunkcount);
				::Sleep(1);
			}

			if (!pFile->Close())
			{
				std::ostringstream s;
				s << "Error writing to file '" << pFile->file_name() << "'" << std::endl;
				MessageBox(NULL,s.str().c_str(),"File Error!!!",0);
			}

			return true;
		}

		unsigned char* Psy3Saver::CreateNewPattern(int ps, int tracks, int lines) {			
			ppPatternData[ps] = new unsigned char[EVENT_SIZE*tracks*lines];
			PatternEvent blank;
			unsigned char * pData = ppPatternData[ps];
			for(int i = 0; i < tracks*lines; i++)
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
}  // namespace psycle

#endif
