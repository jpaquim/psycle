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

class QAction;
class QListWidget;
class QMenu;
class QTextEdit;
class QComboBox;
class QTabWidget;
class QPushButton;

#include <QMainWindow>

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
    MainWindow();

protected:
    void keyPressEvent( QKeyEvent *event );

private slots:
    void onBreedClicked();

private:
    psy::core::Song *song_;
    QWidget *mainWidget_;
    QWidget *sidebar_;

    void setupSound();
    void setupSong();
    void setupGui();
    void setupSignals();

    psy::core::Machine *breed( psy::core::Machine *dad, psy::core::Machine *mum );
    psy::core::Song *createBlankSong();

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

     MachineView *macView_;

     QPushButton *breedBut_;

 };

 #endif
