namespace psy { namespace core {
class Song; }}

#include <psycle/core/constants.h>

#include <QWidget>

class MachineGui;

class MachineView : public QWidget {
public:
    MachineView( psy::core::Song *song, QWidget *parent = 0 );
    int octave();
    psy::core::Song *song() { return m_song; }
    void StopNote( int note, bool bTranspose=true, psy::core::Machine* pMachine=NULL);
    void PlayNote( int note,int velocity,bool bTranspose, psy::core::Machine *pMachine );
    QList<MachineGui*> macGuiList() { return m_macGuiList; }

private:
    MachineGui *m_macGui0;
    MachineGui *m_macGui1;
    MachineGui *m_macGui2;
    MachineGui *m_macGui3;

    QList<MachineGui*> m_macGuiList;

    psy::core::Song *m_song;

    /// For multi-key playback state.
    int notetrack[psy::core::MAX_TRACKS];
    int outtrack;
};
