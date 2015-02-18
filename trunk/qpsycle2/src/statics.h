#ifndef STATICS_H
#define STATICS_H

#include <QString>

namespace psycle{
namespace core {
class Song;
class Player;
}
namespace audiodrivers{
class AudioDriver;
}

}
class QGraphicsView;

namespace qpsycle{
class MachineThemeLoader;

class Globals
{
public:
    Globals(){}
    static void Setup();
    static psycle::core::Song* song();
    static void newSong();
    static MachineThemeLoader* theme();
    static void loadTheme(QString path);
    static psycle::audiodrivers::AudioDriver* audioDriver();
    static void setAuidoDriver(psycle::audiodrivers::AudioDriver* driver);
    static psycle::core::Player* player();
    static void settingsReader();
    static QString getPluginPath();

private:
    static psycle::core::Song* song_;
    static MachineThemeLoader* theme_;
    static QString themePath;
    static psycle::core::Player* player_;
    static psycle::audiodrivers::AudioDriver* audioDriver_;
    static QString pluginPath;


};

}
#endif // STATICS_H
