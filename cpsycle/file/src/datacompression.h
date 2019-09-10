///\file
///\interface beerz77-2 algorithm.
///
/// beerz77-2 algorithm by jeremy evers, loosely based on lz77
/// -2 designates the smaller window, faster compression version
/// designed for decompression on gameboy advance
/// due to it's efficient decompression, it is usefull for many other things... like pattern data.
///
/// SoundSquash and SoundDesquash by jeremy evers
/// designed with speed in mind
/// simple, non adaptave delta predictor, less effective with high frequency content

#ifndef DATACOMPRESSION_H
#define DATACOMPRESSION_H

/// compresses.
/// returns the destination size. remember to delete the destination when done!
unsigned int beerz77comp2(unsigned char const * source, unsigned char ** destination, unsigned int source_size);

/// decompresses.
/// \todo the destination size is NOT returned
/// remember to delete your destination when done!
int beerz77decomp2(unsigned char const * source, unsigned char ** destination);

/// squashes sound.
/// returns the destination size. remember to delete the destination when done!
unsigned int soundsquash(short const * source, unsigned char ** destination, unsigned int source_size);

/// desquashes sound.
/// \todo the destination size is NOT returned
/// remember to delete your destination when done!
int sounddesquash(unsigned char const * source, short ** destination);

#endif
