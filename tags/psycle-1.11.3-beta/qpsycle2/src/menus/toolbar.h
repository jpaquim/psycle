#ifndef TOOLBAR_H
#define TOOLBAR_H

#include "qpsycle2.h"
#include <QToolBar>

namespace qpsycle{
class ToolBar:public QToolBar
{
public:
    ToolBar(MenuSignalHandler *handler);
};
}
#endif // TOOLBAR_H
