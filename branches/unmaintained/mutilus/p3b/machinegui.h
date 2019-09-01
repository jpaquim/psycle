namespace psy { namespace core {
class Machine; }}

class MachineView;

class QCheckBox;
class QMouseEvent;
class QPushButton;
class QAction;

#include <QWidget>

class MachineGui : public QWidget {
  Q_OBJECT
public:
    MachineGui( psy::core::Machine *mac, MachineView *macView, QWidget *parent = 0 );
    void randomiseParameters();
    void resetParameters();
    bool isSelected();
    psy::core::Machine *mac() { return m_mac; }
    void *setMac( psy::core::Machine *mac ) { m_mac = mac; }

public slots:
    void savePreset();
    

protected:
    void mousePressEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );

private:
    psy::core::Machine *m_mac;
    MachineView *m_macView;
    QCheckBox *checkBox;
    QPushButton *m_saveButton;
    QAction *m_saveAction;
    QPushButton *m_loadButton;
    QAction *m_loadAction;

};
