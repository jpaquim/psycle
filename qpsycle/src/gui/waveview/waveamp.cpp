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
#include <qpsyclePch.hpp>

#include "waveamp.hpp"

#include <QGridLayout>

WaveAmp::WaveAmp()
{
	setWindowTitle("Amplify");
	QGridLayout *layout = new QGridLayout(this);
	gain = new QLabel("Gain", this);
	gainSli = new QSlider( Qt::Horizontal, this);
	percent = new QLabel ("0%", this);
	ok = new QPushButton("Ok", this);
	cancel = new QPushButton("Cancel", this);
	
	layout->addWidget(gain, 0, 0, 1, 1);
	layout->addWidget(gainSli, 0, 1, 1, 1);
	layout->addWidget(percent, 0, 2, 1, 1);
	layout->addWidget(ok, 1, 1, 1, 1, Qt::AlignRight);
	layout->addWidget(cancel, 1, 2, 1, 1, Qt::AlignRight);
	
	//connect( gainSli, SIGNAL(sliderMoved(int value)), this, SLOT(SliderChanges(int value)));
}

#if 0
void WaveAmp::SliderChanges(int value)
{
	percent->setText( gainSli->value() + "%");
}
#endif
