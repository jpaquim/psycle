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
class SampleBrowser;
class AudioConfigDlg;
class SettingsDlg;

class QAction;
class QListWidget;
class QMenu;
class QTextEdit;
class QComboBox;
class QStandardItemModel;

class InstrumentsModel;

#include "logconsole.h"

#include <QTabWidget>
#include <QMainWindow>
#include <QUndoCommand>
#include <QUndoGroup>
#include <QUndoView>
	
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
	~MainWindow();
	QUndoStack *undoStack;

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

	void showPatternBox();
	void showMachineView();
	void showPatternView();
	void showWaveEditor();
	void showSequencerView();
	void showAudioConfigDlg();
	void showSettingsDlg();
	void showUndoView();
	void showLogCons();

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

	void createUndoView();

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

	QAction *showPatternBoxAct_;
	QAction *showMachineViewAct_;
	QAction *showPatternViewAct_;
	QAction *showWaveEditorAct_;
	QAction *showSequencerViewAct_;

	QAction *showUnReAct;
	QAction *showLogConsAct;

	QAction *playFromStartAct;
	QAction *playFromSeqPosAct;
	QAction *playPatAct;
	QAction *playStopAct;
	QAction *togglePatBox_;
	QAction *audioConfAct;
	QAction *settingsConfAct;

	QComboBox *macCombo_;
	QComboBox *sampCombo_;
	QComboBox *octCombo_;

	QUndoView *undoView;

	PatternBox *patternBox_;
	LogConsole *logConsole_;
	SampleBrowser *sampleBrowser_;

	TabWidget *views_;
	MachineView *macView_;
	PatternView *patView_;
	WaveView *wavView_;
	SequencerView *seqView_;

	QDockWidget *dock_;
	QDockWidget *dockL_;

	AudioConfigDlg *audioCnfDlg;
	SettingsDlg *settingsDlg;

	InstrumentsModel *instrumentsModel_;
};

#endif
