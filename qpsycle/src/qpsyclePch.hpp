// -*- mode:c++; indent-tabs-mode:t -*-

#if defined __cplusplus
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
#include <qglobal.h>
#ifdef Q_WS_WIN
# define _POSIX_
# include <limits.h>
# undef _POSIX_
#endif
#include <qcoreapplication.h>
#include <qlist.h>
#include <qvariant.h>  // All moc genereated code has this include
#include <qobject.h>
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextcodec.h>

#include <qapplication.h>
#include <qbitmap.h>
#include <qcursor.h>
#include <qdesktopwidget.h>
#include <qevent.h>
#include <qimage.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qstyle.h>
#include <qtimer.h>
#include <qwidget.h>

//psycle-core defines
#include <psycle/core/constants.h>
#include <psycle/core/file.h>
#include <psycle/core/machinekey.hpp>
#include <psycle/core/patternpool.h>
#include <psycle/core/plugininfo.h>
#include <psycle/core/singlepattern.h>

#include <psycle/audiodrivers/audiodriver.h>
#include <psycle/audiodrivers/wavefileout.h>
#endif
