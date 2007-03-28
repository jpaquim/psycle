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

#include "machinetweakdlg.h"

#include <QGridLayout>
#include <QLabel>
#include <QDial>

MachineTweakDlg::MachineTweakDlg( psy::core::Machine *mac, QWidget *parent ) 
    : QDialog( parent )
{
    pMachine_ = mac;
    setWindowTitle( "Machine tweak" );
    initParameterGui();
}

void MachineTweakDlg::initParameterGui()
{
    int numParameters = pMachine_->GetNumParams();
    int cols = pMachine_->GetNumCols();
    int rows = numParameters/cols;
    // Various checks for "non-standard" windows ( lots of parameters, or "odd" parameter numbers)
    if (rows>24)	// check for "too big" windows
    {
        rows=24;
        cols=numParameters/24;
        if (cols*24 != numParameters)
        {
            cols++;
        }
    }
    if ( rows*cols < numParameters) rows++; // check if all the parameters are visible.

    knobPanel = new QWidget( this );
    QGridLayout *layout = new QGridLayout();
    knobPanel->setLayout( layout );

    int x = 0;
    int y = 0;

    for (int knobIdx =0; knobIdx < cols*rows; knobIdx++) {
        int min_v,max_v;

        if ( knobIdx < numParameters ) {
            pMachine_->GetParamRange( knobIdx,min_v,max_v);
            bool bDrawKnob = (min_v==max_v)?false:true;

            if (!bDrawKnob) {
                FHeader* cell = new FHeader();
                headerMap[ knobIdx ] = cell;
                char parName[64];
                pMachine_->GetParamName(knobIdx,parName);
                cell->setText(parName);
                layout->addWidget( cell, x, y );
            } else if ( knobIdx < numParameters ) {
                KnobGroup *knobGroup = new KnobGroup( knobIdx );
                char parName[64];
                pMachine_->GetParamName( knobIdx, parName );
                char buffer[128];
                pMachine_->GetParamValue( knobIdx, buffer );
                knobGroup->setNameText(parName);
                knobGroupMap[ knobIdx ] = knobGroup;
                layout->addWidget( knobGroup, x, y );
            }					
        } else {
            // knob hole
            layout->addWidget( new QLabel(""), x, y );
        }
        y++;
        if ( !(y % rows) ) {
            y = 0;
            x++;
        }
    }
    updateValues();
}

void MachineTweakDlg::updateValues( )
{			
    std::map<int, KnobGroup*>::iterator it = knobGroupMap.begin();
    for ( ; it != knobGroupMap.end(); it++ ) {	
        int knobIdx = it->first;
        int min_v,max_v;
        pMachine_->GetParamRange( knobIdx, min_v, max_v);
        KnobGroup* kGroup = it->second;
        char buffer[128];
        pMachine_->GetParamValue( knobIdx, buffer );
        int val_v = pMachine_->GetParamValue( knobIdx );
        kGroup->knob()->setValue(val_v);
        kGroup->knob()->setRange(min_v,max_v);
        kGroup->setValueText( QString::fromStdString( buffer ) );
    }

    knobPanel->repaint();
}


Knob::Knob( int param ) 
{}

KnobGroup::KnobGroup( int param )
{
    QGridLayout *layout = new QGridLayout();
    setLayout( layout );

    knob_ = new Knob( param );
    nameLbl = new QLabel();
    valueLbl = new QLabel();

    layout->addWidget( knob_, 0, 0, 2, 2 );
    layout->addWidget( nameLbl, 0, 2, 1, 6 );
    layout->addWidget( valueLbl, 1, 2, 2, 6 );
}

void MachineTweakDlg::showEvent( QShowEvent *event )
{
    // FIXME: can adjustSize() be called somewhere else?
    adjustSize();
    QWidget::showEvent( event );
}

void KnobGroup::setKnob( Knob *inKnob )
{
    knob_ = inKnob;
}

Knob *KnobGroup::knob()
{
    return knob_;
}

void KnobGroup::setNameText( const QString & text )
{
    nameLbl->setText( text );
}

void KnobGroup::setValueText( const QString & text )
{
    valueLbl->setText( text );
}

QSize Knob::sizeHint() const
{
    return QSize( 20, 20 );
}

FHeader::FHeader( QWidget *parent )
    : QLabel( parent )
{}
