#ifndef NEWMACHINEDIALOG_H
#define NEWMACHINEDIALOG_H

#include <QDialog>
#include <QMap>
#include <QListWidgetItem>
#include "psycle/core/machinekey.hpp"

namespace qpsycle{

class NewMachineDialog : public QDialog
{
    Q_OBJECT
public:
    NewMachineDialog();
    psycle::core::MachineKey getSelection();

public Q_SLOTS:
    void listItemSelected(QListWidgetItem* listItem);
    void tryAccept();

private:
    QMap<QListWidgetItem*,psycle::core::MachineKey> pluginIdentityMap;
    QListWidget* controllerList;
    QListWidget* generatorList;
    QListWidget* effectList;
    QListWidget* internalList;
    QListWidget* ladspaList;
    QListWidget* nativeList;
    QListWidget* vstList;
    QListWidgetItem* activeListItem=0;

};

}
#endif // NEWMACHINEDIALOG_H
