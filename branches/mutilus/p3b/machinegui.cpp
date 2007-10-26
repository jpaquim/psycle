
#include <psycle/core/machine.h>
#include <psycle/core/song.h>

#include "machinegui.h"
#include "machineview.h"

#include <QLabel>
#include <QGridLayout>
#include <QCheckBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QAction>

#include <stdlib.h>

MachineGui::MachineGui( psy::core::Machine *mac, MachineView *macView, QWidget *parent )
    : QWidget( parent ),
      m_mac( mac ),
      m_macView( macView )
{
    QHBoxLayout *layout = new QHBoxLayout();
    setLayout( layout );

    QLabel *text = new QLabel("hear");
    checkBox = new QCheckBox( this );
    m_saveButton = new QPushButton();
    m_loadButton = new QPushButton();
    m_saveAction = new QAction( QIcon( ":images/save.png" ), "save", macView );
    m_saveButton->addAction( m_saveAction );
    connect( m_saveAction, SIGNAL ( triggered() ),
	     this, SLOT ( savePreset() ) );
    m_loadAction = new QAction( "load", m_loadButton );
    

    layout->addWidget( text );
    layout->addWidget( checkBox );
    layout->addWidget( m_saveButton );
    layout->addWidget( m_loadButton );
    randomiseParameters();
}

void MachineGui::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() == Qt::LeftButton )
    {
        m_macView->song()->seqBus = m_mac->id();
        m_macView->PlayNote( 4, 127, true, m_mac );
    } else if ( e->button() == Qt::RightButton ) 
    {
        randomiseParameters();
    } else if ( e->button() == Qt::RightButton ) 
    {
        resetParameters();
    }
}

void MachineGui::mouseReleaseEvent( QMouseEvent *e )
{
    if ( e->button() == Qt::LeftButton )
    {
        qDebug("stop");
        m_macView->StopNote( 4 );
    }
}

void MachineGui::randomiseParameters()
{
    int numpars = m_mac->GetNumParams();
    for (int c=0; c<numpars; c++)
    {
        int minran,maxran;
        m_mac->GetParamRange(c,minran,maxran);
        int range=(maxran-minran)+1; 
        float random = minran+(range*(rand()/(RAND_MAX + 1.0))); 

        m_mac->SetParameter(c,random);
    }
}

void MachineGui::resetParameters() 
{
/*    int numpars = m_mac->GetNumParams();
    for (int c=0; c<numpars; c++)
    {
        int dv = ((psy::core::Plugin*)m_mac)->GetInfo()->Parameters[c]->DefValue;
        m_mac->SetParameter(c,dv);
    }*/
}

bool MachineGui::isSelected()
{
    if ( checkBox->checkState() == Qt::Checked )
        return true;
    return false;
}

void MachineGui::savePreset() 
{
  qDebug( "yes hello sire" );
}
