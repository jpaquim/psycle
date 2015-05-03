#include <QDebug>
#include "toolbar.h"
#include "qpsycle2.h"
#include <QToolBar>
#include <QToolButton>
#include <menus/menusignalhandler.h>
namespace qpsycle{
ToolBar::ToolBar(MenuSignalHandler* handler): QToolBar("Toolbar")
{
    QToolButton* newButton = new QToolButton();
    newButton->setIcon(QIcon::fromTheme("document-new"));
    newButton->setText("New");
    connect(newButton,SIGNAL(clicked()),handler,SLOT(newProject()));

    QToolButton* openButton = new QToolButton();
    openButton->setIcon(QIcon::fromTheme("document-open"));
    openButton->setText("Open");
    connect(openButton,SIGNAL(clicked()),handler,SLOT(open()));

    QToolButton* saveButton = new QToolButton();
    saveButton->setIcon(QIcon::fromTheme("document-save"));
    saveButton->setText("Save");
    connect(saveButton,SIGNAL(clicked()),handler,SLOT(save()));

    QToolButton* saveWavButton = new QToolButton();
    saveWavButton->setIcon(QIcon::fromTheme("audio-x-generic"));
    saveWavButton->setText("Render Wav");
    connect(saveWavButton,SIGNAL(clicked()),handler,SLOT(render()));

    QToolButton* undoButton = new QToolButton();
    undoButton->setIcon(QIcon::fromTheme("edit-undo"));
    undoButton->setText("Undo");
    connect(undoButton,SIGNAL(clicked()),handler,SLOT(undo()));

    QToolButton* redoButton = new QToolButton();
    redoButton->setIcon(QIcon::fromTheme("edit-redo"));
    redoButton->setText("Redo");
    connect(redoButton,SIGNAL(clicked()),handler,SLOT(redo()));

    QToolButton* recordButton = new QToolButton();
    recordButton->setIcon(QIcon::fromTheme("media-record"));
    recordButton->setText("Record");
    recordButton->setCheckable(true);
    connect(recordButton,SIGNAL(clicked(bool)),handler,SLOT(setRecord(bool)));

    QToolButton* playBeginningButton = new QToolButton;
    playBeginningButton->setIcon(QIcon::fromTheme("media-seek-forward"));
    playBeginningButton->setText("Play From Beginning");
    connect(playBeginningButton,SIGNAL(clicked()),handler,SLOT(playFromBeginning()));

    QToolButton* playButton = new QToolButton();
    playButton->setIcon(QIcon::fromTheme("media-playback-start"));
    playButton->setText("Play");
    connect(playButton,SIGNAL(clicked()),handler,SLOT(play()));

    QToolButton* stopButton = new QToolButton();
    stopButton->setIcon(QIcon::fromTheme("media-playback-stop"));
    stopButton->setText("Stop");
    connect(stopButton,SIGNAL(clicked()),handler,SLOT(stop()));

    this->addWidget(newButton);
    this->addWidget(openButton);
    this->addWidget(saveButton);
    this->addWidget(saveWavButton);
    this->addSeparator();
    this->addWidget(undoButton);
    this->addWidget(redoButton);
    this->addWidget(recordButton);
    this->addSeparator();
    this->addWidget(playBeginningButton);
    this->addWidget(playButton);
    this->addWidget(stopButton);
}
}
