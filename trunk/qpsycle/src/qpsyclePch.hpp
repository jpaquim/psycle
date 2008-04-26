// -*- mode:c++; indent-tabs-mode:t -*-

#ifdef _WIN32
#define DIVERSALIS__OPERATING_SYSTEM__VERSION__MAJOR 5
#define DIVERSALIS__OPERATING_SYSTEM__VERSION__MINOR 0
#define DIVERSALIS__OPERATING_SYSTEM__VERSION__PATCH 0
#include <diversalis/operating_system.hpp> // sets winapi version so must come before <windows.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

#include <psycle/core/signalslib.h>
// QT libraries

#include <QtCore> 
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QColorDialog>
#include <QContextMenuEvent>
#include <QComboBox>
#include <QCompleter>
#include <QDebug>
#include <QDesktopWidget>
#include <QDialog>
#include <QDial>
#include <QDir>
#include <QDirModel>
#include <QDockWidget>
#include <QFile>
#include <QFileDialog>
#include <QtGui>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QModelIndexList>
#include <QModelIndex>
#include <QPainter>
#include <QPushButton>
#include <QSettings>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QStatusBar>
#include <QStyleOption>
#include <QStyleFactory>
#include <QString>
#include <QTextCodec>
#include <QTextEdit>
#include <QTextStream>
#include <QTimer>
#include <QTreeWidget>
#include <QToolBar>
#include <QUndoStack>
#include <QUndoView>
#include <QVBoxLayout>
#include <QWidget>

#include <QtXml/QDomDocument> 

//psycle-core defines
#include <psycle/core/constants.h>
#include <psycle/core/file.h>
#include <psycle/core/machinekey.hpp>
#include <psycle/core/patternpool.h>
#include <psycle/core/plugininfo.h>
#include <psycle/core/singlepattern.h>

#include <psycle/audiodrivers/audiodriver.h>
#include <psycle/audiodrivers/wavefileout.h>

