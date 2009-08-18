#include "Psy3Saver.hpp"
#include "FileIO.hpp"
#include "ProgressDialog.hpp"

#include <psycle/core/machine.h>
#include <psycle/core/song.h>
#include <psycle/core/xmsampler.h>
#include <psycle/core/fileio.h>


namespace psycle  {
	namespace host {

		Psy3Saver::Psy3Saver(psy::core::Song& song)
			: song_(&song)
		{
		}

		Psy3Saver::~Psy3Saver()
		{
		}


		bool Psy3Saver::Save(psy::core::RiffFile* pFile,bool autosave)
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
			for(int i = 0;i < psy::core::XMSampler::MAX_INSTRUMENT;i++){
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
			temp = song_->ticksSpeed(); // m_LinesPerBeat; --- todo 
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
			psy::core::SequenceLine* line = *song_->patternSequence().begin();
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

				psy::core::SequenceLine::iterator sit = line->begin();
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

		/*	for (int i = 0; i < MAX_PATTERNS; i++)
			{
				// check every pattern for validity
				if (IsPatternUsed(i))
				{
					// ok save it
					byte* pSource=new byte[SONGTRACKS*patternLines[i]*EVENT_SIZE];
					byte* pCopy = pSource;

					for (int y = 0; y < patternLines[i]; y++)
					{
						unsigned char* pData = ppPatternData[i]+(y*MULTIPLY);
						memcpy(pCopy,pData,EVENT_SIZE*SONGTRACKS);
						pCopy+=EVENT_SIZE*SONGTRACKS;
					}
					
					int sizez77 = BEERZ77Comp2(pSource, &pCopy, SONGTRACKS*patternLines[i]*EVENT_SIZE);
					zapArray(pSource);

					pFile->Write("PATD",4);
					version = CURRENT_FILE_VERSION_PATD;

					pFile->Write(&version,sizeof(version));
					size = sizez77+(4*sizeof(temp))+strlen(patternName[i])+1;
					pFile->Write(&size,sizeof(size));

					index = i; // index
					pFile->Write(&index,sizeof(index));
					temp = patternLines[i];
					pFile->Write(&temp,sizeof(temp));
					temp = SONGTRACKS; // eventually this may be variable per pattern
					pFile->Write(&temp,sizeof(temp));

					pFile->Write(&patternName[i],strlen(patternName[i])+1);

					pFile->Write(&sizez77,sizeof(sizez77));
					pFile->Write(pCopy,sizez77);
					zapArray(pCopy);

					if ( !autosave ) 
					{
						Progress.m_Progress.StepIt();
						::Sleep(1);
					}
				}
			}
*/
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

					index = i; // index
					pFile->Write(index);

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
				version = XMSampler::VERSION;
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


	}	// namespace host
}  // // namespace psycle