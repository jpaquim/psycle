/***************************************************************************
*   Copyright (C) 2007 Psycledelics Community   *
*   psycle.sourceforge.net   *
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

/**
 * QSynth style knob behaviour based on sf.net/projects/qsynth
 * original (c) Rui Nuno Capela
 * Any dodgy modifications not his fault!
 */


#include <psycle/core/internal_machines.h>
#include <psycle/core/constants.h>

#include "mixertweakdlg.hpp"
#include "../global.hpp"
#include "../configuration.hpp"
#include "../inputhandler.hpp"
#include "machinegui.hpp"
#include <cmath>


#include <QVBoxLayout>
#include <QLabel>
#include <QDial>
#include <QKeyEvent>
#include <QPainter>
#include <QPushButton>
#include <QCompleter>
#include <QModelIndex>

#include <iomanip>

namespace qpsycle {

MixerTweakDlg::MixerTweakDlg( MachineGui *macGui, QWidget *parent ) 
	: MachineTweakDlg( macGui, parent )
{
}

/**
 *
 */
void MixerTweakDlg::initParameterGui()
{

	knobPanelLayout->setMargin( 0 );
	knobPanelLayout->setSpacing( 0 );
	knobPanel->setLayout( knobPanelLayout );

	int numParameters = pMachine_->GetNumParams();
	int cols = pMachine_->GetNumCols();
	int rows = numParameters/cols;

	// Various checks for "non-standard" windows ( lots of parameters, or "odd" parameter numbers)
	if (rows>24) // check for "too big" windows
	{
		rows=24;
		cols=numParameters/24;
		if (cols*24 != numParameters)
		{
			cols++;
		}
	}
	if ( rows*cols < numParameters) rows++; // check if all the parameters are visible.


	int x = 0;
	int y = 0;

	for ( int knobIdx =0; knobIdx < cols*rows; knobIdx++ ) 
	{
		int min_v, max_v;

		if ( knobIdx < numParameters ) {
			pMachine_->GetParamRange( knobIdx,min_v,max_v );

			// FIXME: bit of a crude check to see if we have
			// a knob or a header/gap.
			bool bDrawKnob = (min_v==max_v)?false:true;

			if ( !bDrawKnob ) { // a header or a gap.
				char parName[64];
				pMachine_->GetParamName(knobIdx,parName);
				if(!std::strlen(parName) /* <bohan> don't know what pooplog's plugins use for separators... */ || std::strlen(parName) == 1) {
					knobPanelLayout->addWidget( new KnobHole(), y, x );
				} else {
					SectionHeader* cell = new SectionHeader();
					headerMap[ knobIdx ] = cell;
					cell->setText(parName);
					knobPanelLayout->addWidget( cell, y, x );
				}
			} else if ( knobIdx < numParameters ) { // an actual knob+parameterinfo.
				KnobGroup *knobGroup = new KnobGroup( knobIdx );
				char parName[64];
				pMachine_->GetParamName( knobIdx, parName );
				char buffer[128];
				pMachine_->GetParamValue( knobIdx, buffer );
				knobGroup->setNameText(parName);
				knobGroupMap[ knobIdx ] = knobGroup;
				connect( knobGroup, SIGNAL( changed( KnobGroup* ) ),
						this, SLOT( onKnobGroupChanged( KnobGroup* ) ) );
				knobPanelLayout->addWidget( knobGroup, y, x );
			}
		} else { // no parameters left, but some space left.
			knobPanelLayout->addWidget( new KnobHole(), y, x );
		}
		y++;
		if ( !(y % rows) ) {
			y = 0;
			x++;
		}
	}
	knobPanel->repaint();

	updateValues();
}

void MixerTweakDlg::onKnobGroupChanged( KnobGroup *kGroup ) 
{
	int param = kGroup->knob()->param();
	int value = kGroup->knob()->value();
	pMachine_->SetParameter( param, value );
	char buffer[128];
	
	pMachine_->GetParamValue( param, buffer );
	kGroup->setValueText( QString::fromStdString( buffer ) );
}
} // namespace qpsycle
