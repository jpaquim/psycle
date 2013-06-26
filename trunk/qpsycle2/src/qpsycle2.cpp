#include "qpsycle2.h"
#include "menus/filemenu.h"
#include "menus/editmenu.h"
#include "menus/menusignalhandler.h"
#include "menus/toolbar.h"
#include "MachineView/machineview.h"
#include "statics.h"
#include "PatternView/patternview.h"


#include "psycle/core/player.h"
#include "psycle/core/machinefactory.h"

#include <QtGui>
#include <QTabWidget>

namespace qpsycle{
qpsycle2::qpsycle2(QWidget *parent)
    : QMainWindow(parent)
{
    handler = new MenuSignalHandler(this);
    Statics::Setup();
    new FileMenu(this);
    new EditMenu(this);
    ToolBar* toolBar = new ToolBar(handler);
    this->addToolBar(toolBar);
    QTabWidget* tabs = new QTabWidget(this);

    MachineView* mach = new MachineView();
    QGraphicsView* machineView = new QGraphicsView(mach);
    PatternView* pattern = new PatternView();
    QGraphicsView* patternView = new QGraphicsView(pattern);

    tabs->addTab(machineView,"Machine View");
    tabs->addTab(patternView, "Pattern View");
    Statics::setMachineView(mach);
    Statics::setPatternView(pattern);
    this->setCentralWidget(tabs);
    machineView->adjustSize();
    this->setWindowTitle("QPsycle");

    connect(handler, SIGNAL(sigReload()),mach,SLOT(setup()));
    connect(handler,SIGNAL(sigReload()),pattern,SLOT(reload()));
}

qpsycle2::~qpsycle2()
{
    
}

MenuSignalHandler* qpsycle2::getSignalHandler()
{
    return handler;
}

}
