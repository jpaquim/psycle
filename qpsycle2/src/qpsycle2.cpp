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
#include <QHBoxLayout>
#include <QSplitter>

#include "patternmanager.h"

namespace qpsycle{
qpsycle2::qpsycle2(QWidget *parent)
    : QMainWindow(parent)
{
    handler = new MenuSignalHandler(this);
    Globals::Setup();
    new FileMenu(this);
    new EditMenu(this);
    ToolBar* toolBar = new ToolBar(handler);
    this->addToolBar(toolBar);

    PatternManager* pm = new PatternManager();

    QTabWidget* tabs = new QTabWidget();

    MachineView* mach = new MachineView();
    QGraphicsView* machineView = new QGraphicsView(mach);
    PatternView* pattern = new PatternView();

    connect( pm, &PatternManager::patternSelected, pattern, &PatternView::showPattern);

    tabs->addTab(machineView,"Machine View");
    tabs->addTab(pattern, "Pattern View");
    this->setWindowTitle("QPsycle");


    this->setCentralWidget(tabs);
    this->addDockWidget(Qt::LeftDockWidgetArea, pm);

    connect(handler, &MenuSignalHandler::sigReload, mach, &MachineView::setup );
    connect(handler, &MenuSignalHandler::sigReload, pattern, &PatternView::reload );
    connect(handler, &MenuSignalHandler::sigReload, pm, &PatternManager::patternsChanged );
}

qpsycle2::~qpsycle2()
{
    
}

MenuSignalHandler* qpsycle2::getSignalHandler()
{
    return handler;
}

}
