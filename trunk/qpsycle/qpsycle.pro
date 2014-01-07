#-------------------------------------------------
#
# Project created by QtCreator 2012-05-25T14:03:20
#
#-------------------------------------------------

#TEMPLATE = subdirs

#QMAKE_CXXFLAGS += -std=c++11

#include(../build-systems/qmake/common.pri)
#addSubdirs(../psycle-core)
#addSubdirs(qmake, ../psycle-core)

TARGET = qpsycle2

TEMPLATE = app

QT += xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#QPSYCLE_2_DIR = $$TOP_SRC_DIR/qpsycle2

#BUILD_DIR = $$QPSYCLE_2_DIR/++qmake
#OBJECTS_DIR = $$BUILD_DIR # Where the .o files go.
#MOC_DIR = $$BUILD_DIR # Where intermediate moc files go.
#RCC_DIR = $$BUILD_DIR # Where intermediate resource files go.
#DESTDIR = $$BUILD_DIR # Where the final executable goes.



HEADERS  += \
    src/gui/logconsole.hpp \
    src/gui/mainwindow.hpp \
    src/gui/patternbox.hpp \
    src/gui/samplebrowser.hpp \
    src/gui/undoframework.hpp \
    src/gui/configuration.hpp \
    src/gui/global.hpp \
    src/gui/inputhandler.hpp \
    src/gui/configdlg/audiopage.hpp \
    src/gui/configdlg/behaviourpage.hpp \
    src/gui/configdlg/configdlg.hpp \
    src/gui/configdlg/dirspage.hpp \
    src/gui/configdlg/lookspage.hpp \
    src/gui/machineview/effectgui.hpp \
    src/gui/machineview/generatorgui.hpp \
    src/gui/machineview/machinegui.hpp \
    src/gui/machineview/machinetweakdlg.hpp \
    src/gui/machineview/machineview.hpp \
    src/gui/machineview/mastergui.hpp \
    src/gui/machineview/mixertweakdlg.hpp \
    src/gui/machineview/newmachinedlg.hpp \
    src/gui/machineview/wiredlg.hpp \
    src/gui/machineview/wiregui.hpp \
    src/gui/patternview/linenumbercolumn.hpp \
    src/gui/patternview/patterndraw.hpp \
    src/gui/patternview/patterngrid.hpp \
    src/gui/patternview/patternview.hpp \
    src/gui/patternview/trackheader.hpp \
    src/gui/sequencer/beatruler.hpp \
    src/gui/sequencer/sequencerarea.hpp \
    src/gui/sequencer/sequencerdraw.hpp \
    src/gui/sequencer/sequenceritem.hpp \
    src/gui/sequencer/sequencerline.hpp \
    src/gui/sequencer/sequencerview.hpp \
    src/gui/waveview/waveamp.hpp \
    src/gui/waveview/wavedisplay.hpp \
    src/gui/waveview/waveview.hpp \
    src/mididrivers/alsaseqin.hpp \
    src/model/instrumentsmodel.hpp \
#
# Universalis
#
../universalis/src/universalis.hpp \
           ../universalis/src/universalis/exception.hpp \
           ../universalis/src/universalis/compiler/align.hpp \
           ../universalis/src/universalis/compiler/asm.hpp \
           ../universalis/src/universalis/compiler/attribute.hpp \
           ../universalis/src/universalis/compiler/auto_link.hpp \
           ../universalis/src/universalis/compiler/calling_convention.hpp \
           ../universalis/src/universalis/compiler/concat.hpp \
           ../universalis/src/universalis/compiler/const_function.hpp \
           ../universalis/src/universalis/compiler/constexpr.hpp \
           ../universalis/src/universalis/compiler/deprecated.hpp \
           ../universalis/src/universalis/compiler/dyn_link.hpp \
           ../universalis/src/universalis/compiler/exception.hpp \
           ../universalis/src/universalis/compiler/location.hpp \
           ../universalis/src/universalis/compiler/message.hpp \
           ../universalis/src/universalis/compiler/pack.hpp \
           ../universalis/src/universalis/compiler/pragma.hpp \
           ../universalis/src/universalis/compiler/pure_function.hpp \
           ../universalis/src/universalis/compiler/restrict.hpp \
           ../universalis/src/universalis/compiler/stringize.hpp \
           ../universalis/src/universalis/compiler/thread_local.hpp \
           ../universalis/src/universalis/compiler/token.hpp \
           ../universalis/src/universalis/compiler/typenameof.hpp \
           ../universalis/src/universalis/compiler/virtual.hpp \
           ../universalis/src/universalis/compiler/weak.hpp \
           ../universalis/src/universalis/cpu/atomic_compare_and_swap.hpp \
           ../universalis/src/universalis/cpu/exception.hpp \
           ../universalis/src/universalis/cpu/memory_barriers.hpp \
           ../universalis/src/universalis/detail/config.hpp \
           ../universalis/src/universalis/detail/config.private.hpp \
           ../universalis/src/universalis/detail/project.hpp \
           ../universalis/src/universalis/detail/project.private.hpp \
           ../universalis/src/universalis/os/aligned_alloc.hpp \
           ../universalis/src/universalis/os/clocks.hpp \
           ../universalis/src/universalis/os/dyn_link.hpp \
           ../universalis/src/universalis/os/eol.hpp \
           ../universalis/src/universalis/os/exception.hpp \
           ../universalis/src/universalis/os/fs.hpp \
           ../universalis/src/universalis/os/include_windows_without_crap.hpp \
           ../universalis/src/universalis/os/loggers.hpp \
           ../universalis/src/universalis/os/sched.hpp \
           ../universalis/src/universalis/os/terminal.hpp \
           ../universalis/src/universalis/os/thread_name.hpp \
           ../universalis/src/universalis/stdlib/chrono.hpp \
           ../universalis/src/universalis/stdlib/condition_variable.hpp \
           ../universalis/src/universalis/stdlib/cstdint.hpp \
           ../universalis/src/universalis/stdlib/exception.hpp \
           ../universalis/src/universalis/stdlib/mutex.hpp \
           ../universalis/src/universalis/stdlib/ratio.hpp \
           ../universalis/src/universalis/stdlib/thread.hpp \
           ../universalis/src/universalis/os/detail/clocks.hpp \
           ../universalis/src/universalis/stdlib/detail/chrono/duration_and_time_point.hpp \
           ../universalis/src/universalis/stdlib/detail/chrono/measure_clock.hpp \
#
# Psycle Core
#
../psycle-core/src/psycle/core/commands.h \
           ../psycle-core/src/psycle/core/constants.h \
           ../psycle-core/src/psycle/core/convert_internal_machines.private.hpp \
           ../psycle-core/src/psycle/core/cpu_time_clock.hpp \
           ../psycle-core/src/psycle/core/eventdriver.h \
           ../psycle-core/src/psycle/core/exceptions.h \
           ../psycle-core/src/psycle/core/fileio.h \
           ../psycle-core/src/psycle/core/filterhelper.h \
           ../psycle-core/src/psycle/core/fwd.hpp \
           ../psycle-core/src/psycle/core/instpreview.h \
           ../psycle-core/src/psycle/core/instrument.h \
           ../psycle-core/src/psycle/core/internal_machines.h \
           ../psycle-core/src/psycle/core/internalhost.hpp \
           ../psycle-core/src/psycle/core/internalkeys.hpp \
           ../psycle-core/src/psycle/core/ladspa.h \
           ../psycle-core/src/psycle/core/ladspahost.hpp \
           ../psycle-core/src/psycle/core/ladspamachine.h \
           ../psycle-core/src/psycle/core/machine.h \
           ../psycle-core/src/psycle/core/machinefactory.h \
           ../psycle-core/src/psycle/core/machinehost.hpp \
           ../psycle-core/src/psycle/core/machinekey.hpp \
           ../psycle-core/src/psycle/core/mixer.h \
           ../psycle-core/src/psycle/core/nativehost.hpp \
           ../psycle-core/src/psycle/core/pattern.h \
           ../psycle-core/src/psycle/core/patternevent.h \
           ../psycle-core/src/psycle/core/player.h \
           ../psycle-core/src/psycle/core/playertimeinfo.h \
           ../psycle-core/src/psycle/core/plugin.h \
           ../psycle-core/src/psycle/core/plugincatcher.h \
           ../psycle-core/src/psycle/core/pluginfinder.h \
           ../psycle-core/src/psycle/core/plugininfo.h \
           ../psycle-core/src/psycle/core/preset.h \
           ../psycle-core/src/psycle/core/psy2filter.h \
           ../psycle-core/src/psycle/core/psy3filter.h \
           ../psycle-core/src/psycle/core/psy4filter.h \
           ../psycle-core/src/psycle/core/psyfilterbase.h \
           ../psycle-core/src/psycle/core/sampler.h \
           ../psycle-core/src/psycle/core/sequence.h \
           ../psycle-core/src/psycle/core/sequencer.h \
           ../psycle-core/src/psycle/core/song.h \
           ../psycle-core/src/psycle/core/songserializer.h \
           ../psycle-core/src/psycle/core/timesignature.h \
           ../psycle-core/src/psycle/core/vsthost.h \
           ../psycle-core/src/psycle/core/vstplugin.h \
           ../psycle-core/src/psycle/core/xminstrument.h \
           ../psycle-core/src/psycle/core/xml.h \
           ../psycle-core/src/psycle/core/xmsampler.h \
           ../psycle-core/src/psycle/core/zipreader.h \
           ../psycle-core/src/psycle/core/zipwriter.h \
           ../psycle-core/src/psycle/core/zipwriterstream.h \
#           ../psycle-core/src/seib/vst/CVSTHost.Seib.hpp \
#           ../psycle-core/src/seib/vst/EffectWnd.hpp \
           ../psycle-core/src/psycle/core/detail/config.hpp \
           ../psycle-core/src/psycle/core/detail/config.private.hpp \
           ../psycle-core/src/psycle/core/detail/project.hpp \
           ../psycle-core/src/psycle/core/detail/project.private.hpp \
#
# Psycle-Audiodrivers
#
../psycle-audiodrivers/src/asio/asio.private.hpp \
           ../psycle-audiodrivers/src/psycle/audiodrivers/alsaout.h \
           ../psycle-audiodrivers/src/psycle/audiodrivers/asiointerface.h \
           ../psycle-audiodrivers/src/psycle/audiodrivers/audiodriver.h \
           ../psycle-audiodrivers/src/psycle/audiodrivers/esoundout.h \
           ../psycle-audiodrivers/src/psycle/audiodrivers/gstreamerout.h \
           ../psycle-audiodrivers/src/psycle/audiodrivers/jackout.h \
           ../psycle-audiodrivers/src/psycle/audiodrivers/microsoftdirectsoundout.h \
           ../psycle-audiodrivers/src/psycle/audiodrivers/microsoftmmewaveout.h \
           ../psycle-audiodrivers/src/psycle/audiodrivers/netaudioout.h \
           ../psycle-audiodrivers/src/psycle/audiodrivers/wavefileout.h \
#
# Psycle-Helpers
#
../psycle-helpers/src/psycle/helpers/abstractiff.hpp \
           ../psycle-helpers/src/psycle/helpers/amigasvx.hpp \
           ../psycle-helpers/src/psycle/helpers/appleaiff.hpp \
           ../psycle-helpers/src/psycle/helpers/binread.hpp \
           ../psycle-helpers/src/psycle/helpers/datacompression.hpp \
           ../psycle-helpers/src/psycle/helpers/dither.hpp \
           ../psycle-helpers/src/psycle/helpers/dsp.hpp \
           ../psycle-helpers/src/psycle/helpers/eaiff.hpp \
           ../psycle-helpers/src/psycle/helpers/fft.hpp \
           ../psycle-helpers/src/psycle/helpers/filter.hpp \
           ../psycle-helpers/src/psycle/helpers/hexstring_to_binary.hpp \
           ../psycle-helpers/src/psycle/helpers/hexstring_to_integer.hpp \
           ../psycle-helpers/src/psycle/helpers/math.hpp \
           ../psycle-helpers/src/psycle/helpers/mersennetwister.hpp \
           ../psycle-helpers/src/psycle/helpers/msriff.hpp \
           ../psycle-helpers/src/psycle/helpers/resampler.hpp \
           ../psycle-helpers/src/psycle/helpers/riff.hpp \
           ../psycle-helpers/src/psycle/helpers/riffwave.hpp \
           ../psycle-helpers/src/psycle/helpers/ring_buffer.hpp \
           ../psycle-helpers/src/psycle/helpers/scale.hpp \
           ../psycle-helpers/src/psycle/helpers/value_mapper.hpp \
           ../psycle-helpers/src/psycle/helpers/math/clip.hpp \
           ../psycle-helpers/src/psycle/helpers/math/constants.hpp \
           ../psycle-helpers/src/psycle/helpers/math/erase_all_nans_infinities_and_denormals.hpp \
           ../psycle-helpers/src/psycle/helpers/math/erase_denormals.hpp \
           ../psycle-helpers/src/psycle/helpers/math/log.hpp \
           ../psycle-helpers/src/psycle/helpers/math/math.hpp \
           ../psycle-helpers/src/psycle/helpers/math/rint.hpp \
           ../psycle-helpers/src/psycle/helpers/math/round.hpp \
           ../psycle-helpers/src/psycle/helpers/math/sin.hpp \
           ../psycle-helpers/src/psycle/helpers/math/sincos.hpp \
           ../psycle-helpers/src/psycle/helpers/math/sinseq.hpp \
           ../psycle-helpers/src/psycle/helpers/math/sse_mathfun.h

SOURCES +=\
    src/qpsycle.cpp \
    src/gui/mainwindow.cpp \
    src/gui/patternbox.cpp \
    src/gui/samplebrowser.cpp \
    src/gui/undoframework.cpp \
    src/gui/configuration.cpp \
    src/gui/global.cpp \
    src/gui/inputhandler.cpp \
    src/gui/logconsole.cpp \
    src/gui/configdlg/audiopage.cpp \
    src/gui/configdlg/behaviourpage.cpp \
    src/gui/configdlg/configdlg.cpp \
    src/gui/configdlg/dirspage.cpp \
    src/gui/configdlg/lookspage.cpp \
    src/gui/machineview/effectgui.cpp \
    src/gui/machineview/generatorgui.cpp \
    src/gui/machineview/machinegui.cpp \
    src/gui/machineview/machinetweakdlg.cpp \
    src/gui/machineview/machineview.cpp \
    src/gui/machineview/mastergui.cpp \
    src/gui/machineview/mixertweakdlg.cpp \
    src/gui/machineview/newmachinedlg.cpp \
    src/gui/machineview/wiredlg.cpp \
    src/gui/machineview/wiregui.cpp \
    src/gui/patternview/linenumbercolumn.cpp \
    src/gui/patternview/patterndraw.cpp \
    src/gui/patternview/patterngrid.cpp \
    src/gui/patternview/patternview.cpp \
    src/gui/patternview/trackheader.cpp \
    src/gui/sequencer/beatruler.cpp \
    src/gui/sequencer/sequencerarea.cpp \
    src/gui/sequencer/sequencerdraw.cpp \
    src/gui/sequencer/sequenceritem.cpp \
    src/gui/sequencer/sequencerline.cpp \
    src/gui/sequencer/sequencerview.cpp \
    src/gui/waveview/waveamp.cpp \
    src/gui/waveview/wavedisplay.cpp \
    src/gui/waveview/waveview.cpp \
    src/mididrivers/alsaseqin.cpp \
    src/model/instrumentsmodel.cpp\
#
# Universalis
#
           ../universalis/src/universalis/exception.cpp \
           ../universalis/src/universalis/compiler/exception.compiler.cpp \
           ../universalis/src/universalis/compiler/typenameof.cpp \
           ../universalis/src/universalis/cpu/exception.cpu.cpp \
           ../universalis/src/universalis/os/clocks.cpp \
           ../universalis/src/universalis/os/dyn_link.cpp \
           ../universalis/src/universalis/os/exception.os.cpp \
           ../universalis/src/universalis/os/fs.cpp \
           ../universalis/src/universalis/os/loggers.cpp \
           ../universalis/src/universalis/os/sched.cpp \
           ../universalis/src/universalis/os/terminal.cpp \
           ../universalis/src/universalis/os/thread_name.cpp \
           ../universalis/src/universalis/stdlib/exception.stdlib.cpp \
#
# Psycle Core
#
../psycle-core/src/psycle/core/convert_internal_machines.cpp \
           ../psycle-core/src/psycle/core/eventdriver.cpp \
           ../psycle-core/src/psycle/core/fileio.cpp \
           ../psycle-core/src/psycle/core/instpreview.cpp \
           ../psycle-core/src/psycle/core/instrument.cpp \
           ../psycle-core/src/psycle/core/internal_machines.cpp \
           ../psycle-core/src/psycle/core/internalhost.cpp \
           ../psycle-core/src/psycle/core/ladspahost.cpp \
           ../psycle-core/src/psycle/core/ladspamachine.cpp \
           ../psycle-core/src/psycle/core/machine.cpp \
           ../psycle-core/src/psycle/core/machinefactory.cpp \
           ../psycle-core/src/psycle/core/machinehost.cpp \
           ../psycle-core/src/psycle/core/machinekey.cpp \
           ../psycle-core/src/psycle/core/mixer.cpp \
           ../psycle-core/src/psycle/core/nativehost.cpp \
           ../psycle-core/src/psycle/core/pattern.cpp \
           ../psycle-core/src/psycle/core/patternevent.cpp \
           ../psycle-core/src/psycle/core/player.cpp \
           ../psycle-core/src/psycle/core/playertimeinfo.cpp \
           ../psycle-core/src/psycle/core/plugin.cpp \
           ../psycle-core/src/psycle/core/plugincatcher.cpp \
           ../psycle-core/src/psycle/core/pluginfinder.cpp \
           ../psycle-core/src/psycle/core/plugininfo.cpp \
           ../psycle-core/src/psycle/core/preset.cpp \
           ../psycle-core/src/psycle/core/psy2filter.cpp \
           ../psycle-core/src/psycle/core/psy3filter.cpp \
           ../psycle-core/src/psycle/core/psy4filter.cpp \
           ../psycle-core/src/psycle/core/sampler.cpp \
           ../psycle-core/src/psycle/core/sequence.cpp \
           ../psycle-core/src/psycle/core/sequencer.cpp \
           ../psycle-core/src/psycle/core/song.cpp \
           ../psycle-core/src/psycle/core/songserializer.cpp \
           ../psycle-core/src/psycle/core/timesignature.cpp \
           ../psycle-core/src/psycle/core/vsthost.cpp \
           ../psycle-core/src/psycle/core/vstplugin.cpp \
           ../psycle-core/src/psycle/core/xminstrument.cpp \
           ../psycle-core/src/psycle/core/xml.cpp \
           ../psycle-core/src/psycle/core/xmsampler.cpp \
           ../psycle-core/src/psycle/core/zipreader.cpp \
           ../psycle-core/src/psycle/core/zipwriter.cpp \
           ../psycle-core/src/psycle/core/zipwriterstream.cpp \
#           ../psycle-core/src/seib/vst/CVSTHost.Seib.cpp \
#           ../psycle-core/src/seib/vst/EffectWnd.cpp
#
# Psycle-AudioDrivers
#
#../psycle-audiodrivers/src/asio/asio.cpp \
#           ../psycle-audiodrivers/src/asio/asiodrivers.cpp \
#           ../psycle-audiodrivers/src/asio/asiolist.cpp \
           ../psycle-audiodrivers/src/psycle/audiodrivers/alsaout.cpp \
#           ../psycle-audiodrivers/src/psycle/audiodrivers/asiointerface.cpp \
           ../psycle-audiodrivers/src/psycle/audiodrivers/audiodriver.cpp \
           ../psycle-audiodrivers/src/psycle/audiodrivers/esoundout.cpp \
           ../psycle-audiodrivers/src/psycle/audiodrivers/gstreamerout.cpp \
           ../psycle-audiodrivers/src/psycle/audiodrivers/jackout.cpp \
           ../psycle-audiodrivers/src/psycle/audiodrivers/microsoftdirectsoundout.cpp \
           ../psycle-audiodrivers/src/psycle/audiodrivers/microsoftmmewaveout.cpp \
           ../psycle-audiodrivers/src/psycle/audiodrivers/netaudioout.cpp \
           ../psycle-audiodrivers/src/psycle/audiodrivers/wavefileout.cpp \
#
# Psycle-Helpers
#
           ../psycle-helpers/src/psycle/helpers/abstractiff.cpp \
           ../psycle-helpers/src/psycle/helpers/amigasvx.cpp \
           ../psycle-helpers/src/psycle/helpers/appleaiff.cpp \
           ../psycle-helpers/src/psycle/helpers/binread.cpp \
           ../psycle-helpers/src/psycle/helpers/datacompression.cpp \
           ../psycle-helpers/src/psycle/helpers/dither.cpp \
           ../psycle-helpers/src/psycle/helpers/dsp.cpp \
           ../psycle-helpers/src/psycle/helpers/eaiff.cpp \
           ../psycle-helpers/src/psycle/helpers/fft.cpp \
           ../psycle-helpers/src/psycle/helpers/filter.cpp \
           ../psycle-helpers/src/psycle/helpers/hexstring_to_binary.cpp \
           ../psycle-helpers/src/psycle/helpers/hexstring_to_integer.cpp \
           ../psycle-helpers/src/psycle/helpers/mersennetwister.cpp \
           ../psycle-helpers/src/psycle/helpers/msriff.cpp \
           ../psycle-helpers/src/psycle/helpers/resampler.cpp \
           ../psycle-helpers/src/psycle/helpers/riff.cpp \
           ../psycle-helpers/src/psycle/helpers/riffwave.cpp \



INCLUDEPATH += . \
     src/\
     ../universalis/src/\
     ../diversalis/src/\
     ../psycle-helpers/src/\
     ../psycle-plugins/src/\
     ../external-packages/vst-2.4/\
     ../psycle-core/src/\
     ../psycle-audiodrivers/src/\
     ../universalis/src/\
     ../universalis/src/universalis/stdlib \
     ../universalis/src/universalis/stdlib/detail/chrono \
     ../universalis/src/universalis/detail \
     ../universalis/src/universalis/compiler \
     ../universalis/src/universalis/os \
     ../universalis/src/universalis/os/detail \
     ../universalis/src/universalis \
     ../universalis/src/universalis/cpu \
     ../psycle-core/src/psycle/core \
     ../psycle-core/src/seib/vst \
     ../psycle-audiodrivers/src/asio \
     ../psycle-audiodrivers/src/psycle/audiodrivers \
     ../psycle-helpers/src/psycle/helpers \
     ../psycle-helpers/src/psycle/helpers/math

DEPENDPATH += . \
              src/\
              ../universalis/src \
              ../universalis/src/universalis \
              ../universalis/src/universalis/compiler \
              ../universalis/src/universalis/cpu \
              ../universalis/src/universalis/detail \
              ../universalis/src/universalis/os \
              ../universalis/src/universalis/stdlib \
              ../universalis/src/universalis/os/detail \
              ../universalis/src/universalis/stdlib/detail/chrono\
              ../psycle-core/src/psycle/core\
              ../psycle-core/src/seib/vst\
              ../psycle-core/src/psycle/core/detail \
              ../psycle-audiodrivers/src/asio \
              ../psycle-audiodrivers/src/psycle/audiodrivers \
              ../psycle-helpers/src/psycle/helpers \
              ../psycle-helpers/src/psycle/helpers/math

FORMS += \
    src/gui/configdlg/behaviourpage.ui \
    src/gui/configdlg/dirspage.ui

OTHER_FILES += \
    src/images/pattern_new.bmp \
    src/images/TbMainKnob.bmp \
    src/images/qp.jpg \
    src/images/conf-audio.png \
    src/images/conf-folders.png \
    src/images/conf-general.png \
    src/images/conf-keyboard.png \
    src/images/conf-visual.png \
    src/images/crashed.png \
    src/images/def-background.png \
    src/images/efx-internal.png \
    src/images/efx-native.png \
    src/images/efx-vst.png \
    src/images/gen-internal.png \
    src/images/gen-native.png \
    src/images/gen-vst.png \
    src/images/ladspa.png \
    src/images/machines.png \
    src/images/minus.png \
    src/images/new.png \
    src/images/open.png \
    src/images/pat_addbar.png \
    src/images/pat_delbar.png \
    src/images/pattern-editor.png \
    src/images/pb_addpattoseq.png \
    src/images/pb_clnpat.png \
    src/images/pb_delpat.png \
    src/images/pb_newcat.png \
    src/images/pb_newpat.png \
    src/images/play.png \
    src/images/playselpattern.png \
    src/images/playstart.png \
    src/images/plugin.png \
    src/images/plus.png \
    src/images/print.png \
    src/images/psycle.png \
    src/images/qpsycle.png \
    src/images/recordnotes.png \
    src/images/redo.png \
    src/images/sample-browser.png \
    src/images/save.png \
    src/images/selector.png \
    src/images/seq-add-track.png \
    src/images/seq-collapse.png \
    src/images/seq-del-track.png \
    src/images/seq-down-track.png \
    src/images/seq-expand.png \
    src/images/seq-up-track.png \
    src/images/sequencer.png \
    src/images/stop.png \
    src/images/TbMainKnob.png \
    src/images/undo.png \
    src/images/wave_play.png \
    src/images/wave_playstart.png \
    src/images/wave_stop.png \
    src/images/wave-copy.png \
    src/images/wave-cut.png \
    src/images/wave-open.png \
    src/images/wave-paste.png \
    src/images/wave-save.png \
    src/images/waveed.png \
    src/images/zoom-in.png \
    src/images/zoom-out.png \
    src/qpsycle.rc

RESOURCES += \
    src/qpsycle.qrc


 LIBS += -lz -ldl -lrt -lboost_filesystem -lboost_system -lboost_signals -lboost_thread -lgomp

OTHER_FILES +=

CONFIG      += link_pkgconfig\

 PKGCONFIG += glib-2.0 gstreamer-0.10 gstreamer-plugins-base-0.10 alsa jack soxr


QMAKE_CXXFLAGS += -std=c++11 -fopenmp

QMAKE_CFLAGS_DEBUG += -O0

DEFINES += PSYCLE__ALSA_AVAILABLE
DEFINES += PSYCLE__GSTREAMER_AVAILABLE
DEFINES += PSYCLE__JACK_AVAILABLE

#BUILD_DIR = ++build
#OBJECTS_DIR = $$BUILD_DIR/objects # Where the .o files go.
#MOC_DIR = $$BUILD_DIR/moc # Where intermediate moc files go.
#RCC_DIR = $$BUILD_DIR/rcc # Where intermediate resource files go.
#UI_DIR = $$BUILD_DIR/uic # Where compiled uic files go.
#DESTDIR = $$BUILD_DIR # Where the final executable goes.
