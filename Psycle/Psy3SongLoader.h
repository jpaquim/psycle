#pragma once
#include "ipsysongloader.h"
namespace SF {
	class Psy3SongLoader :
		public IPsySongLoader
	{
	public:
		Psy3SongLoader(void);
		virtual ~Psy3SongLoader(void);
		/// RIFF�t�@�C������Song�f�[�^�����[�h����
		void Load(RiffFile& riffFile,Song& song,const bool fullopen = true);
	private:
		/// �t�@�C�����
		void LoadFileInfo(RiffFile& riffFile,Song& song,const bool fullopen = true);
		/// �\���O���
		void LoadSongInfo(RiffFile& riffFile,Song& song,const bool fullopen = true);
		/// �V�[�P���X�f�[�^
		void LoadSequenceData(RiffFile& riffFile,Song& song,const bool fullopen = true);
		/// �p�^�[���f�[�^
		void LoadPatternData(RiffFile& riffFile,Song& song,const bool fullopen = true);
		/// �}�V���f�[�^�̓ǂݍ���
		void LoadMachineData(RiffFile& riffFile,Song& song,const bool fullopen = true);
		/// �C���X�g�������g�f�[�^�̓ǂݍ���
		void LoadInstrumentData(RiffFile& riffFile,Song& song,const bool fullopen = true);

		/// �\���O���[�h���\�b�h�ւ̃|�C���^
		typedef void (Psy3SongLoader::*pLoaderFunc)(RiffFile&,Song&,const bool);
		/// ���[���ێ��}�b�v
		typedef std::map<std::string,pLoaderFunc> LoadFuncMap;
		LoadFuncMap m_LoadFuncMap;

//		void Load(RiffFile& riffFile,Machine& machine,const bool fullopen = true);
	};
}
