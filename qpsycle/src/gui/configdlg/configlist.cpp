/***************************************************************************
*   Copyright (C) 2007 Psycledelics Community   *
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
#include "configlist.h"

#include <QListWidgetItem>

ConfigList::ConfigList(QWidget *parent) 
{
	setIconSize(QSize(32, 32));
	setMaximumWidth(128);
	setParent(parent);
	CreateItems();
}

void ConfigList::CreateItems()
{
	QListWidgetItem *general = new QListWidgetItem(QIcon(":/images/conf-general.png"), "General", this);	
	QListWidgetItem *audio = new QListWidgetItem(QIcon(":/images/conf-audio.png"), "Audio", this);
	QListWidgetItem *keyborad = new QListWidgetItem(QIcon(":/images/conf-keyboard.png"), "Keyboard", this);
	QListWidgetItem *visual = new QListWidgetItem(QIcon(":/images/conf-visual.png"), "Visual", this);
	general->setSelected(true);
}
