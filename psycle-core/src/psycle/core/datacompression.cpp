///\file
		///\brief implementation file for beerz77-2 algorithm.
		/// beerz77-2 algorithm by jeremy evers, loosely based on lz77
		/// -2 designates the smaller window, faster compression version
		/// designed for decompression on gameboy advance
		/// due to it's efficient decompression, it is usefull for many other things... like pattern data.
		///
		/// SoundSquash and SoundDesquash by jeremy evers
		/// designed with speed in mind
		/// simple, non adaptave delta predictor, less effective with high frequency content
#include "psycleCorePch.hpp"

#include "datacompression.h"

#include <cstring>

#define MIN_REDUNDANT_BYTES_2 3

/// \todo add real detection of type size
typedef unsigned char byte;
/// \todo add real detection of type size
typedef unsigned short WORD;
/// \todo add real detection of type size
typedef unsigned long DWORD;

DataCompression::DataCompression( )
{
}

DataCompression::~DataCompression( )
{
}

int DataCompression::BEERZ77Comp2(byte * pSource, byte ** pDestination, int size)
		{
			// remember to delete your destination when done
			if (pSource)
			{
				byte * pDestPos = *pDestination = new byte[(size*9/8)+5];//worst case
				memset(pDestPos, 0, (size*9/8)+5);
				*pDestPos++ = (byte)0x04; // file version
				*pDestPos++ = (byte)(size & 0xff);
				*pDestPos++ = (byte)((size>>8) & 0xff);
				*pDestPos++ = (byte)((size>>16) & 0xff);
				*pDestPos++ = (byte)((size>>24) & 0xff);

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
							byte* pMatchPosStart;
							for (pMatchPosStart = pTestPos; pMatchPosStart < pTestPos+255+MIN_REDUNDANT_BYTES_2; pMatchPosStart++)
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
			// gimme some data to compress, dummy
			return -1;
		}

bool DataCompression::BEERZ77Decomp2(byte * pSourcePos, byte ** pDestination)
		{
			// remember to delete your destination when done
			if (pSourcePos) 
			{
				if (*pSourcePos++ == 0x04)
				{
					// get file size
					int FileSize = *(DWORD*)pSourcePos;

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
					return true;
				}
			}
			return false;
		}

		/////////////////////////////
		//
		/////////////////////////////

int DataCompression::SoundSquash(signed short * pSource, byte ** pDestination, int size)
		{
			if (pSource)
			{
				byte * pDestPos = *pDestination = new byte[(size*12/4)+5];//worst case-remember words are 2 bytes

				memset(pDestPos, 0, (size*12/4)+5);
				*pDestPos++ = 0x01; // file version
				*pDestPos++ = (byte)(size & 0xff); // size
				*pDestPos++ = (byte)((size>>8) & 0xff);
				*pDestPos++ = (byte)((size>>16) & 0xff);
				*pDestPos++ = (byte)((size>>24) & 0xff);

				// init predictor
				signed short prevprev = 0;
				signed short prev = 0;
				// init bitpos for encoder
				int bitpos = 0;

				while (size > 0)
				{
					// predict that this sample should be last sample + (last sample - previous last sample)
					// and calculate the deviation from that as our error value to store
					signed short t = *pSource++;
					signed short error = (t - (prev+(prev-prevprev))) & 0xFFFF;
					// shuffle our previous values for next value
					prevprev = prev;
					prev = t;

					// encode our error value
					// using this format, low to high

					// 4 bits: # of bits to describe value (x)
					// 1 bit : sign bit
					// x bits: value

					// since we generally have error values that can be described in 8 bits or less,
					// this generally results in 13 or less bits being used to describe each value 
					// (rather than 16). often our values are only require 8 or less bits.  that's how
					// we get lossless wave compression of over 50% in some cases.

					// other methods may be more efficient bitwise, but i doubt you will find much
					// with a better speed:savings ratio for our purposes

					int bits = 0;
					int data;

					// store info
					if (error &0x8000)
					{
						// negative number

						if (!(error &0x4000))
						{
							bits = 15+5;
							data = 0x0f | 0x10 | ((error &0x7fff)<<5);
						}
						else if (!(error &0x2000))
						{
							bits = 14+5;
							data = 0x0e | 0x10 | ((error &0x3fff)<<5);
						}
						else if (!(error &0x1000))
						{
							bits = 13+5;
							data = 0x0d | 0x10 | ((error &0x1fff)<<5);
						}
						else if (!(error &0x0800))
						{
							bits = 12+5;
							data = 0x0c | 0x10 | ((error &0x0fff)<<5);
						}
						else if (!(error &0x0400))
						{
							bits = 11+5;
							data = 0x0b | 0x10 | ((error &0x07ff)<<5);
						}
						else if (!(error &0x0200))
						{
							bits = 10+5;
							data = 0x0a | 0x10 | ((error &0x03ff)<<5);
						}
						else if (!(error &0x0100))
						{
							bits = 9+5;
							data = 0x09 | 0x10 | ((error &0x01ff)<<5);
						}
						else if (!(error &0x0080))
						{
							bits = 8+5;
							data = 0x08 | 0x10 | ((error &0x00ff)<<5);
						}
						else if (!(error &0x0040))
						{
							bits = 7+5;
							data = 0x07 | 0x10 | ((error &0x007f)<<5);
						}
						else if (!(error &0x0020))
						{
							bits = 6+5;
							data = 0x06 | 0x10 | ((error &0x003f)<<5);
						}
						else if (!(error &0x0010))
						{
							bits = 5+5;
							data = 0x05 | 0x10 | ((error &0x001f)<<5);
						}
						else if (!(error &0x0008))
						{
							bits = 4+5;
							data = 0x04 | 0x10 | ((error &0x000f)<<5);
						}
						else if (!(error &0x0004))
						{
							bits = 3+5;
							data = 0x03 | 0x10 | ((error &0x0007)<<5);
						}
						else if (!(error &0x0002))
						{
							bits = 2+5;
							data = 0x02 | 0x10 | ((error &0x0003)<<5);
						}
						else if (!(error &0x0001))
						{
							bits = 1+5;
							data = 0x01 | 0x10 | ((error &0x0001)<<5);
						}
						else 
						{
							bits = 0+5;
							data = 0x00 | 0x10;
						}
					}
					else
					{
						// positive number
						if (error &0x4000)
						{
							bits = 15+5;
							data = 0x0f | 0x00 | ((error &0x7fff)<<5);
						}
						else if (error &0x2000)
						{
							bits = 14+5;
							data = 0x0e | 0x00 | ((error &0x3fff)<<5);
						}
						else if (error &0x1000)
						{
							bits = 13+5;
							data = 0x0d | 0x00 | ((error &0x1fff)<<5);
						}
						else if (error &0x0800)
						{
							bits = 12+5;
							data = 0x0c | 0x00 | ((error &0x0fff)<<5);
						}
						else if (error &0x0400)
						{
							bits = 11+5;
							data = 0x0b | 0x00 | ((error &0x07ff)<<5);
						}
						else if (error &0x0200)
						{
							bits = 10+5;
							data = 0x0a | 0x00 | ((error &0x03ff)<<5);
						}
						else if (error &0x0100)
						{
							bits = 9+5;
							data = 0x09 | 0x00 | ((error &0x01ff)<<5);
						}
						else if (error &0x0080)
						{
							bits = 8+5;
							data = 0x08 | 0x00 | ((error &0x00ff)<<5);
						}
						else if (error &0x0040)
						{
							bits = 7+5;
							data = 0x07 | 0x00 | ((error &0x007f)<<5);
						}
						else if (error &0x0020)
						{
							bits = 6+5;
							data = 0x06 | 0x00 | ((error &0x003f)<<5);
						}
						else if (error &0x0010)
						{
							bits = 5+5;
							data = 0x05 | 0x00 | ((error &0x001f)<<5);
						}
						else if (error &0x0008)
						{
							bits = 4+5;
							data = 0x04 | 0x00 | ((error &0x000f)<<5);
						}
						else if (error &0x0004)
						{
							bits = 3+5;
							data = 0x03 | 0x00 | ((error &0x0007)<<5);
						}
						else if (error &0x0002)
						{
							bits = 2+5;
							data = 0x02 | 0x00 | ((error &0x0003)<<5);
						}
						else if (error &0x0001)
						{
							bits = 1+5;
							data = 0x01 | 0x00 | ((error &0x0001)<<5);
						}
						else
						{
							bits = 0+5;
							data = 0x00 | 0x00;
						}
					}

					// ok so we know how many bits to store, and what those bits are.  so store 'em!

					data <<= bitpos; // shift our bits for storage

					// store our lsb, merging with existing bits

					*pDestPos = *pDestPos | (data &0xff);
					pDestPos++;

					bits -= (8-bitpos);

					// loop for all remaining bits
					while (bits > 0)
					{
						data >>= 8;
						*pDestPos++ = data&0xff;
						bits-=8;
					}
					// calculate what bit to merge at next time
					bitpos = (8+bits)&0x7;
					// rewind our pointer if we ended mid-byte
					if (bitpos)
					{
						pDestPos--;
					}
					// let's do it again, it was fun!
					size--;
				}
				// remember to count that last half-written byte
				if (bitpos)
				{
					pDestPos++;
				}
				return pDestPos - *pDestination;
			}
			// gimme some data, dummy
			return -1;
		}

bool DataCompression::SoundDesquash(byte * pSourcePos, signed short ** pDestination)
		{
			// check validity of data
			if (pSourcePos) 
			{
				if (*pSourcePos++ == 0x01) // version 1 is pretty simple
				{
					const signed short mask[16] = {
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
						0x7fff};

					// get file size
					int FileSize = *(DWORD*)pSourcePos;

					pSourcePos+=4;
					//ok, now we can start decompressing
					signed short* pDestPos = *pDestination = new signed short[FileSize];

					// init our predictor
					signed short prevprev = 0;
					signed short prev = 0;
					// bit counter for decoder
					int bitpos = 0;

					while (FileSize)
					{
						// read a full DWORD because that is 32 bits.  in our worst case we will need
						// 7+5+15 bits, 27, which is easily contained in 32 bits.

						DWORD bits = *(DWORD*)pSourcePos;

						// now shift for our bit position to get the next bit we require

						bits >>= bitpos;
						// low 4 bits are number of valid bits count
						int numbits =bits & 0x0f;
						// next bit is the sign flag
						DWORD sign = bits & 0x010;
						// the remaining bits are our value
						bits>>=5;

						// mask out only the bits that are relevant
						signed short error = (signed short)(bits & mask[numbits]);

						// check for negative values
						if (sign)
						{
							// we need to convert to negative
							signed short error2 = (0xffff << numbits) & 0xffff;
							error = error | error2;
						}

						// and then apply our error value to the prediction
						// sample = last + (last - prev last)

						signed short t = (prev+(prev-prevprev))+error & 0xffff;
						// store our sample
						*pDestPos++ = t;
						// shuffle our previous values for next value
						prevprev = prev;
						prev = t;

						// and shift our read position to the next value in the stream
						bitpos +=numbits+5;
						while (bitpos >= 8)
						{
							bitpos -= 8;
							pSourcePos++;
						}

						// and let's do it again!
						FileSize--;

					}
					return true;
				}
			}
			return false;
		}
