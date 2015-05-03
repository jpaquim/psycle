#ifndef PATTERNVIEW_H
#define PATTERNVIEW_H

#include <QTableView>
#include <QList>
#include <QStandardItemModel>

#include "patternviewpattern.h"
#include "psycle/core/song.h"

namespace qpsycle {


class PatternView : public QTableView
{
Q_OBJECT

public:
    PatternView(QWidget *parent = 0);

public slots:
    void reload();
    void showPattern( int patternNum );

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    psycle::core::Song *song = nullptr;
    int currentPatternNum = 0;
    PatternViewPattern *model = nullptr;
};

}
#endif // PATTERNVIEW_H
