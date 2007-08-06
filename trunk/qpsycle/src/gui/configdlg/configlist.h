#ifndef configlist_H
#define configlist_H

class ConfigList : public QListWidget
{
	Q_OBJECT
public:
	ConfigList();
private:
	CreateItems();
};

#endif
