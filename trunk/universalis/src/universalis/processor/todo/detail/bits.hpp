/* -*- mode:c++, indent-tabs-mode:t -*- */
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2007 psycledelics http://psycle.pastnotecut.org : johan boule

///\file
///\brief detail.
#pragma once
#include <processor/project.hpp>
#if defined OPERATING_SYSTEM__MICROSOFT
	#include <windows.h> // for host-to-net and net-to-host functions
#else
	#include <netinet/in.h> // for host-to-net and net-to-host functions
#endif
namespace processor
{
	/// signed 80-bit ieee-754 floating point number.
	///\see serialize::float80
	typedef compiler::float80 float80;
	/// signed 64-bit ieee-754 floating point number.
	///\see serialize::float64
	typedef compiler::float64 float64;
	/// signed 32-bit ieee-754 floating point number.
	///\see serialize::float32
	typedef compiler::float32 float32;
	/// signed 64-bit integer number.
	///\see serialize::sint64
	typedef compiler::sint64 sint64;
	/// signed 64-bit integer number.
	///\see serialize::uint64
	typedef compiler::uint64 uint64;
	/// signed 32-bit integer number.
	///\see serialize::sint32
	typedef compiler::sint32 sint32;
	/// unsigned 32-bit integer number.
	///\see serialize::uint32
	typedef compiler::uint32 uint32;
	/// signed 16-bit integer number.
	///\see serialize::sint16
	typedef compiler::sint16 sint16;
	/// unsigned 16-bit integer number.
	///\see serialize::uint16
	typedef compiler::uint16 uint16;
	/// signed 8-bit integer number.
	///\see serialize::sint8
	typedef compiler::sint8 sint8;
	/// unsigned 8-bit integer number.
	///\see serialize::uint8
	typedef compiler::uint8 uint8;

	namespace serialize
	{
		/// wrapper around the internal format in which the processor stores numbers,
		/// to allow the serialization in a crossplatform (processor endianess independant) format,
		/// wether the processor uses a big or little endian ordering format internally.
		/// Note that this class itself does nothing, it just serves to distinguish types
		/// so that we can overload the operator<<() and operator>>().
		///\see operator>>()
		///\see operator<<()
		///\see float80
		///\see float64
		///\see float32
		///\see sint64
		///\see uint64
		///\see sint32
		///\see uint32
		///\see sint16
		///\see uint16
		///\see sint8
		///\see uint8
		template<typename Processor_Format> class serializable
		{
		public:
			/// the type of the underlying number.
			typedef Processor_Format processor_format;
			/// creates a new wrapped number.
			/// This is an explicit constructor,
			/// so that the type must exactly match and there is no implicit conversion messing.
			inline explicit serializable(const processor_format & number) throw() : number(number) {}
			/// convertible (implicit conversion) to the const type of the underlying number.
			inline operator const processor_format & () const throw() { return number; }
			/// convertible (implicit conversion) to the mutable type of the underlying number.
			inline operator processor_format & () throw() { return number; }
		private:
			processor_format number;
		};
		
		/// manipulations on bytes.
		namespace bytes
		{
			/// allows to read a const byte at the given offset in any type of const data.
			template<typename Data> class const_bytes
			{
				union u
				{
					Data data;
					processor::uint8 bytes[sizeof data];
				};
				const u & data;
			public:
				/// constuctor.
				inline const_bytes(const Data & data) : data(*reinterpret_cast<const u *>(&data)) {}
				/// implicitely convertible to the const data type.
				inline operator const Data & () const throw() { return data; }
				/// allows to read a const byte at the given offset in any type of const data.
				///\param offset the offset in data to get a const reference to a byte
				///\pre 0 <= offset && and offset < sizeof data
				///\returns the const byte at the given offset in this underlying const data.
				inline const processor::uint8 & operator[](const processor::uint8 & offset) const throw()
				{
					assert(0 <= offset and offset < sizeof data);
					return data.bytes[offset];
				}
			};

			/// allows to read and write a mutable byte at the given offset in any type of mutable data.
			template<typename Data> class bytes
			{
				union u
				{
					Data data;
					processor::uint8 bytes[sizeof data];
				};
				u & data;
			public:
				/// constuctor.
				inline bytes(Data & data) : data(*reinterpret_cast<u *>(&data)) {}
				/// implicitely convertible to the mutable data type.
				inline operator Data & () throw() { return data; }
				/// implicitely convertible to the const data type.
				inline operator const Data & () const throw() { return data; }
				/// allows to read and write a mutable byte at the given offset in any type of mutable data.
				///\param offset the offset in data to get a mutable reference to a byte
				///\pre 0 <= offset && and offset < sizeof data
				///\returns the mutable byte at the given offset in this underlying mutable data.
				inline processor::uint8 & operator[](const processor::uint8 & offset) throw() { return data.bytes[offset]; }
				/// allows to read a const byte at the given offset in any type of const data.
				///\param offset the offset in data to get a const reference to a byte
				///\pre 0 <= offset && and offset < sizeof data
				///\returns the const byte at the given offset in this underlying const data.
				inline const processor::uint8 & operator[](const processor::uint8 & offset) const throw()
				{
					assert(0 <= offset and offset < sizeof data);
					return data.bytes[offset];
				}
			};

			/// reads 1 const byte at the given offset in any type of const data.
			///\param data any type of data
			///\param offset the offset in data to get a const reference to a byte
			///\pre 0 <= offset && and offset < sizeof data
			///\returns the const byte at the given offset in the given const data.
			template<typename Data> inline const processor::uint8 & byte(const Data & data, const processor::uint8 & offset) throw()
			{
				assert(0 <= offset and offset < sizeof data);
				const_bytes<Data> result(data);
				return result[offset];
			}

			/// reads 1 mutable byte at the given offset in any type of mutable data.
			///\param data any type of data
			///\param offset the offset in data to get a reference to a byte
			///\pre 0 <= offset && and offset < sizeof data
			///\returns the mutable byte at the given offset in the given mutable data.
			template<typename Data> inline processor::uint8 & byte(Data & data, const processor::uint8 & offset) throw()
			{
				assert(0 <= offset and offset < sizeof data);
				bytes<Data> result(data);
				return result[offset];
			}

			/// reverses the order, byte per byte, of the given const data.
			/// This does not modifiy the data in place, but rather returns a copy.
			///\returns a copy of the given const data, in byte per byte reserved order
			template<typename Data> inline Data reversed(const Data & data) throw()
			{
				#if 0
					Data result_data;
					const_bytes<Data> result(result_data);
					const const_bytes<Data> original(data);
					for(processor::uint8 offset(0) ; offset < sizeof data ; ++offset) result[offset] = original[sizeof data - 1 - offset];
					return result;
				#endif
				// another way to write the very same thing:
				#if 1
					union u
					{
						Data data;
						processor::uint8 bytes[sizeof data];
					};
					const u & original(reinterpret_cast<u>(data));
					//const u & original(*reinterpret_cast<const u *>(&data));
					u result;
					for(processor::uint8 offset(0) ; offset < sizeof data ; ++offset) result.bytes[offset] = original.bytes[sizeof data - 1 - offset];
					return result.data;
				#endif
			}

			#if 1
				/*
					ntohs Convert a 16-bit quantity from network byte order to host byte order (Big-Endian to Little-Endian).
					ntohl Convert a 32-bit quantity from network byte order to host byte order (Big-Endian to Little-Endian).
					htons Convert a 16-bit quantity from host byte order to network byte order (Little-Endian to Big-Endian).
					htonl Convert a 32-bit quantity from host byte order to network byte order (Little-Endian to Big-Endian).
				*/
		
				/// template specialization for unsigned 32-bit integer.
				inline processor::uint32 reversed(const processor::uint32 & i) throw()
				{
					#if defined PROCESSOR__ENDIAN__BIG
						return ::ntohl(i);
					#elif defined PROCESSOR__ENDIAN__LITTLE
						return ::htonl(i);
					#else
						#error processor endianess not supported
					#endif
				}
	
				/// template specialization for unsigned 16-bit integer.
				inline processor::uint16 reversed(const processor::uint16 & i) throw()
				{
					#if defined PROCESSOR__ENDIAN__BIG
						return ::ntohs(i);
					#elif defined PROCESSOR__ENDIAN__LITTLE
						return ::htons(i);
					#else
						#error processor endianess not supported
					#endif
				}
			#elif 0 && defined PROCESSOR__X86 && defined COMPILER__MICROSOFT
				// intel-syntax x86 asm :-( .. the syntax isn't smart enough to be of any use for little functions (cf gcc inline asm for a more powerful syntax)
				
				/// template specialization for signed 32-bit integer.
				inline processor::sint32 reversed(const processor::sint32 & i) throw()
				{
					processor::sint32 result;
					asm
					{
						mov eax, i
						bswap eax
						mov result, eax // you can take this out but leave it just to be sure
					}
					return result;
				}

				/// template specialization for signed 32-bit integer.
				inline void reverse(processor::sint32 & i) throw()
				{
					asm
					{
						mov eax, i
						bswap eax
						mov i, eax
					}
				}

				/// template specialization for unsigned 32-bit integer.
				inline processor::uint32 reversed(const processor::uint32 & i) throw()
				{
					processor::uint32 result;
					asm
					{
						mov eax, i
						bswap eax
						mov result, eax // you can take this out but leave it just to be sure
					}
					return result;
				}

				/// template specialization for unsigned 32-bit integer.
				inline void reverse(processor::uint32 & i) throw()
				{
					asm
					{
						mov eax, i
						bswap eax
						mov i, eax
					}
				}

				/// template specialization for signed 32-bit integer.
				inline processor::sint32 reversed2(const processor::sint32 & i) throw()
				{
					processor::sint32 result;
					asm
					{
						mov eax, i
						xchg ah, al
						ror eax, 16
						xchg ah, al
						mov result, eax
					}
					return result;
				}

				/// template specialization for unsigned 32-bit integer.
				inline processor::uint32 reversed2(const processor::uint32 & i) throw()
				{
					processor::uint32 result;
					asm
					{
						mov eax, i
						xchg ah, al
						ror eax, 16
						xchg ah, al
						mov result, eax
					}
					return result;
				}

				/// template specialization for signed 16-bit integer.
				inline void reverse(processor::sint16 & i) throw()
				{
					asm
					{
						mov ax, i
						bswap eax
						shr eax, 16
						mov i, ax
					}
				}
			#else
				/// template specialization for unsigned 32-bit integer.
				inline processor::uint32 reversed(const processor::uint32 & i) throw()
				{
					return
						((i & 0x000000ff) << 24) |
						((i & 0x0000ff00) << 8) |
						((i & 0x00ff0000) >> 8) |
						((i & 0xff000000) >> 24);
				}
	
				/// template specialization for unsigned 16-bit integer.
				inline processor::uint16 reversed(const processor::uint16 & i) throw()
				{
					return
						((i & 0x00ff) << 8) |
						((i & 0xff00) >> 8);
				}
			#endif
		}

		#if 0 // works, but not used for now
			/// tests if the processor uses big endian ordering format internally.
			///\returns true if the processor uses big endian ordering format internally
			inline const bool big_endianness() throw()
			{
				class once
				{
				public:
					static const bool big_endianness() throw()
					{
						const processor::uint16 word(1);
						const processor::uint8 & byte(reinterpret_cast<const processor::uint8 &>(word));
						return !byte;
					}
				};
				static const bool cached = once::big_endianness();
				return cached;
			}

			/// tests the processor sign format.
			inline const bool signed_with_complement_of_2() throw()
			{
				class once
				{
				public:
					static const bool signed_with_complement_of_2() throw()
					{
						//http://www.ebroadcast.com.au/lookup/encyclopedia/qu/Quadword.html
						const processor::sint8 signed_byte(-1);
						const processor::uint8 & unsigned_byte(reinterpret_cast<const processor::uint8 &>(signed_byte));
						return unsigned_byte == ~0;
					}
				};
				static const bool cached = once::signed_with_complement_of_2();
				return cached;
			}
		#endif
	}
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// self test

#if defined SELF_TEST__PROCESSOR__BITS
	#include <fstream>
	#include <string>
	namespace processor
	{
		namespace serialize
		{
			void self_test()
			{
				std::ofstream out("/tmp/out", std::ios::out | std::ios::trunc | std::ios::binary);
				
				/// std::string is already streamed in correct order
				{
					const std::string & s("unix");
					out << s << std::endl; 
				}

				/// processor-dependant order
				{
					/// test on byte order
					{
						const processor::sint8 c[]("unix");
						out << c << std::endl;
					}
					/// test on sign format
					{
						const processor::sint8 & i(-1);
						out << i << std::endl;
					}
				}

				/// processor-independant order, always big endian
				{
					/// make sure we use the serializable types and operator<< of the processor::serialize namespace.
					using namespace processor::serialize;

					/// test on byte order
					{
						const uint32 & i('u' << 24 | 'n' << 16 | 'i' << 8 | 'x');
						out << i << std::endl;
					}
					/// test on sign format
					{
						const sint8 & i(-1);
						out << i << std::endl;
					}
				}

				return 0;
			}
		}
	}
#endif

// arch-tag: 5f9a5058-23df-4b85-ad1a-bfd3d5889f20
