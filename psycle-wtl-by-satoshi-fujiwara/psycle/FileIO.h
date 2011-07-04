#ifndef _FILEIO_H
#define _FILEIO_H

/** @file
 *  @brief header file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.7 $
 */

#if defined(_WINAMP_PLUGIN_)
	#include <stdio.h>
#endif // defined(_WINAMP_PLUGIN_)

typedef struct TYPEULONGINV {
  unsigned char hihi;
  unsigned char hilo;
  unsigned char lohi;
  unsigned char lolo;
} ULONGINV;


typedef struct
{
	ULONG _id;
	ULONG _size; // This one should be ULONGINV (it is, at least, in the files I([JAZ]) have tested)
}
RiffChunkHeader;
	/// configuration Exception
	class FileIOErrorException : public std::exception
	{
	public:
		FileIOErrorException(const string& reason)
		{
			m_Reason = reason;
		};
		const TCHAR * what() {return m_Reason.c_str();};
		const string& what_str() { return m_Reason;};
	private:
		string m_Reason;
	};

class RiffFile 
{
public:
	///Exception
	class Exception : public std::exception
	{
	public:
		Exception(const string& reason)
		{
			m_Reason = reason;
		};
		const TCHAR * what() {return m_Reason.c_str();};
		const string& what_str() { return m_Reason;};
	private:
		string m_Reason;
	};

	mutable RiffChunkHeader _header;

	virtual bool Open(const TCHAR* psFileName);
	virtual bool Create(const TCHAR* psFileName, bool overwrite);
	virtual BOOL Close(void);
	virtual void * Read(void* pData, ULONG numBytes);
	virtual void Read(unsigned short &value){int _t;Read(&_t,sizeof(unsigned short));value =  _t;};
	virtual void Read(int &value){int _t;Read(&_t,sizeof(int));value =  _t;};
	virtual void Read(char &value){char _t;Read(&_t,sizeof(char));value =  _t;};
	virtual void Read(unsigned char &value){unsigned char _t;Read(&_t,sizeof(unsigned char));value =  _t;};
	virtual void Read(bool &value){bool _t;Read(&_t,sizeof(bool));value =  _t;};
	virtual void Read(UINT &value){UINT _t;Read(&_t,sizeof(UINT));value =  _t;};
	virtual void Read(float &value){float _t;Read(&_t,sizeof(float));value =  _t;};
	virtual void Read(double &value){double _t;Read(&_t,sizeof(double));value =  _t;};


	virtual const int ReadInt(){int _t;Read(&_t,sizeof(int));return _t;};
	virtual const char ReadChar(){char _t;Read(&_t,sizeof(char));return _t;};
	virtual const unsigned short ReadUShort(){unsigned short _t;Read(&_t,sizeof(unsigned short));return _t;};
	virtual const bool ReadBool(){bool _t;Read(&_t,sizeof(bool));return _t;};
	virtual const UINT ReadUINT(){UINT _t;Read(&_t,sizeof(UINT));return _t;};
	virtual const float ReadFloat(){float _t;Read(&_t,sizeof(float));return _t;};
	virtual const double ReadDouble(){double _t;Read(&_t,sizeof(double));return _t;};


	virtual void Write(void* pData, const ULONG numBytes);
	virtual void Write(const int value){Write((void *)(&value),sizeof(int));};
	virtual void Write(const char value){Write((void *)(&value),sizeof(char));};
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
	virtual const char * ReadString(char* pData, ULONG maxBytes);
	virtual const TCHAR * ReadString(TCHAR* pData, const ULONG maxLength);
	virtual const TCHAR * ReadStringA2T(TCHAR* pData, const ULONG maxLength);
	virtual long GetPos(void);
	
	virtual FILE* GetFile(void) { return NULL; };

	static ULONG FourCC(char *psName);
	TCHAR szName[MAX_PATH];

protected:
	HANDLE _handle;
	bool _modified;
};

class OldPsyFile : public RiffFile
{
public:
	virtual bool Open(const TCHAR* psFileName);
	virtual bool Create(const TCHAR* psFileName, bool overwrite);
	virtual BOOL Close(void);
	virtual BOOL Error();
	virtual void * Read(void* pData, ULONG numBytes);
	
	virtual const int ReadInt(){int _t;Read(&_t,sizeof(int));return _t;};
	virtual const char ReadChar(){char _t;Read(&_t,sizeof(char));return _t;};
	virtual const bool ReadBool(){bool _t;Read(&_t,sizeof(bool));return _t;};
	virtual const UINT ReadUINT(){UINT _t;Read(&_t,sizeof(UINT));return _t;};
	virtual const float ReadFloat(){float _t;Read(&_t,sizeof(float));return _t;};

	virtual void Write(void* pData, const ULONG numBytes);
	
	virtual bool Expect(void* pData, ULONG numBytes);
	virtual long Seek(long offset);
	virtual long Skip(long numBytes);
	virtual bool Eof(void);
	virtual long FileSize(void);
	virtual long GetPos(void);
	

	virtual FILE* GetFile(void) { return _file; };

protected:
	mutable FILE* _file;
};

#endif
