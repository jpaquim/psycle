#include "stdafx.h"

// beerz77-2 algorithm by jeremy evers
// -2 designates the smaller dictionary, faster compression version
// designed for decompression on gameboy advance
// due to it's efficient decompression, it is usefull for many other things... like pattern data.

#define MIN_REDUNDANT_BYTES_2 3

int BEERZ77Comp2(byte * pSource, byte ** pDestination, int size)
{
	// remember to delete your destination when done
	if (pSource)
	{
		byte * pDestPos = *pDestination = new byte[(size*9/8)+4];//worst case
		memset(pDestPos, 0, (size*9/8)+4);
		*pDestPos++ = 0x03; // file version
		*pDestPos++ = (byte)size & 0xff;
		*pDestPos++ = (byte)(size>>8) & 0xff;
		*pDestPos++ = (byte)(size>>16) & 0xff;

		// we will compress pSource into pDest
		byte * pSlidingWindow = pSource;
		byte * pCurrentPos = pSource;
		byte * pTestPos;
		byte * pUncompressedCounter = 0;

		while (pCurrentPos < pSource+size)
		{
			// update our sliding window
			pSlidingWindow = pCurrentPos - 0xff - MIN_REDUNDANT_BYTES_2; // maximum search offset
			// check for overflow!
			if (pSlidingWindow < pSource)
			{
				pSlidingWindow = pSource;
			}

			// check our current position against our sliding window
			byte * pBestMatch = pCurrentPos;
			int BestMatchLength = 0;

			// now to find the best match in our string
			for (pTestPos = pSlidingWindow; pTestPos < pCurrentPos-MIN_REDUNDANT_BYTES_2; pTestPos++)
			{
				// check for a match
				if (*pTestPos == *pCurrentPos)
				{
					// set our pointers
					byte* pMatchPosEnd = pCurrentPos;
					for (byte* pMatchPosStart = pTestPos; pMatchPosStart < pTestPos+255+MIN_REDUNDANT_BYTES_2; pMatchPosStart++)
					{
						// check for pointer overflow
						if (pMatchPosStart >= pCurrentPos)
						{
							pMatchPosStart--;
							break;
						}
						// check for match
						else if (*pMatchPosStart == *pMatchPosEnd)
						{
							pMatchPosEnd++;
							if (pMatchPosEnd >= pSource+size)
							{
								pMatchPosStart--;
								break;
							}
						}
						// check for end of match
						else
						{
							// there is no match
							break;
						}
					}
					// check for best match
					if (pMatchPosStart - pTestPos > BestMatchLength)
					{
						BestMatchLength = pMatchPosStart - pTestPos;
						pBestMatch = pMatchPosStart;
					}
				}
			}
			// now to see what we need to write -
			// either a new byte or an offset/length to a string

			if (BestMatchLength >= MIN_REDUNDANT_BYTES_2)
			{
				// we write our string offset/length
				// write our flag
				*pDestPos++ = 0;
				// now we write our compression info
				// first, our LENGTH
				*pDestPos++ = (BestMatchLength - MIN_REDUNDANT_BYTES_2);
				// second, our OFFSET
				WORD Output = (pCurrentPos - pBestMatch);
				*pDestPos++ = Output&0xff;
				// update the pointer
				pCurrentPos += BestMatchLength;
				pUncompressedCounter = 0;
			}
			else
			{
				BestMatchLength = 1;
				// we have an uncompressed byte
				// add it to our uncompressed string
				// if we have one
				if (pUncompressedCounter)
				{
					// it's cool, increment our counter
					*pUncompressedCounter += 1;
					// check for max string length
					if (*pUncompressedCounter == 255)
					{
						pUncompressedCounter = 0;
					}
				}
				else 
				{
					// we need to start a new string
					pUncompressedCounter = pDestPos;
					// we write a byte
					// write our flag
					*pDestPos++ = 1;
				}
				// now we write our byte
				// and update the pointer
				*pDestPos++ = *pCurrentPos++;
			}
			// do some interface crap if you like
		}
		return pDestPos - *pDestination;
	}
	else
	{
		// gimme some data to compress, dummy
		return -1;
	}
}

void BEERZ77Decomp2(byte * pSourcePos, byte ** pDestination)
{
	// remember to delete your destination when done
	if ((pSourcePos) && (*pSourcePos == 0x03))
	{
		// get file size
		int FileSize = *(DWORD*)pSourcePos;
		FileSize >>= 8;

		pSourcePos+=4;

		//ok, now we can start decompressing
		byte* pWindowPos;
		byte* pDestPos = *pDestination = new byte[FileSize];

		WORD Offset;
		WORD Length;

		while (FileSize > 0)
		{
			// get our flag
			if (Length = *pSourcePos++)
			{
				// we have an unique string to copy
				memcpy(pDestPos,pSourcePos,Length);

				pSourcePos += Length;
				pDestPos += Length;
				FileSize -= Length;
			}
			else
			{
				// we have a redundancy
				// load length and offset
				Length  = (*pSourcePos++) + MIN_REDUNDANT_BYTES_2;
				Offset  = (*pSourcePos++);

				pWindowPos = pDestPos - Offset - Length;
				memcpy(pDestPos,pWindowPos,Length);

				pDestPos += Length;
				FileSize -= Length;
			}
		}
	}
}