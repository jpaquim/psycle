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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

namespace psy { namespace core {
class Song;
class SinglePattern;
class Machine;
}}

class MachineView;
class MachineGui;
class PatternView;
class WaveView;
class SequencerView;
class PatternBox;
class AudioConfigDlg;

class QAction;
class QListWidget;
class QMenu;
class QTextEdit;
class QComboBox;

#include <QTabWidget>
#include <QMainWindow>


// Subclassing as we need to override event().
class TabWidget : public QTabWidget {
public:
	TabWidget( QWidget *parent = 0 );
protected:
	bool event( QEvent *event );
};


class MainWindow : public QMainWindow
{
Q_OBJECT
public:
	MainWindow();

protected:
	void keyPressEvent( QKeyEvent *event );
	void timerEvent( QTimerEvent *ev );

private slots:
	void onNewSongRequest();
	void onOpenSongRequest();
	void onSaveSongRequest();

	void undo();
	void redo();

	void aboutQpsycle();

	void refreshSampleComboBox();
	void onMachineComboBoxIndexChanged( int newIndex );
	void onSampleComboBoxIndexChanged( int newIndex );
	void onOctaveComboBoxIndexChanged( int newIndex );

	void onPatternSelectedInPatternBox( psy::core::SinglePattern* selectedPattern );
	void onNewMachineCreated( psy::core::Machine *mac );
	void onMachineChosen( MachineGui *macGui );
	void onMachineDeleted();
	void onMachineRenamed();

	void onPatternDeleted();
	void onPatternNameChanged();

	void onAddPatternToSequencerRequest( psy::core::SinglePattern* );
	void onCategoryColorChanged();

	void playFromStart();
	void playFromSeqPos();
	void playStop();

	void showAudioConfigDlg();

private:
	psy::core::Song *song_;

	void setupSound();
	void setupSong();
	void setupGui();
	void setupSignals();

	bool songHasChanged();
	psy::core::Song *createBlankSong();
	void loadSong( psy::core::Song *song );

	void populateMachineCombo();
	void initSampleCombo();
    
	void createActions();
	void createMenus();
	void createToolBars();
	void createStatusBar();

	QMenu *fileMenu;
	QMenu *editMenu;
	QMenu *viewMenu;
	QMenu *configMenu;
	QMenu *performMenu;
	QMenu *communityMenu;
	QMenu *helpMenu;

	QToolBar *fileToolBar;
	QToolBar *editToolBar;
	QToolBar *playToolBar;
	QToolBar *machToolBar;
	QToolBar *octToolBar_;

	QAction *newAct;
	QAction *openAct;
	QAction *saveAct;
	QAction *undoAct;
	QAction *redoAct;
	QAction *aboutAct;
	QAction *quitAct;
	QAction *playFromStartAct;
	QAction *playFromSeqPosAct;
	QAction *playPatAct;
	QAction *playStopAct;

	QAction *togglePatBox_;

	QAction *audioConfAct;

	QComboBox *macCombo_;
	QComboBox *sampCombo_;
	QComboBox *octCombo_;

	PatternBox *patternBox_;

	TabWidget *views_;
	MachineView *macView_;
	PatternView *patView_;
	WaveView *wavView_;
	SequencerView *seqView_;

	QDockWidget *dock_;

	AudioConfigDlg *audioCnfDlg;
};

#endif
