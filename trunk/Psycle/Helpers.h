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


#endif
