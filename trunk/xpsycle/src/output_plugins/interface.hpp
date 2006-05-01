
// copyright 2006 johan boule

/***************************************************************************
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

#ifndef PSYCLE__OUTPUT_PLUGINS__INTERFACE
#define PSYCLE__OUTPUT_PLUGINS__INTERFACE

namespace psycle
{
	namespace output_plugins
	{
		class interface
		{
			public:
				/// the type of callback used by the output plugins to request audio data from psycle
				class callback_type
				{
					public:
						float        virtual   samples_per_second() = 0;
						unsigned int virtual   channels() = 0;
						float const  virtual * process(unsigned int samples) = 0;
				};

			public:
				typedef interface & (*new_function_type)(callback_type);
				typedef void (*delete_function_type)(interface &);

			protected:
				///\param callback callback used to request audio data from psycle
				interface(callback_type callback) : callback_(callback) {}

			protected:
				/// callback used to  request audio data from psycle
				callback_type & callback() { return callback_; }
			private:
				callback_type   callback_;
		};

		#define PSYCLE__OUTPUT_PLUGIN__INSTANCIATOR(typename) \
			extern "C" \
			{
				PSYCLE__OUTPUT_PLUGIN__DETAIL__DYNAMIC_LINK__EXPORT \
				interface & \
				PSYCLE__OUTPUT_PLUGIN__DETAIL__CALLING_CONVENTION \
				PSYCLE__OUTPUT_PLUGIN__INSTANCIATOR__SYMBOL(new)() \
				{
					return *new typename; \
				}

				PSYCLE__OUTPUT_PLUGIN__DETAIL__DYNAMIC_LINK__EXPORT \
				void \
				PSYCLE__OUTPUT_PLUGIN__DETAIL__CALLING_CONVENTION \
				PSYCLE__OUTPUT_PLUGIN__INSTANCIATOR__SYMBOL(delete)(interface & instance) \
				{ \
					delete &instance; \
				} \
			}

		#define PSYCLE__OUTPUT_PLUGIN__INSTANCIATOR__SYMBOL(symbol) psycle__output_plugin__instanciator__##symbol

		// [bohan] shit fuck still no universalis in xpsycle? #include <universalis/compiler.hpp>
		// [bohan] i'm really fed up of reinventing the wheel everytime..
		#if !defined _WIN32 && !defined _WIN64
			#define PSYCLE__OUTPUT_PLUGIN__DETAIL__DYNAMIC_LINK__EXPORT
			#define PSYCLE__OUTPUT_PLUGIN__DETAIL__CALLING_CONVENTION
		#elif defined _MSC_VER
			#define PSYCLE__OUTPUT_PLUGIN__DETAIL__DYNAMIC_LINK__EXPORT __declspec(dllexport)
			#define PSYCLE__OUTPUT_PLUGIN__DETAIL__CALLING_CONVENTION __cdecl
		#else
			#error please add definition for your compiler
		#endif
	}
}

#endif
