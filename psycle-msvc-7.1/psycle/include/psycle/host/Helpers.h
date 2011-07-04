#ifndef HELPERS_H
#define HELPERS_H

#define F_PI 3.14159265358979323846f

int _httoi(const TCHAR *value);

class CValueMapper
{

public:
	CValueMapper();
	~CValueMapper();


	static inline float Map_255_1(int iByte)
	{	
		if(iByte>=0&&iByte<=256)
			return CValueMapper::fMap_255_1[iByte];
		else	
			return iByte * 0.00390625f;
	}

	static inline float Map_255_100(int iByte)
	{
		if(iByte>=0&&iByte<=256)
			return CValueMapper::fMap_255_100[iByte];
		else	
			return iByte*0.390625f;
	}
	static float fMap_255_1[257];
	static float fMap_255_100[257];

};


inline float fast_log2(float f) 
{ 
//  assert( f > 0. ); 
//  assert( sizeof(f) == sizeof(int) ); 
//  assert( sizeof(f) == 4 ); 
  int i = (*(int *)&f); 
  return (((i&0x7f800000)>>23)-0x7f)+(i&0x007fffff)/(float)0x800000; 
} 

inline int f2i(float flt) 
{ 
  int i; 
  static const double half = 0.5f; 
  _asm 
  { 
	 fld flt 
	 fsub half 
	 fistp i 
  } 
  return i;
}

inline int f2iclip16(float flt) 
{ 
	if (flt <-32767.0f)
	{
		return -32767;
	}
	if (flt > 32767.0f)
	{
		return 32767;
	}
	return f2i(flt);
}

inline int f2iclip18(float flt) 
{ 
	if (flt <-131071.0f)
	{
		return -131071;
	}
	if (flt > 131071.0f)
	{
		return 131071;
	}
	return f2i(flt);
}

inline int f2iclip20(float flt) 
{ 
	if (flt <-524287.0f)
	{
		return -524287;
	}
	if (flt > 524287.0f)
	{
		return 524287;
	}
	return f2i(flt);
}

inline int f2iclip24(float flt) 
{ 
	if (flt <-8388607.0f)
	{
		return -8388607;
	}
	if (flt > 8388607.0f)
	{
		return 8388607;
	}
	return f2i(flt);
}

inline int f2iclip32(float flt) 
{ 
	if (flt <-2147483647.0f)
	{
		return -2147483647;
	}
	if (flt > 2147483647.0f)
	{
		return 2147483647;
	}
	return f2i(flt);
}

#endif
