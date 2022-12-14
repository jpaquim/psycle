/*
** This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
** You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**
** copyright 2003-2021 members of the psycle project http://psycle.sourceforge.net ; jeremy evers
**
** \brief implementation beerz77-2 algorithm.
**
** beerz77-2 algorithm by jeremy evers, loosely based on lz77
** -2 designates the smaller window, faster compression version
** designed for decompression on gameboy advance
** due to it's efficient decompression, it is usefull for many other things... like pattern data.
**
** SoundSquash and SoundDesquash by jeremy evers
** designed with speed in mind
** simple, non adaptave delta predictor, less effective with high frequency content
** simple bit encoder
*/

#include "../../detail/prefix.h"


#include "datacompression.h"
/* std */
#include <assert.h>
#include <stdlib.h>
#include <string.h> 

#define TRUE 1
#define FALSE 0

static unsigned char const MIN_REDUNDANT_BYTES_2 = 3;
static unsigned int ReadLittleEndian32(unsigned char const * ptr) { return (ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | ptr[0]; }


unsigned int beerz77comp2(unsigned char const * pSource, unsigned char ** pDestination, unsigned int size) {		
	unsigned char * pDestPos;
	assert(pSource);
	pDestPos = *pDestination = (unsigned char*) malloc(size * 9 / 8 + 5); /* worst case */
	memset(pDestPos, 0, size * 9 / 8 + 5);
	*pDestPos++ = (unsigned char)(0x04); /* file version */
	*pDestPos++ = (unsigned char)((size      ) & 0xff);
	*pDestPos++ = (unsigned char)((size >>  8) & 0xff);
	*pDestPos++ = (unsigned char)((size >> 16) & 0xff);
	*pDestPos++ = (unsigned char)((size >> 24) & 0xff);

	/* we will compress pSource into pDest */
	{
	unsigned char const * pSlidingWindow = pSource;
	unsigned char const * pCurrentPos = pSource;
	unsigned char const * pTestPos;
	unsigned char       * pUncompressedCounter = 0;

	while(pCurrentPos < pSource + size) {
		unsigned char const * pBestMatch;
		ptrdiff_t BestMatchLength;
		/* update our sliding window */
		pSlidingWindow = pCurrentPos - 0xff - MIN_REDUNDANT_BYTES_2; /* maximum search offset */
		/* check for overflow! */
		if(pSlidingWindow < pSource) pSlidingWindow = pSource;

		/* check our current position against our sliding window */
		pBestMatch = pCurrentPos;
		BestMatchLength = 0;

		/* now to find the best match in our string */
		for(pTestPos = pSlidingWindow; pTestPos < pCurrentPos - MIN_REDUNDANT_BYTES_2; ++pTestPos) {
			/* check for a match */
			if(*pTestPos == *pCurrentPos) {
				/* set our pointers */
				unsigned char const * pMatchPosEnd = pCurrentPos;
				unsigned char const * pMatchPosStart;
				for(pMatchPosStart = pTestPos; pMatchPosStart < pTestPos + 255 + MIN_REDUNDANT_BYTES_2; ++pMatchPosStart) {
					/* check for pointer overflow */
					if(pMatchPosStart >= pCurrentPos) {
						--pMatchPosStart;
						break;
					}
					/* check for match */
					else if(*pMatchPosStart == *pMatchPosEnd) {
						++pMatchPosEnd;
						if(pMatchPosEnd >= pSource + size) {
							--pMatchPosStart;
							break;
						}
					} else { /* check for end of match */
						/* there is no match */
						break;
					}
				}				
				/* check for best match */
				if(pMatchPosStart - pTestPos > BestMatchLength) {
					BestMatchLength = pMatchPosStart - pTestPos;
					pBestMatch = pMatchPosStart;
				}
			}
		}
		
		/*
		** now to see what we need to write -
		** either a new byte or an offset/length to a string
		*/

		if(BestMatchLength >= MIN_REDUNDANT_BYTES_2) {
			unsigned short Output;
			/*
			** we write our string offset/length
			** write our flag
			*/
			*pDestPos++ = 0;
			/*
			** now we write our compression info
			** first, our LENGTH
			*/
			*pDestPos++ = BestMatchLength - MIN_REDUNDANT_BYTES_2;
			/* second, our OFFSET */
			Output = pCurrentPos - pBestMatch;
			*pDestPos++ = Output & 0xff;
			/* update the pointer */
			pCurrentPos += BestMatchLength;
			pUncompressedCounter = 0;
		} else {
			BestMatchLength = 1;
			/*
			** we have an uncompressed byte
			** add it to our uncompressed string
			** if we have one
			*/
			if(pUncompressedCounter) {
				/* it's cool, increment our counter */
				*pUncompressedCounter += 1;
				/* check for max string length */
				if(*pUncompressedCounter == 255) pUncompressedCounter = 0;
			} else {
				/* we need to start a new string */
				pUncompressedCounter = pDestPos;
				/*
				** we write a byte
				** write our flag
				*/
				*pDestPos++ = 1;
			}
			/*
			** now we write our byte
			**  and update the pointer
			*/
			*pDestPos++ = *pCurrentPos++;
		}
	}
	}
	return pDestPos - *pDestination;
}

int beerz77decomp2(unsigned char const * pSourcePos, unsigned char ** pDestination, size_t* destsize) {
	assert(pSourcePos);
	*destsize = 0;
	if(*pSourcePos++ == 0x04) {
		unsigned char* pWindowPos;
		unsigned char* pDestPos;
		unsigned short Offset;
		unsigned short Length;

		/*
		** get file size
		**  This is done byte by byte to avoid endianness issues
		*/
		int FileSize = (int)(ReadLittleEndian32(pSourcePos));

		pSourcePos += 4;

		/* ok, now we can start decompressing */
		pDestPos = *pDestination = malloc(FileSize);


		while(FileSize > 0) {
			/* get our flag */
			if((Length = *pSourcePos++)) {
				/* we have an unique string to copy */
				memcpy(pDestPos, pSourcePos, Length);

				pSourcePos += Length;
				pDestPos += Length;
				FileSize -= Length;
			} else {
				/*
				** we have a redundancy
				** load length and offset
				*/
				Length  = (*pSourcePos++) + MIN_REDUNDANT_BYTES_2;
				Offset  = *pSourcePos++;

				pWindowPos = pDestPos - Offset - Length;
				memcpy(pDestPos, pWindowPos, Length);

				pDestPos += Length;
				FileSize -= Length;
			}
		}
		*destsize = pDestPos - *pDestination;
		return TRUE;
	}
	return FALSE;
}

unsigned int soundsquash(short const * pSource, unsigned char ** pDestination, unsigned int size) {		
	unsigned char * pDestPos = *pDestination = malloc(size * 12 / 4 + 5); /* worst case ; remember we use words of 2 bytes */
	/* init predictor */
	short prevprev = 0;
	short prev = 0;
	/* init bitpos for encoder */
	int bitpos = 0;	
	int bits = 0;
	int data;

	assert(pSource);
	memset(pDestPos, 0, size * 12 / 4 + 5);
	*pDestPos++ = (unsigned char)(0x01); /* file version */
	*pDestPos++ = (unsigned char)((size      ) & 0xff);
	*pDestPos++ = (unsigned char)((size >>  8) & 0xff);
	*pDestPos++ = (unsigned char)((size >> 16) & 0xff);
	*pDestPos++ = (unsigned char)((size >> 24) & 0xff);

	

	while(size) {
		/*
		** predict that this sample should be last sample + (last sample - previous last sample)
		** and calculate the deviation from that as our error value to store
		*/
		short t = *pSource++;
		short error = (t - (prev + (prev - prevprev))) & 0xffff;
		/* shuffle our previous values for next value */
		prevprev = prev;
		prev = t;

		/*
		** encode our error value
		** using this format, low to high
		**
		** 4 bits: # of bits to describe value (x)
		**  1 bit : sign bit
		**  x bits: value
		**
		** since we generally have error values that can be described in 8 bits or less,
		** this generally results in 13 or less bits being used to describe each value 
		** (rather than 16). often our values only require 8 or less bits.  that's how
		** we get lossless wave compression of over 50% in some cases.
		**
		** other methods may be more efficient bitwise, but i doubt you will find much
		** with a better speed:savings ratio for our purposes
		*/

		/* store info */
		if(error & 0x8000) {
			/* negative number */
			if(!(error & 0x4000)) {
				bits = 15 + 5;
				data = 0x0f | 0x10 | ((error & 0x7fff) << 5);
			} else if(!(error & 0x2000)) {
				bits = 14 + 5;
				data = 0x0e | 0x10 | ((error & 0x3fff) << 5);
			} else if(!(error & 0x1000)) {
				bits = 13 + 5;
				data = 0x0d | 0x10 | ((error & 0x1fff) << 5);
			} else if(!(error & 0x0800)) {
				bits = 12 + 5;
				data = 0x0c | 0x10 | ((error & 0x0fff) << 5);
			} else if(!(error & 0x0400)) {
				bits = 11 + 5;
				data = 0x0b | 0x10 | ((error & 0x07ff) << 5);
			} else if(!(error & 0x0200)) {
				bits = 10 + 5;
				data = 0x0a | 0x10 | ((error & 0x03ff) << 5);
			} else if(!(error & 0x0100)) {
				bits = 9 + 5;
				data = 0x09 | 0x10 | ((error & 0x01ff) << 5);
			} else if (!(error & 0x0080)) {
				bits = 8 + 5;
				data = 0x08 | 0x10 | ((error & 0x00ff) << 5);
			} else if(!(error & 0x0040)) {
				bits = 7 + 5;
				data = 0x07 | 0x10 | ((error & 0x007f) << 5);
			} else if(!(error & 0x0020)) {
				bits = 6 + 5;
				data = 0x06 | 0x10 | ((error & 0x003f) << 5);
			} else if(!(error & 0x0010)) {
				bits = 5 + 5;
				data = 0x05 | 0x10 | ((error & 0x001f) << 5);
			} else if(!(error & 0x0008)) {
				bits = 4 + 5;
				data = 0x04 | 0x10 | ((error & 0x000f) << 5);
			} else if(!(error & 0x0004)) {
				bits = 3 + 5;
				data = 0x03 | 0x10 | ((error & 0x0007) << 5);
			} else if(!(error & 0x0002)) {
				bits = 2 + 5;
				data = 0x02 | 0x10 | ((error & 0x0003) << 5);
			} else if(!(error & 0x0001)) {
				bits = 1 + 5;
				data = 0x01 | 0x10 | ((error & 0x0001) << 5);
			} else {
				bits = 0 + 5;
				data = 0x00 | 0x10;
			}
		} else {
			/* positive number */
			if(error & 0x4000) {
				bits = 15 + 5;
				data = 0x0f | 0x00 | ((error & 0x7fff) << 5);
			} else if(error & 0x2000) {
				bits = 14 + 5;
				data = 0x0e | 0x00 | ((error & 0x3fff) << 5);
			} else if(error & 0x1000) {
				bits = 13 + 5;
				data = 0x0d | 0x00 | ((error & 0x1fff) << 5);
			} else if(error & 0x0800) {
				bits = 12 + 5;
				data = 0x0c | 0x00 | ((error & 0x0fff) << 5);
			} else if(error & 0x0400) {
				bits = 11 + 5;
				data = 0x0b | 0x00 | ((error & 0x07ff) << 5);
			} else if(error & 0x0200) {
				bits = 10 + 5;
				data = 0x0a | 0x00 | ((error & 0x03ff) << 5);
			} else if(error & 0x0100) {
				bits = 9 + 5;
				data = 0x09 | 0x00 | ((error & 0x01ff) << 5);
			} else if(error & 0x0080) {
				bits = 8 + 5;
				data = 0x08 | 0x00 | ((error & 0x00ff) << 5);
			} else if(error & 0x0040) {
				bits = 7 + 5;
				data = 0x07 | 0x00 | ((error & 0x007f) << 5);
			} else if(error & 0x0020) {
				bits = 6 + 5;
				data = 0x06 | 0x00 | ((error & 0x003f) << 5);
			} else if(error & 0x0010) {
				bits = 5 + 5;
				data = 0x05 | 0x00 | ((error & 0x001f) << 5);
			} else if(error & 0x0008) {
				bits = 4 + 5;
				data = 0x04 | 0x00 | ((error & 0x000f) << 5);
			} else if(error & 0x0004) {
				bits = 3 + 5;
				data = 0x03 | 0x00 | ((error & 0x0007) << 5);
			} else if(error & 0x0002) {
				bits = 2 + 5;
				data = 0x02 | 0x00 | ((error & 0x0003) << 5);
			} else if(error & 0x0001) {
				bits = 1 + 5;
				data = 0x01 | 0x00 | ((error & 0x0001) << 5);
			} else {
				bits = 0 + 5;
				data = 0x00 | 0x00;
			}
		}

		/* ok so we know how many bits to store, and what those bits are.  so store 'em! */

		data <<= bitpos; /* shift our bits for storage */

		/* store our lsb, merging with existing bits */

		*pDestPos = *pDestPos | (data & 0xff);
		++pDestPos;

		bits -= 8 - bitpos;

		/* loop for all remaining bits */
		while(bits > 0) {
			data >>= 8;
			*pDestPos++ = data & 0xff;
			bits -= 8;
		}
		/* calculate what bit to merge at next time */
		bitpos = (8 + bits) & 0x7;
		/* rewind our pointer if we ended mid-byte */
		if(bitpos) --pDestPos;
		/* let's do it again, it was fun! */
		--size;
	}
	/* remember to count that last half-written byte */
	if(bitpos) ++pDestPos;
	return pDestPos - *pDestination;
}

int sounddesquash(unsigned char const * pSourcePos, short ** pDestination) {
	unsigned int FileSize;
	short * pDestPos;
	/* init our predictor */
	short prevprev = 0;
	short prev = 0;
	/* bit counter for decoder */
	int bitpos = 0;
	assert(pSourcePos);
	/* check validity of data */
	if(*pSourcePos++ == 0x01) { /* version 1 is pretty simple */
		const short mask[16] = {
			0x0000,
			0x0001,
			0x0003,
			0x0007,
			0x000f,
			0x001f,
			0x003f,
			0x007f,
			0x00ff,
			0x01ff,
			0x03ff,
			0x07ff,
			0x0fff,
			0x1fff,
			0x3fff,
			0x7fff
		};

		/*
		** get file size
		**  this is done byte-by-byte to avoid endianness issues
		*/
		FileSize = ReadLittleEndian32(pSourcePos);
		pSourcePos += 4;
		
		/* ok, now we can start decompressing */
		pDestPos = *pDestination = malloc(FileSize * sizeof(short));

		/* init our predictor */
		prevprev = 0;
		prev = 0;
		/* bit counter for decoder */
		bitpos = 0;

		while(FileSize) {
			unsigned int bits;
			int numbits;
			unsigned int sign;
			short error;
			short t;

			/* read a full uint32_t. in our worst case we will need 7 + 5 + 15 bits, 27, which is easily contained in 32 bits. */
			bits = ReadLittleEndian32(pSourcePos);
			/* note, we do not increment pSourcePos. */

			/* now shift for our bit position to get the next bit we require */

			bits >>= bitpos;
			/* low 4 bits are number of valid bits count */
			numbits = bits & 0x0f;
			/* next bit is the sign flag */
			sign = bits & 0x010;
			/* the remaining bits are our value */
			bits >>= 5;

			/* mask out only the bits that are relevant */
			error = (short)(bits & mask[numbits]);

			/* check for negative values */
			if(sign) {
				/* we need to convert to negative */
				short error2 = (0xffff << numbits) & 0xffff;
				error |= error2;
			}

			/*
			** and then apply our error value to the prediction
			** sample = last + (last - prev last)
			*/

			t = ((prev + (prev - prevprev)) + error) & 0xffff;
			/* store our sample */
			*pDestPos++ = t;
			/* shuffle our previous values for next value */
			prevprev = prev;
			prev = t;

			/* and shift our read position to the next value in the stream */
			bitpos += numbits + 5;
			while(bitpos >= 8) {
				bitpos -= 8;
				++pSourcePos;
			}

			/* and let's do it again! */
			--FileSize;
		}
		return TRUE;
	}
	return FALSE;
}


