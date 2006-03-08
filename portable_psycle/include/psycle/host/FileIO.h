#pragma once
#if defined _WINAMP_PLUGIN_
	#include <cstdio>
#endif
///\file
///\brief interface file for psycle::host::RiffFile.
namespace psycle
{
	namespace host
	{
		/// little endian 32-bit integer.
		class ULONGINV
		{
		public:
			unsigned char hihi;
			unsigned char hilo;
			unsigned char lohi;
			unsigned char lolo;
		};

		class RiffChunkHeader
		{
		public:
			ULONG _id;
			ULONG _size; // This one should be ULONGINV (it is, at least, in the files I([JAZ]) have tested)
		};

		/// riff file format.
		class RiffFile
		{
		public:
			RiffChunkHeader _header;
			virtual bool Open(char* psFileName);
			virtual bool Create(char* psFileName, bool overwrite);
			virtual BOOL Close(void);
			virtual bool Read(void* pData, ULONG numBytes);
			virtual bool Write(void* pData, ULONG numBytes);
			virtual bool Expect(void* pData, ULONG numBytes);
			virtual long Seek(long offset);
			virtual long Skip(long numBytes);
			virtual bool Eof(void);
			virtual long FileSize(void);
			virtual bool ReadString(char* pData, ULONG maxBytes);
			virtual long GetPos(void);
			virtual FILE* GetFile(void) { return NULL; };
			static ULONG FourCC(char *psName);
			char szName[MAX_PATH];
		protected:
			HANDLE _handle;
			bool _modified;
		};

		class OldPsyFile : public RiffFile
		{
		public:
			virtual bool Open(char* psFileName);
			virtual bool Create(char* psFileName, bool overwrite);
			virtual BOOL Close(void);
			virtual BOOL Error();
			virtual bool Read(void* pData, ULONG numBytes);
			virtual bool Write(void* pData, ULONG numBytes);
			virtual bool Expect(void* pData, ULONG numBytes);
			virtual long Seek(long offset);
			virtual long Skip(long numBytes);
			virtual bool Eof(void);
			virtual long FileSize(void);
			virtual long GetPos(void);
			virtual FILE* GetFile(void) { return _file; };
		protected:
			FILE* _file;
		};
	}
}
