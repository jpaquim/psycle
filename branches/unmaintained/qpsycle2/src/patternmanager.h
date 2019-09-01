#ifndef PATTERNMANAGER_H
#define PATTERNMANAGER_H

#include <QDockWidget>

namespace psycle{
namespace core{
class Song;
}
}


namespace Ui {
class PatternManager;
}

namespace qpsycle{
class PatternManager : public QDockWidget
{
    Q_OBJECT

public:
    explicit PatternManager(QWidget *parent = 0);
    ~PatternManager();

    void patternsChanged();

signals:
    void patternSelected( int patternNum );

private slots:
    void selectedPatternChanged(int selectedPattern);
    void newButtonPressed();
    void deleteButtonPressed();

private:
    Ui::PatternManager *ui;
};
}
#endif // PATTERNMANAGER_H
