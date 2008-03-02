// -*- mode:c++; indent-tabs-mode:t -*-
/******************************************************************************
*  copyright 2007 members of the psycle project http://psycle.sourceforge.net *
*                                                                             *
*  This program is free software; you can redistribute it and/or modify       *
*  it under the terms of the GNU General Public License as published by       *
*  the Free Software Foundation; either version 2 of the License, or          *
*  (at your option) any later version.                                        *
*                                                                             *
*  This program is distributed in the hope that it will be useful,            *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of             *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
*  GNU General Public License for more details.                               *
*                                                                             *
*  You should have received a copy of the GNU General Public License          *
*  along with this program; if not, write to the                              *
*  Free Software Foundation, Inc.,                                            *
*  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                  *
******************************************************************************/

#include <QtGui>

#include "configdlg.hpp"
#include "audiopage.hpp"
#include "behaviourpage.hpp"
#include "lookspage.hpp"
#include "dirspage.hpp"

namespace qpsycle {


	ConfigDialog::ConfigDialog()
	{
		contentsWidget = new QListWidget;
		contentsWidget->setViewMode(QListView::IconMode);
		contentsWidget->setIconSize(QSize(96, 84));
		contentsWidget->setMovement(QListView::Static);
		contentsWidget->setMaximumWidth(128);
		contentsWidget->setSpacing(12);

		pagesWidget = new QStackedWidget;
		pagesWidget->addWidget( new AudioConfigDlg );
		pagesWidget->addWidget( new SettingsDlg );
		pagesWidget->addWidget( new LooksPage );
		pagesWidget->addWidget( new DirsPage );

		QPushButton *closeButton = new QPushButton(tr("Close"));

		createIcons();
		contentsWidget->setCurrentRow(0);

		connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

		QHBoxLayout *horizontalLayout = new QHBoxLayout;
		horizontalLayout->addWidget(contentsWidget);
		horizontalLayout->addWidget(pagesWidget, 1);

		QHBoxLayout *buttonsLayout = new QHBoxLayout;
		buttonsLayout->addStretch(1);
		buttonsLayout->addWidget(closeButton);

		QVBoxLayout *mainLayout = new QVBoxLayout;
		mainLayout->addLayout(horizontalLayout);
		mainLayout->addStretch(1);
		mainLayout->addSpacing(12);
		mainLayout->addLayout(buttonsLayout);
		setLayout(mainLayout);

		setWindowTitle(tr("Config Dialog"));
	}

	void ConfigDialog::createIcons()
	{
		QListWidgetItem *configButton = new QListWidgetItem(contentsWidget);
		configButton->setIcon(QIcon(":/images/config.png"));
		configButton->setText(tr("Audio"));
		configButton->setTextAlignment(Qt::AlignHCenter);
		configButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

		QListWidgetItem *updateButton = new QListWidgetItem(contentsWidget);
		updateButton->setIcon(QIcon(":/images/update.png"));
		updateButton->setText(tr("Behaviour"));
		updateButton->setTextAlignment(Qt::AlignHCenter);
		updateButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

		QListWidgetItem *queryButton = new QListWidgetItem(contentsWidget);
		queryButton->setIcon(QIcon(":/images/query.png"));
		queryButton->setText(tr("Looks"));
		queryButton->setTextAlignment(Qt::AlignHCenter);
		queryButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);


		QListWidgetItem *dirsButton = new QListWidgetItem(contentsWidget);
		dirsButton->setIcon(QIcon(":/images/query.png"));
		dirsButton->setText(tr("Directories"));
		dirsButton->setTextAlignment(Qt::AlignHCenter);
		dirsButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);


		connect(contentsWidget,
			SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
			this, SLOT(changePage(QListWidgetItem *, QListWidgetItem*)));
	}

	void ConfigDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
	{
		if (!current)
			current = previous;

		pagesWidget->setCurrentIndex(contentsWidget->row(current));
	}

}
