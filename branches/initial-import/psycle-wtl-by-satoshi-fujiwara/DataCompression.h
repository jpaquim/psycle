#ifndef _DATA_COMPRESSION_H
#define _DATA_COMPRESSION_H

/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.3 $
 */
int BEERZ77Comp2(byte * pSource, byte ** pDestination, int size);
bool BEERZ77Decomp2(byte * pSourcePos, byte ** pDestination);

int SoundSquash(signed short * pSource, byte ** pDestination, int size);
bool SoundDesquash(byte * pSourcePos, signed short ** pDestination);

#endif