// ChunkFile.h: interface for the ChunkFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHUNKFILE_H__D9BFF743_6923_43DF_9793_75C3D49B0C2A__INCLUDED_)
#define AFX_CHUNKFILE_H__D9BFF743_6923_43DF_9793_75C3D49B0C2A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef enum
{
	fsClosed = 0,
	fsOpen,
	fsAlreadyOpen,
	fsInvalidFile,
	fsInvalidChunk,
	fsChunkExists,
	fsEof,

} FileStatus;


class ChunkFile  
{
public:
	ChunkFile();
	virtual ~ChunkFile();

	virtual bool OpenForRead(const char* psFilename);
	// Opens the file psFilename for reading. Reads the file header, positionates to the first
	// chunk and returns true. If the file can't be opened, or this function is called without
	// closing a previous file, it returns false
	// Use the function "ReadNextChunk()" to get the first chunk.
	virtual bool OpenForWrite(const char* psFilename, bool bOverwrite=true);
	// Opens the file psFilename for writing.
	// If bOverwrite is true and the file exists, the file is recreated, opened, positionated
	// on the first chunk and returns true.
	// If it is false and it exists, it reads the file header, positionates to the first chunk
	// and returns true.
	// If the file can't be opened, or this function is called without closing a previous file,
	// it returns false.
	virtual void Close(void);
	// Closes the file.

	virtual bool Read(void* pData, unsigned long ulNumBytes);
	// Raw Read function. pData points to the buffer to be read from file and ulNumBytes
	// indicates the number of bytes to read.
	virtual bool Write(void *pData, unsigned long ulNumBytes);
	// Raw Write function. pData points to the buffer to write to the file and ulNumBytes
	// indicates the number of bytes to write.
	virtual bool Expect(void *pData, unsigned long ulNumBytes);
	// Raw conditioncheck. pData points to a string which contains the element to check, and
	// ulNumBytes contains the bytes to check. The data is read from the file and compared byte
	// by byte with pData, returning true only when all bytes are the same. Else, false.

	virtual bool ReadNextChunk(ChunkHeader &chHeader,void *pData,bool bHeaderOnly=false);
	// Finds the next chunk in the file (i.e. skips all data until the next chunk) reads its
	// header and fills chHeader with it. If bHeaderOnly is false, then it reads the content
	// of the Chunk and fills the buffer pointed by pData. If everything is ok, returns true.
	// Else, returns false.
	virtual bool ReadChunk(ChunkHeader &chHeader,void *pData,bool bHeaderOnly=false);
	// Finds the chunk indicated in chHeader.chID. If it finds it, reads the complete header
	// and fills chHeader with it. If bHeaderOnly is false, then it reads the content
	// of the Chunk and fills the buffer pointed by pData. If everything is ok, returns true.
	// Else, returns false.
	virtual bool WriteChunk(ChunkHeader &chHeader, void *pData,bool bOverwrite=true);
	// Writes the chunkHeader specified in chHeader to the file, and the following pData.
	// If overwrite is true and the chunk exists, it is overwritten (moving it to another place
	// if size has changed), and returns true. If it is false and exists, it returns false.

};

#endif // !defined(AFX_CHUNKFILE_H__D9BFF743_6923_43DF_9793_75C3D49B0C2A__INCLUDED_)
