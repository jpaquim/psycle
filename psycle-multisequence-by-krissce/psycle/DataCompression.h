#ifndef _DATA_COMPRESSION_H
#define _DATA_COMPRESSION_H

int BEERZ77Comp2(byte * pSource, byte ** pDestination, int size);
bool BEERZ77Decomp2(byte * pSourcePos, byte ** pDestination);

int SoundSquash(signed short * pSource, byte ** pDestination, int size);
bool SoundDesquash(byte * pSourcePos, signed short ** pDestination);

#endif