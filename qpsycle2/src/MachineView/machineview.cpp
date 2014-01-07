#include "MachineView/machineview.h"
#include "MachineView/guimachineelement.h"
#include "MachineView/machinethemeloader.h"
#include "MachineView/generatorguielement.h"
#include "MachineView/mixerguielement.h"
#include "MachineView/newmachinedialog.h"
#include "MachineView/effectguielement.h"
#include "statics.h"

#include "psycle/core/machinefactory.h"
#include "psycle/core/internalkeys.hpp"
#include "psycle/core/song.h"
#include "psycle/core/machine.h"
#include "psycle/core/constants.h"

#include <QPoint>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include "../statics.h"
namespace qpsycle{

/*
 *Create a new workspace, nothing in it.
 */
MachineView::MachineView()
{
    //create Gui elements for all of the machines
  
    setup();
}

void MachineView::setup()
{
    clear();
    items.clear();
    psycle::core::Song* song = Statics::song();
    psycle::core::Machine* machine;
    psycle::core::MachineKey key;
    psycle::core::PluginInfo info;
    GuiMachineElement* item;

    for(psycle::core::Machine::id_type c(0); c < psycle::core::MAX_MACHINES; ++c) if(song->machine(c)) {
        machine = song->machine(c);
        key = machine->getMachineKey();
        info = psycle::core::MachineFactory::getInstance().getFinder().info(key);
        if(info.role()==psycle::core::MachineRole::CONTROLLER ||info.role()==psycle::core::MachineRole::GENERATOR){
            item = new GeneratorGuiElement(QPoint(machine->GetPosX(),machine->GetPosY()),machine );
            this->addItem(item);
            items.insert(machine,item);
        }else if( info.role()==psycle::core::MachineRole::EFFECT){
            item = new EffectGuiElement(QPoint(machine->GetPosX(),machine->GetPosY()),machine);
            this->addItem(item);
            items.insert(machine,item);
        }else if(key.index()==0){ //this means it is the master, comparing machineRole didn't work for this one.
            item = new MixerGuiElement(QPoint(machine->GetPosX(),machine->GetPosY()),machine);
            this->addItem(item);
            items.insert(machine,item);
        }else{
            std::cerr<<"this point should never be reached.  Unknown machine role in MachineView()\n";
        }
    }


    //draw in the connections.
    MachineViewLine* line;
    foreach(psycle::core::Machine* mac, items.keys()){
        for ( int w=0; w < psycle::core::MAX_CONNECTIONS; w++ ) {
                if (mac->_connection[w]) { // a connection exists
                    GuiMachineElement* srcMacGui = items.value(mac);  //find the gui for the generating machine
                        if ( srcMacGui!=0 ) {
                            psycle::core::Machine *pout = Statics::song()->machine(mac->_outputMachines[w]); //find the receiving machine
                            GuiMachineElement* dstMacGui = items.value(pout);  //and it's gui.
                            if ( dstMacGui != 0 ) {
                                line = new MachineViewLine(0,0,0,0);//add in the line.
                                line->setWire(mac->FindOutputWire(mac->_outputMachines[w]));
                                this->addItem(line);
                                srcMacGui->addOutLineInitial(line);
                                dstMacGui->addInLineInitial(line);
                            }
                        }
                    }
                 }

    }




}

/*
 *Deal with double clicks to add new machines.
 */
void MachineView::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event){
    if(itemAt(event->scenePos(),QTransform())!=0) //click on the ojbect, not the scene
       QGraphicsScene::mouseDoubleClickEvent(event);
    else{
        NewMachineDialog* di = new NewMachineDialog();
        if(di->exec()==QDialog::Accepted){ //we actually got a machine to add

            //figure out what machine to get.
            psycle::core::MachineKey key = di->getSelection();
            psycle::core::PluginInfo info = psycle::core::MachineFactory::getInstance().getFinder().info(key);
            psycle::core::Machine* machine = psycle::core::MachineFactory::getInstance().CreateMachine(key);
            Statics::song()->AddMachine(machine);

            //create the UI elememt
            if(info.role()==psycle::core::MachineRole::GENERATOR||info.role()==psycle::core::MachineRole::CONTROLLER){
               this->addItem( new GeneratorGuiElement(event->scenePos().toPoint(),machine));
            }else{
               this->addItem(new EffectGuiElement(event->scenePos().toPoint(),machine));
            }
        }

    }
}


}
