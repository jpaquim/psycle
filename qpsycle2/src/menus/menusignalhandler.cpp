#include "menusignalhandler.h"
#include <QFileDialog>
#include "qpsycle2.h"
#include <iostream>
#include "statics.h"
#include "MachineView/machineview.h"
#include "QGraphicsView"

namespace qpsycle{
MenuSignalHandler::MenuSignalHandler(qpsycle2* window)
{
    qpsy = window;
}
void MenuSignalHandler::newProject(){
    Statics::song()->clear();
    std::cerr<<"cleared";
    Q_EMIT sigReload();
}

void MenuSignalHandler::save(){
    Statics::song()->save(lastfile.toStdString());
}

void MenuSignalHandler::open(){
    QString str = QFileDialog::getOpenFileName(qpsy,"Open","~/","Psycle Projects (*.psy)");
    std::cerr<<"loading "<<str.toStdString()<<std::endl;
    Statics::player()->stop();
    Statics::song()->load(str.toStdString());
    lastfile = str;
    Q_EMIT sigReload();
    std::cerr<<"loaded "<<str.toStdString()<<std::endl;
}

void MenuSignalHandler::saveAs(){
    QString str = QFileDialog::getSaveFileName(qpsy,"Save As","~/","Psycle Projects (*.psy)");
    Statics::song()->save(str.toStdString(),3);
    lastfile  = str;
    std::cerr<<"saving as "<<str.toStdString()<<std::endl;
}

void MenuSignalHandler::render(){

}

void MenuSignalHandler::properties(){

}

void MenuSignalHandler::revert(){

}

void MenuSignalHandler::exit(){
    qpsy->close();
}

void MenuSignalHandler::undo(){

}

void MenuSignalHandler::redo(){

}

void MenuSignalHandler::patternCut(){

}

void MenuSignalHandler::patternCopy(){

}

void MenuSignalHandler::patternPaste(){

}

void MenuSignalHandler::patternMixPaste(){

}

void MenuSignalHandler::setRecord(bool /*b*/){

}

void MenuSignalHandler::playFromBeginning(){
    Statics::audioDriver()->set_started(true);
    Statics::player()->start(0);
}

void MenuSignalHandler::play(){
    Statics::audioDriver()->set_started(true);
    Statics::player()->start(Statics::player()->playPos());
}

void MenuSignalHandler::stop(){
    Statics::player()->stop();
//    Statics::audioDriver()->set_started(false);
//    settings = Statics::audioDriver()->playbackSettings();
    playpos = Statics::player()->playPos();
}
}
