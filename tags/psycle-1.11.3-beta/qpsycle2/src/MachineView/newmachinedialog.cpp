#include "newmachinedialog.h"
#include "psycle/core/pluginfinder.h"
#include "psycle/core/machinefactory.h"

#include <QGridLayout>
#include <QListWidget>
#include <QDialogButtonBox>

namespace qpsycle {

NewMachineDialog::NewMachineDialog()
{

    this->setLayout(new QGridLayout(this));
    this->setWindowTitle("New Machine");
    //this->resize(500,500);
    psycle::core::PluginFinder finder = psycle::core::MachineFactory::getInstance().getFinder();

    controllerList =  new QListWidget;
    generatorList = new QListWidget;
    effectList = new QListWidget;
    internalList = new QListWidget;
    ladspaList = new QListWidget;
    nativeList = new QListWidget;
    vstList = new QListWidget;

    //iterate through the machines
    int i = psycle::core::Hosts::INTERNAL;
    while(finder.hasHost(psycle::core::Hosts::type(i))){//for each type of machine
        std::map < psycle::core::MachineKey, psycle::core::PluginInfo >::const_iterator it = finder.begin(psycle::core::Hosts::type(i));
        while(it!=finder.end(psycle::core::Hosts::type(i))){

            //grab info and record this machine for future use.
            const psycle::core::PluginInfo & info = it->second;
            const psycle::core::MachineKey & key = it->first;
            QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(info.name()));
            pluginIdentityMap.insert(item,key);

            //create lists based on the type of machine
            switch(key.host()){
            case psycle::core::Hosts::INTERNAL:
                internalList->addItem(item);
                break;
            case psycle::core::Hosts::NATIVE:
                nativeList->addItem(item);
                break;
            case psycle::core::Hosts::LADSPA:
                ladspaList->addItem(item);
                break;
            case psycle::core::Hosts::VST:
                vstList->addItem(item);
                break;
            default:
                break;
            };


            item = new QListWidgetItem(QString::fromStdString(info.name()));
            pluginIdentityMap.insert(item,key);
            //create lists based on the role of the machine
            switch(info.role()){
            case psycle::core::MachineRole::CONTROLLER:
                controllerList->addItem(item);
                break;
            case psycle::core::MachineRole::EFFECT:
                effectList->addItem(item);
                break;
            case psycle::core::MachineRole::GENERATOR:
                generatorList->addItem(item);
                break;
            default:
                break;
            };
            it++;
        }
        i++;
    }//end iteration


    QTabWidget * machineTabs = new QTabWidget(this);
    QListWidget* lists[7] = {controllerList, generatorList, effectList, internalList, ladspaList, nativeList, vstList};
    for(int i=0;i<7;i++){
        connect( lists[i], SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(listItemSelected(QListWidgetItem*) ) );

    }
    machineTabs->addTab(controllerList,"Controllers");
    machineTabs->addTab(generatorList,"Generators");
    machineTabs->addTab(effectList,"Effects");
    machineTabs->addTab(internalList,"Internal");
    machineTabs->addTab(ladspaList,"LADSPA");
    machineTabs->addTab(nativeList,"Native");
    machineTabs->addTab(vstList,"VST");

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                    | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(tryAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));


    this->layout()->addWidget(machineTabs);
    this->layout()->addWidget(buttonBox);

}

void NewMachineDialog::listItemSelected(QListWidgetItem* listItem){
    activeListItem = listItem;
}

void NewMachineDialog::tryAccept(){
    if (activeListItem!=0){
        accept();
    }

}

psycle::core::MachineKey NewMachineDialog::getSelection(){
    return(pluginIdentityMap.find(activeListItem).value());
}
}
