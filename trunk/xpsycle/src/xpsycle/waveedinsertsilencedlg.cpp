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
#include <ngrs/nalignlayout.h>
#include <ngrs/nalignconstraint.h>
#include <ngrs/ntablelayout.h>

namespace psycle
{
	namespace host
	{
		WaveEdInsertSilenceDialog::WaveEdInsertSilenceDialog()
		{
			setTitle("Insert Silence");

			NPanel* table = new NPanel();
				NTableLayout tableLayout(2,3);
				tableLayout.setVGap(5);
				tableLayout.setHGap(5);
				table->setLayout(tableLayout);
				m_atStart = new NCheckBox("At start");
				m_atEnd = new NCheckBox("At end");
				m_atCursor = new NCheckBox("At cursor");
			
			table->add(m_atStart, NAlignConstraint(nAlLeft,1,0),true);
			table->add(m_atEnd , NAlignConstraint(nAlLeft,1,1),true);
			table->add(m_atCursor, NAlignConstraint(nAlLeft,1,2),true);
			
			m_atStart->clicked.connect(this, &WaveEdInsertSilenceDialog::onInsStartClicked);
			m_atEnd->clicked.connect(this, &WaveEdInsertSilenceDialog::onInsEndClicked);
			m_atCursor->clicked.connect(this, &WaveEdInsertSilenceDialog::onInsCursorClicked);

			m_atStart->setCheck(true);
			m_atEnd->setCheck(false);
			m_atCursor->setCheck(false);

			m_time = new NEdit;
			table->add(m_time,NAlignConstraint(nAlLeft,0,1),true);
			m_time->setText("0.000");
			NPanel* okPanel = new NPanel();
			okPanel->setLayout(NAlignLayout(5,5));
			{
				NButton *btn = new NButton("Cancel", false);
				btn->clicked.connect(this, &WaveEdInsertSilenceDialog::onCancelClicked);
				okPanel->add(btn,nAlRight);
			}
			{
				NButton *btn = new NButton("Ok", false);
				btn->clicked.connect(this, &WaveEdInsertSilenceDialog::onOkClicked);
				okPanel->add(btn,nAlRight);
			}

			pane()->add(okPanel,nAlBottom);
			{
				NLabel *lbl = new NLabel("Insert (in secs):");
				table->add(lbl,NAlignConstraint(nAlLeft,0,0),true);
			}
			pane()->add(table,nAlClient);
			pack();
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
