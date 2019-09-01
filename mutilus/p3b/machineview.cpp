
#include <psycle/core/pluginfinder.h>
#include <psycle/core/song.h>
#include <psycle/core/patterndata.h>
#include <psycle/core/machine.h>

#include "machineview.h"
#include "machinegui.h"

#include <QGridLayout>
#include <QSettings>

#include <iostream>

MachineView::MachineView( psy::core::Song *song, QWidget *parent )
    : QWidget( parent ),
      m_song( song )
{
    QGridLayout *layout = new QGridLayout();
    setLayout( layout );

    QSettings settings;
    std::string psyclePluginsPath = settings.value( "plugins/psyclePath" ).toString().toStdString();
    std::string ladspaPath = settings.value( "plugins/ladspaPath" ).toString().toStdString();
    std::string pluginLibName = settings.value( "mainPlugin/libName" ).toString().toStdString();

    psy::core::PluginFinder finder_( psyclePluginsPath, ladspaPath );

    psy::core::PluginFinderKey key;
    std::map< psy::core::PluginFinderKey, psy::core::PluginInfo >::const_iterator it = finder_.begin();
    for ( ; it != finder_.end(); it++ ) {
        key = it->first;
        const psy::core::PluginInfo & info = it->second;
        if ( info.type() == psy::core::MACH_PLUGIN ) {
           if ( info.mode() == psy::core::MACHMODE_GENERATOR ) {
                std::cout << info.libName() << std::endl;
               if ( info.libName() == pluginLibName ) {
                   break;
               }
            }
        }
    }

    for ( int i = 0; i < 4; i++ )
    {
        psy::core::Machine *mac = m_song->createMachine( finder_, key, 0, 0 );
        if ( mac ) {
            MachineGui *macGui = new MachineGui( mac, this, this );
            m_song->InsertConnection( mac->id() , psy::core::MASTER_INDEX, 1.0f);
            m_macGuiList << macGui;
        }
    }

    layout->addWidget( m_macGuiList[0], 0, 0, 1, 1 );
    layout->addWidget( m_macGuiList[1], 0, 1, 1, 2 );
    layout->addWidget( m_macGuiList[2], 1, 0, 2, 1 );
    layout->addWidget( m_macGuiList[3], 1, 1, 2, 2 );

    outtrack = 0;
    notetrack[psy::core::MAX_TRACKS];
    for ( int i=0; i<psy::core::MAX_TRACKS; i++ ) notetrack[i]=120;

}

void MachineView::PlayNote( int note,int velocity,bool bTranspose, psy::core::Machine *pMachine )
{
    qDebug("play");
    // stop any notes with the same value
    StopNote(note,bTranspose,pMachine);

    if(note<0) return;

    // octave offset
    if(note<120) {
        if(bTranspose)
            note+= octave()*12;
        if (note > 119)
            note = 119;
    }

    // build entry
    psy::core::PatternEvent entry;
    entry.setNote( note );
    entry.setInstrument( song()->auxcolSelected );
//    entry.setMachine( song()->seqBus );	// Not really needed.

    entry.setCommand( 0 );
    entry.setParameter( 0 );

    // play it
    if(pMachine==NULL)
    {
        int mgn = song()->seqBus;

        if (mgn < psy::core::MAX_MACHINES) {
		pMachine = song()->machine( mgn );
        }
    }

    if (pMachine) {
        // pick a track to play it on	
        //        if(bMultiKey)
        {
            int i;
            for (i = outtrack+1; i < song()->tracks(); i++)
            {
                if (notetrack[i] == 120) {
                    break;
                }
            }
            if (i >= song()->tracks()) {
                for (i = 0; i <= outtrack; i++) {
                    if (notetrack[i] == 120) {
                        break;
                    }
                }
            }
            outtrack = i;
        }// else  {
        //   outtrack=0;
        //}
        // this should check to see if a note is playing on that track
        if (notetrack[outtrack] < 120) {
            StopNote(notetrack[outtrack], bTranspose, pMachine);
        }

        // play
        notetrack[outtrack]=note;
        pMachine->Tick(outtrack, entry );
    }
}

void MachineView::StopNote( int note, bool bTranspose, psy::core::Machine * pMachine )
{
    if (!(note >=0 && note < 128)) return;

    // octave offset
    if(note<120) {
        if(bTranspose) note+= octave()*12;
        if (note > 119) note = 119;
    }

    if(pMachine==NULL) {
        int mgn = song()->seqBus;

        if (mgn < psy::core::MAX_MACHINES) {
		pMachine = song()->machine( mgn );
        }

        for(int i=0; i<song()->tracks(); i++) {
            if(notetrack[i]==note) {
                notetrack[i]=120;
                // build entry
                psy::core::PatternEvent entry;
                entry.setNote( 120+0 );
                entry.setInstrument( song()->auxcolSelected );
                entry.setMachine( song()->seqBus );
                entry.setCommand( 0 );
                entry.setParameter( 0 );

                // play it
                if (pMachine) {
                    pMachine->Tick( i, entry );
                }
            }
        }

    }
}


int MachineView::octave() {
    return 4;
}
