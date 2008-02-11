// -*- mode:c++; indent-tabs-mode:t -*-
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
	#ifndef WAVEVIEW_H
	#define WAVEVIEW_H

#include <QWidget>
#include <QSpinBox>


class QStandardItemModel;
class QComboBox;
class QLineEdit;
class QPushButton;
class QSlider;
class QLabel;
class QCheckBox;

namespace qpsycle {

class InstrumentsModel;
class WaveDisplay;
class HexSpinBox;

class WaveView : public QWidget
{
	Q_OBJECT

public:
	WaveView( InstrumentsModel *instrumentsModel_,
			QWidget *parent = 0);

public slots:
	void indexSpinChanged(int newidx);
	void reset();

	void onLoadButtonClicked();
	void onKillButtonClicked();
	void onZoomInButtonClicked();
	void onZoomOutButtonClicked();

	void nameChanged();
	void onListenPressed();
	void onListenReleased();
	void onVolSliderMoved(int newval);
	void onPanSliderMoved(int newval);
	void onCoarseTuneChanged();
	void onFineTuneChanged();

	void onRandPanChanged(int newstate);
	void onRandResChanged(int newstate);
	void onRandCutoffChanged(int newstate);

	void onNNAChanged(int newstate);
	void onLoopChanged(int newstate);

private:
	HexSpinBox *instIndexSpin_;
	QLineEdit *instName_;
	QComboBox *outputCbx_;

	QPushButton *zoomInBtn_, *zoomOutBtn_;
	QSlider *volSlider_, *panSlider_;
	QLabel *volLabel_, *panLabel_;

	/// instrument tuning widgets
	QSpinBox *octaveSpin_, *semiSpin_, *centSpin_;

	/// randomization widgets
	QCheckBox *randPanChk_, *randResChk_, *randCutChk_;

	QComboBox *nnaCbx_;
	QComboBox *loopCbx_;

	InstrumentsModel *instrumentsModel_;
	WaveDisplay *waveDisplay_;

	//used to ensure the instrument isn't modified while initializing the controls
	bool resettingwidgets_;
};



class HexSpinBox : public QSpinBox
{
Q_OBJECT
public:
	HexSpinBox(QWidget *parent = 0) : QSpinBox(parent) {}

	QString textFromValue ( int value ) const;
	int valueFromText ( const QString & text ) const;
	QValidator::State validate ( QString &input, int &pos ) const;
};

} // namespace qpsycle

#endif
