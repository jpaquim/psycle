#include "statics.h"
#include "psycle/core/machinefactory.h"
#include "psycle/audiodrivers/gstreamerout.h"
#include "psycle/audiodrivers/jackout.h"
#include "psycle/audiodrivers/alsaout.h"
#include "universalis/os/fs.hpp"

#include <QFile>
#include <QTextStream>

namespace qpsycle{

void Statics::Setup(){

    //read in settings
    settingsReader();


    theme_ = new MachineThemeLoader(themePath);
    player_ = &(psycle::core::Player::singleton());
    psycle::core::MachineFactory::getInstance().Initialize(player_);
    psycle::core::MachineFactory::getInstance().setPsyclePath(pluginPath.toStdString());
    psycle::core::MachineFactory::getInstance().setLadspaPath("/usr/lib/ladspa/");
    newSong();
    player_->song(*Statics::song_);
    audioDriver_ = new psycle::audiodrivers::GStreamerOut();
    player_->setDriver(*audioDriver_);
    audioDriver()->set_started(true);

}

void Statics::loadTheme(QString path){
    theme_ = new MachineThemeLoader(path);
}

MachineThemeLoader* Statics::theme(){
    return theme_;
}

void Statics::newSong(){
    song_ = new psycle::core::Song();
}

psycle::core::Song* Statics::song(){
   if (song_==0)
        newSong();
   return song_;
}

psycle::core::Player* Statics::player(){
    return player_;
}

psycle::audiodrivers::AudioDriver* Statics::audioDriver(){
    return audioDriver_;
}

QString Statics::getPluginPath(){
    return pluginPath;
}

void Statics::settingsReader(){
    //open settings file
    QString fileString = QString::fromStdString(universalis::os::fs::home_app_local("qpsycle").string()+"/config");
    std::cerr<<"configFile: "<<fileString.toStdString()<<std::endl;
    QFile* file = new QFile(fileString);
    file->open(QIODevice::ReadOnly);
    QTextStream stream(file);
    QString string = *new QString();

    //read settings file
    while(!stream.atEnd()){
        string=stream.readLine();
        if (!string.isEmpty()){
            if(string.left(5)==QString("Theme"))
                themePath=string.mid(6);
            if(string.left(11)==QString("Plugin_Path"))
                pluginPath=string.mid(12);
        }
    }
    std::cerr<<"Plugin path:"<<pluginPath.toStdString()<<std::endl;


}

MachineView* Statics::getMachineView() {
    return machineView;
}

void Statics::setMachineView(MachineView* view){
    machineView = view;
}

PatternView* Statics::getPatternView() {
    return patternView;
}

void Statics::setPatternView(PatternView* view){
    patternView = view;
}



psycle::core::Song* Statics::song_ = 0 ;
MachineThemeLoader* Statics::theme_;
QString Statics::themePath;
QString Statics::pluginPath;
psycle::core::Player* Statics::player_;
psycle::audiodrivers::AudioDriver* Statics::audioDriver_;
MachineView* Statics::machineView = 0;
PatternView* Statics::patternView = 0;

}
