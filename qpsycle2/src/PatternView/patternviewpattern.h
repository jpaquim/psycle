#ifndef PATTERNVIEWPATTERN_H
#define PATTERNVIEWPATTERN_H

#include <QAbstractTableModel>

namespace psycle{
namespace core{
class Pattern;
}
}

class QFont;
class QPainter;

namespace qpsycle{

class PatternViewPattern : public QAbstractTableModel
{
public:
    enum ColumnType{
        Note,
        Instrument,
        Machine,
        Command,
        Param,
        Volume,
        NColumns
    };

    PatternViewPattern(psycle::core::Pattern* pattern, QObject *parent = nullptr);



private:
    QString generateHexString(uint8_t string) const;

private:
    psycle::core::Pattern* pattern;


    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
};

}
#endif // PATTERNVIEWPATTERN_H
