#ifndef INSTRUMENTSMODEL_H
#define INSTRUMENTSMODEL_H

namespace psy { namespace core {
		class Song;
	}}

#include <QStandardItemModel>

class InstrumentsModel : public QStandardItemModel {
Q_OBJECT
public:
	InstrumentsModel( psy::core::Song *song, int rows, int columns, QObject * parent = 0 );
	~InstrumentsModel();

	bool loadInstrument( int instrIndex, QString pathToWavfile );
	void clearInstrument( int instrIndex );

	int selectedInstrumentIndex();
	bool slotIsEmpty( int instrIndex );
		       
private:
	psy::core::Song *song_;
};

#endif
