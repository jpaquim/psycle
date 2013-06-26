#ifndef MENUSIGNALHANDLER_H
#define MENUSIGNALHANDLER_H

#include <QObject>
#include "psycle/audiodrivers/audiodriver.h"

namespace qpsycle{
class qpsycle2;
class MenuSignalHandler: public QObject
{
Q_OBJECT
public:
    MenuSignalHandler(qpsycle2* window);

public Q_SLOTS:
    void newProject();
    void save();
    void open();
    void saveAs();
    void render();
    void properties();
    void revert();
    void exit();
    void undo();
    void redo();
    void patternCut();
    void patternCopy();
    void patternPaste();
    void patternMixPaste();
    void setRecord(bool b);
    void playFromBeginning();
    void play();
    void stop();

Q_SIGNALS:
    void sigReload();

private:
    qpsycle2 * qpsy;
    double playpos;
    QString lastfile;
    psycle::audiodrivers::AudioDriverSettings settings;

};
}

#endif // MENUSIGNALHANDLER_H
