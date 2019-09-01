#include "filemenu.h"
#include "menusignalhandler.h"
#include "qpsycle2.h"
#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QMenuBar>

namespace qpsycle{

FileMenu::FileMenu(qpsycle2* window)
{
    QMenu* fileMenu = new QMenu(window);
    fileMenu->setTitle("File");
    window->menuBar()->addMenu(fileMenu);

    MenuSignalHandler* handler = window->getSignalHandler();

    QAction* newAction = new QAction(window);
    newAction->setText("New");
    QObject::connect(newAction,SIGNAL(triggered()),handler,SLOT(newProject()));

    QAction* saveAction = new QAction(window);
    saveAction->setText("Save");
    QObject::connect(saveAction,SIGNAL(triggered()),handler,SLOT(save()));

    QAction* saveAsAction = new QAction(window);
    saveAsAction->setText("Save As...");
    QObject::connect(saveAsAction,SIGNAL(triggered()),handler,SLOT(saveAs()));

    QAction* openAction = new QAction(window);
    openAction->setText("Open");
    QObject::connect(openAction,SIGNAL(triggered()),handler,SLOT(open()));

    QAction* renderAction = new QAction(window);
    renderAction-> setText("Render to WAV");
    QObject::connect(renderAction,SIGNAL(triggered()),handler,SLOT(render()));

    QAction* propertiesAcction = new QAction(window);
    propertiesAcction->setText("Song Properties");
    QObject::connect(propertiesAcction,SIGNAL(triggered()),handler,SLOT(properties()));

    QAction* revertAction = new QAction(window);
    revertAction->setText("Revert To Saved");
    QObject::connect(revertAction,SIGNAL(triggered()),handler,SLOT(revert()));

    QAction* exitAction = new QAction(window);
    exitAction->setText("Exit");
    QObject::connect(exitAction,SIGNAL(triggered()),handler,SLOT(exit()));

    fileMenu->addAction(newAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(saveAsAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(renderAction);
    fileMenu->addSeparator();
    fileMenu->addAction(propertiesAcction);
    fileMenu->addSeparator();
    fileMenu->addAction(revertAction);
    fileMenu->addAction(exitAction);
}

}
