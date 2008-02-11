/* -*- mode:c++, indent-tabs-mode:t -*- */
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.sourceforge.net
// copyright 2004-2007 Boaz Harrosh: original idea for property emulation submited to wine: http://www.winehq.org/hypermail/wine-devel/2004/03/0180.html.

///\file
#pragma once
#include "../pragmatic/attribute.hpp"
#if defined DIVERSALIS__COMPILER__MICROSOFT
	//#region UNIVERSALIS
		//#region COMPILER
			//#region PROPERTY
				#define UNIVERSALIS__COMPILER__PROPERTY__READER_AND_WRITER(type, name, reader, writer) UNIVERSALIS__COMPILER__ATTRIBUTE(property(get = reader, put = writer)) type name
				#define UNIVERSALIS__COMPILER__PROPERTY__READER(           type, name, reader        ) UNIVERSALIS__COMPILER__ATTRIBUTE(property(get = reader,             )) type name
				#define UNIVERSALIS__COMPILER__PROPERTY__WRITER(           type, name,         writer) UNIVERSALIS__COMPILER__ATTRIBUTE(property(              put = writer)) type name
			//#endregion
		//#endregion
	//#endregion
#else
	#include <cstddef> // offsetof
	//#region UNIVERSALIS
		//#region COMPILER
			//#region PROPERTY
				#if defined DIVERSALIS__COMPILER__GNU
					// Original offsetof will warn on GCC so below will turn this warning off.
					// 010<<4 (128) is to avoid alignment fixup.
					#undef  offsetof
					#define offsetof(       class, member             ) offsetof__align(class, member, 010<<4)
					#define offsetof__align(class, member, align_bytes) (reinterpret_cast<std::size_t>(reinterpret_cast<std::ptrdiff_t>(&static_cast<class*>(align_bytes)->member)) - align_bytes)
				#endif

				#define UNIVERSALIS__COMPILER__PROPERTY__WRITER(           type, name,         writer) \
					UNIVERSALIS__COMPILER__PROPERTY__DETAIL \
					( \
						type, \
						name, \
						writer, \
						UNIVERSALIS__COMPILER__PROPERTY__DETAIL__OPERATOR__WRITER(writer) \
					)

				#define UNIVERSALIS__COMPILER__PROPERTY__READER(           type, name, reader        ) \
					UNIVERSALIS__COMPILER__PROPERTY__DETAIL \
					( \
						type, \
						name, \
						reader, \
						UNIVERSALIS__COMPILER__PROPERTY__DETAIL__OPERATOR__READER(reader) \
					)

				#define UNIVERSALIS__COMPILER__PROPERTY__READER_AND_WRITER(type, name, reader, writer) \
					UNIVERSALIS__COMPILER__PROPERTY__DETAIL \
					( \
						type, \
						name, \
						reader, \
						UNIVERSALIS__COMPILER__PROPERTY__DETAIL__OPERATOR__READER(reader) \
						UNIVERSALIS__COMPILER__PROPERTY__DETAIL__OPERATOR__WRITER(writer) \
						/* Operators that aren't supported by the given type simply won't be instanciated thanks to template SFINAE. */ \
						UNIVERSALIS__COMPILER__PROPERTY__DETAIL__OPERATOR__READER_AND_WRITER__BINARY(!) \
						UNIVERSALIS__COMPILER__PROPERTY__DETAIL__OPERATOR__READER_AND_WRITER__BINARY(~) \
						UNIVERSALIS__COMPILER__PROPERTY__DETAIL__OPERATOR__READER_AND_WRITER__BINARY(&) \
						UNIVERSALIS__COMPILER__PROPERTY__DETAIL__OPERATOR__READER_AND_WRITER__BINARY(|) \
						UNIVERSALIS__COMPILER__PROPERTY__DETAIL__OPERATOR__READER_AND_WRITER__BINARY(^) \
						UNIVERSALIS__COMPILER__PROPERTY__DETAIL__OPERATOR__READER_AND_WRITER__BINARY(*) \
						UNIVERSALIS__COMPILER__PROPERTY__DETAIL__OPERATOR__READER_AND_WRITER__BINARY(/) \
						UNIVERSALIS__COMPILER__PROPERTY__DETAIL__OPERATOR__READER_AND_WRITER__BINARY(+) \
						UNIVERSALIS__COMPILER__PROPERTY__DETAIL__OPERATOR__READER_AND_WRITER__BINARY(-) \
						UNIVERSALIS__COMPILER__PROPERTY__DETAIL__OPERATOR__READER_AND_WRITER__UNARY(++) \
						UNIVERSALIS__COMPILER__PROPERTY__DETAIL__OPERATOR__READER_AND_WRITER__UNARY(--) \
						UNIVERSALIS__COMPILER__PROPERTY__DETAIL__OPERATOR__WRITER_AND_READER__UNARY(++) \
						UNIVERSALIS__COMPILER__PROPERTY__DETAIL__OPERATOR__WRITER_AND_READER__UNARY(--) \
					)

				///\internal
				//#region DETAIL
					///\internal
					#define UNIVERSALIS__COMPILER__PROPERTY__DETAIL(type_, name, reader_or_writer, accessors) \
						/** \internal */ \
						/** We need a template so that only the supported operators are instanciated thanks to SFINAE .*/
						template<typename Type> \
						class universalis__compiler__property__class__##name \
						{ \
							private: \
								UNIVERSALIS__COMPILER__ASSERT__STATIC((!sizeof *this)); \
								/** \internal */ \
								/** This function returns the instance of the class which has the accessor member function(s). */ \
								/** The accessor member function pointer passed as argument is not used but let us determine the type of its class. */ \
								/** We use the address in memory of this property class instance */ \
								/** to get the address in memory of the class instance which has the accessor member function(s). */ \
								/** Note that offsetof will probably always return zero anyway, so both addresses will be the same. */ \
								template<typename Member_Function_Holder_Class, typename Result, typename Argument> \
								Member_Function_Holder_Class inline & instance(Result (Member_Function_Holder_Class::*reader_or_writer)(Argument)) const throw() \
								{ \
									void * const pointer(reinterpret_cast<char*>(this) - offsetof(Member_Function_Holder_Class, name)); \
									return *reinterpret_cast<Member_Function_Holder_Class*>(pointer); \
								}
								typedef Type type; \
							public: \
								accessors \
						}; universalis__compiler__property__class__##name<type_> name
					///\internal
					#define UNIVERSALIS__COMPILER__PROPERTY__DETAIL__OPERATOR__READER(reader) \
								/** \internal */ \
								type inline operator()() const { return instance(reader).reader(); } \
								/** \internal */ \
								inline operator type  () const { return (*this)(); }
					///\internal
					#define UNIVERSALIS__COMPILER__PROPERTY__DETAIL__OPERATOR__WRITER(writer) \
								/** \internal */ \
								void inline operator()(type value) { instance(writer).writer(value); } \
								/** \internal */ \
								type inline operator= (type value) { (*this)(value); return value; }
					///\internal
					#define UNIVERSALIS__COMPILER__PROPERTY__DETAIL__OPERATOR__READER_AND_WRITER__BINARY($) \
								/** \internal */ \
								type inline operator $= (type value) {      value = *this              $ value  ; *this =     value ; return value; }
					///\internal
					#define UNIVERSALIS__COMPILER__PROPERTY__DETAIL__OPERATOR__READER_AND_WRITER__UNARY( $) \
								/** \internal */ \
								type inline operator $  (          ) { type value  (*this), new_value (value $) ; *this = new_value ; return value; }
					///\internal
					#define UNIVERSALIS__COMPILER__PROPERTY__DETAIL__OPERATOR__WRITER_AND_READER__UNARY( $) \
								/** \internal */ \
								type inline operator $  (int       ) { type value  (*this);            $ value  ; *this =     value ; return value; }
				//#endregion
			//#endregion
		//#endregion
	//#endregion
#endif
