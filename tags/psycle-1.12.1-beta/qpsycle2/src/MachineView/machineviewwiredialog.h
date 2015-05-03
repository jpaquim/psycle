#ifndef MACHINEVIEWWIREDIALOG_H
#define MACHINEVIEWWIREDIALOG_H

#include <MachineView/machineviewline.h>
#include <QDialog>

namespace qpsycle{

class MachineViewWireDialog : public QDialog
{
    Q_OBJECT

public:
    MachineViewWireDialog(MachineViewLine* line);

public slots:
    void updateVolume(int i);
    void deleteWire();

private:
    MachineViewLine* wire;
};

}
#endif // MACHINEVIEWWIREDIALOG_H
