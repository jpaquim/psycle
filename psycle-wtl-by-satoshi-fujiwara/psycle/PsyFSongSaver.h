#pragma once


#include "ipsysongsaver.h"
namespace SF {
	class PsyFSongSaver :
		public IPsySongSaver
	{
	public:
		PsyFSongSaver(void);
		virtual ~PsyFSongSaver(void);
		/// RIFF�t�@�C������Song�f�[�^�����[�h����
		void Save(RiffFile& riffFile,::Song& song);
	private:
		/// �t�@�C�����
		void SaveFileInfo(RiffFile& riffFile,::Song& song);
		/// �\���O���
		void SaveSongInfo(RiffFile& riffFile,::Song& song);
		/// �V�[�P���X�f�[�^
		void SaveSequenceData(RiffFile& riffFile,::Song& song);
		/// �p�^�[���f�[�^
		void SavePatternData(RiffFile& riffFile,::Song& song);
		/// �}�V���f�[�^
		void SaveMachineData(RiffFile& riffFile,::Song& song);
		/// �C���X�g�������g�f�[�^
		void SaveInstrumentData(RiffFile& riffFile,::Song& song);

		CProgressDialog m_Progress;

	};
}