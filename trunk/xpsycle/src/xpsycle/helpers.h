/***************************************************************************
 *   Copyright (C) 2006 by Stefan   *
 *   natti@linux   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef HELPERS_H
#define HELPERS_H


		#define F_PI 3.14159265358979323846f

//		template<typename x> void hexstring_to_integer(std::string const &, x &);

		/// linearly maps a byte (0 to 255) to a float (0 to 1).
		class CValueMapper
		{
		public:
			/// contructor.
			CValueMapper();
			/// destructor.
			virtual ~CValueMapper() throw();
			/// maps a byte (0 to 255) to a float (0 to 1).
			static inline float Map_255_1(int iByte)
			{	
				if(iByte>=0&&iByte<=256)
					return CValueMapper::fMap_255_1[iByte];
				else	
					return iByte * 0.00390625f;
			}
			/// maps a byte (0 to 255) to a float (O to 100).
			static inline float Map_255_100(int iByte)
			{
				if(iByte>=0&&iByte<=256)
					return CValueMapper::fMap_255_100[iByte];
				else	
					return iByte*0.390625f;
			}
		private:
			static float fMap_255_1[257];
			static float fMap_255_100[257];
		};


		inline float fast_log2(float f) 
		{ 
			//assert( f > 0. ); 
			//assert( sizeof(f) == sizeof(int) ); 
			//assert( sizeof(f) == 4 ); 
			int i = (*(int *)&f); 
			return (((i&0x7f800000)>>23)-0x7f)+(i&0x007fffff)/(float)0x800000; 
		} 

		/// converts a floating point number to an integer.
		///\todo this is not portable
		inline int f2i(float flt) 
		{ 
			//int i; 
//			static const double half = 0.5f; 
//			asm 
//			{ 
//				fld flt 
//				fsub half 
//				fistp i 
//			} 
			return (int) flt;
		}
		
		/// clipping.
		///\todo use a single template function that accept any type.
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

		/// clipping.
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

		/// clipping.
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

		/// clipping.
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

		/// clipping.
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
