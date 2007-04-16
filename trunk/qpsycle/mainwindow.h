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

#include "psycore/song.h"

class MachineView;
class MachineGui;
class PatternView;
class WaveView;
class SequencerView;
class PatternBox;

class QAction;
class QListWidget;
class QMenu;
class QTextEdit;
class QComboBox;
class QTabWidget;

#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();


protected:
    void keyPressEvent( QKeyEvent *event );
    void timerEvent( QTimerEvent *ev );

private slots:
    void newSong();
    void open();
    void save();
    void undo();
    void redo();
    void about();
    void refreshSampleComboBox();
    void onMachineComboBoxIndexChanged( int newIndex );
    void onSampleComboBoxIndexChanged( int newIndex );
    void onPatternSelectedInPatternBox( psy::core::SinglePattern* selectedPattern );
    void onNewMachineCreated( int bus );
    void onMachineGuiChosen( MachineGui *macGui );
    void onPatternDeleted();
    void onAddPatternToSequencerRequest( psy::core::SinglePattern* );
    void onPatternNameChanged();
    void onCategoryColorChanged();
    void onOctaveComboBoxIndexChanged( int newIndex );
    void onMachineDeleted( int macIndex );
    void onMachineRenamed();

 private:
    psy::core::Song *song_;

    void setupSound();
    void setupSong();
    void setupGui();
    void setupSignals();

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
     QAction *playStartAct;
     QAction *playAct;
     QAction *playPatAct;
     QAction *stopAct;

     QAction *togglePatBox_;

     QComboBox *macCombo_;
     QComboBox *sampCombo_;
     QComboBox *octCombo_;

     PatternBox *patternBox_;

     QTabWidget *views_;
     MachineView *macView_;
     PatternView *patView_;
     WaveView *wavView_;
     SequencerView *seqView_;

     QDockWidget *dock_;
 };

 #endif
