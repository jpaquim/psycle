/***************************************************************************
*   Copyright (C) 2006 by  Neil Mather   *
*   nmather@sourceforge   *
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

 #include <QMainWindow>

 class QAction;
 class QListWidget;
 class QMenu;
 class QTextEdit;
 class QComboBox;

 #include "psycore/song.h"

 class MainWindow : public QMainWindow
 {
     Q_OBJECT

 public:
     MainWindow();

 private slots:
     void newSong();
     void open();
     void save();
     void undo();
     void redo();
     void about();

 private:
    psy::core::Song *song_;
    
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

     QComboBox *genCombo;
     QComboBox *fxCombo;
     QComboBox *sampCombo;
 };

 #endif
