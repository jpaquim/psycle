#ifndef MACHINETHEMELOADER_H
#define MACHINETHEMELOADER_H

#include <QString>
#include <QPixmap>
#include <QPoint>

/*
 *Loads in the pixmaps from a psycle theme  (not sure it is complete, just grabbed the
 *format from the default one.
 */
namespace qpsycle{
class MachineThemeLoader
{
public:
    MachineThemeLoader(QString path);
    QPixmap generator;
    QPixmap effect;
    QPixmap master;

    //generator
    QPixmap generatorVU;
    QPixmap generatorVUPeak;
    QPixmap generatorPan;
    QPixmap generatorMute;
    QPixmap generatorSolo;
    int generatorVuLen;
    int generatorPanLen;
    QPoint generatorVuDest;
    QPoint generatorPanDest;
    QPoint generatorMuteDest;
    QPoint generatorSoloDest;
    QPoint generatorNameDest;

    //effect
    QPixmap effectVU;
    QPixmap effectVUPeak;
    QPixmap effectPan;
    QPixmap effectMute;
    QPixmap effectBypass;
    int effectVuLen;
    int effectPanLen;
    QPoint effectVuDest;
    QPoint effectPanDest;
    QPoint effectMuteDest;
    QPoint effectBypassDest;
    QPoint effectNameDest;

};

}

#endif // MACHINETHEMELOADER_H
