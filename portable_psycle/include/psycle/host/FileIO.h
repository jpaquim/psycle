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
		/// big-endian 32-bit unsigned integer.
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
		/// RIFF has a counterpart, RIFX, that is used to define RIFF file formats
		/// that use the Motorola integer byte-ordering format rather than the Intel format.
		/// A RIFX file is the same as a RIFF file, except that the first four bytes are "RIFX" instead of "RIFF",
		/// and integer byte ordering is represented in Motorola format.
		/// <bohan> haha... now the problem is...
		/// <bohan> " the first four bytes are "RIFX" instead of "RIFF" ",
		/// <bohan> so, on x86 cpus, should we read "XFIR"?
		class RiffFile
		{
		public:
			RiffChunkHeader _header;
			virtual bool Open(char const* psFileName);
			virtual bool Create(char* psFileName, bool overwrite);
			virtual BOOL Close(void);
			virtual bool Read(void* pData, ULONG numBytes);
			// <bohan> added const, damnit!!!
			virtual bool Write(const void * pData, ULONG numBytes);
			virtual bool Expect(void* pData, ULONG numBytes);
			virtual long Seek(long offset);
			virtual long Skip(long numBytes);
			virtual bool Eof(void);
			virtual long FileSize(void);
			virtual bool ReadString(std::string &string);
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
