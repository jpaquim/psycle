#include "MachineView/guimachineelement.h"
#include "MachineView/machineview.h"
#include "MachineView/machinethemeloader.h"
#include "MachineView/machineviewguielement.h"
#include "statics.h"

#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>

#include "psycle/core/constants.h"



namespace qpsycle{

GuiMachineElement::GuiMachineElement(){
    setZValue(5);
}


void GuiMachineElement::paint(QPainter */*painter*/, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/){
}

QRectF GuiMachineElement::boundingRect() const{
    return background->boundingRect();
}

void GuiMachineElement::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    if (moving ==false && lineDrawing== true){
    activeLine->setX2(event->scenePos().x());
    activeLine->setY2(event->scenePos().y());
    } else if (moving==true){
        QPointF p = event->pos()-event->lastPos();
        this->moveBy(p.x(),p.y());
        Q_FOREACH(MachineViewLine* line,outlines){
            line->setX1(line->x1()+p.x());
            line->setY1(line->y1()+p.y());
        }
        Q_FOREACH(MachineViewLine* line,inlines){
            line->setY2(line->y2()+p.y());
            line->setX2(line->x2()+p.x());
        }
    }
}

void GuiMachineElement::mousePressEvent(QGraphicsSceneMouseEvent *event){

    if (event->button()==Qt::RightButton){
        moving = false;
        if(canHaveOutputs()){
            activeLine = new MachineViewLine(0,0,event->scenePos().x(),event->scenePos().y());
            activeLine->setSource(this);
            activeLine->setPen(*new QPen(Qt::black));
            this->scene()->addItem(activeLine);
            lineDrawing = true;
        }
    } else if(event->button()==Qt::LeftButton){
        moving = true;
    }
}


void GuiMachineElement::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    if (event->button()==Qt::RightButton&&lineDrawing==true){
        if( this->scene()->itemAt(event->scenePos(),QTransform()) ==0 ||  //nothing here to connect to.
                this->scene()->itemAt(event->scenePos(),QTransform())==activeLine ||
                this->scene()->itemAt(event->pos(),QTransform())==this) {
            this->scene()->removeItem(activeLine);

        } else { //there is a machine here.
            GuiMachineElement* outMachine = ((GuiMachineElement*)(this->scene()->itemAt(event->scenePos(),QTransform())->parentItem()));

            //Try to make the connection
            activeLine->setWire(Globals::song()->InsertConnection(*machine,*outMachine->getMachine()));


            if(activeLine->getWire()>-1){ //we have a proper connection.
                outMachine->addInLine(activeLine);
                outlines.append(activeLine);
                activeLine->setSource(this);
                activeLine->setDest(((GuiMachineElement*)outMachine));
            } else{ //bad connection target
                this->scene()->removeItem(activeLine);
            }
        }
    } else if(event->button()==Qt::LeftButton){
        //moving = 0;
        //lineDrawing = 0;
    }
}

//add a connection (as known about by the gui element.
void GuiMachineElement::addInLine(MachineViewLine* line){
    inlines.append(line);
}

GuiMachineElement::~GuiMachineElement(){
  foreach (MachineViewLine* line, outlines){
    delete line;
  }
}


void GuiMachineElement::mouseDoubleClickEvent(QGraphicsSceneMouseEvent */*event*/){
}

//add a connection (as known about by the gui element.
void GuiMachineElement::addInLineInitial(MachineViewLine* line){
    inlines.append(line);
    line->setDest(this);
}

//add a connection (as known about by the gui element.
void GuiMachineElement::addOutLineInitial(MachineViewLine* line){
    outlines.append(line);
    line->setSource(this);
}



}
