/***************************************************************************
*   Copyright (C) 2006 by  Neil Mather   *
*   nmather@sourceforge   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#include <QGraphicsScene>
#include <QPainter>
#include <iostream>
#include <vector>

 #include "sequencerview.h"
 #include "sequencerdraw.h"
 #include "sequencerline.h"
 #include "sequenceritem.h"
 #include "sequencerarea.h"


 SequencerDraw::SequencerDraw( SequencerView *seqView )
 {
    seqView_ = seqView;
    beatPxLength_ = 5;
    int lineHeight_ = 30;

    QGraphicsScene *scene_ = new QGraphicsScene(this);
    scene_->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene_->setBackgroundBrush(Qt::black);

    setAlignment ( Qt::AlignLeft | Qt::AlignTop );
    setScene(scene_);
    setSceneRect( 0,0, width(), height() );

    seqArea_ = new SequencerArea( this );

    bool isFirst = true;
    int count = 0;
    std::vector<psy::core::SequenceLine*>::iterator it = sequencerView()->song()->patternSequence()->begin();
    for ( ; it < sequencerView()->song()->patternSequence()->end(); it++) {
        psy::core::SequenceLine* seqLine = *it;
        SequencerLine* line = new SequencerLine();
        line->setParentItem( seqArea_ );
        if (isFirst) {
            sequencerView()->setSelectedLine( line );
            isFirst = false;
        }
        //line->itemClick.connect(this, &SequencerGUI::onSequencerItemClick);
 //       lines.push_back(line);
        line->setSequenceLine( seqLine );
        line->setPos( 0, count*lineHeight_ );
//        line->click.connect(this, &SequencerGUI::onSequencerLineClick);
//        scrollArea_->resize();
//        lastLine_ = line;
 //       selectedLine_ = line;
        // now iterate the sequence entries
        psy::core::SequenceLine::iterator iter = seqLine->begin();
        for(; iter!= seqLine->end(); ++iter)
        {
            psy::core::SequenceEntry* entry = iter->second;
            SequencerItem* item = new SequencerItem();
            item->setParentItem( line );
//            item->click.connect(line,&SequencerGUI::SequencerLine::onSequencerItemClick);
            item->setSequenceEntry( entry );
//            line->items.push_back(item);

        }
        count++;
   }
    scene_->addItem( seqArea_ );
//    lines.clear();
//    scrollArea_->removeChilds();

 }

int SequencerDraw::beatPxLength( ) const
{
    return beatPxLength_;
}

SequencerLine* SequencerDraw::selectedLine() 
{
    return selectedLine_;
}

void SequencerDraw::addPattern( psy::core::SinglePattern *pattern )
{
    if ( selectedLine() ) {
        selectedLine()->addItem( pattern );
    }
}
