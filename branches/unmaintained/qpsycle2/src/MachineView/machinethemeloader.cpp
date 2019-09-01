#include "MachineView/machinethemeloader.h"
#include <QString>
#include <QFile>
#include <QIODevice>
#include <QTextStream>

#include <iostream>

namespace qpsycle{

MachineThemeLoader::MachineThemeLoader(QString path)
{
    QFile* file = new QFile(path);
    file->open(QIODevice::ReadOnly);
    QTextStream stream(file);
    QString string;
    QStringList list;
    QPixmap tempBitmap(path.replace(".psm",".bmp"));

    while(!stream.atEnd()){
        string=stream.readLine();
        if(!string.isEmpty() && string.at(0)!='/'){
            string.remove('\"');
            string.replace('=',',');
            list = string.split(',',QString::SkipEmptyParts);
            std::string str = list.first().toStdString();
            //do stuff with the input, first create images when needed
            if (str=="master_source"){
                master = tempBitmap.copy(list.at(1).toInt(),list.at(2).toInt(),list.at(3).toInt(),list.at(4).toInt());
            }else if(str=="generator_source"){
                generator = tempBitmap.copy(list.at(1).toInt(),list.at(2).toInt(),list.at(3).toInt(),list.at(4).toInt());
            }else if(str=="generator_vu0_source"){
                generatorVU=tempBitmap.copy(list.at(1).toInt(),list.at(2).toInt(),list.at(3).toInt(),list.at(4).toInt());
            }else if(str=="generator_vu_peak_source"){
                generatorVUPeak=tempBitmap.copy(list.at(1).toInt(),list.at(2).toInt(),list.at(3).toInt(),list.at(4).toInt());
            }else if(str=="generator_pan_source"){
                generatorPan=tempBitmap.copy(list.at(1).toInt(),list.at(2).toInt(),list.at(3).toInt(),list.at(4).toInt());
            }else if(str=="generator_mute_source"){
                generatorMute=tempBitmap.copy(list.at(1).toInt(),list.at(2).toInt(),list.at(3).toInt(),list.at(4).toInt());
            }else if(str=="generator_solo_source"){
                generatorSolo=tempBitmap.copy(list.at(1).toInt(),list.at(2).toInt(),list.at(3).toInt(),list.at(4).toInt());
            }else if(str=="effect_source"){
                effect=tempBitmap.copy(list.at(1).toInt(),list.at(2).toInt(),list.at(3).toInt(),list.at(4).toInt());
            }else if(str=="effect_vu0_source"){
                effectVU=tempBitmap.copy(list.at(1).toInt(),list.at(2).toInt(),list.at(3).toInt(),list.at(4).toInt());
            }else if(str=="effect_vu_peak_source"){
                effectVUPeak=tempBitmap.copy(list.at(1).toInt(),list.at(2).toInt(),list.at(3).toInt(),list.at(4).toInt());
            }else if(str=="effect_pan_source"){
                effectPan=tempBitmap.copy(list.at(1).toInt(),list.at(2).toInt(),list.at(3).toInt(),list.at(4).toInt());
            }else if(str=="effect_mute_source"){
                effectMute=tempBitmap.copy(list.at(1).toInt(),list.at(2).toInt(),list.at(3).toInt(),list.at(4).toInt());
            }else if(str=="effect_bypass_source"){
                effectBypass=tempBitmap.copy(list.at(1).toInt(),list.at(2).toInt(),list.at(3).toInt(),list.at(4).toInt());


            //done with creating images, now with where to put them..
            }else if(str=="generator_vu_dest"){

            }else if(str=="generator_pan_dest"){

            }else if(str=="generator_mute_dest"){
                generatorMuteDest = QPoint(list.at(1).toInt(),list.at(2).toInt());
            }else if(str=="generator_solo_dest"){
                generatorSoloDest = QPoint(list.at(1).toInt(),list.at(2).toInt());
            }else if(str=="generator_name_dest"){
                generatorNameDest = QPoint(list.at(1).toInt(),list.at(2).toInt());
            }else if(str=="effect_vu_dest"){

            }else if(str=="effect_pan_dest"){

            }else if(str=="effect_mute_dest"){
                effectMuteDest = QPoint(list.at(1).toInt(),list.at(2).toInt());
            }else if(str=="effect_bypass_dest"){
                effectBypassDest = QPoint(list.at(1).toInt(),list.at(2).toInt());
            }else if(str=="effect_name_dest"){
                effectNameDest = QPoint(list.at(1).toInt(),list.at(2).toInt());
            }else{
                std::cerr<<"Unknown parameter \""<<str<<"\""<<std::endl;
            }//end input cases

        }
    }//end input loop
}//end constructor

}//namespace qpsycle

