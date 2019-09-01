#include "editmenu.h"
#include "menusignalhandler.h"
#include "qpsycle2.h"

#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QMenuBar>

namespace qpsycle{

EditMenu::EditMenu(qpsycle2* window)
{
    MenuSignalHandler* handler = window->getSignalHandler();

    QMenu* editMenu = new QMenu(window);
    editMenu->setTitle("Edit");
    window->menuBar()->addMenu(editMenu);

    QAction* undoAction = new QAction(window);
    undoAction->setText("undo");
    QObject::connect(undoAction,SIGNAL(triggered()),handler,SLOT(undo()));

    QAction* redoAction = new QAction(window);
    redoAction->setText("redo");
    QObject::connect(redoAction,SIGNAL(triggered()),handler,SLOT(redo()));

    QAction* patternCutAction = new QAction(window);
    patternCutAction->setText("Pattern Cut");
    QObject::connect(patternCutAction,SIGNAL(triggered()),handler,SLOT(patternCut()));

    QAction* patternCopyAction = new QAction(window);
    patternCopyAction->setText("Pattern Copy");
    QObject::connect(patternCopyAction,SIGNAL(triggered()),handler,SLOT(patternCopy()));

    QAction* patternPasteAction = new QAction(window);
    patternPasteAction->setText("Pattern Paste");
    QObject::connect(patternPasteAction,SIGNAL(triggered()),handler,SLOT(patternPaste()));

    QAction* patternMixPasteAction = new QAction(window);
    patternMixPasteAction->setText("Pattern Mix Paste");
    QObject::connect(patternMixPasteAction,SIGNAL(triggered()),handler,SLOT(patternMixPaste()));

    editMenu->addAction(undoAction);
    editMenu->addAction(redoAction);
    editMenu->addSeparator();
    editMenu->addAction(patternCutAction);
    editMenu->addAction(patternCopyAction);
    editMenu->addAction(patternCopyAction);
    editMenu->addAction(patternPasteAction);
    editMenu->addAction(patternMixPasteAction);
}

}
