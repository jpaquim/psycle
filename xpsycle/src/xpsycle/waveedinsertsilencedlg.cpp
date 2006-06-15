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
#include "waveedinsertsilencedlg.h"
#include <ngrs/nbutton.h>
#include <ngrs/ncheckbox.h>
#include <ngrs/nedit.h>
#include <ngrs/nlabel.h>

namespace psycle
{
	namespace host
	{
		WaveEdInsertSilenceDialog::WaveEdInsertSilenceDialog()
		{
			setPosition(0, 0, 247, 87);
			setPositionToScreenCenter();
			setTitle("Insert Silence");
			m_atStart = new NCheckBox("At start");
			m_atEnd = new NCheckBox("At end");
			m_atCursor = new NCheckBox("At cursor");
			
			pane()->add(m_atStart);
			pane()->add(m_atEnd);
			pane()->add(m_atCursor);
			
			m_atStart->clicked.connect(this, &WaveEdInsertSilenceDialog::onInsStartClicked);
			m_atEnd->clicked.connect(this, &WaveEdInsertSilenceDialog::onInsEndClicked);
			m_atCursor->clicked.connect(this, &WaveEdInsertSilenceDialog::onInsCursorClicked);
			
			m_atStart->setPosition(88,22,80,13);
			m_atEnd->setPosition(88,36,80,13);
			m_atCursor->setPosition(88,50,90,13);

			m_atStart->setCheck(true);
			m_atEnd->setCheck(false);
			m_atCursor->setCheck(false);

			m_time = new NEdit;
			pane()->add(m_time);
			m_time->setText("0.000");
			m_time->setPosition(19,32,56,13);
			{
				NButton *btn = new NButton("Ok", false);
				btn->clicked.connect(this, &WaveEdInsertSilenceDialog::onOkClicked);
				pane()->add(btn);
				btn->setPosition(181, 7, 60, 15);
			}
			{
				NButton *btn = new NButton("Cancel", false);
				btn->clicked.connect(this, &WaveEdInsertSilenceDialog::onCancelClicked);
				pane()->add(btn);
				btn->setPosition(181, 24, 60, 15);
			}
			{
				NLabel *lbl = new NLabel("Insert (in secs):");
				pane()->add(lbl);
				lbl->setPosition(21,17,53,14);
			}
		}

		void WaveEdInsertSilenceDialog::onInsStartClicked( NButtonEvent* ev)
		{
			m_atStart->setCheck(true);
			m_atEnd->setCheck(false);
			m_atCursor->setCheck(false);
			m_atStart->repaint();
			m_atEnd->repaint();
			m_atCursor->repaint();
		}
		void WaveEdInsertSilenceDialog::onInsEndClicked( NButtonEvent* ev)
		{
			m_atStart->setCheck(false);
			m_atEnd->setCheck(true);
			m_atCursor->setCheck(false);
			m_atStart->repaint();
			m_atEnd->repaint();
			m_atCursor->repaint();
		}
		void WaveEdInsertSilenceDialog::onInsCursorClicked( NButtonEvent* ev)
		{
			m_atStart->setCheck(false);
			m_atEnd->setCheck(false);
			m_atCursor->setCheck(true);
			m_atStart->repaint();
			m_atEnd->repaint();
			m_atCursor->repaint();
		}
		void WaveEdInsertSilenceDialog::onOkClicked( NButtonEvent* ev) 
		{
			std::string temp;
			temp = 	m_time->text();
			if(!(timeInSecs=atof(temp.c_str())))
				doClose(false);

			if(m_atStart->checked()) 
				insertPos=at_start;
			else if(m_atEnd->checked()) 
				insertPos=at_end;
			else 
				insertPos=at_cursor;

			doClose(true);
		}

		void WaveEdInsertSilenceDialog::onCancelClicked( NButtonEvent* ev) 
		{
			doClose(false);
		}
}}
