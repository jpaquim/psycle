/***************************************************************************
*   Copyright (C) 2006 by  Stefan   *
*   natti@linux   *
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

#include <QtGui>

 #include "mainwindow.h"

 MainWindow::MainWindow()
 {
     QWidget *workArea = new QWidget();
     QHBoxLayout *layout = new QHBoxLayout;
     QWidget *patternBox = new QWidget();
     QVBoxLayout *patternBoxLayout = new QVBoxLayout;
     patternBoxLayout->addWidget(new QTreeView());
     patternBox->setLayout(patternBoxLayout);
     QTabWidget *views = new QTabWidget();
     
     QWidget *macView = new QWidget();
     macView->setPalette(QPalette(QColor(0, 0, 0)));
     macView->setAutoFillBackground(true);
     QGridLayout *macLay = new QGridLayout();
     macView->setLayout(macLay);

     QWidget *patView = new QWidget();
     QGridLayout *patLay = new QGridLayout();
     patLay->addWidget(new QTextEdit());
     patView->setLayout(patLay);

     QWidget *wavView = new QWidget();
     QGridLayout *wavLay = new QGridLayout();
     wavLay->addWidget(new QTextEdit());
     wavView->setLayout(wavLay);

     QWidget *seqView = new QWidget();
     QGridLayout *seqLay = new QGridLayout();
     seqLay->addWidget(new QTextEdit());
     seqView->setLayout(seqLay);

     views->addTab(macView, "Machine View");
     views->addTab(patView, "Pattern View");
     views->addTab(wavView, "Wave Editor");
     views->addTab(seqView, "Sequencer View");

     layout->addWidget(patternBox);
     layout->addWidget(views);
     workArea->setLayout(layout);
     setCentralWidget(workArea);

     createActions();
     createMenus();
     createToolBars();
     createStatusBar();

     setWindowTitle(tr("] Psycle Modular Music Creation Studio [ ( Q alpha ) "));

 }

 void MainWindow::newSong()
 {

 }

 void MainWindow::open()
 {

 }

 void MainWindow::save()
 {
     QString fileName = QFileDialog::getSaveFileName(this,
                         tr("Choose a file name"), ".",
                         tr("HTML (*.html *.htm)"));
     if (fileName.isEmpty())
         return;
     QFile file(fileName);
     if (!file.open(QFile::WriteOnly | QFile::Text)) {
         QMessageBox::warning(this, tr("Dock Widgets"),
                              tr("Cannot write file %1:\n%2.")
                              .arg(fileName)
                              .arg(file.errorString()));
         return;
     }

     QTextStream out(&file);
     QApplication::setOverrideCursor(Qt::WaitCursor);
     out << textEdit->toHtml();
     QApplication::restoreOverrideCursor();

     statusBar()->showMessage(tr("Saved '%1'").arg(fileName), 2000);
 }

 void MainWindow::undo()
 {
     QTextDocument *document = textEdit->document();
     document->undo();
 }

 void MainWindow::redo()
 {
     QTextDocument *document = textEdit->document();
     document->redo();
 }

 void MainWindow::about()
 {
    QMessageBox::about(this, tr("About qpsycle"),
             tr("It makes music and stuff."));
 }

 void MainWindow::createActions()
 {
     newAct = new QAction(QIcon(":/images/new.png"), tr("&New Song"), this);
     newAct->setShortcut(tr("Ctrl+N"));
     newAct->setStatusTip(tr("Create a new song"));
     connect(newAct, SIGNAL(triggered()), this, SLOT(newSong()));

     openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
     openAct->setShortcut(tr("Ctrl+O"));
     openAct->setStatusTip(tr("Open an existing song"));
     connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

     saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save..."), this);
     saveAct->setShortcut(tr("Ctrl+S"));
     saveAct->setStatusTip(tr("Save the current song"));
     connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

     undoAct = new QAction(QIcon(":/images/undo.png"), tr("&Undo"), this);
     undoAct->setShortcut(tr("Ctrl+Z"));
     undoAct->setStatusTip(tr("Undo the last action"));
     connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()));

     redoAct = new QAction(QIcon(":/images/redo.png"), tr("&Redo"), this);
     redoAct->setShortcut(tr("Ctrl+Y"));
     redoAct->setStatusTip(tr("Redo the last undone action"));
     connect(redoAct, SIGNAL(triggered()), this, SLOT(redo()));

     quitAct = new QAction(tr("&Quit"), this);
     quitAct->setShortcut(tr("Ctrl+Q"));
     quitAct->setStatusTip(tr("Quit the application"));
     connect(quitAct, SIGNAL(triggered()), this, SLOT(close()));

     aboutAct = new QAction(tr("&About qpsycle"), this);
     aboutAct->setStatusTip(tr("About qpsycle"));
     connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

     playStartAct = new QAction(QIcon(":/images/playstart.png"), tr("&Play from start"), this);
     playAct = new QAction(QIcon(":/images/play.png"), tr("Play from &edit position"), this);
     playPatAct = new QAction(QIcon(":/images/playselpattern.png"), tr("Play selected p&attern"), this);
     stopAct = new QAction(QIcon(":images/stop.png"), tr("&Stop playback"), this);

 }

 void MainWindow::createMenus()
 {
     fileMenu = menuBar()->addMenu(tr("&File"));
     fileMenu->addAction(newAct);
     fileMenu->addAction(openAct);
     fileMenu->addAction(saveAct);
     fileMenu->addSeparator();
     fileMenu->addAction(quitAct);

     editMenu = menuBar()->addMenu(tr("&Edit"));
     editMenu->addAction(undoAct);
     editMenu->addAction(redoAct);

     viewMenu = menuBar()->addMenu(tr("&View"));
     configMenu = menuBar()->addMenu(tr("&Configuration"));
     performMenu = menuBar()->addMenu(tr("&Performance"));
     communityMenu = menuBar()->addMenu(tr("&Community"));

     menuBar()->addSeparator();

     helpMenu = menuBar()->addMenu(tr("&Help"));
     helpMenu->addAction(aboutAct);
 }

 void MainWindow::createToolBars()
 {
     fileToolBar = addToolBar(tr("File"));
     fileToolBar->addAction(newAct);
     fileToolBar->addAction(openAct);
     fileToolBar->addAction(saveAct);

     editToolBar = addToolBar(tr("Edit"));
     editToolBar->addAction(undoAct);
     editToolBar->addAction(redoAct);

     playToolBar = addToolBar(tr("Play"));
     playToolBar->addAction(playStartAct);
     playToolBar->addAction(playAct);
     playToolBar->addAction(playPatAct);
     playToolBar->addAction(stopAct);

     machToolBar = addToolBar(tr("Machines"));
     genCombo = new QComboBox();
     fxCombo = new QComboBox();
     sampCombo = new QComboBox();
     QLabel *genLabel = new QLabel(" Gen: ");
     QLabel *fxLabel = new QLabel(" FX: ");
     QLabel *sampLabel = new QLabel(" Samples: ");
     machToolBar->addWidget(genLabel);
     machToolBar->addWidget(genCombo);
     machToolBar->addWidget(fxLabel);
     machToolBar->addWidget(fxCombo);
     machToolBar->addWidget(sampLabel);
     machToolBar->addWidget(sampCombo);

 }

 void MainWindow::createStatusBar()
 {
     statusBar()->showMessage(tr("Ready"));
 }

