#include "configlist.h"

ConfigList::ConfigList() 
{
}

ConfigList::CreateItems()
{
	QListWidgetItem *general = new QListWidgetItem(this);	
	QListWidgetItem *keyborad = new QListWidgetItem(this);
	QListWidgetItem *visual = new QListWidgetItem(this);
}
