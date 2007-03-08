/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created: Thu Mar 8 16:20:08 2007
**      by: The Qt Meta Object Compiler version 59 (Qt 4.2.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mainwindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.2.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_MainWindow[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x08,
      22,   11,   11,   11, 0x08,
      29,   11,   11,   11, 0x08,
      36,   11,   11,   11, 0x08,
      43,   11,   11,   11, 0x08,
      50,   11,   11,   11, 0x08,
      58,   11,   11,   11, 0x08,
      91,   82,   11,   11, 0x08,
     141,  125,   11,   11, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MainWindow[] = {
    "MainWindow\0\0newSong()\0open()\0save()\0undo()\0redo()\0about()\0"
    "refreshSampleComboBox()\0newIndex\0onSampleComboBoxIndexChanged(int)\0"
    "selectedPattern\0"
    "onPatternSelectedInPatternBox(psy::core::SinglePattern*)\0"
};

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow,
      qt_meta_data_MainWindow, 0 }
};

const QMetaObject *MainWindow::metaObject() const
{
    return &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow))
	return static_cast<void*>(const_cast<MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: newSong(); break;
        case 1: open(); break;
        case 2: save(); break;
        case 3: undo(); break;
        case 4: redo(); break;
        case 5: about(); break;
        case 6: refreshSampleComboBox(); break;
        case 7: onSampleComboBoxIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: onPatternSelectedInPatternBox((*reinterpret_cast< psy::core::SinglePattern*(*)>(_a[1]))); break;
        }
        _id -= 9;
    }
    return _id;
}
