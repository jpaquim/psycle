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

#include "psycore/machine.h"
#include "psycore/inputhandler.h"
#include "psycore/global.h"
#include "psycore/configuration.h"

#include "machinetweakdlg.h"
#include "machinegui.h"

#include <QGridLayout>
#include <QLabel>
#include <QDial>
#include <QKeyEvent>
#include <QPainter>

MachineTweakDlg::MachineTweakDlg( MachineGui *macGui, QWidget *parent ) 
    : QDialog( parent )
{
    pMachine_ = macGui->mac();
    m_macGui = macGui;
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
    layout->setMargin( 0 );
    layout->setSpacing( 0 );
    knobPanel->setLayout( layout );

    int x = 0;
    int y = 0;

    for ( int knobIdx =0; knobIdx < cols*rows; knobIdx++ ) {
        int min_v,max_v;

        if ( knobIdx < numParameters ) {
            pMachine_->GetParamRange( knobIdx,min_v,max_v);
            bool bDrawKnob = (min_v==max_v)?false:true;

            if ( !bDrawKnob ) {
                FHeader* cell = new FHeader();
                headerMap[ knobIdx ] = cell;
                char parName[64];
                pMachine_->GetParamName(knobIdx,parName);
                cell->setText(parName);
                layout->addWidget( cell, y, x );
            } else if ( knobIdx < numParameters ) {
                KnobGroup *knobGroup = new KnobGroup( knobIdx );
                char parName[64];
                pMachine_->GetParamName( knobIdx, parName );
                char buffer[128];
                pMachine_->GetParamValue( knobIdx, buffer );
                knobGroup->setNameText(parName);
                knobGroupMap[ knobIdx ] = knobGroup;
                connect( knobGroup, SIGNAL( changed( KnobGroup* ) ),
                         this, SLOT( onKnobGroupChanged( KnobGroup* ) ) );
                layout->addWidget( knobGroup, y, x );
            }					
        } else {
            // knob hole
            layout->addWidget( new QLabel(""), y, x );
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

void MachineTweakDlg::onKnobGroupChanged( KnobGroup *kGroup ) 
{
    int param = kGroup->knob()->param();
    int value = kGroup->knob()->value();
    pMachine_->SetParameter( param, value );
    char buffer[128];
    int val_v = pMachine_->GetParamValue( param );
    pMachine_->GetParamValue( param, buffer );
    kGroup->setValueText( QString::fromStdString( buffer ) );
}

void MachineTweakDlg::showEvent( QShowEvent *event )
{
    // FIXME: can adjustSize() be called somewhere else?
    adjustSize();
    QWidget::showEvent( event );
}

void MachineTweakDlg::keyPressEvent( QKeyEvent *event )
{
    if ( event->key() == Qt::Key_W && event->modifiers() == Qt::ControlModifier ) {
        reject();
    } else {
        if ( !event->isAutoRepeat() ) {
            int command = psy::core::Global::pConfig()->inputHandler().getEnumCodeByKey( psy::core::Key( event->modifiers(), event->key() ) );
            int note = NULL;
            note = m_macGui->noteFromCommand( command );
            if (note) {
                emit notePress( note, pMachine_ );   
            }
        }
    }
}

void MachineTweakDlg::keyReleaseEvent( QKeyEvent *event )
{
    if ( !event->isAutoRepeat() ) {
        int command = psy::core::Global::pConfig()->inputHandler().getEnumCodeByKey( psy::core::Key( event->modifiers(), event->key() ) );
        int note = NULL;
        note = m_macGui->noteFromCommand( command );
        if (note) {
            emit noteRelease( note );   
        }
    }
}



KnobGroup::KnobGroup( int param )
{
    QGridLayout *layout = new QGridLayout();
    layout->setMargin( 0 );
    layout->setSpacing( 0 );
    setLayout( layout );
    setFixedSize( LABEL_WIDTH, K_YSIZE ); // FIXME: unfix the size.

    knob_ = new Knob( param );

    nameLbl = new QLabel();
    QPalette plt = nameLbl->palette();
    plt.setBrush( QPalette::Window, QBrush( QColor( 194, 190, 210 ) ) );
    plt.setBrush( QPalette::WindowText, QBrush( Qt::black ) );
    nameLbl->setPalette( plt );
    nameLbl->setFont( QFont( "Verdana", 8 ) );
    nameLbl->setFixedSize( 150, K_XSIZE/2 );
    nameLbl->setIndent( 5 );
    nameLbl->setAutoFillBackground( true );

    valueLbl = new QLabel();
    QPalette plt1 = valueLbl->palette();
    plt1.setBrush( QPalette::Window, QBrush( QColor( 121, 109, 156 ) ) );
    plt1.setBrush( QPalette::WindowText, QBrush( Qt::white ) );
    valueLbl->setPalette( plt1 );
    valueLbl->setFont( QFont( "Verdana", 8 ) );
    valueLbl->setFixedSize( 150, K_YSIZE/2 );
    valueLbl->setIndent( 5 );
    valueLbl->setAutoFillBackground( true );

    connect( knob_, SIGNAL( valueChanged( int ) ),
             this, SLOT( onKnobChanged() ) );
    connect( knob_, SIGNAL( sliderPressed() ),
             this, SLOT( onKnobPressed() ) );
    connect( knob_, SIGNAL( sliderReleased() ),
             this, SLOT( onKnobReleased() ) );

    layout->addWidget( knob_, 0, 0, 2, 2, Qt::AlignLeft );
    layout->addWidget( nameLbl, 0, 2, 1, 7, Qt::AlignLeft );
    layout->addWidget( valueLbl, 1, 2, 1, 7, Qt::AlignLeft );
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

void KnobGroup::onKnobChanged()
{
    emit changed( this );
}

void KnobGroup::onKnobPressed()
{
    QPalette plt = nameLbl->palette();
    plt.setBrush( QPalette::Window, QBrush( QColor( 169, 148, 172 ) ) );
    plt.setBrush( QPalette::WindowText, QBrush( Qt::black ) );
    nameLbl->setPalette( plt );

    QPalette plt1 = valueLbl->palette();
    plt1.setBrush( QPalette::Window, QBrush( QColor( 114, 90, 139 ) ) );
    plt1.setBrush( QPalette::WindowText, QBrush( QColor( 255, 238, 68 ) ) );
    valueLbl->setPalette( plt1 );
}

void KnobGroup::onKnobReleased()
{
    QPalette plt = nameLbl->palette();
    plt.setBrush( QPalette::Window, QBrush( QColor( 194, 190, 210 ) ) );
    plt.setBrush( QPalette::WindowText, QBrush( Qt::black ) );
    nameLbl->setPalette( plt );

    QPalette plt1 = valueLbl->palette();
    plt1.setBrush( QPalette::Window, QBrush( QColor( 121, 109, 156 ) ) );
    plt1.setBrush( QPalette::WindowText, QBrush( Qt::white ) );
    valueLbl->setPalette( plt1 );
}

QSize KnobGroup::sizeHint() const
{
    return QSize( LABEL_WIDTH, K_YSIZE );
}

/**
 * Knob class.
 * 
 */
Knob::Knob( int param ) 
    : param_( param )
{
    setFixedSize( K_XSIZE, K_YSIZE ); // FIXME: unfix the size.
}

void Knob::paintEvent( QPaintEvent *ev )
{
    QPainter painter(this);
    QRectF target( 0, 0, K_XSIZE, K_YSIZE );
    QPixmap pixmap(":/images/TbMainKnob.png");

    int amp = maximum() - minimum();
    int rel = value() - minimum();

    if ( amp > 0 ) {
        int frame = ( K_NUMFRAMES * rel )/amp;
        int xn = frame * K_XSIZE;
        QRectF source( xn, 0, K_XSIZE, K_YSIZE );

        painter.drawPixmap( target, pixmap, source );
    }
}

QSize Knob::sizeHint() const
{
    return QSize( K_XSIZE, K_YSIZE );
}



/**
 * FHeader class.
 * 
 */
FHeader::FHeader( QWidget *parent )
    : QLabel( parent )
{
    setFont( QFont( "Verdana", 8 ) );
    setIndent( 10 );
}

void FHeader::paintEvent( QPaintEvent *ev )
{
    QPainter painter(this);

    painter.fillRect( 0, 0, width(), height()/4, QColor( 194, 190, 210 ) );
    painter.fillRect( 0, height()/4, width(), height()/2, Qt::black );
    QRectF textRect( indent(), height()/4, width(), height()/2 );
    painter.setPen( Qt::white );
    painter.drawText( textRect, Qt::AlignLeft | Qt::AlignVCenter, text() );
    painter.fillRect( 0, (height()*3)/4, width(), height()/4, QColor( 121, 109, 156 ) );
}
