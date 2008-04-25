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
#include <psycle/core/binread.h>
#include <psycle/core/constants.h>
#include <psycle/core/file.h>
#include <psycle/core/internal_machines.h>
#include <psycle/core/machinefactory.h>
#include <psycle/core/machine.h>
#include <psycle/core/patternpool.h>
#include <psycle/core/patternsequence.h>
#include <psycle/core/player.h>
#include <psycle/core/plugin.h>
#include <psycle/core/pluginfinder.h>
#include <psycle/core/singlepattern.h>
#include <psycle/core/song.h>


#include <psycle/audiodrivers/wavefileout.h>
#if defined PSYCLE__ALSA_AVAILABLE
	#include <psycle/audiodrivers/alsaout.h>
#endif
#if defined PSYCLE__JACK_AVAILABLE
	#include <psycle/audiodrivers/jackout.h>
#endif
#if defined PSYCLE__ESOUND_AVAILABLE
	#include <psycle/audiodrivers/esoundout.h>
#endif
#if defined PSYCLE__GSTREAMER_AVAILABLE
	#include <psycle/audiodrivers/gstreamerout.h>
#endif
#if defined PSYCLE__MICROSOFT_DIRECT_SOUND_AVAILABLE
	#include <psycle/audiodrivers/microsoftdirectsoundout.h>
#endif
#if defined PSYCLE__MICROSOFT_MME_AVAILABLE
	#include <psycle/audiodrivers/microsoftmmewaveout.h>
#endif
