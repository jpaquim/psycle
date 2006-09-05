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
#include <ngrs/ndialog.h>

class NSlider;
class NCheckBox;
class NEdit;
class NLabel;
class NButtonEvent;
namespace psycle
{
	namespace host
	{
		class WaveEdMixDialog : public NDialog
		{
		public:
			WaveEdMixDialog();
			NSlider		*m_srcVol;
			NSlider		*m_destVol;
			NCheckBox	*m_bFadeIn;
			NCheckBox	*m_bFadeOut;
			NEdit		*m_fadeInTime;
			NEdit		*m_fadeOutTime;
			NLabel		*m_destVolText;
			NLabel		*m_srcVolText;
		public:
			float srcVol;
			float destVol;
			bool bFadeIn;
			bool bFadeOut;
			float fadeInTime;
			float fadeOutTime;
		protected:
			void onOkClicked(NButtonEvent *ev);
			void onCancelClicked(NButtonEvent *ev);
		public:
			void onDestSliderMoved( NSlider* slider );
			void onSrcSliderMoved( NSlider* slider );
			void OnBnClickedFadeoutcheck(NButtonEvent *ev);
			void OnBnClickedFadeincheck(NButtonEvent *ev);
		};

}}
