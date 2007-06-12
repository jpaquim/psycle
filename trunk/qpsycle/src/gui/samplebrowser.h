#ifndef SAMPLEBROWSER_H
#define SAMPLEBROWSER_H

namespace psy { namespace core {
		class Song;
	}}

#include <QWidget>

class QPushButton;
class QStandardItemModel;
class QListView;
class QDirModel;
class QTreeView;

class SampleBrowser : public QWidget {
Q_OBJECT
public:
	SampleBrowser( psy::core::Song *song, QWidget *parent = 0 );
	~SampleBrowser();

	psy::core::Song *song() { return m_song; }

public slots:
	void onAddToLoadedSamples();

signals:
	void sampleAdded();

private:
	QStandardItemModel *loadedSamplesModel;
	QListView *loadedSamplesView;

	QDirModel *dirModel;
	QTreeView *dirTree;

	QPushButton *button_addToLoadedSamples;

	psy::core::Song *m_song;

};

#endif
