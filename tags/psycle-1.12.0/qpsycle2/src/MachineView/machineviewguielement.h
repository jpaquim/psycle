#ifndef MACHINEVIEWGUIELEMENT_H
#define MACHINEVIEWGUIELEMENT_H

/*
 * An interface for all of the elements in the machine veiw.
 */

namespace qpsycle{

class MachineViewLine;

class MachineViewGuiElement
{
public:
    MachineViewGuiElement(){}
    virtual bool canAcceptConnections()const =0;
    virtual void addInLine(MachineViewLine* line) =0;
    virtual ~MachineViewGuiElement(){}

};
}
#endif // MACHINEVIEWGUIELEMENT_H
