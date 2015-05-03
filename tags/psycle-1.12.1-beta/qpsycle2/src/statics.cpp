#include "statics.h"

#include "psycle/core/song.h"
#include "psycle/core/player.h"

#include "psycle/core/machinefactory.h"
#include "psycle/audiodrivers/gstreamerout.h"
#include "universalis/os/fs.hpp"
#include "psycle/audiodrivers/audiodriver.h"
#include "MachineView/machinethemeloader.h"


#include <QFile>
#include <QTextStream>
#include <QGraphicsView>


namespace qpsycle{

void Globals::Setup(){

    //read in settings
    settingsReader();


    theme_ = new MachineThemeLoader(themePath);
    player_ = &(psycle::core::Player::singleton());
    psycle::core::MachineFactory::getInstance().Initialize(player_);
    psycle::core::MachineFactory::getInstance().setPsyclePath(pluginPath.toStdString());
    psycle::core::MachineFactory::getInstance().setLadspaPath("/usr/lib/ladspa/");
    newSong();
    player_->song(*Globals::song_);
    audioDriver_ = new psycle::audiodrivers::GStreamerOut();
    player_->setDriver(*audioDriver_);
    audioDriver()->set_started(true);
    emit

}

void Globals::loadTheme(QString path){
    theme_ = new MachineThemeLoader(path);
}

MachineThemeLoader* Globals::theme(){
    return theme_;
}

void Globals::newSong(){
    song_ = new psycle::core::Song();
}

psycle::core::Song* Globals::song(){
   if (song_==0)
        newSong();
   return song_;
}

psycle::core::Player* Globals::player(){
    return player_;
}

psycle::audiodrivers::AudioDriver* Globals::audioDriver(){
    return audioDriver_;
}

QString Globals::getPluginPath(){
    return pluginPath;
}

void Globals::settingsReader(){
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




psycle::core::Song* Globals::song_ = nullptr ;
MachineThemeLoader* Globals::theme_ = nullptr;
QString Globals::themePath;
QString Globals::pluginPath;
psycle::core::Player* Globals::player_ = nullptr;
psycle::audiodrivers::AudioDriver* Globals::audioDriver_ = nullptr;

}
