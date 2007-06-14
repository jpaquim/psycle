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

#include "waveamp.h"

#include <QHBoxLayout>

WaveAmp::WaveAmp()
{
	setWindowTitle("Amplify");
	QHBoxLayout *layout = new QHBoxLayout();
	gain = new QLabel("Gain", this);
	gainSli = new QSlider( Qt::Horizontal, this);
	ok = new QPushButton("Ok", this);
	cancel = new QPushButton("Cancel", this);
	
	layout->addWidget(gain);
	layout->addWidget(gainSli);
	layout->addWidget(ok);
	layout->addWidget(cancel);
	
}
