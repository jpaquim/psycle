#ifndef MACHINEVIEW_H
#define MACHINEVIEW_H

#include <QGraphicsView>
#include <QMap>

#include "MachineView/machineviewguielement.h"
#include "MachineView/guimachineelement.h"


#include "psycle/core/machine.h"

/*
 *The machine veiw itself.  Is mostly just a scene, doesn't do much itself.
 */
namespace qpsycle{


class MachineView  : public QGraphicsScene
{
Q_OBJECT

public:
    MachineView();

public Q_SLOTS:
    void setup();

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

private:
    QMap<psycle::core::Machine*,GuiMachineElement*> items;
};

}

#endif // MACHINEVIEW_H
