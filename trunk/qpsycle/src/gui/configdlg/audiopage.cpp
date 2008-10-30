/***************************************************************************
*   Copyright (C) 2007 by Psycledelics Community   *
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
#include <psycle/core/player.h>
#include <psycle/audiodrivers/audiodriver.h>

#include "audiopage.hpp"
#include "../global.hpp"
#include "../configuration.hpp"

#include <QGridLayout>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>

namespace qpsycle {

AudioPage::AudioPage(QWidget * parent)
	: QWidget(parent)
{
	setWindowTitle(tr("Select Audio Driver"));
	config_ = Global::pConfig();
	selectedDriver_ = 0;

	QVBoxLayout *mainLay = new QVBoxLayout();

	QGroupBox *audio_driverGroup = new QGroupBox("Audio Driver", this);
	QGridLayout *audio_driverLay = new QGridLayout();

	audio_driverLay->setAlignment(Qt::AlignTop);
	audio_driverGroup->setLayout(audio_driverLay);

	audio_driverCbx_ = new QComboBox(this);
	audio_driverLbl_ = new QLabel(tr("Audio Driver"), this);
	audio_deviceBox_ = new QLineEdit("", this);
	audio_deviceLbl_ = new QLabel(tr("Audio Device"), this);

	audio_restartBtn_ = new QPushButton(tr("Restart Audio Driver"), this);


	connect(audio_driverCbx_, SIGNAL(currentIndexChanged(QString const &)),
			this, SLOT(onDriverSelected(QString const &)));
	connect(audio_restartBtn_, SIGNAL(clicked()),
			this, SLOT(onRestartDriver()));

	audio_driverLay->addWidget(audio_driverLbl_, 0, 0);
	audio_driverLay->addWidget(audio_deviceLbl_, 1, 0);
	audio_driverLay->addWidget(audio_driverCbx_, 0, 1);
	audio_driverLay->addWidget(audio_deviceBox_, 1, 1);
	audio_driverLay->addWidget(audio_restartBtn_, 0, 2);

	QGroupBox *midi_driverGroup = new QGroupBox("MIDI Driver", this);
	QGridLayout *midi_driverLay = new QGridLayout();

	midi_driverLay->setAlignment(Qt::AlignTop);
	midi_driverGroup->setLayout(midi_driverLay);

	midi_driverCbx_ = new QComboBox(this);
	//test purpose
	midi_driverCbx_->addItem("ALSA Sequencer");
	midi_driverCbx_->addItem("G0 DirectMIDI Driver");

	//end
	midi_driverLbl_ = new QLabel(tr("MIDI Driver"), this);
	midi_deviceBox_ = new QLineEdit("", this);
	midi_deviceLbl_ = new QLabel(tr("MIDI Device"), this);

	midi_restartBtn_ = new QPushButton(tr("Restart MIDI Driver"), this);

	midi_driverLay->addWidget(midi_driverLbl_, 0, 0);
	midi_driverLay->addWidget(midi_deviceLbl_, 1, 0);
	midi_driverLay->addWidget(midi_driverCbx_, 0, 1);
	midi_driverLay->addWidget(midi_deviceBox_, 1, 1);
	midi_driverLay->addWidget(midi_restartBtn_, 0, 2);

	mainLay->addWidget(audio_driverGroup);
	mainLay->addWidget(midi_driverGroup);

	setLayout(mainLay);

	initDriverList();
}

void AudioPage::initDriverList() {
	std::map<std::string, psy::core::AudioDriver*> & driverMap =  config_->driverMap();
	for(std::map<std::string, psy::core::AudioDriver*>::iterator i(driverMap.begin()), e(driverMap.end()); i != e; ++i) {
		if(!i->second->info().show()) continue;
		QString driverName = QString::fromStdString(i->first);
		audio_driverCbx_->addItem(driverName);
		if(i->second == Global::pConfig()->_pOutputDriver) {
			audio_driverCbx_->setCurrentIndex(audio_driverCbx_->count() - 1);
			if(driverName == "alsa")
				audio_deviceBox_->setText(i->second->settings().deviceName().c_str());
		}
	}
}

void AudioPage::onDriverSelected(QString const & text) {
	std::map<std::string, psy::core::AudioDriver*> & driverMap =  config_->driverMap();
	std::map<std::string, psy::core::AudioDriver*>::iterator i(driverMap.find(text.toStdString()));
	if(i != driverMap.end()) {
		psy::core::AudioDriver* driver = i->second;
		selectedDriver_ = driver;
		if(text == "alsa") {
			audio_deviceBox_->setText(i->second->settings().deviceName().c_str());
			audio_deviceBox_->setVisible(true);
			audio_deviceLbl_->setVisible(true);
		} else {
			audio_deviceBox_->setText("");
			audio_deviceBox_->setVisible(false);
			audio_deviceLbl_->setVisible(false);
		}
	}
}

void AudioPage::onRestartDriver() {
	if(selectedDriver_) {
		// set the device
		if(!audio_deviceBox_->text().isEmpty()) {
			psy::core::AudioDriverSettings settings = selectedDriver_->settings();
			settings.setDeviceName( audio_deviceBox_->text().toStdString() );
			selectedDriver_->setSettings(settings);
		}
		// set new Driver to Player
		psy::core::Player::singleton().setDriver(*selectedDriver_);

		//check that it worked
		///\todo find a better/more descriptive way to do this
		if(
			psy::core::Player::singleton().driver().info().name() == "silent" &&
			selectedDriver_->info().name() != "silent"
		) {
			std::string drivername = selectedDriver_->info().name();
			QString msg = QString(tr("The %1 driver failed to load.")).arg(drivername.c_str());

			if(drivername == "alsa")
				msg.append(tr(" Did you specify a valid alsa device name?"));
			else if(drivername == "jack" )
				msg.append(tr(" Are you sure jackd is running?"));

			QMessageBox::warning(
				this, tr("Driver Restart Failed"), msg,
				QMessageBox::Ok, QMessageBox::Ok
			);

		}
	}
}

} // namespace qpsycle
