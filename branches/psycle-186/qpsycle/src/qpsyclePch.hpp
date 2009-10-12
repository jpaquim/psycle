
#pragma once

#include <packageneric/pre-compiled.private.hpp>

#include <psycle/core/signalslib.h>

// QT libraries
#include <qglobal.h>
#ifdef Q_WS_WIN
# define _POSIX_
# include <limits.h> ///\todo doc what's that hack for?
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

