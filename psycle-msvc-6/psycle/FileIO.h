#ifndef _FILEIO_H
#define _FILEIO_H


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

class RiffFile
{
public:
	RiffChunkHeader _header;

	virtual bool Open(char* psFileName);
	virtual bool Create(char* psFileName, bool overwrite);
	virtual void Close(void);
	virtual bool Read(void* pData, ULONG numBytes);
	virtual bool Write(void* pData, ULONG numBytes);
	virtual bool Expect(void* pData, ULONG numBytes);
	virtual ULONG Seek(ULONG offset);
	virtual ULONG Skip(ULONG numBytes);

	virtual FILE* GetFile(void) { return NULL; };

	static ULONG FourCC(char *psName);

protected:
	HANDLE _handle;
	bool _modified;
};

class OldPsyFile : public RiffFile
{
public:
	virtual bool Open(char* psFileName);
	virtual bool Create(char* psFileName, bool overwrite);
	virtual void Close(void);
	virtual bool Read(void* pData, ULONG numBytes);
	virtual bool Write(void* pData, ULONG numBytes);
	virtual bool Expect(void* pData, ULONG numBytes);
	virtual ULONG Seek(ULONG offset);
	virtual ULONG Skip(ULONG numBytes);
	

	virtual FILE* GetFile(void) { return _file; };

protected:
	FILE* _file;
};

#endif
