///\file
///\brief interface file for psycle::host::RiffFile.
#pragma once
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
			virtual bool Open(std::string psFileName);
			virtual bool Create(std::string psFileName, bool overwrite);
			virtual BOOL Close(void);

			virtual bool Read(void* pData, ULONG numBytes);
			virtual void Read(int &value){int _t;Read(&_t,sizeof(int));value =  _t;};
			virtual void Read(char &value){char _t;Read(&_t,sizeof(char));value =  _t;};
			virtual void Read(unsigned char &value){unsigned char _t;Read(&_t,sizeof(unsigned char));value =  _t;};
			virtual void Read(short &value){short _t;Read(&_t,sizeof(short));value =  _t;};
			virtual void Read(unsigned short &value){unsigned short _t;Read(&_t,sizeof(unsigned short));value =  _t;};
			virtual void Read(bool &value){bool _t;Read(&_t,sizeof(bool));value =  _t;};
			virtual void Read(UINT &value){UINT _t;Read(&_t,sizeof(UINT));value =  _t;};
			virtual void Read(float &value){float _t;Read(&_t,sizeof(float));value =  _t;};
			virtual void Read(double &value){double _t;Read(&_t,sizeof(double));value =  _t;};

			virtual const int ReadInt(int numbytes=sizeof(int)){int _t;Read(&_t,numbytes);return _t;};
			virtual const char ReadChar(){char _t;Read(&_t,sizeof(char));return _t;};
			virtual const unsigned char ReadUChar(){ unsigned char _t;Read(&_t,sizeof(unsigned char));return _t;};
			virtual const short ReadShort(){ short _t;Read(&_t,sizeof(short));return _t;};
			virtual const unsigned short ReadUShort(){unsigned short _t;Read(&_t,sizeof(unsigned short));return _t;};
			virtual const bool ReadBool(){bool _t;Read(&_t,sizeof(bool));return _t;};
			virtual const UINT ReadUINT(){UINT _t;Read(&_t,sizeof(UINT));return _t;};
			virtual const float ReadFloat(){float _t;Read(&_t,sizeof(float));return _t;};
			virtual const double ReadDouble(){double _t;Read(&_t,sizeof(double));return _t;};

			virtual bool Write(const void * pData, ULONG numBytes);
			virtual void Write(const int value){Write((void *)(&value),sizeof(int));};
			virtual void Write(const char value){Write((void *)(&value),sizeof(char));};
			virtual void Write(const unsigned char value){Write((void *)(&value),sizeof(unsigned char));};
			virtual void Write(const short value){Write((void *)(&value),sizeof(short));};
			virtual void Write(const unsigned short value){Write((void *)(&value),sizeof(unsigned short));};
			virtual void Write(const bool value){Write((void *)(&value),sizeof(bool));};
			virtual void Write(const UINT value){Write((void *)(&value),sizeof(UINT));};
			virtual void Write(const float value){Write((void *)(&value),sizeof(float));};
			virtual void Write(const double value){Write((void *)(&value),sizeof(double));};

			virtual bool Expect(void* pData, ULONG numBytes);
			virtual long Seek(long offset);
			virtual long Skip(long numBytes);
			virtual bool Eof(void);
			virtual long FileSize(void);
			virtual bool ReadString(std::string &string);
			virtual bool ReadString(char* pData, ULONG maxBytes);
			const TCHAR * RiffFile::ReadStringA2T(TCHAR* pData, const ULONG maxLength);
			virtual long GetPos(void);
			virtual FILE* GetFile(void) { return NULL; };
			static ULONG FourCC(char const *psName);
			std::string szName;
		protected:
			HANDLE _handle;
			bool _modified;
		};

		class OldPsyFile : public RiffFile
		{
		public:
			virtual bool Open(std::string psFileName);
			virtual bool Create(std::string psFileName, bool overwrite);
			virtual BOOL Close(void);
			virtual BOOL Error();
			virtual bool Read(void* pData, ULONG numBytes);

			///\todo replace std::pow with bit-shifting.
			virtual const int ReadInt(int numbytes=sizeof(int)){int _t;Read(&_t,numbytes);return _t&int(std::pow(256.,numbytes)-1);};
			virtual const char ReadChar(){char _t;Read(&_t,sizeof(char));return _t;};
			virtual const bool ReadBool(){bool _t;Read(&_t,sizeof(bool));return _t;};
			virtual const UINT ReadUINT(){UINT _t;Read(&_t,sizeof(UINT));return _t;};
			virtual const float ReadFloat(){float _t;Read(&_t,sizeof(float));return _t;};

			virtual bool Write(const void* pData, ULONG numBytes);
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
