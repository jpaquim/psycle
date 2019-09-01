#include "qpsycle2.h"

#include <QFileDialog>
#include <QGraphicsView>
#include <iostream>

#include "statics.h"
#include "menusignalhandler.h"
#include "MachineView/machineview.h"
#include "psycle/core/player.h"

namespace qpsycle{
MenuSignalHandler::MenuSignalHandler(qpsycle2* window)
{
    qpsy = window;
}
void MenuSignalHandler::newProject(){
    Globals::song()->clear();
    std::cerr<<"cleared";
    emit sigReload();
}

void MenuSignalHandler::save(){
    Globals::song()->save(lastfile.toStdString());
}

void MenuSignalHandler::open(){
    QString str = QFileDialog::getOpenFileName(qpsy,"Open","~/","Psycle Projects (*.psy)");
    std::cerr<<"loading "<<str.toStdString()<<std::endl;
    Globals::player()->stop();
    Globals::song()->load(str.toStdString());
    lastfile = str;
    emit sigReload();
    std::cerr<<"loaded "<<str.toStdString()<<std::endl;
}

void MenuSignalHandler::saveAs(){
    QString str = QFileDialog::getSaveFileName(qpsy,"Save As","~/","Psycle Projects (*.psy)");
    Globals::song()->save(str.toStdString(),3);
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
    Globals::audioDriver()->set_started(true);
    Globals::audioDriver()->setPlaybackSettings(settings);
    Globals::player()->start(0);
}

void MenuSignalHandler::play(){
    Globals::audioDriver()->set_started(true);
    Globals::player()->start(Globals::player()->playPos());
}

void MenuSignalHandler::stop(){
    Globals::player()->stop();
//    Globals::audioDriver()->set_started(false);
    settings = Globals::audioDriver()->playbackSettings();
    playpos = Globals::player()->playPos();
}
}
