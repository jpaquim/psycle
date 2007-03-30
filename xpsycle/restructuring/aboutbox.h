/***************************************************************************
*   Copyright (C) 2007 by  Stefan Nattkemper  *
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
#ifndef ABOUTBOX_H
#define ABOUTBOX_H

#include <ngrs/dialog.h>
#include <ngrs/memo.h>

namespace ngrs {
	class NMemo;
}

namespace psy {
	namespace host {
		
		class AboutBox : public ngrs::Dialog {
		public:

			AboutBox();

			~AboutBox();

		private:

			ngrs::Memo* memo;

			std::string author;
			std::string bugs;
			std::string home;
			std::string building;
			std::string testing;
			std::string gpl;
			std::string maillist;

			void init();
			void initText();

			void onClsBtn( ngrs::ButtonEvent* ev );

		};

	}
}

#endif
