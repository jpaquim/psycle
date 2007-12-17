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
#include "wiregui.hpp"
#include "wiredlg.hpp"
#include "machinegui.hpp"

#include <psycle/core/machine.h>

#include <iomanip>
#include <sstream>

#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>

namespace qpsycle {

const int SliderSize = 1024;
const int MaxVolume = 4; //multiplier, not dB
const float Scaler = (SliderSize*SliderSize)/(float)MaxVolume;  //262144.0f



WireDlg::WireDlg(WireGui *wire, QWidget *parent, QPoint where) 
	: QDialog(parent)
	, m_wireGui(wire)
{
	setModal(false);
	setSizeGripEnabled(true);

	m_volSlider = new QSlider(this);
	m_volSlider->setTickPosition( QSlider::TicksBothSides );
	m_volSlider->setRange(0, SliderSize);
	m_volSlider->setTickInterval(SliderSize/16);
	m_volSlider->setPageStep(SliderSize/16);
	m_volSlider->setSingleStep(SliderSize/128);
	m_volLabel = new QLabel(this);
	m_volLabel->setAlignment( Qt::AlignCenter );
	m_volLabel->setTextFormat( Qt::PlainText );


	m_sourceMac = m_wireGui->sourceMacGui()->mac();
	m_destMac = m_wireGui->destMacGui()->mac();
	m_wireId = (int)m_destMac->FindInputWire( m_sourceMac->id() );
	m_destMac->GetWireVolume( m_wireId, m_volume );

	connect( m_volSlider, SIGNAL(valueChanged(int)), this, SLOT(sliderMoved(int)) );
	connect( this, SIGNAL(volumeChanged(float)), m_wireGui, SLOT(onVolumeChanged(float)) );

	m_volSlider->setValue( (int)sqrtf( m_volume*Scaler ) );

	std::ostringstream oss;
	oss << m_sourceMac->GetEditName() << " -> ";
	oss << m_destMac->GetEditName() << " - Connection Volume";
	setWindowTitle( oss.str().c_str() );

	resize(80, 200);
	move(where);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(m_volSlider);
	layout->addWidget(m_volLabel);
	layout->setAlignment(m_volSlider, Qt::AlignHCenter );
	setLayout(layout);
}

void WireDlg::sliderMoved(int newval)
{
	float newvol = (newval * newval) / Scaler;
	float f;
	m_destMac->GetWireVolume( m_wireId, f );
	if (f != newvol)
	{
		///\todo undo code
		//m_pParent->AddMacViewUndo();
		m_destMac->SetWireVolume( m_wireId, newvol );
		emit volumeChanged(newvol);
	}

	std::ostringstream oss;

	if( newvol == 1.0f ) {
		oss << "100.00%" << std::endl << "0.0 dB";
	} else if ( newvol <= 0.0f ) {
		oss << "0.00%" << std::endl << "-inf dB";
	} else {
		oss.setf(std::ios::fixed, std::ios::floatfield);
		oss << std::setprecision(2) << newvol*100 << "%" << std::endl;
		oss << std::setprecision(1) << std::showpos << 20.0f * log10(newvol) << " dB";
	}

	m_volLabel->setText( oss.str().c_str() );
}

void WireDlg::wireChanged()
{
	m_sourceMac = m_wireGui->sourceMacGui()->mac();
	m_destMac = m_wireGui->destMacGui()->mac();
	m_wireId = (int)m_destMac->FindInputWire( m_sourceMac->id() );
	m_destMac->GetWireVolume( m_wireId, m_volume );
}

} // namespace qpsycle
