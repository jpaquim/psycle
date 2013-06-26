#ifndef STATICS_H
#define STATICS_H

#include "psycle/core/song.h"
#include "psycle/core/player.h"
#include "psycle/audiodrivers/audiodriver.h"
#include "PatternView/patternview.h"
#include "MachineView/machineview.h"

#include "MachineView/machinethemeloader.h"
#include <QGraphicsView>


namespace qpsycle{

class Statics
{
public:
    Statics(){};
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
    static MachineView* getMachineView();
    static void setMachineView(MachineView *view);
    static PatternView* getPatternView();
    static void setPatternView(PatternView* view);
  
private:
    static psycle::core::Song* song_;
    static MachineThemeLoader* theme_;
    static QString themePath;
    static psycle::core::Player* player_;
    static psycle::audiodrivers::AudioDriver* audioDriver_;
    static QString pluginPath;
    static MachineView* machineView;
    static PatternView* patternView;


};

}
#endif // STATICS_H
